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
  temp = sbrk(20 * 2);
  printf(1, "%d sbrk %d\n", ret,temp);
//  thread_create(&th, func2, (void*) 200);
  
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
  
  temp = sbrk(20 * 2);
  printf(1, "user0 sbrk %d\n", temp);


  thread_create(&th[0], func, (void*) 0);
  thread_create(&th[1], func, (void*) 1);
  thread_join(th[0], &retval[0]);
  thread_join(th[1], &retval[1]);
  temp = sbrk(20 * 2);
  printf(1, "user1 sbrk %d\n", temp);

  thread_create(&th[0], func, (void*) 0);
  thread_create(&th[1], func, (void*) 1);
  thread_create(&th[2], func, (void*) 2);
  thread_join(th[0], &retval[0]);
  thread_join(th[1], &retval[1]);
  thread_join(th[2], &retval[2]);
  temp = sbrk(20 * 2);
  printf(1, "user2 sbrk %d\n", temp);
  exit();
}
