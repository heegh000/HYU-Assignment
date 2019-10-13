#ifndef __BPT_H__
#define __BPT_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "disk_space_manager.h"

// 파일 디스크립터
extern int fd;


// print 함수
void print_page(pagenum_t page_num);


// open 
int open_table(char *pathname);


// find
pagenum_t find_leaf(int64_t key);
int db_find(int64_t key, char * ret_val);

int cut(int length);


// insert
record_t* make_record(int64_t key, char* value);

pagenum_t make_leaf_page(page_t* new_page);
pagenum_t make_internal_page(page_t* new_page);

int get_insertion_index(pagenum_t parent_page_num, pagenum_t child_page_num);

int insert_into_leaf(page_t* target_page, record_t* record);
int insert_into_leaf_after_splitting(page_t* target_page, record_t* record);

int insert_into_internal(page_t* target_page, int insertion_index, int64_t key, pagenum_t right_child_page_num);
int insert_into_internal_after_splitting(page_t* target_page, int insertion_index, int64_t key, pagenum_t right_child_page_num);

int insert_into_parent(page_t* left_child_page, int64_t key, pagenum_t right_child_page_num);

int insert_into_new_root(page_t* left_child_page, int64_t key, page_t* right_child_page);
int start_new_tree(record_t* record);

int db_insert(int64_t key, char * value);


// delete
int get_neighbor_index(page_t* parent_page, pagenum_t target_page_num);

int remove_entry_from_page(page_t* target_page, int64_t key);

int coalesce_page(page_t* target_page, page_t* neighbor_page, pagenum_t neighbor_index, int64_t k_prime);
int redistribute_page(page_t* target_page, page_t* neighbor_page, int neighbor_index, int k_prime_index, int64_t k_prime);

int delete_entry(page_t* target_page, int64_t key);
int db_delete(int64_t key);

#endif
