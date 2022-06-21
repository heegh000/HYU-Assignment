#include "types.h"
#include "stat.h"
#include "user.h"
#include "param.h"

typedef struct thread_sg {
  rwlock_t rw;
  int fd;   
} thread_safe_guard;


thread_safe_guard* 
thread_safe_guard_init(int fd) {
  thread_safe_guard* tsg = (thread_safe_guard*)malloc(sizeof(thread_safe_guard));
  rwlock_init(&tsg->rw);
  tsg->fd = fd;
  return tsg;
}

int  
thread_safe_pread(thread_safe_guard* file_guard, void* addr, int n, int off ) {

  int ret_val;

  rwlock_acquire_readlock(&file_guard->rw);
  
  ret_val = pread(file_guard->fd, addr, n, off);

  rwlock_release_readlock(&file_guard->rw);

  return ret_val;
}


int thread_safe_pwrite(thread_safe_guard* file_guard, void* addr, int n, int off ) {

  int ret_val;

  rwlock_acquire_writelock(&file_guard->rw);

  ret_val = pwrite(file_guard->fd, addr, n, off);

  rwlock_release_writelock(&file_guard->rw);

  return ret_val;
}


void
thread_safe_guard_destroy(thread_safe_guard* file_guard) {
  free(file_guard);
} 
