#ifndef __TRX_MANAGER_H__
#define __TRX_MANAGER_H__

#include <list>
#include <unordered_map>
#include <utility>
#include <vector>
#include "lock_manager.h"
#include "disk_space_manager.h"
#include "buffer_manager.h"


using namespace std;

struct lock_t;
struct trx_t;


extern pthread_mutex_t trx_table_mutex;
extern unordered_map<int, trx_t*> trx_table;
extern int trx_num; 

typedef enum trx_state { IDLE, RUNNING, WAITING } trx_state;

typedef struct log_t {
	pagenum_t page_num;
	int table_id;
	int64_t key;
	char old_value[120];
} log_t;

typedef struct trx_t {

        int trx_id;
        enum trx_state state;
        list<struct lock_t*> trx_locks;
 	pthread_cond_t trx_cond;
	lock_t* trx_wait_lock;
	vector<struct log_t*> history;
	
} trx_t;

int begin_trx();
int end_trx(int trx_id);
int abort_trx(int trx_id);

#endif
