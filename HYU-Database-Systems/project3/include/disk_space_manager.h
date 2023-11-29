#ifndef __DISK_SPACE_MANAGER_H__
#define __DISK_SPACE_MANAGER_H__

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#define LEAF_ORDER 32
#define INTERNAL_ORDER 249
#define PAGE_SIZE 4096

typedef uint64_t pagenum_t;

extern int fd[11];

typedef struct record_t {

	int64_t key;
	char value[120];

} record_t;


typedef struct internal_t {

	int64_t key;
	pagenum_t page_num;

} internal_t;


// file_read_page()와 file_write_page()에서 PAGE_SIZE만큼
// 읽고 쓰기 위해 선언한 구조체인 header_page_t와 temp_page_t
 
typedef struct header_page_t {

	pagenum_t free_page_num;
	pagenum_t root_page_num;
	uint64_t num_of_page;
	char reserved[4072];

} header_page_t;


typedef struct node_page_t {

	pagenum_t parent_page_num;
	int32_t is_leaf;
	int32_t num_of_key;
	char reserved[104];

	union {
		pagenum_t right_sibling_num;
		pagenum_t left_page_num;
	};

	union {
		record_t record[LEAF_ORDER - 1];
		internal_t internal[INTERNAL_ORDER - 1];
	};

} node_page_t;


// in memory에서만 사용하는 구조체이므로 필요한 정보를 모두 담고
// file_read_page()와 file_write_page() 함수에서 경우에 맞게
// 디스크에 읽고 쓰면 된다는 아이디어를 정희석 선배님의 도움으로 얻었습니다.

typedef struct page_t {

	pagenum_t page_num;
	int table_id;
	
	pagenum_t free_page_num;
	pagenum_t root_page_num;
	uint64_t num_of_page;

	pagenum_t parent_page_num;
	int32_t is_leaf;
	int32_t num_of_key;

	union {
		pagenum_t right_sibling_num;
		pagenum_t left_page_num;
	};

	union {
		record_t record[LEAF_ORDER - 1];
		internal_t internal[INTERNAL_ORDER - 1];
	};
} page_t;


pagenum_t file_alloc_page(int table_id);

void file_free_page(int table_id, pagenum_t pagenum);

void file_read_page(int table_id, pagenum_t pagenum, page_t* dest);

void file_write_page(int table_id, pagenum_t pagenum, const page_t* src);

#endif
