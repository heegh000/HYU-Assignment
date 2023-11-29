#include "buffer_manager.h"

int head = -1;
int tail = -1;

buffer_t* buffer = NULL;
int num_of_buffer = 0; 
pthread_mutex_t buffer_pool_mutex = PTHREAD_MUTEX_INITIALIZER;


void buffer_page_lock(int buffer_index) {
	pthread_mutex_lock(&(buffer[buffer_index].buffer_page_mutex));
}

int buffer_page_trylock(int buffer_index) {
	return pthread_mutex_trylock(&(buffer[buffer_index].buffer_page_mutex));
}

void buffer_page_unlock(int buffer_index) {
	pthread_mutex_unlock(&(buffer[buffer_index].buffer_page_mutex));
}


buffer_t* get_buffer_pointer(int table_id, pagenum_t pagenum) {

	int buffer_index = get_buffer_index(table_id, pagenum);

	if(!buffer[buffer_index].metadata.is_used)
		disk_to_buffer(buffer_index, table_id, pagenum);

	buffer[buffer_index].metadata.is_pinned++;	

	arrange_LRU_list(buffer_index);

	return &buffer[buffer_index];
}

// 버퍼안에 찾는 페이지가 있으면 buffer idex, 없으면 비어있는 버퍼 인덱스, 그거마저 없으면 -1  반환
int get_page_in_buffer(int table_id, pagenum_t pagenum) {
	int index = -1;
	int empty_index = -1;

	for (int i = 0; i < num_of_buffer; i++) {
		if (buffer[i].metadata.is_used && buffer[i].metadata.table_id == table_id && buffer[i].metadata.page_num == pagenum) {
			index = i;
			break;		
		}

		if(!buffer[i].metadata.is_used && empty_index == -1)
			empty_index = i;
	}	

	if(index == -1)
		index = empty_index;

	
	return index;
}

// 비어있는 버퍼가 있으면 buffer index, 없으면 -1 반환
int get_empty_buffer_index() {
	for (int i = 0; i < num_of_buffer; i++)
		if (!buffer[i].metadata.is_used)
			return i;

	return -1;
}

int get_buffer_index(int table_id, pagenum_t pagenum) {
	int buffer_index = get_page_in_buffer(table_id, pagenum);

	// 버퍼가 꽉차있는 경우
	if(buffer_index == -1) {
		buffer_index = get_LRU_index();
			
		//LRU 버퍼가 더티 페이지인 경우
		if(buffer[buffer_index].metadata.is_dirty)
			buffer_to_disk(buffer_index);

		buffer[buffer_index].metadata.is_used = 0;
	}

	return buffer_index;
}

int get_LRU_index() {
	int LRU_index = head;

	while (buffer[LRU_index].metadata.is_pinned)
		LRU_index = buffer[LRU_index].metadata.next;

	return LRU_index;
}


void arrange_LRU_list(int target_index) {

	int next_index;
	int prev_index;

	// LRU 리스트에 아무것도 없는 경우
	if(head == -1 && tail == -1) {
		buffer[target_index].metadata.next = target_index;
		buffer[target_index].metadata.prev = target_index;
		head = target_index;
		tail = target_index;
		return;
	}

	// 새로 들어온 버퍼
	if(buffer[target_index].metadata.next == -1 && buffer[target_index].metadata.prev == -1) {
		buffer[tail].metadata.next = target_index;
		buffer[target_index].metadata.next = target_index;
		buffer[target_index].metadata.prev = tail;
		tail = target_index;
		return;
	}

	// 가장 최근에 접근한 버퍼에 또 접근한 경우	
	if (tail == target_index)
		return;

	
	// 가장 최근에 접근하지않은 버퍼에 접근한 경우
	if (head == target_index) {
		next_index = buffer[target_index].metadata.next;
		buffer[next_index].metadata.prev = next_index;
		head = next_index;
	}

	// 중간에 있는 버퍼에 접근한 경우
	else {

		next_index = buffer[target_index].metadata.next;
		prev_index = buffer[target_index].metadata.prev;

		buffer[next_index].metadata.prev = prev_index;
		buffer[prev_index].metadata.next = next_index;
	}

	buffer[tail].metadata.next = target_index;
	buffer[target_index].metadata.next = target_index;
	buffer[target_index].metadata.prev = tail;
	tail = target_index;
}

void clean_metadata(int buffer_index) {
	buffer[buffer_index].metadata.table_id = 0;
	buffer[buffer_index].metadata.page_num = 0;
	buffer[buffer_index].metadata.is_dirty = 0;
	buffer[buffer_index].metadata.is_pinned = 0;
	buffer[buffer_index].metadata.next = -1;
	buffer[buffer_index].metadata.prev = -1;
	buffer[buffer_index].metadata.is_used = 0;

}

void unpin(int buffer_index) {
	buffer[buffer_index].metadata.is_pinned--;
}

void start_table(int table_id, pagenum_t pagenum, const page_t* src) {

	int buffer_index = get_empty_buffer_index();

	// 버퍼가 꽉차있는 경우
	if(buffer_index == -1) {
		buffer_index = get_LRU_index();

		//LRU 버퍼가 더티 페이지인 경우
		if(buffer[buffer_index].metadata.is_dirty)
			buffer_to_disk(buffer_index);

		buffer[buffer_index].metadata.is_used = 0;
	}

	user_to_buffer(buffer_index, src);
	
	buffer_to_disk(buffer_index);

	buffer[buffer_index].metadata.table_id = table_id;
	buffer[buffer_index].metadata.page_num = pagenum;
	buffer[buffer_index].metadata.is_dirty = 0;
	buffer[buffer_index].metadata.is_pinned = 0;
	buffer[buffer_index].metadata.is_used = 1;

	arrange_LRU_list(buffer_index);
	
}

void buffer_write_to_disk_table(int table_id) {

	for(int i = 0; i < num_of_buffer; i++) {
		if(buffer[i].metadata.table_id == table_id) {

			if(buffer[i].metadata.is_dirty)
				buffer_to_disk(i);
			else {
				buffer[i].metadata.table_id = 0;
				buffer[i].metadata.page_num = 0;
				buffer[i].metadata.is_pinned = 0;
				buffer[i].metadata.is_used = 0;
			}
			arrange_LRU_list(i);
		}
	}


}



pagenum_t buffer_alloc_page(int table_id) {

	int header_buffer_index = get_buffer_index(table_id, 0);

	if(!buffer[header_buffer_index].metadata.is_used)
		disk_to_buffer(header_buffer_index, table_id, 0);

	pagenum_t new_free_page_num = buffer[header_buffer_index].header_page.free_page_num;

	int free_buffer_index;

	// 새로운 free page를 만들어야되는 경우
	if(new_free_page_num == 0) {
		new_free_page_num = buffer[header_buffer_index].header_page.num_of_page;
		
		buffer[header_buffer_index].header_page.free_page_num = buffer[header_buffer_index].header_page.num_of_page + 1;
		buffer[header_buffer_index].header_page.num_of_page = buffer[header_buffer_index].header_page.num_of_page + 100;

		free_buffer_index = get_buffer_index(table_id, new_free_page_num);

		buffer[free_buffer_index].metadata.table_id = table_id;
		buffer[free_buffer_index].metadata.page_num = new_free_page_num;
		buffer[free_buffer_index].metadata.is_dirty = 0;
		buffer[free_buffer_index].metadata.is_used = 1;

		file_alloc_page(table_id);
	}

	// 기존에 있던 free page를 가지고 오는 경우
	else {
		free_buffer_index = get_buffer_index(table_id, new_free_page_num);

		disk_to_buffer(free_buffer_index, table_id, new_free_page_num);

		buffer[header_buffer_index].header_page.free_page_num = buffer[free_buffer_index].node_page.parent_page_num;

		file_alloc_page(table_id);
	}

	arrange_LRU_list(header_buffer_index);
	arrange_LRU_list(free_buffer_index);

	return new_free_page_num;
}

void buffer_free_page(int table_id, pagenum_t pagenum) {
	
	int header_buffer_index = get_buffer_index(table_id, 0);

	if(!buffer[header_buffer_index].metadata.is_used)
		disk_to_buffer(header_buffer_index, table_id, 0);

	int free_buffer_index = get_buffer_index(table_id, pagenum);
	
	if(!buffer[free_buffer_index].metadata.is_used)
		disk_to_buffer(free_buffer_index, table_id, pagenum);

	
	int old_free_page_num = buffer[header_buffer_index].header_page.free_page_num;

	buffer[header_buffer_index].header_page.free_page_num = pagenum;
	buffer[free_buffer_index].node_page.parent_page_num = old_free_page_num;

	file_free_page(table_id, pagenum);

	arrange_LRU_list(header_buffer_index);
	arrange_LRU_list(free_buffer_index);	
}


int buffer_read_page(int table_id, pagenum_t pagenum, page_t* dest) {

	int buffer_index = get_buffer_index(table_id, pagenum);

	if(!buffer[buffer_index].metadata.is_used)
		disk_to_buffer(buffer_index, table_id, pagenum);

	buffer_to_user(buffer_index, dest);

	buffer[buffer_index].metadata.is_pinned++;

	arrange_LRU_list(buffer_index);

	return buffer_index;
}

int buffer_write_page(int table_id, pagenum_t pagenum, const page_t* src) {

	int buffer_index = get_buffer_index(table_id, pagenum);

	user_to_buffer(buffer_index, src);

	arrange_LRU_list(buffer_index);

	return buffer_index;
}

void disk_to_buffer(int buffer_index, int table_id, pagenum_t page_num) {

	page_t* temp_page = (page_t*)malloc(sizeof(page_t));
	file_read_page(table_id, page_num, temp_page);

	memset(&buffer[buffer_index], 0, PAGE_SIZE);

	//디스크에서 버퍼로 옮기는 페이지가 헤더페이지인 경우
	if (temp_page->page_num == 0) {
		buffer[buffer_index].header_page.free_page_num = temp_page->free_page_num;
		buffer[buffer_index].header_page.root_page_num = temp_page->root_page_num;
		buffer[buffer_index].header_page.num_of_page = temp_page->num_of_page;
	}

	//디스크에서 버퍼로 옮기는 페이지가 리프나 인터널인 경우
	else {
		buffer[buffer_index].node_page.parent_page_num = temp_page->parent_page_num;
		buffer[buffer_index].node_page.is_leaf = temp_page->is_leaf;
		buffer[buffer_index].node_page.num_of_key = temp_page->num_of_key;

		//리프인 경우
		if (temp_page->is_leaf) {
			buffer[buffer_index].node_page.right_sibling_num = temp_page->right_sibling_num;
			for (int i = 0; i < temp_page->num_of_key; i++) {
				memcpy(&buffer[buffer_index].node_page.record[i], &temp_page->record[i], sizeof(record_t));
			}
		}
		//인터널인 경우
		else {
			buffer[buffer_index].node_page.left_page_num = temp_page->left_page_num;
			for (int i = 0; i < temp_page->num_of_key; i++) {
				memcpy(&buffer[buffer_index].node_page.internal[i], &temp_page->internal[i], sizeof(internal_t));
			}
		}
	}
	
	buffer[buffer_index].metadata.table_id = table_id;
	buffer[buffer_index].metadata.page_num = page_num;
	buffer[buffer_index].metadata.is_dirty = 0;
	buffer[buffer_index].metadata.is_pinned = 0;
	buffer[buffer_index].metadata.is_used = 1;

	free(temp_page);
}

void buffer_to_user(int buffer_index, page_t* dest) {

	dest->page_num = buffer[buffer_index].metadata.page_num;
	dest->table_id = buffer[buffer_index].metadata.table_id;
	
	//버퍼에서 읽어올 페이지가 헤더인 경우
	if (buffer[buffer_index].metadata.page_num == 0) {
		dest->free_page_num = buffer[buffer_index].header_page.free_page_num;
		dest->root_page_num = buffer[buffer_index].header_page.root_page_num;
		dest->num_of_page = buffer[buffer_index].header_page.num_of_page;
	}

	//버퍼에서 읽어올 페이지가 리프나 인터널인 경우
	else {
		dest->parent_page_num = buffer[buffer_index].node_page.parent_page_num;
		dest->is_leaf = buffer[buffer_index].node_page.is_leaf;
		dest->num_of_key = buffer[buffer_index].node_page.num_of_key;

		if (buffer[buffer_index].node_page.is_leaf) {

			dest->right_sibling_num = buffer[buffer_index].node_page.right_sibling_num;
			for (int i = 0; i < buffer[buffer_index].node_page.num_of_key; i++) {
				memcpy(&dest->record[i], &buffer[buffer_index].node_page.record[i], sizeof(record_t));
			}
		}
		else {
			dest->left_page_num = buffer[buffer_index].node_page.left_page_num;
			for (int i = 0; i < buffer[buffer_index].node_page.num_of_key; i++) {
				memcpy(&dest->internal[i], &buffer[buffer_index].node_page.internal[i], sizeof(internal_t));
			}
		}
	}
}

void user_to_buffer(int buffer_index, const page_t* src) {

	memset(&buffer[buffer_index], 0, PAGE_SIZE);

	//버퍼에 쓸 페이지가 헤더 페이지인 경우
	if (src->page_num == 0) {
		buffer[buffer_index].header_page.free_page_num = src->free_page_num;
		buffer[buffer_index].header_page.root_page_num = src->root_page_num;
		buffer[buffer_index].header_page.num_of_page = src->num_of_page;
	}

	//버퍼에 쓸 페이지가 리프나 인터널인 경우
	else {
		buffer[buffer_index].node_page.parent_page_num = src->parent_page_num;
		buffer[buffer_index].node_page.is_leaf = src->is_leaf;
		buffer[buffer_index].node_page.num_of_key = src->num_of_key;
	
		// 리프인 경우
		if (src->is_leaf) {
			buffer[buffer_index].node_page.right_sibling_num = src->right_sibling_num;
			for (int i = 0; i < src->num_of_key; i++) {
				memcpy(&buffer[buffer_index].node_page.record[i], &src->record[i], sizeof(record_t));
			}
		}
		
		// 인터널인 경우
		else {
			buffer[buffer_index].node_page.left_page_num = src->left_page_num;
			for (int i = 0; i < src->num_of_key; i++) {
				memcpy(&buffer[buffer_index].node_page.internal[i], &src->internal[i], sizeof(internal_t));
			}
		}
	}
	
	buffer[buffer_index].metadata.table_id = src->table_id;
	buffer[buffer_index].metadata.page_num = src->page_num;
	buffer[buffer_index].metadata.is_dirty = 1;
	buffer[buffer_index].metadata.is_used = 1;
}

void buffer_to_disk(int buffer_index) {

	page_t* temp_page = (page_t*)malloc(sizeof(page_t));
	memset(temp_page, 0, sizeof(page_t));

	// 디스크에 쓸 페이지가 헤더페이지인 경우
	if (buffer[buffer_index].metadata.page_num == 0) {
		temp_page->free_page_num = buffer[buffer_index].header_page.free_page_num;
		temp_page->root_page_num = buffer[buffer_index].header_page.root_page_num;
		temp_page->num_of_page = buffer[buffer_index].header_page.num_of_page;
	}


	// 디스크에 쓸 페이지가 리프나 인터널인 경우
	else {
		temp_page->parent_page_num = buffer[buffer_index].node_page.parent_page_num;
		temp_page->is_leaf = buffer[buffer_index].node_page.is_leaf;
		temp_page->num_of_key = buffer[buffer_index].node_page.num_of_key;

		// 리프인 경우
		if (buffer[buffer_index].node_page.is_leaf) {
			temp_page->right_sibling_num = buffer[buffer_index].node_page.right_sibling_num;
			for (int i = 0; i < buffer[buffer_index].node_page.num_of_key; i++) {
				memcpy(&temp_page->record[i], &buffer[buffer_index].node_page.record[i], sizeof(record_t));
			}
		}
		
		// 인터널인 경우
		else {
			temp_page->left_page_num = buffer[buffer_index].node_page.left_page_num;
			for (int i = 0; i < buffer[buffer_index].node_page.num_of_key; i++) {
				memcpy(&temp_page->internal[i], &buffer[buffer_index].node_page.internal[i], sizeof(internal_t));
			}
		}
	}
	
	file_write_page(buffer[buffer_index].metadata.table_id, buffer[buffer_index].metadata.page_num, temp_page);

	buffer[buffer_index].metadata.table_id = 0;
	buffer[buffer_index].metadata.page_num = 0;
	buffer[buffer_index].metadata.is_dirty = 0;
	buffer[buffer_index].metadata.is_pinned = 0;
	buffer[buffer_index].metadata.is_used = 0;

	free(temp_page);

}
