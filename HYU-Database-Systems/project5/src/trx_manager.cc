#include "trx_manager.h"
#include "bpt.h"

pthread_mutex_t trx_table_mutex = PTHREAD_MUTEX_INITIALIZER;
unordered_map<int, trx_t*> trx_table;
int trx_num = 0;


int begin_trx() {

	pthread_mutex_lock(&trx_table_mutex);

	trx_t* new_trx = new trx_t;
	new_trx->trx_id = trx_num;
	trx_num++;
	new_trx->state = IDLE;
	new_trx->trx_wait_lock = NULL;
	new_trx->trx_cond = PTHREAD_COND_INITIALIZER;

	trx_table.insert(make_pair(new_trx->trx_id, new_trx));	

	pthread_mutex_unlock(&trx_table_mutex);
	return new_trx->trx_id;
}

int end_trx(int trx_id) {
	
        pthread_mutex_lock(&trx_table_mutex);
	
	if(trx_table.find(trx_id) == trx_table.end()) {
		pthread_mutex_unlock(&trx_table_mutex);		
		return -1;
	}

	unordered_map<int, trx_t*>::iterator map_it = trx_table.find(trx_id);
	trx_t* target_trx = map_it->second;

	list<lock_t*>::iterator list_it;

	pthread_mutex_lock(&lock_table_mutex);

	for(list_it = target_trx->trx_locks.begin(); list_it != target_trx->trx_locks.end();) {
		list_it = unlock_record(list_it);
	}
	
	pthread_mutex_unlock(&lock_table_mutex);

	for(int i = 0; i < target_trx->history.size(); i++)
		free(target_trx->history[i]);

	trx_table.erase(map_it);
	delete target_trx;

        pthread_mutex_unlock(&trx_table_mutex);

	return 0;

}

int abort_trx(int trx_id) {
	pthread_mutex_lock(&trx_table_mutex);
	
	unordered_map<int, trx_t*>::iterator map_it = trx_table.find(trx_id);
	trx_t* target_trx = map_it->second;

	page_t* leaf_page = (page_t*)malloc(sizeof(page_t));
	pagenum_t leaf_page_num;
	int buffer_index;

	for(int i = target_trx->history.size()-1; i >= 0; ) {

		pthread_mutex_lock(&buffer_pool_mutex);		

		leaf_page_num = find_leaf(target_trx->history[i]->table_id, target_trx->history[i]->key);

		if(leaf_page_num == 0) {
			pthread_mutex_unlock(&buffer_pool_mutex);
			pthread_mutex_unlock(&trx_table_mutex);
			return -1;
		}

		buffer_index = buffer_read_page(target_trx->history[i]->table_id, leaf_page_num, leaf_page);
	
		if(buffer_page_trylock(buffer_index) != 0) {
			pthread_mutex_unlock(&buffer_pool_mutex);
			continue;	
		}

		pthread_mutex_unlock(&buffer_pool_mutex);
		
		int j = 0;
		for(j = 0; j < leaf_page->num_of_key; j++) {
			if(target_trx->history[i]->key == leaf_page->record[j].key)
				break;

		}

		if(j == leaf_page->num_of_key) {
			buffer_page_unlock(buffer_index);
			pthread_mutex_unlock(&trx_table_mutex);
			free(leaf_page);
			return -1;
		}
		else {
			memcpy(leaf_page->record[j].value, target_trx->history[i]->old_value, 120);

			pthread_mutex_lock(&buffer_pool_mutex);
			buffer_write_page(target_trx->history[i]->table_id, leaf_page_num, leaf_page);
			pthread_mutex_unlock(&buffer_pool_mutex);
	
			buffer_page_unlock(buffer_index);		
		}

		j--;
	}
	pthread_mutex_unlock(&trx_table_mutex);
	free(leaf_page);
	end_trx(trx_id);
}
