#define NPROC        64  // maximum number of processes
#define KSTACKSIZE 4096  // size of per-process kernel stack
#define NCPU          8  // maximum number of CPUs
#define NOFILE       16  // open files per process
#define NFILE       100  // open files per system
#define NINODE       50  // maximum number of active i-nodes
#define NDEV         10  // maximum major device number
#define ROOTDEV       1  // device number of file system root disk
#define MAXARG       32  // max exec arguments
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       1000  // size of file system in blocks
#define MAXCAPACITY  (NPROC+1) // max capacity of processes for mlfq, minheap
#define BIGNUM       10000 // for stride scheduling
#define NTHREAD      15  // maximum number of thread per process
#define HLEVEL        0  // high level
#define MLEVEL        1  // middle level
#define LLEVEL        2  // low level
#define HALLOT       20  // time allotment of high queue
#define MALLOT       40  // time allotment of middle queue
#define HQUANTUM      5  // time quantum of high queue
#define MQUANTUM     10  // time quantum of middle queue
#define LQUANTUM     20  // time quantum of low queue

