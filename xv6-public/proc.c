#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"
#include "minheap.h"

struct{
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;

int mlfqpv = 0;
int mlfqtickets = 0;

int sumtickets = 0;

extern void forkret(void);
extern void trapret(void);

extern int sys_uptime();
extern int passti;
extern struct spinlock tickslock;

extern struct node strheap[3];

static void wakeup1(void *chan);
static void thread_wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();

  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;
  int i;
  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;
  p->level = 0;
  p->idx = p - ptable.proc; 
  p->ticks = 0;
  p->tickets = 0;

  p->thid = 0;
  p->nextid = 0;
  p->mainth = p;
  p->nextth = p;
  p->prevth = p;
  p->recentth = p;

  for(i = 0; i < NTHREAD; i++)
    p->stack[i] = -1;

  p->runblenum = 0;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }


  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
 
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");
  
  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);
  p->state = RUNNABLE;
  p->mainth->runblenum++;
  enqueue(p->idx, 0);
  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->mainth->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->mainth->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->mainth->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->mainth->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();
  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->mainth->pgdir, curproc->mainth->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }

  np->sz = curproc->mainth->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->mainth->name, sizeof(curproc->mainth->name));

  pid = np->pid;

  acquire(&ptable.lock);
  np->state = RUNNABLE;
  np->mainth->runblenum++;
  enqueue(np->idx, 0);
  release(&ptable.lock);

  return pid;
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *cur = myproc();
  struct proc *p;
  int fd;

  if(cur == initproc)
    panic("init exiting");

  for(p = cur->nextth; p != cur; p = p->nextth) {
    if(p->state != ZOMBIE) {
      for(fd = 0; fd < NOFILE; fd++) {
        if(p->ofile[fd]) {
          fileclose(p->ofile[fd]);
          p->ofile[fd] = 0;
        }
      }
      
      begin_op();
      iput(p->cwd);
      end_op();
      p->cwd = 0;
    }
  }
  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(cur->ofile[fd]){
      fileclose(cur->ofile[fd]);
      cur->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(cur->cwd);
  end_op();
  cur->cwd = 0;

  acquire(&ptable.lock);

  clear_thread(cur->idx);

  // Parent might be sleeping in wait().
  wakeup1(cur->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == cur || p->parent->pid == cur->pid){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  cur->state = ZOMBIE;
  sumtickets -= cur->tickets;

  sched();
  panic("zombie exit");
}

void
clear_thread(int idx) {
  
  struct proc *cur = &ptable.proc[idx];
  struct proc *p;
  int i = 0;  

  //acquire(&ptable.lock);
  
  cur->pgdir = cur->mainth->pgdir;
  //cur->parent = cur->mainth->parent;
  cur->pid = cur->mainth->pid;
  cur->tickets = cur->mainth->tickets;
  cur->runblenum = cur->mainth->runblenum;
 
  for(p = cur->nextth; p != cur; p = p->nextth) {
    kfree(p->kstack);
    p->kstack = 0;
    p->pid = 0;
    p->parent = 0;
    p->name[0] = 0;
    p->killed = 0;
    p->level = 0;
    p->idx = 0;
    p->ticks = 0;
    p->tickets = 0;
    p->state = UNUSED;
    p->join = 0;

    for(i = 0; i < NTHREAD; i++)
      p->stack[i] = -1;

    cur->runblenum--;
  }
  
  
 // release(&ptable.lock);
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  int i;
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->mainth->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->level = 0;
        p->idx = 0;
        p->ticks = 0;
        p->tickets = 0;
        p->state = UNUSED;

        p->join = 0;
        p->thid = 0;
        p->mainth = 0;
        p->nextth = 0;
        p->prevth = 0;
        p->recentth = 0;
		p->runblenum = 0;

        for(i = 0; i < NTHREAD; i++)
          p->stack[i] = -1;
        
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }
    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  // struct proc* p;
  struct cpu *c = mycpu();
  c->proc = 0;

  struct proc *mainth;

  int idx = 0;
  uint beforeproctick = 0;
  uint beforetick = 0;
  int passval;

  int minpv = 0;
  int pbcount = 0;
  int mlfqpvadd = 0;  

  for(;;){
    // Enable interrupts on this processor.
    sti();
    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
  
    minpv = getpvheap();

    //pick the mlfq scheduler
    if(mlfqpv < minpv || minpv == -1) {

      idx = pickprocmlfq();  
 
      if(idx == -1) {
        mlfqpvadd++;
      	if(mlfqpvadd >= 100) {
       		mlfqtickets = 100 - sumtickets;
        	mlfqpv += BIGNUM / mlfqtickets;	
			mlfqpvadd = 0;
		}
        release(&ptable.lock);
        continue;
      }     

      mainth = &ptable.proc[idx];

      idx = thread_pick(idx);
      
      if(idx == -1) {
        release(&ptable.lock);
		continue;
	  }

      beforetick = sys_uptime();
      beforeproctick = mainth->ticks;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.

      c->proc = &ptable.proc[idx];
	  switchuvm(&ptable.proc[idx]);
      
	  ptable.proc[idx].state = RUNNING; 
	  mainth->runblenum--;      

      if(mainth->ticks == (MALLOT - 1) && mainth->level == 1 && ptable.proc[idx].killed == 0) {
        acquire(&tickslock);
        passti += 1;
        release(&tickslock);        
      } 

      swtch(&(c->scheduler), ptable.proc[idx].context);
      switchkvm();

      mainth->ticks++;
      passti = 0;
      
      
      mlfqtickets = 100 - sumtickets; 
      
      pbcount += sys_uptime() - beforetick;
      
	  mlfqpv += (mainth->ticks - beforeproctick) * (BIGNUM / mlfqtickets);

      if(mainth->level == HLEVEL && mainth->ticks >= HALLOT) {
         mainth->ticks = 0;
         mainth->level = MLEVEL;
      }

      else if(mainth->level == MLEVEL && mainth->ticks >= MALLOT) {  
         mainth->ticks = 0;
         mainth->level = LLEVEL;
      }    
  
      if(mainth->runblenum > 0) {
        if(mainth->level != -1) {
		  enqueue(mainth->idx, mainth->level);
        }
        else {
          passval = getpvheap();
		  if(mlfqpv < passval || passval == -1)
            passval = mlfqpv;
          heapinsert(mainth->idx, passval);
        }
      }

      if(pbcount >= 200) {
        pbcount = 0;
        priorityboost();
      } 

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }

  //picks other proceesses of the stride scheduler
  else {
      
      idx = heapdelete();

	  mainth = &ptable.proc[idx];
	  idx = thread_pick(idx);
      if(idx == -1) {
        release(&ptable.lock);
		continue;
	  }
     
      beforeproctick = mainth->ticks;

      c->proc = &ptable.proc[idx];
      switchuvm(&ptable.proc[idx]);

      ptable.proc[idx].state = RUNNING; 
	  mainth->runblenum--;

      swtch(&(c->scheduler), ptable.proc[idx].context);
      switchkvm();

      mainth->ticks++;
      passti = 0;

      if(mainth->runblenum >= 1) {
        minpv += (BIGNUM / mainth->tickets) * (mainth->ticks - beforeproctick);
        heapinsert(mainth->idx, minpv);
      }

      c->proc = 0;
    }
     
    release(&ptable.lock);
  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();
  
  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1) {
    cprintf("pid: %d, ncli: %d\n", myproc()->pid, mycpu()->ncli);
    panic("sched locks");
  }
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;  
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
int
yield(void)
{
  struct proc *p = myproc();
  acquire(&ptable.lock);  //DOC: yieldlock
  p->state = RUNNABLE;
  p->mainth->runblenum++;
  sched();
  release(&ptable.lock);
  return 0;
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;  
  
  if(p->mainth->runblenum > 0) 
    thread_sched();
  else
    sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}


//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;
  struct proc *mainth;
  int passval;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if(p->state == SLEEPING && p->chan == chan) {
      
      p->state = RUNNABLE;
	  mainth = p->mainth;
	  mainth->runblenum ++;
      
	  if(mainth->runblenum == 1) {
        
        if(mainth->level != -1) {
  	      if (chan != &ticks) {
            mainth->level = 0;
  		    mainth->ticks = 0;
			enqueue(mainth->idx, HLEVEL);
  		  }
  		  else {
  	 	    enqueue(mainth->idx, mainth->level);
  		  }
  
        }
        else {
          passval = getpvheap();
  
          if(mlfqpv < passval || passval == -1)
            passval = mlfqpv;
  
          heapinsert(mainth->idx, passval);
        }  
      }
    }
  }
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;
  struct proc *th;
  struct proc *mainth;
  int passval;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      mainth = p->mainth;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING) {
        p->state = RUNNABLE;
		mainth->runblenum++;
        
        if(mainth->runblenum == 1) {

          if(mainth->level != -1) {
            mainth->level = HLEVEL;
            mainth->ticks = 0;
            enqueue(mainth->idx, HLEVEL);
          }
          else {
            passval = getpvheap();
      
            if(mlfqpv < passval || passval == -1)
              passval = mlfqpv;
      
            heapinsert(mainth->idx, passval);
          }
        }
      }
      
      for(th = p->nextth; th != p; th = th->nextth) {
        th->killed = 1;
        if(th->state == SLEEPING) {
          th->state = RUNNABLE;
          mainth->runblenum++; 
          
          if(mainth->runblenum == 1) {
 
            if(mainth->level != -1) {
              mainth->level = HLEVEL;
              mainth->ticks = 0;
              enqueue(mainth->idx, HLEVEL);
            }
            else {
              passval = getpvheap();
      
              if(mlfqpv < passval || passval == -1)
                passval = mlfqpv;
      
              heapinsert(mainth->idx, passval);
            }
          }
        }
      }

      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %d %d %s %s %d", p->idx,p->pid, p->thid,state, p->name, p->mainth->runblenum);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }


  cprintf("mlfq\n");
  showmlfq();
  cprintf("mlfq pass val: %d", mlfqpv);
  cprintf("\n\nstride heap\n");
  showheap();
  cprintf("sumtickets: %d\n", sumtickets);
  
}

void
addtick(int idx, int num) 
{
  acquire(&ptable.lock);
  ptable.proc[idx].mainth->ticks += num;
  release(&ptable.lock);
}


//move the processes of middle/low queues to high queue
void
priorityboost(void)
{
  int idx;
  struct proc *p;

  for (;;) {
    if (!isfull(HLEVEL) && !isempty(MLEVEL)) {
      idx = dequeue(MLEVEL);
      
	  p = &ptable.proc[idx];

      p->level = HLEVEL;
      p->ticks = 0;
      
      enqueue(idx, HLEVEL);
    }
    else
      break;
  }
  for (;;) {
    if (!isfull(HLEVEL) && !isempty(LLEVEL)) {
      idx = dequeue(LLEVEL);

	  p = &ptable.proc[idx];

      p->level = HLEVEL;
      p->ticks = 0;
      
      enqueue(idx, HLEVEL);
    }
    else
      break;
  }
}


int
getlev(void)
{
  return myproc()->mainth->level;
}

int 
set_cpu_share(int reqtickets) 
{

  acquire(&ptable.lock);

  struct proc *mainth = myproc()->mainth; 

  if(reqtickets <= 0) {
    release(&ptable.lock);
    return -1;
  }

  int proctickets = mainth->tickets;
  int sum = sumtickets + reqtickets - proctickets;

  if(sum > 80) {
      release(&ptable.lock);
      return -1;
  }

  if(mainth->level == -1 && proctickets) {
    mainth->tickets = reqtickets;
  }

  else {   
    mainth->level = -1;
    mainth->tickets = reqtickets;
  }
  sumtickets = sum;
  release(&ptable.lock);
  return 0; 
  
}

static struct proc*
thread_allocproc(void)
{
  struct proc *cur;
  struct proc *p;
  struct proc *mainth;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
	  goto found;
  
  release(&ptable.lock);
  return 0;

found:

  cur = myproc();
  mainth = cur->mainth;

  p->pid = mainth->pid;
  p->state = EMBRYO;
  p->idx = p - ptable.proc;
  p->parent = mainth->parent;
  p->join = cur;
  p->pgdir = mainth->pgdir;
  p->thid = ++mainth->nextid;
  p->mainth = mainth;
  p->nextth = mainth;
  p->prevth = mainth->prevth;
  
  p->prevth->nextth = p;
  mainth->prevth = p;
  
  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }

  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

int
thread_create(thread_t* thread, void* (*start_routine) (void*), void *arg) 
{
  struct proc *th;
  struct proc *mainth;
  int i;
  uint sz, usp;
  uint ustack[2];
  
  th = thread_allocproc();
  
  acquire(&ptable.lock);
  mainth = th->mainth;

  *th->tf = *(myproc()->tf);
  th->tf->eax = 0;
  
  for(i = 0; i < NOFILE; i++)
    if(mainth->ofile[i])
      th->ofile[i] = filedup(mainth->ofile[i]);
  th->cwd = idup(mainth->cwd);

  safestrcpy(th->name, mainth->name, sizeof(mainth->name));


  for(i = 0; i < NTHREAD; i++) 
    if(mainth->stack[i] != -1)
	  break;

  if(i != NTHREAD) {
	 usp = mainth->stack[i];
	 th->stackbot = usp;
	 mainth->stack[i] = -1;
  }
  else {
    sz = mainth->sz;
    sz = PGROUNDUP(sz);
	if( (sz = allocuvm(mainth->pgdir, sz, sz + 2*PGSIZE)) == 0) {
      kfree(th->kstack);
	  
      th->kstack = 0;
	  mainth->prevth = th->prevth;
	  th->prevth->nextth = mainth;
	 
	  th->state = UNUSED;
      th->pid = 0;
      th->parent = 0;
      th->join = 0;
      th->pgdir = 0;
      th->thid = 0;
      th->mainth = 0;
      th->nextth = 0;
      th->prevth = 0;
      release(&ptable.lock);
	  return -1;
	}
	clearpteu(mainth->pgdir, (char*)(sz - 2*PGSIZE));

	usp = sz;
	mainth->sz = sz;
    th->stackbot = sz;
  }
  
  ustack[0] = 0xffffffff;
  ustack[1] = (uint) arg;
  usp -= 8;
  
  if( copyout(mainth->pgdir, usp, ustack, 8) < 0) {
    kfree(th->kstack); 
    th->kstack = 0;
        
    mainth->prevth = th->prevth;
    th->prevth->nextth = mainth;
    
    for(i = 0; i < NTHREAD; i++) 
      if(mainth->stack[i] == -1)
        break;
    
    mainth->stack[i] = usp + 8;

    th->state = UNUSED;
    th->pid = 0;
    th->parent = 0;
    th->join = 0;
    th->pgdir = 0;
    th->thid = 0;
    th->mainth = 0;
    th->nextth = 0;
    th->prevth = 0;
    release(&ptable.lock);
    return -1;
  }

  th->tf->eip = (uint)start_routine;
  th->tf->esp = usp;
  
  th->state = RUNNABLE;

  mainth->runblenum++;
  *thread = th->thid;
  release(&ptable.lock);

  return 0;
}

void
thread_exit(void *retval) 
{

  struct proc *cur = myproc();
  int fd;

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(cur->ofile[fd]){
      fileclose(cur->ofile[fd]);
      cur->ofile[fd] = 0;
    }
  }
  begin_op();
  iput(cur->cwd);
  end_op();
  cur->cwd = 0;

  acquire(&ptable.lock);
  
  thread_wakeup1(cur->join);
  cur->retval = retval;
  cur->state = ZOMBIE;
  thread_sched();

  panic("zombie thread");
}

int 
thread_join (thread_t thread, void ** retval)
{
 
  struct proc *cur = myproc();
  struct proc *mainth = cur->mainth;
  struct proc *p;

  int exist, i;

  acquire(&ptable.lock);

  for(;;) {
    exist = 0;
    
	for(p = cur->nextth; p != cur; p = p->nextth) {
      if(p->thid != thread || p->join != cur)
	    continue;

      exist = 1;
      if(p->state == ZOMBIE) {
        *retval = p->retval;
        kfree(p->kstack);
        p->kstack = 0;
        
		for(i = 0; i < NTHREAD; i++)
          if(mainth->stack[i] == -1)
		    break;
	     
        mainth->stack[i] = p->stackbot;
        
        p->pid = 0;
        p->idx = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->state = UNUSED;

        p->nextth->prevth = p->prevth;
        p->prevth->nextth = p->nextth;
	    
        p->join = 0;
        p->thid = 0;
        p->mainth = 0;
        p->nextth = 0;
        p->prevth = 0;
        
        release(&ptable.lock);
		return 0;
	  }
	}

	if(!exist) {
      release(&ptable.lock);
	  return -1;
	}
    
    sleep(cur, &ptable.lock);
  }
}

void
thread_scheduler(void) 
{

  //struct cpu *c = mycpu();
  struct proc *p;
  struct proc *cur = myproc();
  
  if(cur->mainth->runblenum == 0) {
	yield_no_thread();
  } 
  else {
    for(p = cur->nextth; p != cur; p = p->nextth) {
      if(p->state != RUNNABLE)
        continue;
      
        
      p->state = RUNNING;
      p->mainth->runblenum--;
      p->mainth->recentth = p;
      mycpu()->proc = p;
	  switchuvm(p);
      swtch(&cur->context, p->context);
	  return;
    }
    if(p == cur &&p->state == RUNNABLE) {
      p->state = RUNNING; 
	  p->mainth->runblenum--;
      p->mainth->recentth = p;
      mycpu()->proc = p;
    }
  }
}

void
thread_sched(void) 
{
  int intena;
  struct proc *p = myproc(); 
  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1) {
    cprintf("pid: %d, ncli: %d\n", myproc()->pid, mycpu()->ncli);
    panic("sched locks");
  }  

  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");  

  intena = mycpu()->intena;
  thread_scheduler();
  mycpu()->intena = intena;
}

int
thread_yield(void) 
{
  struct proc *p = myproc();
  acquire(&ptable.lock);
  p->state = RUNNABLE;
  p->mainth->runblenum++;
  thread_sched();
  release(&ptable.lock);
  return 0;
}

int
yield_no_thread(void)
{
  sched();
  release(&ptable.lock);
  return 0;
}


void
thread_sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;
  thread_sched();


  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock) {  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

static void
thread_wakeup1(void *chan)
{
  struct proc *p;
  struct proc *cur = myproc();

  for(p = cur->nextth; p != cur; p = p->nextth) {
    if(p->state == SLEEPING && p->chan == chan && p == cur->join) { 
	  p->state = RUNNABLE;
	  p->mainth->runblenum++;
    }
  }
}


void
thread_wakeup(void *chan)
{
  acquire(&ptable.lock);
  thread_wakeup1(chan);
  release(&ptable.lock);
}

int
thread_pick(int idx)
{
  struct proc *p;
  struct proc *mainth = &ptable.proc[idx];
  struct proc *recentth = mainth->recentth;

  for(p = recentth->nextth; p != recentth; p = p->nextth) {
    if(p->state == RUNNABLE) {
      mainth->recentth = p;
	  return p->idx;
	}
  }
  
  if(p->state == RUNNABLE) {
    mainth->recentth = p;
    return p->idx;
  }

  return -1;
}

void 
prepare_exec (int idx)
{
  struct proc *cur = &ptable.proc[idx];
  struct proc *mainth = cur->mainth;
  struct proc *p;
  int i, fd;  

  for(p = cur->nextth; p != cur; p = p->nextth) {
    if(p->state != ZOMBIE) {
      
      for(fd = 0; fd < NOFILE; fd++) {
        if(p->ofile[fd]) {
          fileclose(p->ofile[fd]);
          p->ofile[fd] = 0;
        }
      }
      
      begin_op();
      iput(p->cwd);
      end_op();
      p->cwd = 0;
    }
  }
  acquire(&ptable.lock);
  
  cur->pid = mainth->pid;
  cur->level =  mainth->level;
  cur->ticks = mainth->ticks;
  cur->tickets = mainth->tickets;
  cur->parent = mainth->parent;
  cur->killed = mainth->killed;  
    
  cur->thid = 0;
  cur->nextid = 0;

  
  for(i = 0; i < NTHREAD; i++)
    cur->stack[i] = -1;

  
  for(p = cur->nextth; p != cur; p = p->nextth) {
    kfree(p->kstack);
    p->kstack = 0;
    p->pid = 0;
    p->parent = 0;
    p->name[0] = 0;
    p->killed = 0;
    p->level = 0;
    p->idx = 0;
    p->ticks = 0;
    p->tickets = 0;
    p->state = UNUSED;
    p->join = 0;
    p->thid = 0;    
    for(i = 0; i < NTHREAD; i++)
      p->stack[i] = -1;
  }
  
  cur->mainth = cur;
  cur->nextth = cur;
  cur->prevth = cur;
  cur->recentth = cur;
  cur->runblenum = 0;

  release(&ptable.lock);
}
