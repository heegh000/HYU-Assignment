#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"


typedef struct thsg {
  rwlock_t rwlock;
  int fd;   
} thread_safe_guard;

thread_safe_guard* 
thread_safe_guard_init(int fd) {
  thread_safe_guard* thsg = (thread_safe_guard*)malloc(sizeof(thread_safe_guard));
  rwlock_init(&(thsg->rwlock));
  thsg->fd = fd;
  return thsg;
}

int  
thread_safe_pread(thread_safe_guard* file_guard, void* addr, int n, int off ) {

  int ret_val;

  rwlock_acquire_readlock(&(file_guard->rwlock));
  
  ret_val = pread(file_guard->fd, addr, n, off);

  rwlock_release_readlock(&(file_guard->rwlock));

  return ret_val;
}


int thread_safe_pwrite(thread_safe_guard* file_guard, void* addr, int n, int off ) {

  int ret_val;

  rwlock_acquire_writelock(&(file_guard->rwlock));

  ret_val = pwrite(file_guard->fd, addr, n, off);

  rwlock_release_writelock(&(file_guard->rwlock));

  return ret_val;
}


void
thread_safe_guard_destroy(thread_safe_guard* file_guard) {
  free(file_guard);
}
