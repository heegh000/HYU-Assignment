#ifndef __BUFFER_MANAGER_H__
#define __BUFFER_MANAGER_H__

#include "disk_space_manager.h"
#include <stdlib.h>


typedef struct meatadata_t {

	int table_id;
	pagenum_t page_num;
	int is_dirty;
	int is_pinned;
	int next;
	int prev;
	int is_used;

} metadata_t;

typedef struct buffer_t {

	union {
		node_page_t node_page;
		header_page_t header_page;
	};

	metadata_t metadata;

} buffer_t;

buffer_t* get_buffer_pointer(int table_id, pagenum_t pagenum);
int get_page_in_buffer(int table_id, pagenum_t pagenum);
int get_empty_buffer_index();
int get_buffer_index(int table_id, pagenum_t pagenum);
int get_LRU_index();
void arrange_LRU_list(int target_index);

void clean_metadata(int buffer_index);

void unpin(int buffer_index);

void start_table(int table_id, pagenum_t pagenum, const page_t* src);

void buffer_write_to_disk_table(int table_id);

pagenum_t buffer_alloc_page(int table_id);
void buffer_free_page(int table_id, pagenum_t pagenum);
int buffer_read_page(int table_id, pagenum_t pagenum, page_t* dest);
int buffer_write_page(int talbe_id ,pagenum_t pagenum, const page_t* src);

void disk_to_buffer(int buffer_index, int table_id, pagenum_t page_num);
void buffer_to_user(int buffer_index, page_t* dest);
void user_to_buffer(int buffer_index, const page_t* src);
void buffer_to_disk(int buffer_index);

#endif
