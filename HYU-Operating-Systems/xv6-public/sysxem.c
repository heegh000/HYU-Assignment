#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "xem.h"

int
sys_xem_init(void)
{
  int xem;
  
  if(argint(0, &xem) < 0) 
    return -1;

  return xem_init( (xem_t*) xem);
}

int
sys_xem_wait(void)
{
  int xem;
  
  if(argint(0, &xem) < 0) 
    return -1;
  return xem_wait( (xem_t*) xem);
}

int 
sys_xem_unlock(void)
{
  int xem;
  
  if(argint(0, &xem) < 0) 
    return -1;

  return xem_unlock( (xem_t*) xem);
}

int
sys_rwlock_init(void)
{
  int rwlock;
  
  if(argint(0, &rwlock) < 0)
    return -1;
  
  return rwlock_init( (rwlock_t*) rwlock);
}

int
sys_rwlock_acquire_readlock(void)
{
  int rwlock;
  
  if(argint(0, &rwlock) < 0)
    return -1;
  
  return rwlock_acquire_readlock( (rwlock_t*) rwlock);

}

int
sys_rwlock_acquire_writelock(void)
{
  int rwlock;
  
  if(argint(0, &rwlock) < 0)
    return -1;
  
  return rwlock_acquire_writelock( (rwlock_t*) rwlock);


}

int 
sys_rwlock_release_readlock(void)
{
  int rwlock;
  
  if(argint(0, &rwlock) < 0)
    return -1;
  
  return rwlock_release_readlock( (rwlock_t*) rwlock);

}

int
sys_rwlock_release_writelock(void)
{
  int rwlock;
  
  if(argint(0, &rwlock) < 0)
    return -1;
  
  return rwlock_release_writelock( (rwlock_t*) rwlock);

}
