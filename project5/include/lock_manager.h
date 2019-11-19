#ifndef __TRX_MANAGER_H__
#define __TRX_MANAGER_H__

#include <stdlib.h>
#include <list>
#include <pthread.h>
#include <vector>


using namespace std;

enum trx_state { IDLE, RUNNING, WAITING };

enum lock_mode { SHARED, EXCLUSIVE};
enum lock_state { GO, WAIT, ABORT };

typedef struct trx_t {

	int trx_id;
	enum trx_state state;
	list<struct lock_t*> trx_locks;
	int num_of_access;
	struct lock_t* wait_lock;

} trx_t;


typedef struct lock_t {

	int table_id;
	int record_id;
	enum lock_mode mode;
	enum lock_state state;
	trx_t* trx;
	lock_t* next_lock;
	lock_t* prev_lock;
} lock_t;


int begin_trx();
int end_trx(int tid);

#endif


