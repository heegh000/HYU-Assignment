#include "types.h"
#include "stat.h"
#include "user.h"

extern int sumtickets;


int
main(int argc, char *argv[]) 
{

  int pid = fork();

  if(pid > 0) {
    set_cpu_share(10);
    wait();
  }
  else if (pid == 0) {
    set_cpu_share(0);
    sleep(200);
    printf(1, "change\n");
    set_cpu_share(10);
    sleep(200);
    exit();
  }

  printf(1, "done\n");
  sleep(200);

  exit();
}
