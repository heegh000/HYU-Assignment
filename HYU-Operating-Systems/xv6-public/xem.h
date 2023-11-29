struct wqueue {
  int arr[NTHREAD+1];
  int head;
  int tail;	
};

typedef struct x{
  int count;
  struct wqueue waiting;
  struct spinlock lk;		
} xem_t;



typedef struct rw{
  xem_t racelock;
  xem_t starvlock;
  xem_t sharelock;
  int reader;
} rwlock_t;
