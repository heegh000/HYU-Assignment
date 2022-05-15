#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->mainth->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_getppid(void)
{
  return myproc()->parent->pid;  
}

int 
sys_yield(void)
{
  return yield();
}


int 
sys_getlev(void)
{
  return getlev();
}

int
sys_set_cpu_share(void)
{
  int tickets;

  if(argint(0, &tickets) < 0)
    return -1;
  
  return set_cpu_share(tickets);
}

int
sys_thread_create(void)
{
  int thread;
  int start_routine;
  int arg;

  if(argint(0, &thread) < 0)
    return -1;
  if(argint(1, &start_routine) < 0)
    return -1;
  if(argint(2, &arg) < 0)
    return -1;
  

  return thread_create((thread_t*) thread, (void*) start_routine, (void*) arg);
}


int
sys_thread_exit(void)
{
  int retval;

  if(argint(0, &retval) < 0)
    return -1;

  thread_exit((void*) retval);

  return -1;
}

int
sys_thread_join(void)
{
  int thread;
  int retval;

  if(argint(0, &thread) < 0)
    return -1;
  if(argint(1, &retval) < 0)
    return -1; 
  
  return thread_join((thread_t) thread, (void**) retval);
  
}

/*
int
sys_thread_create(void)
{
  thread_t* arg0;
  void* (*arg1)(void*);
  void* arg2;

  if(argint(0, (int*) &arg0) < 0)
    return -1;
  if(argint(1, (int*) &arg1) < 0)
    return -1;
  if(argint(2, (int*) &arg2) < 0)
    return -1;
  

  return thread_create( arg0, arg1, arg2);
}


int
sys_thread_exit(void)
{
  void* arg0;

  if(argint(0,(int*) &arg0) < 0)
    return -1;

  thread_exit( arg0);

  return -1;
}

int
sys_thread_join(void)
{
  thread_t arg0;
  void** arg1;

  if(argint(0, (int*) &arg0) < 0)
    return -1;
  if(argint(1, (int*) &arg1) < 0)
    return -1; 
  
  return thread_join(arg0, arg1);
  
}
*/
