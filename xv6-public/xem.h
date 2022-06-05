struct wqueue {
  int arr[15+1];
  int head;
  int tail;	
};

typedef struct x{
  int count;
  struct wqueue waiting;
  struct spinlock lk;		
} xem_t;



typedef struct rw{
  xem_t sharelock;
  xem_t readlock;
  xem_t writelock;
  int reader;
} rwlock_t;
