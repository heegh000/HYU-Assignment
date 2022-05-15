#include "types.h"
#include "stat.h"
#include "user.h"


void* 
func(void* arg) 
{

  int ret = (int) arg;
  printf(1, "addr: %d, arg: %d\n", &ret, ret);  
  printf(1, "HELLO, Here is func\n"); 
    
  thread_exit((void*)ret);
  return 0;
}

void*
func2 (void* arg)
{
  int ret = (int) arg;
  printf(1, "addr: %d, arg: %d\n", &ret, ret);  
  printf(1, "Hello, Here is func2\n");
  thread_exit((void*)ret);
  return 0;
}

int
main(int argc, char *argv[]) 
{
  thread_t th[2];
  void* retval[2];
  int addr;
  printf(1, "addr: %d\n", &addr);  
  printf(1, "HELLO, Im main\n");

  thread_create(&th[0], func, (void*) 100);
  thread_create(&th[1], func2, (void*) 0);

  thread_join(th[0], &retval[0]);
  thread_join(th[1], &retval[1]);

  printf(1, "retval1: %d, retval2: %d\n", retval[0], retval[1]);
  
  thread_create(&th[0], func, (void*) 50);
  thread_join(th[0], &retval[0]);

  printf(1, "retval: %d\n", retval[0]);
  exit();
}
