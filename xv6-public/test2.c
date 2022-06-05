

#include "types.h"
#include "stat.h"
#include "user.h"

rwlock_t rw;
xem_t xem;
volatile int gcnt;

void nop() { }

void*
read_func(void* arg) {
  
  rwlock_acquire_readlock(&rw);
  xem_wait(&xem);
  printf(1, "R%d, gcnt: %d\n", (int) arg, gcnt);
  xem_unlock(&xem);
  sleep(100);
  rwlock_release_readlock(&rw);
  printf(1, "R%d, read lock\n", (int) arg);
  thread_exit(arg);

  return 0;
}


void*
write_func(void* arg) {
  rwlock_acquire_writelock(&rw);
  gcnt++;
  printf(1, "W%d, gcnt %d\n", (int) arg, gcnt);
  rwlock_release_writelock(&rw); 
  printf(1, "W%d, write lock\n", (int) arg);
  thread_exit(arg);
  return 0;
}


void*
func(void* arg) 
{
  int tmp;
  int i;
  printf(1, "arg: %d\n", (int) arg);
//xem_wait(&xem);
  for(i = 0; i < 100000; i++) {
    tmp = gcnt;
    tmp++;
	asm volatile("call %P0"::"i"(nop));
    gcnt = tmp;
  }
  
//xem_unlock(&xem);
  thread_exit( (void*)arg); 
  return 0; 
}


int
main(int argc, char *argv[])
{
  thread_t th[10];
  void* ret[10];

  xem_init(&xem);
  rwlock_init(&rw);

  for(int i = 0; i < 10; i++) {
    if(i == 5)
     thread_create(&th[i], write_func, (void*) i);
    else
      thread_create(&th[i], read_func, (void*) i);
  }

  for(int i = 0; i < 10; i++) {
    thread_join(th[i], &ret[i]);
  }
  printf(1, "gcnt: %d\n", gcnt);
  exit();
  return 0;
}
