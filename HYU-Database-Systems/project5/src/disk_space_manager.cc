#include "disk_space_manager.h"


int fd_arr[11] = {0, };

/* free page list에서 free page의 페이지 번호를 반환해주는 함수
*/
pagenum_t file_alloc_page(int table_id) {

	int fd = fd_arr[table_id];

	pagenum_t free_page_num;
	pread(fd, &free_page_num, sizeof(pagenum_t), 0);

	// free page가 없는 경우
	if (free_page_num == 0) {
		uint64_t num_of_page;
		pread(fd, &num_of_page, sizeof(uint64_t), 16);

		page_t free_page;

		free_page.is_leaf = 1;
		free_page.num_of_key = 0;
		free_page.right_sibling_num = 0;
		
		//100개씩 free page 추가
		for (int i = 0; i < 99; i++) {
			free_page.parent_page_num = num_of_page + 1 + i;
			file_write_page(table_id, num_of_page + i, &free_page);
		}
		
		free_page.parent_page_num = 0;
		file_write_page(table_id, num_of_page + 99, &free_page);


		//헤더 페이지 수정정
		pwrite(fd, &num_of_page, sizeof(pagenum_t), 0);

		num_of_page += 100;
		pwrite(fd, &num_of_page, sizeof(uint64_t), 16);

		pagenum_t first_free_page_num = num_of_page - 100;

		pagenum_t new_free_page_num;
                pread(fd, &new_free_page_num, sizeof(pagenum_t), first_free_page_num * PAGE_SIZE);
                pwrite(fd, &new_free_page_num, sizeof(pagenum_t), 0);

		return first_free_page_num;
	}

	// free page가 있는 경우
	else {
		pagenum_t new_free_page_num;
		pread(fd, &new_free_page_num, sizeof(pagenum_t), free_page_num * PAGE_SIZE);
		pwrite(fd, &new_free_page_num, sizeof(pagenum_t), 0);
		return free_page_num;
	}
}


/* 인자로 받은 페이지 번호에 해당하는 페이지를 free page list에 넣어주는 함수
*/

void file_free_page(int table_id, pagenum_t pagenum) {

	int fd = fd_arr[table_id];

	pagenum_t header_free_page_num;
	pread(fd, &header_free_page_num, sizeof(pagenum_t), 0);

	pwrite(fd, &pagenum, sizeof(pagenum_t), 0);
	pwrite(fd, &header_free_page_num, sizeof(pagenum_t), pagenum * PAGE_SIZE);

}

void file_read_page(int table_id, pagenum_t pagenum, page_t* dest) {

	int fd = fd_arr[table_id];

	dest->page_num = pagenum;
	dest->table_id = table_id;

	// 읽는 페이지가 헤더인 경우
	if (pagenum == 0) {
		
		//크기가 4096인 header_page를 선언하여 읽어오고 필요한 정보를 dest에 담음
		header_page_t header_page;
	
		pread(fd, &header_page, PAGE_SIZE, pagenum * PAGE_SIZE);

		dest->free_page_num = header_page.free_page_num;
		dest->root_page_num = header_page.root_page_num;
		dest->num_of_page = header_page.num_of_page;
		return;
	}

	//크기가 4096인 temp_page를 선언하여 읽어오고 필요한 정보를 dest에 담음
	node_page_t temp_page;

	pread(fd, &temp_page, PAGE_SIZE, pagenum * PAGE_SIZE);

	dest->parent_page_num = temp_page.parent_page_num;
	dest->is_leaf = temp_page.is_leaf;
	dest->num_of_key = temp_page.num_of_key;


	// 읽는 페이지가 leaf인 경우
	if (temp_page.is_leaf) {
		dest->right_sibling_num = temp_page.right_sibling_num;
		for (int i = 0; i < temp_page.num_of_key; i++) {
			memcpy(&dest->record[i], &temp_page.record[i], sizeof(record_t));
		}

	}

	// 읽는 페이지가 internal인 경우
	else {
		dest->left_page_num = temp_page.left_page_num;;
		for (int i = 0; i < temp_page.num_of_key; i++) {
			memcpy(&dest->internal[i], &temp_page.internal[i], sizeof(internal_t));
		}

	}

}

void file_write_page(int table_id, pagenum_t pagenum, const page_t* src) {

	int fd = fd_arr[table_id];

	// 쓰는 페이지가 헤더인 경우
	if (pagenum == 0) {
		
		//크기가 4096인 header_page에 필요한 정보를 src로부터 가져와 임시로 담고, disk에 써줌
		header_page_t header_page;
		memset(&header_page, 0, sizeof(header_page));
		header_page.free_page_num = src->free_page_num;
		header_page.root_page_num = src->root_page_num;
		header_page.num_of_page = src->num_of_page;

		pwrite(fd, &header_page, PAGE_SIZE, pagenum * PAGE_SIZE);
		return;
	}


	//크기가 4096인 temp_page에 필요한 정보를 src로부터 가져와 임시로 담고, disk에 써줌
	node_page_t temp_page;
	memset(&temp_page, 0, PAGE_SIZE);
	temp_page.parent_page_num = src->parent_page_num;
	temp_page.is_leaf = src->is_leaf;
	temp_page.num_of_key = src->num_of_key;

	// 쓰는 페이지가 leaf인 경우
	if (temp_page.is_leaf) {
		temp_page.right_sibling_num = src->right_sibling_num;
		for (int i = 0; i < temp_page.num_of_key; i++) {
			memcpy(&temp_page.record[i], &src->record[i], sizeof(record_t));
		}
	}

	// 쓰는 페이지가 internal인 경우
	else {
		temp_page.left_page_num = src->left_page_num;
		for (int i = 0; i < temp_page.num_of_key; i++) {
			memcpy(&temp_page.internal[i], &src->internal[i], sizeof(internal_t));
		}

	}

	pwrite(fd, &temp_page, PAGE_SIZE, pagenum * PAGE_SIZE);
}
