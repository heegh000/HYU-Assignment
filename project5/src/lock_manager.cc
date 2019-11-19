#include "lock_manager.h"

pthread_mutex_t trx_id_lock;
int trx_num = 1;
vector<trx_t*> trx_list;

int begin_trx() {

	pthread_mutex_lock(&trx_id_lock);

	trx_t* new_trx = (trx_t*)malloc(sizeof(trx_t));

	new_trx->trx_id = trx_num;
	trx_num++;
	new_trx->state = IDLE;
	new_trx->wait_lock = NULL;

	trx_list.push_back(new_trx);

	pthread_mutex_unlock(&trx_id_lock);

	return 0;
}

int end_trx(int tid) {
	
        pthread_mutex_lock(&trx_id_lock);
	
	trx_t* target_trx = NULL;

	for(int i = 0; i < trx_list.size(); i++) {
		if(trx_list[i]->trx_id == tid) {
			target_trx = trx_list[i];
			trx_list.erase(trx_list.begin() + i);
		}
	}

	list<lock_t*>::iterator list_iter;
	
	for(list_iter = target_trx->trx_locks.begin(); list_iter != target_trx->trx_locks.end(); list_iter++) {
	//	unlock(*list_iter); 이후 LOCK_MANAGER에서 구현 예정
	}
	
	free(target_trx);

        pthread_mutex_unlock(&trx_id_lock);

	return 0;

}
