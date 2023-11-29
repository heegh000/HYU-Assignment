#ifndef __BPT_H__
#define __BPT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unordered_map>
#include <list>
#include "disk_space_manager.h"
#include "buffer_manager.h"
#include "lock_manager.h"
#include "trx_manager.h"

using namespace std;


struct trx_t;
struct lock_t;
struct buffer_t;


extern char* pathname_arr[11];
// print 함수
void print_page(int table_id, pagenum_t page_num);
void print_buffer(int buffer_index);

int init_db(int num_buf);
int open_table(char *pathname);
int close_table(int table_id);
int shutdown_db();
int join_table(int table_id_1, int table_id_2, char * pathname);

//transaction 함수
int db_find(int table_id, int64_t key, char* ret_val, int trx_id);

int db_update(int table_id, int64_t key, char* values, int trx_id);

// find
pagenum_t find_leaf(int table_id, int64_t key);
int db_find(int table_id, int64_t key, char * ret_val);

int cut(int length);


// insert
record_t* make_record(int64_t key, char* value);

pagenum_t make_leaf_page(int table_id, page_t* new_page);
pagenum_t make_internal_page(int table_id, page_t* new_page);

int get_insertion_index(int table_id, pagenum_t parent_page_num, pagenum_t child_page_num);

int insert_into_leaf(int table_id, page_t* target_page, record_t* record);
int insert_into_leaf_after_splitting(int table_id, page_t* target_page, record_t* record);

int insert_into_internal(int table_id, page_t* target_page, int insertion_index, int64_t key, pagenum_t right_child_page_num);
int insert_into_internal_after_splitting(int table_id, page_t* target_page, int insertion_index, int64_t key, pagenum_t right_child_page_num);

int insert_into_parent(int table_id, page_t* left_child_page, int64_t key, pagenum_t right_child_page_num);


int insert_into_new_root(int table_id, page_t* left_child_page, int64_t key, page_t* right_child_page);
int start_new_tree(int table_id, record_t* record);

int db_insert(int table_id, int64_t key, char * value);


// delete
int get_neighbor_index(page_t* parent_page, pagenum_t target_page_num);

int remove_entry_from_page(int table_id, page_t* target_page, int64_t key);

int adjust_root(int table_id, page_t* root_page);

int coalesce_page(int table_id, page_t* target_page, page_t* neighbor_page, pagenum_t neighbor_index, int64_t k_prime);
int redistribute_page(int table_id, page_t* target_page, page_t* neighbor_page, int neighbor_index, int k_prime_index, int64_t k_prime);

int delete_entry(int table_id, page_t* target_page, int64_t key);
int db_delete(int table_id , int64_t key);

#endif
