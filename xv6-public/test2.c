#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

#define FILESIZE        (16*1024*1024)  // 16 MB
#define BUFSIZE         512
#define BUF_PER_FILE    ((FILESIZE) / (BUFSIZE))
#define NUM_STRESS      4

int
main(int argc, char *argv[])
{
    int fd, i, j; 
    int r;
    int total;
    char *path = (argc > 1) ? argv[1] : "hugefile";
    char data[BUFSIZE];
    char buf[BUFSIZE];

    printf(1, "hugefiletest starting\n");
    const int sz = sizeof(data);
    for (i = 0; i < sz; i++) {
        data[i] = i % 128;
    }

    printf(1, "1. create test\n");
    fd = open(path, O_CREATE | O_RDWR);
    for(i = 0; i < BUF_PER_FILE; i++){
        if (i % 1000 == 0){
            printf(1, "%d bytes written\n", i * BUFSIZE);
        }
        if ((r = write(fd, data, sizeof(data))) != sizeof(data)){
            printf(1, "write returned %d : failed\n", r);
            exit();
        }
    }
    printf(1, "%d bytes written\n", BUF_PER_FILE * BUFSIZE);
    close(fd);

    printf(1, "2. read test\n");
    fd = open(path, O_RDONLY);
    for (i = 0; i < BUF_PER_FILE; i++){
        if (i % 1000 == 0){
            printf(1, "%d bytes read\n", i * BUFSIZE);
        }
        if ((r = read(fd, buf, sizeof(data))) != sizeof(data)){
            printf(1, "read returned %d : failed\n", r);
            exit();
        }
        for (j = 0; j < sz; j++) {
            if (buf[j] != data[j]) {
                printf(1, "data inconsistency detected\n");
                exit();
            }
        }
    }
    printf(1, "%d bytes read\n", BUF_PER_FILE * BUFSIZE);
    close(fd);

    printf(1, "3. stress test\n");
    total = 0;
    for (i = 0; i < NUM_STRESS; i++) {
        printf(1, "stress test...%d \n", i);
        if(unlink(path) < 0){
            printf(1, "rm: %s failed to delete\n", path);
            exit();
        }

        fd = open(path, O_CREATE | O_RDWR);
        for(j = 0; j < BUF_PER_FILE; j++){
            if (j % 1000 == 0){
                printf(1, "%d bytes totally written\n", total);
            }
            if ((r = write(fd, data, sizeof(data))) != sizeof(data)){
                printf(1, "write returned %d : failed\n", r);
                exit();
            }
            total += sizeof(data);
        }
        printf(1, "%d bytes written\n", total);
        close(fd);
    }

    exit();
}
/*
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

#define NUM_THREAD       10
#define FILESIZE         (1600*1024)  // 1.6 MB
#define BUFSIZE          512
#define FSIZE_PER_THREAD ((FILESIZE) / (NUM_THREAD))

char *filepath = "myfile";
int fd; // Shared among threads

void pwritetest();
void preadtest();

int
main(int argc, char *argv[])
{
  // pwrite test
  printf(1, "1. Start pwrite test\n");
  pwritetest();
  printf(1, "Finished\n");
  
  // pread test
  printf(1, "2. Start pread test\n");
  preadtest();
  printf(1, "Finished\n");

  exit();
}


void
pwritetestmain(void *arg)
{
  int tid = (int) arg;
  int r, i, off;
  char data[BUFSIZE];

  int start = FSIZE_PER_THREAD * tid;
  int end = start + FSIZE_PER_THREAD;

  for(i = 0; i < BUFSIZE; i++)
    data[i] = (tid + i) % 128;

  printf(1, "Thread #%d is writing (%d ~ %d)\n", tid, start, end);
  
  for(off = start; off < end; off+=BUFSIZE){
    if ((off / BUFSIZE) % 300 == 0){
      printf(1, "Thread %d: %d bytes written\n", tid, off - start);
    }
    if ((r = pwrite(fd, data, sizeof(data), off)) != sizeof(data)){
      printf(1, "pwrite returned %d : failed\n", r);
      exit();
    }
  }

  printf(1, "Thread %d: writing finished\n", tid);
  thread_exit((void *)0);
}

void
pwritetest()
{
  thread_t threads[NUM_THREAD];
  int i;
  void* retval;
  
  // Open file (file is shared among thread)
  fd = open(filepath, O_CREATE | O_RDWR);

  for(i = 0; i < NUM_THREAD; i++){
    if(thread_create(&threads[i], (void*)pwritetestmain, (void*)i) != 0){
      printf(1, "panic at thread_create\n");
      close(fd);
      return;
    }
  }

  for (i = 0; i < NUM_THREAD; i++){
    if (thread_join(threads[i], &retval) != 0){
      printf(1, "panic at thread_join\n");
      close(fd);
      return; 
    }
  }
  close(fd);
}

void
preadtestmain(void *arg)
{
  int tid = (int) arg;
  int r, off, i;
  char buf[BUFSIZE];
  
  int start = FSIZE_PER_THREAD * tid;
  int end = start + FSIZE_PER_THREAD;

  printf(1, "Thread #%d is reading (%d ~ %d)\n", tid, start, end);
  
  for(off = start; off < end; off+=BUFSIZE){
    if ((r = pread(fd, buf, sizeof(buf), off)) != sizeof(buf)){
      printf(1, "pread returned %d : failed\n", r);
      exit();
    }
    for (i = 0; i < BUFSIZE; i++) {
      if (buf[i] != (tid + i) % 128) {
        printf(1, "data inconsistency detected\n");
        exit();
      }
    }
  }
  thread_exit((void *)0);
}

void
preadtest()
{
  thread_t threads[NUM_THREAD];
  int i;
  void* retval;
  
  // Open file (file is shared among thread)
  fd = open(filepath, O_RDONLY);

  for(i = 0; i < NUM_THREAD; i++){
    if(thread_create(&threads[i], (void*)preadtestmain, (void*)i) != 0){
      printf(1, "panic at thread_create\n");
      close(fd);
      return;
    }
  }

  for (i = 0; i < NUM_THREAD; i++){
    if (thread_join(threads[i], &retval) != 0){
      printf(1, "panic at thread_join\n");
      close(fd);
      return; 
    }
  }
  close(fd);
} */
/*
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
}*/
