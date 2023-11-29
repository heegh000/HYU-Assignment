#include "types.h"
#include "user.h"

#define NTHREADS 10

int c = 10;

void* thread_func(void* arg) {
  int thread_id = (int)arg;
  printf(1, "Thread %d is running\n", thread_id);
  c -= 1;
  printf(1, "%d\n", c);
  sleep(10);
  int* ret;
  thread_exit(&ret);
  printf(1, "ASDSADASDSADSAD");
  return ret;
}

int main() {
  int i;
  int thread_args[NTHREADS];
  thread_t threads[NTHREADS];
  // Create threads

  printf(1, "%d\n", &thread_func);
  for (i = 0; i < NTHREADS; i++) {
    thread_args[i] = i;
    printf(1, "for문 : %d\n", thread_args[i]);
    if (thread_create(&threads[i], thread_func, (void*)&thread_args[i]) != 0) {
      printf(1, "Failed to create thread\n");
      exit();
    }
    sleep(10);
  }
  // // Wait for threads to finish
  for (i = 0; i < NTHREADS; i++) {
    if (thread_join(threads[i], 0) != 0) {
      printf(1, "Failed to join thread\n");
      exit();
    }
  }
  exit();
}