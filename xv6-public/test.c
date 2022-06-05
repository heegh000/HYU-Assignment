#include "types.h"
#include "stat.h"
#include "user.h"

xem_t sem;

void *
test_without_sem(void *arg)
{
  int id = (int)arg;
  for(int i = 0; i < 10; ++i)
    printf(1, "%d", id);
  thread_exit(0);
  return 0;
}

void *
test_with_sem(void *arg)
{
  int id = (int)arg;
  xem_wait(&sem);
  for(int i = 0; i < 10; ++i)
    printf(1, "%d", id);
  xem_unlock(&sem);
  thread_exit(0);
  return 0;
}

int
main(int argc, char *argv[])
{
  void *ret;
  const int N = 10;
  thread_t t[N];

  printf(1, "1. Test without any synchronization\n");
  for(int i = 0; i < N; ++i) {
    if(thread_create(&t[i], test_without_sem, (void*)(i)) < 0) {
      printf(1, "panic at thread create\n");
      exit();
    }
  }
  for(int i = 0; i < N; ++i) {
    if(thread_join(t[i], &ret) < 0) {
      printf(1, "panic at thread join\n");
      exit();
    }
  }
  printf(1, "\nIts sequence could be mixed\n");

  printf(1, "2. Test with synchronization of a binary semaphore\n");
  xem_init(&sem);
  xem_wait(&sem);
  for(int i = 0; i < N; ++i) {
    if(thread_create(&t[i], test_with_sem, (void*)(i)) < 0) {
      printf(1, "panic at thread create\n");
      exit();
    }
  }
  xem_unlock(&sem);
  for(int i = 0; i < N; ++i) {
    if(thread_join(t[i], &ret) < 0) {
      printf(1, "panic at thread join\n");
      exit();
    }
  }
  printf(1, "\nIts sequence must be sorted\n");

  printf(1, "3. Test with synchronization of a semaphore with 3 users\n");
  xem_init(&sem);
  sem.count = 3;
  xem_wait(&sem);
  for(int i = 0; i < N; ++i) {
    if(thread_create(&t[i], test_with_sem, (void*)(i)) < 0) {
      printf(1, "panic at thread create\n");
      exit();
    }
  }
  xem_unlock(&sem);
  for(int i = 0; i < N; ++i) {
    if(thread_join(t[i], &ret) < 0) {
      printf(1, "panic at thread join\n");
      exit();
    }
  }
  printf(1, "\nIts sequence could be messy\n");
  exit();
}
/*
#include "types.h"
#include "stat.h"
#include "user.h"

void*
func2 (void* arg)
{
  int ret = (int) arg;
  printf(1, "Hello, Im func2 ddr: %d, arg: %d\n", &ret, ret);  
  thread_exit((void*)ret);  
  return 0;
}

void* 
func(void* arg) 
{

  int ret = (int) arg;
//  thread_t th;
//  void* retval;
 char* temp;
  int addr;
  sleep(10);
  printf(1, "Hello Im func addr: %d\n", &addr);
  sleep(5);
  temp = malloc(3000 * 2);
  printf(1, "thread%d malloc %d\n", ret,temp);
//  thread_create(&th, func2, (void*) 30000);
  
//  thread_join(th, &retval);
  
//  ret = (int)retval + (int)arg;
//  printf(1, "func: arg: %d, retval: %d\n", ret, retval);  
  
  thread_exit((void*)ret);
  return 0;
}


int
main(int argc, char *argv[]) 
{
  thread_t th[3];
  void* retval[3];
  int addr;
  char* temp;
  printf(1, "HELLO, Im main addr: %d\n", &addr);

  thread_create(&th[0], func, (void*) 0);
  thread_join(th[0], &retval[0]);
  
  temp = malloc(3000 * 2);
  printf(1, "main0 malloc %d\n", temp);


  thread_create(&th[0], func, (void*) 0);
  thread_create(&th[1], func, (void*) 1);
  thread_join(th[0], &retval[0]);
  thread_join(th[1], &retval[1]);
  temp = malloc(3000 * 2);
  printf(1, "main1 malloc %d\n", temp);

  thread_create(&th[0], func, (void*) 0);
  thread_create(&th[1], func, (void*) 1);
  thread_create(&th[2], func, (void*) 2);
  thread_join(th[0], &retval[0]);
  thread_join(th[1], &retval[1]);
  thread_join(th[2], &retval[2]);
  temp = malloc(3000 * 2);
  printf(1, "main2 malloc %d\n", temp);
  exit();
}*/
