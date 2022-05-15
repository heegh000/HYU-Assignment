#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "mlfq.h"

// three queues that have each different priority level
struct cqueue mlfq[3];

extern struct proc* myproc(void);

int 
isempty (int level) 
{
  if(mlfq[level].head == mlfq[level].tail)
    return 1;
  else
    return 0;
}


int
isfull (int level)
{
  if(mlfq[level].head == (mlfq[level].tail + 1) % MAXCAPACITY)
    return 1;
  else
    return 0;
}

// return the input value on success or -1 on failure
int
enqueue(int idx, int level)
{
  int i = 0;
  if(!isempty(level)) {
    for(i = mlfq[level].head +1; i != mlfq[level].tail; i = (i+1) % MAXCAPACITY)
      if(mlfq[level].procidx[i] == idx)
	    return -1;
    if (mlfq[level].procidx[i] == idx)
      return -1;
  }
  
  if(isfull(level))
    return -1;

  mlfq[level].tail = (mlfq[level].tail+1) % MAXCAPACITY;
  (mlfq[level].procidx)[mlfq[level].tail] = idx;  

  return idx;
}

// return the head of the queue on success or -1 on failure
int
dequeue(int level)
{
  if (isempty(level))
    return -1;

  mlfq[level].head = (mlfq[level].head + 1) % MAXCAPACITY;

  return mlfq[level].procidx[mlfq[level].head];
}


void
showqueue(int level)
{
  int i; 

  if(isempty(level)) {
    cprintf("The queue is empty\n");
    return;
  }

  for(i = mlfq[level].head + 1; i != mlfq[level].tail; i = (i+1) % MAXCAPACITY) 
    cprintf("%d, ", mlfq[level].procidx[i]);
  cprintf("%d\n", mlfq[level].procidx[i]);
}

void
showmlfq()
{
  cprintf("-----------------------\n");
  showqueue(0);
  showqueue(1);
  showqueue(2);
  cprintf("-----------------------\n");
}


int
pickprocmlfq()
{
  
  if (!isempty(0))
    return dequeue(0);
  

  else if (!isempty(1)) 
    return dequeue(1);


  else if (!isempty(2)) 
    return dequeue(2);

  return -1;
}

/*
void
priorityboost() 
{

  int idx;

  for (;;) {
    if (!isfull(0) && !isempty(1)) {
      idx = dequeue(1);

      ptable.proc[idx].level = 1;
      
      enqueue(0, idx);
    }
    else
      break;
  }
  for (;;) {
    if (!isfull(0) && !isempty(2)) {
      idx = dequeue(2);  
  
    ptable.proc[idx].level = 2;

      enqueue(0, idx);
    }
    else
      break;
  }
}

*/
