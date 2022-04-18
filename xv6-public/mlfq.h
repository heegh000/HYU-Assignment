// circular queue to store idx of proc in proc array
struct cqueue {
  int procidx[MAXCAPACITY];
  int head;
  int tail;
};

// check state of the queue
int isempty(int);
int isfull(int);
// return the input value on success or -1 on failure
int enqueue(int, int);
// return the head of the queue on success or -1 on failure
int dequeue(int);
// for debugging
void showqueue(int);
void showmlfq();
// pick a next process to run
// return index of process on success or -1 on failure
int pickprocmlfq();
