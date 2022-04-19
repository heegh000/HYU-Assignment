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

extern int istiin;
extern int sys_uptime();
extern int passti;
extern struct spinlock tickslock;


extern struct node strheap[3];

static void wakeup1(void *chan);

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

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
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
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);
  np->state = RUNNABLE;
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
  struct proc *curproc = myproc();
  struct proc *p;
  int fd;

  if(curproc == initproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(curproc->ofile[fd]){
      fileclose(curproc->ofile[fd]);
      curproc->ofile[fd] = 0;
    }
  }
  begin_op();
  iput(curproc->cwd);
  end_op();
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  wakeup1(curproc->parent);

  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup(initproc);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sumtickets -= curproc->tickets;

  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
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
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->level = 0;
        p->idx = 0;
        p->ticks = 0;
        p->tickets = 0;
        p->state = UNUSED;
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

  int idx = 0;
  uint beforeproctick = 0;
  uint beforetick = 0;

  int minpv = 0;
  int pbcount = 0;

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
      	if(istiin) {
       		mlfqtickets = 100 - sumtickets;
        	mlfqpv += BIGNUM / mlfqtickets;	
			istiin = 0;
		}
        release(&ptable.lock);
        continue;
      }    


      beforetick = sys_uptime();
      beforeproctick = ptable.proc[idx].ticks;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = &ptable.proc[idx];
      switchuvm(&ptable.proc[idx]);
      ptable.proc[idx].state = RUNNING; 
      

      if(ptable.proc[idx].ticks == 9 && ptable.proc[idx].level == 1 && ptable.proc[idx].killed == 0) {
        acquire(&tickslock);
        passti += 1;
        release(&tickslock);        
      } 

      swtch(&(c->scheduler), ptable.proc[idx].context);
      switchkvm();
      
      ptable.proc[idx].ticks++;
      passti = 0;
      
      
      mlfqtickets = 100 - sumtickets; 
      
      pbcount += sys_uptime() - beforetick;
      
	  mlfqpv += (ptable.proc[idx].ticks - beforeproctick) * (BIGNUM / mlfqtickets);

      if(ptable.proc[idx].level == 0 && ptable.proc[idx].ticks >= 5) {
         ptable.proc[idx].ticks = 0;
         ptable.proc[idx].level = 1;
       }
       else if(ptable.proc[idx].level == 1 && ptable.proc[idx].ticks >= 10) {  
         ptable.proc[idx].ticks = 0;
         ptable.proc[idx].level = 2;
       }    
  
      if(ptable.proc[idx].state == RUNNABLE) {
        if(ptable.proc[idx].level != -1) {
          enqueue(idx, ptable.proc[idx].level);
        }
        else {
          int passval = getpvheap();
          if(mlfqpv < passval || passval == -1)
            passval = mlfqpv;

          heapinsert(idx, passval);
        }
      }
      if(pbcount >= 100) {
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

      c->proc = &ptable.proc[idx];
      switchuvm(&ptable.proc[idx]);
      ptable.proc[idx].state = RUNNING;   
      swtch(&(c->scheduler), ptable.proc[idx].context);
      switchkvm();

      if(ptable.proc[idx].state == RUNNABLE) {
        minpv += BIGNUM / ptable.proc[idx].tickets;
        heapinsert(idx, minpv);
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
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
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

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if(p->state == SLEEPING && p->chan == chan) {
      
      p->state = RUNNABLE;
      if(p->level != -1) {
	    if (chan != &ticks) {
			p->level = 0;
			p->ticks = 0;
       		enqueue(p->idx, 0);
		}
		else {
			enqueue(p->idx, p->level);
		}

      }
      else {
        int passval = getpvheap();

        if(mlfqpv < passval || passval == -1)
          passval = mlfqpv;

        heapinsert(p->idx, passval);
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
  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING) {
        p->state = RUNNABLE;

        if(p->level != -1) {
          p->level = 0;
          p->ticks = 0;
          enqueue(p->idx, 0);
        }
        else {
          int passval = getpvheap();
      
          if(mlfqpv < passval || passval == -1)
            passval = mlfqpv;
      
          heapinsert(p->idx, passval);
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
    cprintf("%d %s %s", p->pid, state, p->name);
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
  ptable.proc[idx].ticks += num;
  release(&ptable.lock);
}


//move the processes of middle/low queues to high queue
void
priorityboost(void)
{
  int idx;
    
  for (;;) {
    if (!isfull(0) && !isempty(1)) {
      idx = dequeue(1);

      ptable.proc[idx].level = 0;
      ptable.proc[idx].ticks = 0;
      
      enqueue(idx, 0);
    }
    else
      break;
  }
  for (;;) {
    if (!isfull(0) && !isempty(2)) {
      idx = dequeue(2);

      ptable.proc[idx].level = 0;
      ptable.proc[idx].ticks = 0;
      
      enqueue(idx, 0);
    }
    else
      break;
  }
}


int
getlev(void)
{
  return myproc()->level;
}

int 
set_cpu_share(int reqtickets) 
{

  acquire(&ptable.lock);

  if(reqtickets <= 0) {
    release(&ptable.lock);
    return -1;
  }

  int proctickets = ptable.proc[myproc()->idx].tickets;
  int sum = sumtickets + reqtickets - proctickets;

  if(sum > 80) {
      release(&ptable.lock);
      return -1;
  }

  if(proctickets) {
    ptable.proc[myproc()->idx].tickets = reqtickets;
  }

  else {   
    ptable.proc[myproc()->idx].level = -1;
    ptable.proc[myproc()->idx].tickets = reqtickets;
  }
  sumtickets = sum;
  release(&ptable.lock);
  return 0; 
  
}
