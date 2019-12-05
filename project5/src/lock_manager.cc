#include "lock_manager.h"

pthread_mutex_t lock_table_mutex = PTHREAD_MUTEX_INITIALIZER;
unordered_map<int, list<lock_t*>> lock_table;


lock_state lock_record (int table_id, int64_t key, pagenum_t page_num, lock_mode mode, int trx_id) {
	

	pthread_mutex_lock(&lock_table_mutex);
	
	/*
	if(mode == SHARED)
		printf("Slock tid: %d\n", trx_id);
	else
		printf("Xlock tid: %d\n", trx_id);
	*/
	if(lock_table.find(page_num) == lock_table.end()) {
		lock_t* new_lock = new lock_t;

		new_lock->table_id = table_id;
		new_lock->key = key;
		new_lock->page_num = page_num;
		new_lock->trx_id = trx_id;
		new_lock->mode = mode;
		new_lock->state = RUN;
		new_lock->wait_lock = NULL;
		
		pthread_mutex_lock(&trx_table_mutex);

		unordered_map<int, trx_t*>::iterator map_it = trx_table.find(trx_id);
		trx_t* own_trx = map_it->second;
                new_lock->own_trx = own_trx;
                own_trx->trx_locks.push_back(new_lock);
		own_trx->trx_wait_lock = NULL;

                pthread_mutex_unlock(&trx_table_mutex);

		list<lock_t*> lock_list;
                lock_list.push_back(new_lock);
                lock_table.insert(make_pair(page_num, lock_list));

		pthread_mutex_unlock(&lock_table_mutex);		

		return new_lock->state;
	}


	else {

		unordered_map<int, list<lock_t*>>::iterator map_it = lock_table.find(page_num);
		list<lock_t*>::iterator list_it;

		lock_t* retry_lock = NULL;
		lock_t* last_ex_lock = NULL;
		int count = 0;
		int rem_count = 0;
		int same_obj_count = 0;
		int in_same_list = 0;
		int rem_sh_count = 0;
 		for(list_it = map_it->second.begin(); list_it != map_it->second.end(); list_it++) {
			if((*list_it)->table_id == table_id && (*list_it)->key == key) {
				count++;
				
				if((*list_it)->trx_id == trx_id) {
					same_obj_count++;
					retry_lock = (*list_it);					
					rem_count = count;
					if(retry_lock->mode != mode) {
						if(retry_lock->mode == EXCLUSIVE) {
							pthread_mutex_unlock(&lock_table_mutex);
							return retry_lock->state;	
						}
						
						else if(retry_lock->mode == SHARED) {
							in_same_list = 1;
							rem_sh_count = count;
						}
						
					}
				}
				
				else {
					if((*list_it)->mode == EXCLUSIVE)
						last_ex_lock = (*list_it);
				
				}
			}
		}


		if(count == 0) {
			lock_t* new_lock = new lock_t;
			new_lock->table_id = table_id;
			new_lock->key = key;
			new_lock->page_num = page_num;
			new_lock->trx_id = trx_id;
			new_lock->mode = mode;
			new_lock->state = RUN;
			new_lock->wait_lock = NULL;

			pthread_mutex_lock(&trx_table_mutex);
			unordered_map<int, trx_t*>::iterator map_trx_it = trx_table.find(trx_id);
			trx_t* own_trx = map_trx_it->second;
		        new_lock->own_trx = own_trx;
		        own_trx->trx_locks.push_back(new_lock);
			own_trx->trx_wait_lock = NULL;

		        pthread_mutex_unlock(&trx_table_mutex);


			map_it->second.push_back(new_lock);

			pthread_mutex_unlock(&lock_table_mutex);

                	return new_lock->state;
		}

		if(in_same_list) {

			//lock obj 생성
			if (same_obj_count == 1) {
	
				if (last_ex_lock) {
					pthread_mutex_unlock(&lock_table_mutex);
					return DEADLOCK;
				}

				if(rem_sh_count != 1 || count != 1) { 
					int is_deadlock = detect_deadlock(trx_id, table_id, key, page_num, 1);
					if(is_deadlock) {
						pthread_mutex_unlock(&lock_table_mutex);
	
						return DEADLOCK;
					}	
				}
							

				lock_t* new_lock = new lock_t;
				new_lock->table_id = table_id;
				new_lock->key = key;
				new_lock->page_num = page_num;
				new_lock->trx_id = trx_id;
				new_lock->mode = EXCLUSIVE;
					
				if(rem_sh_count == 1 && count == 1) 
					new_lock->state = RUN;
				else 
					new_lock->state = WAIT;

				pthread_mutex_lock(&trx_table_mutex);

		        	unordered_map<int, trx_t*>::iterator map_trx_it = trx_table.find(trx_id);
		        	trx_t* own_trx = map_trx_it->second;
		        	new_lock->own_trx = own_trx;
			        own_trx->trx_locks.push_back(new_lock);
				if(new_lock->state == WAIT)				
					own_trx->trx_wait_lock = new_lock;

				pthread_mutex_unlock(&trx_table_mutex);
			
				map_it->second.push_back(new_lock);

				pthread_mutex_unlock(&lock_table_mutex);
				return new_lock->state;
				
			}

			// 재시도
			else if(same_obj_count == 2) {
				if(rem_sh_count == 1 && rem_count == 2) 
					retry_lock->state = RUN;
				
				
				pthread_mutex_unlock(&lock_table_mutex);
				return retry_lock -> state;
			}

		}

		if(retry_lock) {
			if(retry_lock->mode == SHARED) {
				if(retry_lock->wait_lock == NULL) 
					retry_lock->state = RUN;
				

				
				pthread_mutex_unlock(&lock_table_mutex);
				return retry_lock->state;
			}

			else if(retry_lock->mode == EXCLUSIVE) {
				if(rem_count == 1)
					retry_lock->state = RUN;
				
		
				pthread_mutex_unlock(&lock_table_mutex);
                		return retry_lock->state;
			}
		}

		else {

			int is_deadlock = detect_deadlock(trx_id, table_id, key, page_num, 0);
	
			if(is_deadlock) {
				pthread_mutex_unlock(&lock_table_mutex);
				return DEADLOCK;
			}

			lock_t* new_lock = new lock_t;
			new_lock->table_id = table_id;
			new_lock->key = key;
			new_lock->page_num = page_num;
			new_lock->trx_id = trx_id;
			new_lock->mode = mode;

			if(mode == SHARED) {
				if(last_ex_lock == NULL) {
					new_lock->state = RUN;
					new_lock->wait_lock = NULL;
				}
				else {
					new_lock->state = WAIT;
					new_lock->wait_lock = last_ex_lock;
					last_ex_lock->next_locks.push_back(new_lock);
				}
			}
			else if(mode == EXCLUSIVE) {
				if(count != 0) {
					new_lock->state = WAIT;
					new_lock->wait_lock = NULL;
				}
			}
	

			pthread_mutex_lock(&trx_table_mutex);

	        	unordered_map<int, trx_t*>::iterator map_trx_it = trx_table.find(trx_id);
	        	trx_t* own_trx = map_trx_it->second;
	        	new_lock->own_trx = own_trx;
		        own_trx->trx_locks.push_back(new_lock);
			if(new_lock->state == WAIT)				
				own_trx->trx_wait_lock = new_lock;

			pthread_mutex_unlock(&trx_table_mutex);
	
			map_it->second.push_back(new_lock);
			
			pthread_mutex_unlock(&lock_table_mutex);
			return new_lock->state;
		}	
	}
	printf("dsadasd");
}

list<lock_t*>::iterator unlock_record (list<lock_t*>::iterator target_it) {

	unordered_map<int, list<lock_t*>>::iterator map_it = lock_table.find((*target_it)->page_num);
	map_it->second.remove((*target_it));

	vector<lock_t*>::iterator vec_it;
        lock_t* next_lock = NULL;

        for(vec_it = (*target_it)->next_locks.begin(); vec_it != (*target_it)->next_locks.end(); vec_it++) {
                next_lock = (*vec_it);
                next_lock->wait_lock = NULL;
        }

	list<lock_t*>::iterator list_it;
	trx_t* own_trx = NULL;
	
	for(list_it = map_it->second.begin(); list_it != map_it->second.end(); list_it++) {
		if((*list_it)->table_id == (*target_it)->table_id && (*list_it)->key == (*target_it)->key) {		
			own_trx = (*list_it)->own_trx;
			
			pthread_cond_signal(&own_trx->trx_cond);
			if((*list_it)->mode == EXCLUSIVE)
				break;
		}	
	}

	own_trx = (*target_it)->own_trx;
	
	return own_trx->trx_locks.erase(target_it);

	
}



int detect_deadlock(int trx_id, int table_id, int64_t key, pagenum_t page_num, int same_line) {
	
	pthread_mutex_lock(&trx_table_mutex);

	unordered_map<int, list<lock_t*>>::iterator map_it = lock_table.find(page_num);
	list<lock_t*>::reverse_iterator reverse_it;

	int is_deadlock = 0;

	if(same_line) {
		for(reverse_it = map_it->second.rbegin(); reverse_it != map_it->second.rend(); reverse_it++) {
			if((*reverse_it)->table_id == table_id && (*reverse_it)->key == key) {
				if((*reverse_it)->mode == EXCLUSIVE)
					is_deadlock = 1;

				if((*reverse_it)->own_trx->trx_wait_lock)
					if((*reverse_it)->own_trx->trx_wait_lock->page_num != page_num)
						is_deadlock = detect_deadlock(trx_id, table_id, key, (*reverse_it)->own_trx->trx_wait_lock->page_num, 0);
				if(is_deadlock);
					break;
				
			}	
		}
		
	}
	else {
		for(reverse_it = map_it->second.rbegin(); reverse_it != map_it->second.rend(); reverse_it++) {
			if((*reverse_it)->table_id == table_id && (*reverse_it)->key == key) {

				if((*reverse_it)->trx_id == trx_id) 
					is_deadlock = 1;

				if((*reverse_it)->own_trx->trx_wait_lock)
					if((*reverse_it)->own_trx->trx_wait_lock->page_num != page_num)
						is_deadlock = detect_deadlock(trx_id, table_id, key, (*reverse_it)->own_trx->trx_wait_lock->page_num, 0);

				if(is_deadlock)
					break;

			}
	

		}
	}

	
	pthread_mutex_unlock(&trx_table_mutex);

	return is_deadlock;
	
}




