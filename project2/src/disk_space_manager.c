#include "disk_space_manager.h"

int fd;


/* free page list에서 free page의 페이지 번호를 반환해주는 함수
*/
pagenum_t file_alloc_page() {

	pagenum_t free_page_num;
	pread(fd, &free_page_num, sizeof(pagenum_t), 0);

	// free page가 없는 경우
	if (free_page_num == 0) {
		int64_t num_of_page;
		pread(fd, &num_of_page, sizeof(uint64_t), 16);

		page_t free_page;
		free_page.is_leaf = 1;
		free_page.num_of_key = 0;

		//100개씩 free page 추가
		for (int i = 0; i < 99; i++) {
			free_page.parent_page_num = num_of_page + 1 + i;
			file_write_page(num_of_page + i, &free_page);
		}
		
		free_page.parent_page_num = 0;
		file_write_page(num_of_page + 99, &free_page);


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

void file_free_page(pagenum_t pagenum) {

	pagenum_t header_free_page_num;
	pread(fd, &header_free_page_num, sizeof(pagenum_t), 0);

	pwrite(fd, &pagenum, sizeof(pagenum_t), 0);
	pwrite(fd, &header_free_page_num, sizeof(pagenum_t), pagenum * PAGE_SIZE);

}

void file_read_page(pagenum_t pagenum, page_t* dest) {

	dest->page_num = pagenum;

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
	temp_page_t temp_page;

	pread(fd, &temp_page, PAGE_SIZE, pagenum * PAGE_SIZE);

	dest->parent_page_num = temp_page.parent_page_num;
	dest->is_leaf = temp_page.is_leaf;
	dest->num_of_key = temp_page.num_of_key;


	// 읽는 페이지가 leaf인 경우
	if (temp_page.is_leaf) {
		dest->right_sibling_num = temp_page.right_sibling_num;
		for (int i = 0; i < temp_page.num_of_key; i++) {
			dest->record[i].key = temp_page.record[i].key;
			strcpy(dest->record[i].value, temp_page.record[i].value);
		}

	}

	// 읽는 페이지가 internal인 경우
	else {
		dest->left_page_num = temp_page.left_page_num;;
		for (int i = 0; i < temp_page.num_of_key; i++) {
			dest->internal[i].key = temp_page.internal[i].key;
			dest->internal[i].page_num = temp_page.internal[i].page_num;
		}

	}

}

void file_write_page(pagenum_t pagenum, const page_t* src) {

	// 쓰는 페이지가 헤더인 경우
	if (pagenum == 0) {
		
		//크기가 4096인 header_page에 필요한 정보를 src로부터 가져와 임시로 담고, disk에 써줌
		header_page_t header_page = {0, };
		memset(&header_page, 0, sizeof(header_page));
		header_page.free_page_num = src->free_page_num;
		header_page.root_page_num = src->root_page_num;
		header_page.num_of_page = src->num_of_page;

		pwrite(fd, &header_page, PAGE_SIZE, pagenum * PAGE_SIZE);
		return;
	}


	//크기가 4096인 temp_page에 필요한 정보를 src로부터 가져와 임시로 담고, disk에 써줌
	temp_page_t temp_page = {0, };
	memset(&temp_page, 0, sizeof(temp_page));
	temp_page.parent_page_num = src->parent_page_num;
	temp_page.is_leaf = src->is_leaf;
	temp_page.num_of_key = src->num_of_key;

	// 쓰는 페이지가 leaf인 경우
	if (temp_page.is_leaf) {
		temp_page.right_sibling_num = src->right_sibling_num;
		for (int i = 0; i < temp_page.num_of_key; i++) {
			temp_page.record[i].key = src->record[i].key;
			strcpy(temp_page.record[i].value, src->record[i].value);
		}
	}

	// 쓰는 페이지가 internal인 경우
	else {
		temp_page.left_page_num = src->left_page_num;
		for (int i = 0; i < temp_page.num_of_key; i++) {
			temp_page.internal[i].key = src->internal[i].key;
			temp_page.internal[i].page_num = src->internal[i].page_num;
		}

	}

	pwrite(fd, &temp_page, PAGE_SIZE, pagenum * PAGE_SIZE);
}
