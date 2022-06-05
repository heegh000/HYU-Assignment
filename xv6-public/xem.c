#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "spinlock.h"
#include "proc.h"
#include "xem.h"

int 
wq_init (struct wqueue* wq) 
{
  int i;

  wq->head = 0;
  wq->tail = 0;
  
  for(i = 0; i < NTHREAD + 1; i++)
    wq->arr[i] = -1;
  return 0;
}

int
wq_isempty (struct wqueue* wq) 
{
  if(wq->head == wq->tail)
    return 1;
  else
    return 0;
}

int
wq_isfull (struct wqueue* wq) 
{
  if(wq->head == (wq->tail + 1) % ( NTHREAD + 1 ) )
    return 1;
  else
    return 0;	
}

int
wq_enqueue (int idx, struct wqueue* wq)
{
  int i = 0;
  if(!wq_isempty(wq)) {
    for(i = wq->head + 1; i != wq->tail; i = (i+1) % (NTHREAD + 1)) {
      if(wq->arr[i] == idx)
        return -1;
    }
    if(wq->arr[i] == idx)
      return -1;
  }

  if(wq_isfull(wq))
    return -1;

  wq->tail = (wq->tail + 1) % (NTHREAD + 1);
  wq->arr[wq->tail] = idx;
  return idx;
}

int
wq_dequeue (struct wqueue* wq) 
{
  int retval;
  
  if(wq_isempty(wq))
    return -1;

  wq->head = (wq->head + 1) % (NTHREAD + 1);
  retval = wq->arr[wq->head];
  wq->arr[wq->head] = -1;
  
  return retval;
}

int
wq_peak (struct wqueue* wq) 
{
  int head;
  int retval;
  
  if(wq_isempty(wq)) 
    return -1;
  
  head = (wq->head + 1) % (NTHREAD + 1);
  retval = wq->arr[head];
  
  return retval;
}

void
wq_showqueue(struct wqueue* wq)
{
  int i;

  if(wq_isempty(wq)) {
    cprintf("The waiting queue is empty\n");
    return;
  }

  cprintf("waiting queue: ");
  for(i = wq->head + 1; i != wq->tail; i = (i + 1) % (NTHREAD + 1))
    cprintf("%d, ", wq->arr[i]);
  cprintf("%d\n", wq->arr[i]);
}

int 
xem_init(xem_t *semaphore) 
{
  initlock(&semaphore->lk, "semaphore"); 
  semaphore->count = 1;
  wq_init(&semaphore->waiting);
  return 0;
}

int
xem_wait(xem_t *semaphore) 
{
  struct proc* cur = myproc();
  acquire(&semaphore->lk);
  semaphore->count--;
  if(semaphore->count < 0) {
    wq_enqueue(cur->idx, &semaphore->waiting);
    sleep(cur, &semaphore->lk);
  } 
  release(&semaphore->lk);
  return 0;
}

int
xem_unlock(xem_t* semaphore) 
{
  int next;
  acquire(&semaphore->lk);
  semaphore->count++; 
  next = wq_dequeue(&semaphore->waiting);
  if(next != -1) {
    xem_wakeup(next);
  }
  
  release(&semaphore->lk);
  return 0;
}

int 
rwlock_init(rwlock_t *rwlock) 
{
  xem_init(&rwlock->racelock);
  xem_init(&rwlock->starvlock);
  xem_init(&rwlock->sharelock);
  rwlock->reader = 0;
  return 0;
}

int 
rwlock_acquire_readlock(rwlock_t *rwlock) 
{
  xem_wait(&rwlock->starvlock);
  xem_wait(&rwlock->racelock);
  
  rwlock->reader++;
  if(rwlock->reader == 1)
    xem_wait(&rwlock->sharelock);

  xem_unlock(&rwlock->racelock);
  xem_unlock(&rwlock->starvlock);
  return 0;
}

int 
rwlock_acquire_writelock(rwlock_t *rwlock) 
{

  xem_wait(&rwlock->starvlock);
  xem_wait(&rwlock->sharelock);

  return 0;
}

int 
rwlock_release_readlock(rwlock_t *rwlock) 
{
  xem_wait(&rwlock->racelock);
  
  rwlock->reader--;
  if(rwlock->reader == 0)
    xem_unlock(&rwlock->sharelock);

  xem_unlock(&rwlock->racelock);
  return 0;
}

int 
rwlock_release_writelock(rwlock_t *rwlock) 
{
  xem_unlock(&rwlock->sharelock);
  xem_unlock(&rwlock->starvlock);
  return 0;
}
