#ifndef __LOCK_MANAGER_H__
#define __LOCK_MANAGER_H__

#include <stdlib.h>
#include <pthread.h>
#include <list>
#include <vector>
#include <unordered_map>
#include "trx_manager.h"
#include "disk_space_manager.h"

using namespace std;

struct lock_t;
struct trx_t;


extern pthread_mutex_t lock_table_mutex;
extern unordered_map<int, list<lock_t*>> lock_table;


typedef enum lock_mode { SHARED, EXCLUSIVE } lock_mode;
typedef enum lock_state { RUN, WAIT, DEADLOCK } lock_state;

typedef struct lock_t {

	int table_id;
	int64_t key;
	pagenum_t page_num;
	int trx_id;
	enum lock_mode mode;
	enum lock_state state;
	trx_t* own_trx;
	lock_t* wait_lock;
	vector<lock_t*> next_locks;

} lock_t;

lock_state lock_record (int table_id, int64_t key, pagenum_t page_num, lock_mode mode, int trx_id);

list<lock_t*>::iterator unlock_record (list<lock_t*>::iterator target_it);

int detect_deadlock(int trx_id, int table_id, int64_t key, pagenum_t page_num, int same_line);

#endif
