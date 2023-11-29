#include "bpt.h"

int fd_arr[11] = {0,};
buffer_t* buffer;
int num_of_buffer;
char* pathname_arr[11];



void print_buffer(int buffer_index) {

	if(buffer[buffer_index].metadata.is_used == 0) 
		return;

	printf("==============\n");
	printf("buffer %d, table id: %d, page num: %ld\n", buffer_index, buffer[buffer_index].metadata.table_id, buffer[buffer_index].metadata.page_num);

	printf("is pinned: %d, is used: %d\n", buffer[buffer_index].metadata.is_pinned, buffer[buffer_index].metadata.is_used);

	if(buffer[buffer_index].metadata.page_num == 0) {
		printf("free page num: %ld\n", buffer[buffer_index].header_page.free_page_num);
		printf("root page num: %ld\n", buffer[buffer_index].header_page.root_page_num);
		printf("num of page: %ld\n", buffer[buffer_index].header_page.num_of_page);
	}

	else {
        	printf("pagenum: %ld\n", buffer[buffer_index].metadata.page_num);
        	printf("num_of_key: %d\n",  buffer[buffer_index].node_page.num_of_key);
        	if (buffer[buffer_index].node_page.is_leaf) {
                	for (int i = 0; i <  buffer[buffer_index].node_page.num_of_key; i++)
                        	printf("%ld |",  buffer[buffer_index].node_page.record[i].key);
                	printf("sibling num: %ld\n",  buffer[buffer_index].node_page.right_sibling_num);

        	}


        	else {
               	 	printf("key: ");
                	for (int i = 0; i <  buffer[buffer_index].node_page.num_of_key; i++) {
                        	printf("%ld |",  buffer[buffer_index].node_page.internal[i].key);
                	}	

                	printf("\npage_num: %ld |",  buffer[buffer_index].node_page.left_page_num);
                	for (int i = 0; i <  buffer[buffer_index].node_page.num_of_key; i++) {
                        	printf("%ld |",  buffer[buffer_index].node_page.internal[i].page_num);
                	}
        	}	
	
        	printf("\nparentpage: %ld\n",  buffer[buffer_index].node_page.parent_page_num);
	        printf("===============\n");
	}

	

}

void print_page(int table_id, pagenum_t page_num) {
	page_t* page = (page_t*)malloc(sizeof(page_t));
	
	file_read_page(table_id, page_num, page);


	if (page_num == 0) {
		printf("free_page_num: %ld\n", page->free_page_num);
		printf("root_page_num: %ld\n", page->root_page_num);
		printf("num_of_page: %ld\n", page->num_of_page);
		return;
	}




	printf("==============\n");
	printf("pagenum: %ld\n", page->page_num);
	printf("num_of_key: %d\n", page->num_of_key);
	if (page->is_leaf) {
		for (int i = 0; i < page->num_of_key; i++)
			printf("%ld |", page->record[i].key);
		printf("sibling num: %ld\n", page->right_sibling_num);

	}


	else {
		printf("key: ");
		for (int i = 0; i < page->num_of_key; i++) {
			printf("%ld |", page->internal[i].key);
		}

		printf("\npage_num: %ld |:", page->left_page_num);
		for (int i = 0; i < page->num_of_key; i++) {
			printf("%ld |", page->internal[i].page_num);
		}
	}

	printf("\nparentpage: %ld\n", page->parent_page_num);
	printf("===============\n");
	free(page);
}


// DB를 시작하는 함수, 성공하면 0, 실패하면 -1 반환
int init_db(int num_buf) {
	num_of_buffer = num_buf;
	buffer = (buffer_t*)malloc(sizeof(buffer_t) * num_buf);

	for (int i = 0; i < num_buf; i++) {
		memset(&buffer[i], 0, sizeof(buffer) * num_buf);
		clean_metadata(i);
	}
}



// 데이터 파일을 여는 함수
// 성공하면 유니크한 아이디 반환, 실패시 음수 반환
int open_table(char *pathname) {

	int table_id = -1;
	int first_empty_id = -1;

	for (int i = 1; i < 11; i++) {

		if (pathname_arr[i] == 0 && first_empty_id == -1) 
			first_empty_id = i;

		if (pathname_arr[i] != 0) {
			if (strcmp(pathname_arr[i], pathname) == 0) {
				table_id = i;
				break;
			}
		}
	}

	if (table_id == -1)
		table_id = first_empty_id;

	if (table_id == -1) {
		printf("full table fail\n");
		return -1;
	}

	pathname_arr[table_id] = (char*) malloc(sizeof(char)*(strlen(pathname)+1));

	strcpy(pathname_arr[table_id], pathname);

	int open_fd = open(pathname, O_RDWR | O_SYNC);

	// 파일을 성공적으로 여는 경우
	if (open_fd != -1) {
		fd_arr[table_id] = open_fd;
		return table_id;
	}


	open_fd = creat(pathname, 0777);
	if (open_fd == -1)
		return -1;

	open_fd = open(pathname, O_RDWR | O_SYNC);
	if (open_fd == -1)
		return -1;

	// 파일을 만들어줬을때 헤더 페이지를 새로 만들어줌
	page_t* header_page = (page_t*)malloc(sizeof(page_t));
	memset(header_page, 0, sizeof(page_t));

	if (header_page == NULL) {
		printf("open_table() dynamic dynamic allocation error\n");
		return -1;
	}

	header_page->page_num = 0;
	header_page->table_id = table_id;
	header_page->free_page_num = 0;
	header_page->root_page_num = 0;
	header_page->num_of_page = 1;

	fd_arr[table_id] = open_fd;

	start_table(table_id, 0, header_page);

	free(header_page);

	return table_id;
}



int close_table(int table_id) {

	// 열리지 않은 테이블의 경우
	if(fd_arr[table_id] == 0)
		return -1;

	buffer_write_to_disk_table(table_id);

	return 0;
}

int shutdown_db() {

	for(int i = 1; i < 11; i++) {
		
		if(pathname_arr[i] != NULL)	
			free(pathname_arr[i]);

		// 열리지 않은 테이블의 경우
		if(fd_arr[i] == 0)
			continue;

		// 열린 테이블에 대해 반영
		else
			close_table(i);

	}

	free(buffer);

	return 0;
}



pagenum_t find_leaf(int table_id, int64_t key) {

	page_t* page = (page_t*)malloc(sizeof(page_t));

	if (page == NULL) {
		printf("find_leaf() dynamic allocation error\n");
		return 0;
	}

	pagenum_t page_num;

	int buffer_index = buffer_read_page(table_id, 0, page);

	//root가 없는 경우 헤더페이지 번호 반환
	if (page->root_page_num == 0) {
		free(page);
		unpin(buffer_index);
		return 0;
	}

	page_num = page->root_page_num;

	unpin(buffer_index);

	buffer_index = buffer_read_page(table_id, page_num, page);

	while (!page->is_leaf) {
		int i = 0;

		if (key < page->internal[0].key)
			page_num = page->left_page_num;

		else {
			i++;
			while (i < page->num_of_key) {
				if (key >= page->internal[i].key) i++;
				else break;
			}

		}

		page_num = page->internal[i - 1].page_num;

		unpin(buffer_index);
		buffer_index = buffer_read_page(table_id, page_num, page);
	}

	free(page);
	unpin(buffer_index);

	return page_num;
}


// 찾는 키가 존재하는지, 존재하지 않는지 확인 
// 존재하면 0, 존재하지않으면 -1 반환
int db_find(int table_id, int64_t key, char* ret_val) {
	int i = 0;

	if (ret_val == NULL) {
		printf("ret_val must be allocated\n");
		return 0;
	}

	page_t* page = (page_t*)malloc(sizeof(page_t));

	if (page == NULL) {
		printf("db_find() dynamic allocation error\n");
		return -1;
	}

	pagenum_t page_num = find_leaf(table_id, key);

	// root가 없는 경우 실패
	if (page_num == 0) {
		free(page);
		return -1;
	}

	int buffer_index = buffer_read_page(table_id, page_num, page);

	for (i = 0; i < page->num_of_key; i++)
		if (key == page->record[i].key)
			break;


	// 찾는 키가 없을 경우 실패
	if (i == page->num_of_key) {
		free(page);
		unpin(buffer_index);
		return -1;
	}

	strcpy(ret_val, page->record[i].value);

	free(page);
	unpin(buffer_index);

	return 0;
}

int cut(int length) {
	if (length % 2 == 0)
		return length / 2;
	else
		return length / 2 + 1;
}


record_t* make_record(int64_t key, char* value) {
	record_t* new_record = (record_t*)malloc(sizeof(record_t));

	if (new_record == NULL) {
		perror("make_record() dynamic allocation error");
		exit(EXIT_FAILURE);
	}

	else {
		new_record->key = key;
		strcpy(new_record->value, value);
	}

	return new_record;
}


// buffer_alloc_page()를 호출하여 프리 페이지를 할당받고, 그 페이지를 leaf 페이지로 만들어줌
pagenum_t make_leaf_page(int table_id, page_t* new_page) {

	pagenum_t new_page_num = buffer_alloc_page(table_id);

	new_page->page_num = new_page_num;
	new_page->table_id = table_id;
	new_page->parent_page_num = 0;
	new_page->is_leaf = 1;
	new_page->num_of_key = 0;
	new_page->right_sibling_num = 0;

	return new_page_num;
}

// buffer_alloc_page()를 호출하여 프리 페이지를 할당받고, 그 페이지를 인터널 페이지로 만들어줌
pagenum_t make_internal_page(int table_id, page_t* new_page) {

	pagenum_t new_page_num = buffer_alloc_page(table_id);

	new_page->page_num = new_page_num;
	new_page->table_id = table_id;
	new_page->parent_page_num = 0;
	new_page->is_leaf = 0;
	new_page->num_of_key = 0;

	new_page->left_page_num = 0;

	return new_page_num;
}



int get_insertion_index(int table_id, pagenum_t parent_page_num, pagenum_t child_page_num) {

	int insertion_index = 0;

	page_t parent_page;
	int buffer_index = buffer_read_page(table_id, parent_page_num, &parent_page);

	if (parent_page.left_page_num == child_page_num) {
		unpin(buffer_index);
		return insertion_index;
	}

	while (insertion_index < parent_page.num_of_key && parent_page.internal[insertion_index].page_num != child_page_num)
		insertion_index++;

	unpin(buffer_index);
	return insertion_index + 1;
}

// leaf page에 레코드를 넣음
int insert_into_leaf(int table_id, page_t* target_page, record_t* record) {

	int i;
	int insertion_point = 0;

	while (insertion_point < target_page->num_of_key && target_page->record[insertion_point].key < record->key)
		insertion_point++;

	for (i = target_page->num_of_key; i > insertion_point; i--) {
		target_page->record[i].key = target_page->record[i - 1].key;
		strcpy(target_page->record[i].value, target_page->record[i - 1].value);
	}

	target_page->record[insertion_point].key = record->key;
	strcpy(target_page->record[insertion_point].value, record->value);
	target_page->num_of_key++;

	buffer_write_page(table_id, target_page->page_num, target_page);

	return 0;
}

// leaf page에 레코드를 넣음
// split을 진행하여 insert_into_parent() 함수를 호출
int insert_into_leaf_after_splitting(int table_id, page_t* target_page, record_t* record) {

	page_t* new_page = (page_t*)malloc(sizeof(page_t));

	if (new_page == NULL) {
		printf("insert_into_leaf_after_splitting() dynamic allocation error\n");
		return -1;
	}


	pagenum_t new_page_num = make_leaf_page(table_id, new_page);

	int insertion_index;
	int i, j;

	record_t* temp_records = (record_t*)malloc(sizeof(record_t) * LEAF_ORDER);

	if (temp_records == NULL) {
		printf("insert_into_leaf_after_splitting() dynamic allocation error\n");
		free(new_page);
		return -1;
	}

	insertion_index = 0;
	while (insertion_index < LEAF_ORDER - 1 && target_page->record[insertion_index].key < record->key)
		insertion_index++;


	for (i = 0, j = 0; i < target_page->num_of_key; i++, j++) {
		if (j == insertion_index)
			j++;
		temp_records[j].key = target_page->record[i].key;
		strcpy(temp_records[j].value, target_page->record[i].value);
	}

	temp_records[insertion_index].key = record->key;
	strcpy(temp_records[insertion_index].value, record->value);

	target_page->num_of_key = 0;

	int split = cut(LEAF_ORDER - 1);

	for (i = 0; i < split; i++) {
		target_page->record[i].key = temp_records[i].key;
		strcpy(target_page->record[i].value, temp_records[i].value);
		target_page->num_of_key++;
	}

	for (i = split, j = 0; i < LEAF_ORDER; i++, j++) {
		new_page->record[j].key = temp_records[i].key;
		strcpy(new_page->record[j].value, temp_records[i].value);
		new_page->num_of_key++;
	}

	new_page->parent_page_num = target_page->parent_page_num;

	new_page->right_sibling_num = target_page->right_sibling_num;
	target_page->right_sibling_num = new_page_num;

	int new_key = new_page->record[0].key;

	buffer_write_page(table_id, target_page->page_num, target_page);
	buffer_write_page(table_id, new_page_num, new_page);

	free(temp_records);
	free(new_page);

	return insert_into_parent(table_id, target_page, new_key, new_page_num);
}

// internal page에 key와 right_child_page_num을 넣음
int insert_into_internal(int table_id, page_t* target_page, int insertion_index, int64_t key, pagenum_t right_child_page_num) {

	int i;

	for (i = target_page->num_of_key; i > insertion_index; i--) {
		target_page->internal[i].key = target_page->internal[i - 1].key;
		target_page->internal[i].page_num = target_page->internal[i - 1].page_num;
	}

	target_page->internal[insertion_index].key = key;
	target_page->internal[insertion_index].page_num = right_child_page_num;
	target_page->num_of_key++;

	buffer_write_page(table_id, target_page->page_num, target_page);

	return 0;
}

// internal page에 key와 right_child_page_num을 넣음
// split을 진행하여 insert_into_parent() 함수를 호출
int insert_into_internal_after_splitting(int table_id, page_t* target_page, int insertion_index, int64_t key, pagenum_t right_child_page_num) {

	page_t* new_page = (page_t*)malloc(sizeof(page_t));

	if (new_page == NULL) {
		printf("insert_into_internal_after_splitting() dynamic allocation error\n");
		return -1;
	}

	pagenum_t new_page_num = make_internal_page(table_id, new_page);

	int i, j;

	internal_t* temp_internals = (internal_t*)malloc(sizeof(internal_t) * INTERNAL_ORDER);

	if (temp_internals == NULL) {
		printf("insert_into_internal_after_splitting() dynamic allocation error\n");
		free(new_page);
		return -1;
	}

	for (i = 0, j = 0; i < target_page->num_of_key; i++, j++) {
		if (insertion_index == j)
			j++;
		temp_internals[j].key = target_page->internal[i].key;
		temp_internals[j].page_num = target_page->internal[i].page_num;
	}

	temp_internals[insertion_index].key = key;
	temp_internals[insertion_index].page_num = right_child_page_num;


	int split = cut(INTERNAL_ORDER);

	target_page->num_of_key = 0;

	for (i = 0; i < split - 1; i++) {
		target_page->internal[i].key = temp_internals[i].key;
		target_page->internal[i].page_num = temp_internals[i].page_num;
		target_page->num_of_key++;
	}

	int k_prime = temp_internals[split - 1].key;
	new_page->left_page_num = temp_internals[split - 1].page_num;

	for (i = split, j = 0; i < INTERNAL_ORDER; i++, j++) {
		new_page->internal[j].key = temp_internals[i].key;
		new_page->internal[j].page_num = temp_internals[i].page_num;
		new_page->num_of_key++;
	}

	new_page->parent_page_num = target_page->parent_page_num;


	// new_page의 child들의 부모를 new_page로 바꿔줌
	page_t* child_page = (page_t*)malloc(sizeof(page_t));

	if (child_page == NULL) {
		printf("insert_into_internal_after_splitting() dynamic allocation error\n");
		free(new_page);
		free(temp_internals);
		return -1;
	}

	int child_buffer_index = buffer_read_page(table_id, new_page->left_page_num, child_page);
	child_page->parent_page_num = new_page_num;
	buffer_write_page(table_id, new_page->left_page_num, child_page);
	unpin(child_buffer_index);

	for (i = 0; i < new_page->num_of_key; i++) {
		child_buffer_index = buffer_read_page(table_id, new_page->internal[i].page_num, child_page);
		child_page->parent_page_num = new_page_num;
		buffer_write_page(table_id, new_page->internal[i].page_num, child_page);
		unpin(child_buffer_index);
	}

	buffer_write_page(table_id, target_page->page_num, target_page);
	buffer_write_page(table_id, new_page_num, new_page);

	free(temp_internals);
	free(new_page);
	free(child_page);

	return insert_into_parent(table_id, target_page, k_prime, new_page_num);
}


int insert_into_parent(int table_id, page_t* left_child_page, int64_t key, pagenum_t right_child_page_num) {

	int insertion_index;
	int success;

	page_t* parent_page = (page_t*)malloc(sizeof(page_t));

	if (parent_page == NULL) {
		printf("insert_into_parent() dynamic allocation error\n");
		return -1;
	}

	pagenum_t parent_page_num = left_child_page->parent_page_num;
	int parent_buffer_index = buffer_read_page(table_id, parent_page_num, parent_page);

	// 새로운 root를 만들어줘야하는 경우
	if (parent_page_num == 0) {
		page_t* right_child_page = (page_t*)malloc(sizeof(page_t));

		if (right_child_page == NULL) {
			printf("insert_into_parent() dynamic allocation error\n");
			free(parent_page);
			unpin(parent_buffer_index);

			return -1;
		}

		int right_buffer_index = buffer_read_page(table_id, right_child_page_num, right_child_page);

		success = insert_into_new_root(table_id, left_child_page, key, right_child_page);

		free(parent_page);
		free(right_child_page);

		unpin(parent_buffer_index);
		unpin(right_buffer_index);

		return success;
	}

	insertion_index = get_insertion_index(table_id, parent_page_num, left_child_page->page_num);

	// overflow가 안일어나는 경우
	if (parent_page->num_of_key < INTERNAL_ORDER - 1)
		success = insert_into_internal(table_id, parent_page, insertion_index, key, right_child_page_num);
	// overflow가 일어나는 경우
	else
		success = insert_into_internal_after_splitting(table_id, parent_page, insertion_index, key, right_child_page_num);

	free(parent_page);
	unpin(parent_buffer_index);

	return success;
}

// 새로운 루트를 만들어주는 함수
int insert_into_new_root(int table_id, page_t* left_child_page, int64_t key, page_t* right_child_page) {
	page_t* root_page = (page_t*)malloc(sizeof(page_t));

	if (root_page == NULL) {
		printf("insert_into_new_root() dynamic allocation error\n");
		return -1;
	}

	pagenum_t root_page_num = make_internal_page(table_id, root_page);

	root_page->left_page_num = left_child_page->page_num;
	root_page->internal[0].key = key;
	root_page->internal[0].page_num = right_child_page->page_num;
	root_page->num_of_key++;

	// 새로운 root를 부모로 설정
	left_child_page->parent_page_num = root_page_num;
	right_child_page->parent_page_num = root_page_num;

	// 헤더 페이지의 root_page_num 수정
	page_t* header_page = (page_t*)malloc(sizeof(page_t));
	if (header_page == NULL) {
		printf("insert_into_new_root() dynamic allocation error\n");
		free(root_page);
		return -1;
	}

	int buffer_index = buffer_read_page(table_id, 0, header_page);

	header_page->root_page_num = root_page_num;

	buffer_write_page(table_id, left_child_page->page_num, left_child_page);
	buffer_write_page(table_id, right_child_page->page_num, right_child_page);
	buffer_write_page(table_id, root_page_num, root_page);
	buffer_write_page(table_id, 0, header_page);

	free(root_page);
	free(header_page);

	unpin(buffer_index);

	return 0;
}


//새로운 트리를 시작해주는 함수
int start_new_tree(int table_id, record_t* record) {
	page_t* root_page = (page_t*)malloc(sizeof(page_t));

	if (root_page == NULL) {
		printf("start_new_tree() dynamic allocation error\n");
		return -1;
	}

	pagenum_t root_page_num = make_leaf_page(table_id, root_page);

	root_page->right_sibling_num = 0;
	root_page->record[0].key = record->key;
	strcpy(root_page->record[0].value, record->value);
	root_page->num_of_key++;

	//헤더 페이지의 root_page_num 수정
	page_t* header_page = (page_t*)malloc(sizeof(page_t));

	if (header_page == NULL) {
		printf("start_new_tree() dynamic allocation error\n");
		free(root_page);
		return -1;
	}

	int buffer_index = buffer_read_page(table_id, 0, header_page);

	header_page->root_page_num = root_page_num;

	buffer_write_page(table_id, root_page_num, root_page);
	buffer_write_page(table_id, 0, header_page);

	free(root_page);
	free(header_page);

	unpin(buffer_index);

	return 0;
}

// key와 value를 넣어주는 함수
// 성공시 0, 실패시 -1 반환
int db_insert(int table_id, int64_t key, char * value) {

	record_t* record;
	page_t* header_page;
	page_t* target_page;
	pagenum_t target_page_num;
	int success;

	char* ret_val = (char*)malloc(sizeof(char) * 120);

	if (ret_val == NULL) {
		printf("db_insert() dynamic allocation error\n");
		return -1;
	}


	// 넣으려는 키가 이미 있으면 실패
	if (db_find(table_id, key, ret_val) == 0) {
		success = -1;
		free(ret_val);
		return success;
	}

	free(ret_val);

	record = make_record(key, value);

	header_page = (page_t*)malloc(sizeof(page_t));

	if (header_page == NULL) {
		printf("db_insert() dynamic allocation error\n");
		return -1;
	}

	int header_buffer_index = buffer_read_page(table_id, 0, header_page);

	// 트리가 없는 경우 새로운 트리를 만들어줌
	if (header_page->root_page_num == 0) {
		success = start_new_tree(table_id, record);

		free(record);
		free(header_page);

		unpin(header_buffer_index);
		return success;
	}

	target_page = (page_t*)malloc(sizeof(page_t));

	if (target_page == NULL) {
		printf("db_insert() dynamic allocation error\n");

		free(record);
		free(header_page);

		unpin(header_buffer_index);

		return -1;
	}

	target_page_num = find_leaf(table_id, key);
	int target_buffer_index = buffer_read_page(table_id, target_page_num, target_page);

	if (target_page->num_of_key < LEAF_ORDER - 1)
		success = insert_into_leaf(table_id, target_page, record);

	else
		success = insert_into_leaf_after_splitting(table_id, target_page, record);

	free(record);
	free(header_page);
	free(target_page);

	unpin(header_buffer_index);
	unpin(target_buffer_index);

	return success;
}


int get_neighbor_index(page_t* parent_page, pagenum_t target_page_num) {
	int i = 0;

	if (parent_page->left_page_num == target_page_num)
		return -2;

	for (i = 0; i < parent_page->num_of_key; i++) {
		if (parent_page->internal[i].page_num == target_page_num)
			return i - 1;
	}
}

//target_page에서 해당 key와 오른쪽에 있는 value혹은 page_num을 삭제해주는 함수
int remove_entry_from_page(int table_id, page_t* target_page, int64_t key) {

	int i = 0;


	if (target_page->is_leaf) {

		// 해당 key의 위치를 찾음
		while (target_page->record[i].key != key)
			i++;

		// 해당 key의 위치 이후에 있는 key와 value를 한칸씩 당겨줌
		for (++i; i < target_page->num_of_key; i++) {
			target_page->record[i - 1].key = target_page->record[i].key;
			strcpy(target_page->record[i - 1].value, target_page->record[i].value);
		}
	}
	else
	{
		// 해당 key의 위치를 찾음
		while (target_page->internal[i].key != key)
			i++;

		// 해당 key의 위치 이후에 있는 key와 page_num을 한칸씩 당겨줌
		for (++i; i < target_page->num_of_key; i++) {
			target_page->internal[i - 1].key = target_page->internal[i].key;
			target_page->internal[i - 1].page_num = target_page->internal[i].page_num;
		}
	}


	target_page->num_of_key--;

	buffer_write_page(table_id, target_page->page_num, target_page);

	return 0;
}


int adjust_root(int table_id, page_t* root_page) {

	if (root_page->num_of_key > 0)
		return 0;

	page_t* header_page = (page_t*)malloc(sizeof(page_t));

	if (header_page == NULL) {
		printf("adjust_root() dynamic allocation error\n");
		return -1;
	}

	int header_buffer_index = buffer_read_page(table_id, 0, header_page);

	if (root_page->is_leaf) {
		header_page->root_page_num = 0;

		buffer_free_page(table_id, root_page->page_num);

	}

	else {
		header_page->root_page_num = root_page->left_page_num;

		page_t* new_root_page = (page_t*)malloc(sizeof(page_t));

		if (new_root_page == NULL) {
			printf("adjust_root() dynamic allocation error\n");
			free(header_page);
			unpin(header_buffer_index);
			return -1;
		}

		int new_root_buffer_index = buffer_read_page(table_id, root_page->left_page_num, new_root_page);

		new_root_page->parent_page_num = 0;

		buffer_write_page(table_id, new_root_page->page_num, new_root_page);
		unpin(new_root_buffer_index);

		buffer_free_page(table_id, root_page->page_num);
	
		free(new_root_page);
	}

	header_page->free_page_num = root_page->page_num;

	buffer_write_page(table_id, 0, header_page);
	free(header_page);

	unpin(header_buffer_index);

	return 0;
}

// Merge해주는 함수
int coalesce_page(int table_id, page_t* target_page, page_t* neighbor_page, pagenum_t neighbor_index, int64_t k_prime) {

	// 만약 target_page가 가장 왼쪽에 있는 page라면 neighbor_page와 swap
	if (neighbor_index == -2) {
		page_t* temp_page = target_page;
		target_page = neighbor_page;
		neighbor_page = temp_page;
	}

	// target_page가 leaf page인 경우
	// target_page의 key와 value들을 neighbor_page로 옮김
	if (target_page->is_leaf) {

		int target_num_of_key_before = target_page->num_of_key;

		for (int i = 0, j = neighbor_page->num_of_key; i < target_num_of_key_before; i++, j++) {
			neighbor_page->record[j].key = target_page->record[i].key;
			strcpy(neighbor_page->record[j].value, target_page->record[i].value);
			neighbor_page->num_of_key++;
			target_page->num_of_key--;
		}
		neighbor_page->right_sibling_num = target_page->right_sibling_num;

	}

	// target_page가 internal page인 경우
	// neighbor_page로 k_prime을 옮긴후 target_page의 key와 page_num들을 옮김
	else {
		int target_num_of_key_before = target_page->num_of_key;
		int neighbor_num_of_key_before = neighbor_page->num_of_key;

		neighbor_page->internal[neighbor_page->num_of_key].key = k_prime;
		neighbor_page->internal[neighbor_page->num_of_key].page_num = target_page->left_page_num;
		neighbor_page->num_of_key++;

		for (int i = 0, j = neighbor_page->num_of_key; i < target_num_of_key_before; i++, j++) {
			neighbor_page->internal[j].key = target_page->internal[i].key;
			neighbor_page->internal[j].page_num = target_page->internal[i].page_num;
			neighbor_page->num_of_key++;
			target_page->num_of_key--;
		}


		// target_page에서 neighbor_page로 옮겨진 child들의 부모를 바꿈
		page_t* child_page = (page_t*)malloc(sizeof(page_t));

		if (child_page == NULL) {
			printf("coalesce_page() dynamic allocation error\n");
			return -1;
		}

		int child_buffer_index;

		for (int i = neighbor_num_of_key_before; i < neighbor_page->num_of_key; i++) {
			child_buffer_index = buffer_read_page(table_id, neighbor_page->internal[i].page_num, child_page);
			child_page->parent_page_num = neighbor_page->page_num;
			buffer_write_page(table_id, child_page->page_num, child_page);
			unpin(child_buffer_index);
		}

		free(child_page);
	}


	buffer_write_page(table_id, target_page->page_num, target_page);
	buffer_write_page(table_id, neighbor_page->page_num, neighbor_page);

	page_t* parent_page = (page_t*)malloc(sizeof(page_t));

	if (parent_page == NULL) {
		printf("coalesce_page() dynamic allocation error\n");
		return -1;
	}

	int parent_buffer_index = buffer_read_page(table_id, neighbor_page->parent_page_num, parent_page);

	buffer_free_page(table_id, target_page->page_num);

	int success = delete_entry(table_id, parent_page, k_prime);

	free(parent_page);

	unpin(parent_buffer_index);

	return success;

}

// redistribute해주는 함수
int redistribute_page(int table_id, page_t* target_page, page_t* neighbor_page, int neighbor_index, int k_prime_index, int64_t k_prime) {

	int i;

	page_t* parent_page = (page_t*)malloc(sizeof(page_t));

	if (parent_page == NULL) {
		printf("redistribute_page() dynamic allocation error\n");
		return -1;
	}

	int parent_buffer_index = buffer_read_page(table_id, target_page->parent_page_num, parent_page);

	//target_page가 가장 왼쪽에 있는 페이지인 경우
	if (neighbor_index == -2) {

		//target_page가 leaf page인 경우
		if (target_page->is_leaf) {

			target_page->record[target_page->num_of_key].key = neighbor_page->record[0].key;
			strcpy(target_page->record[target_page->num_of_key].value, neighbor_page->record[0].value);

			parent_page->internal[k_prime_index].key = neighbor_page->record[1].key;

			for (i = 0; i < neighbor_page->num_of_key - 1; i++) {
				neighbor_page->record[i].key = neighbor_page->record[i + 1].key;
				strcpy(neighbor_page->record[i].value, neighbor_page->record[i + 1].value);

			}
		}
		//target_page가 internal page인 경우
		else {
			page_t* last_child_page = (page_t*)malloc(sizeof(page_t));

			if (last_child_page == NULL) {
				printf("redistribute_page() dynamic allocation error\n");
				free(parent_page);
				unpin(parent_buffer_index);
				return -1;
			}

			int last_child_buffer_index = buffer_read_page(table_id, neighbor_page->left_page_num, last_child_page);

			target_page->internal[target_page->num_of_key].key = k_prime;
			target_page->internal[target_page->num_of_key].page_num = neighbor_page->left_page_num;
			last_child_page->parent_page_num = target_page->page_num;
			parent_page->internal[k_prime_index].key = neighbor_page->internal[0].key;

			neighbor_page->left_page_num = neighbor_page->internal[0].page_num;
			for (i = 0; i < neighbor_page->num_of_key - 1; i++) {
				neighbor_page->internal[i].key = neighbor_page->internal[i + 1].key;
				neighbor_page->internal[i].page_num = neighbor_page->internal[i + 1].page_num;
			}

			buffer_write_page(table_id, last_child_page->page_num, last_child_page);
			free(last_child_page);

			unpin(last_child_buffer_index);
		}
	}

	//target_page가 가장 왼쪽에 있는 페이지가 아닌 경우
	else {
		// target_page가 leaf_page인 경우
		if (target_page->is_leaf) {

			for (i = target_page->num_of_key; i > 0; i--) {
				target_page->record[i].key = target_page->record[i - 1].key;
				strcpy(target_page->record[i].value, target_page->record[i - 1].value);
			}

			target_page->record[0].key = neighbor_page->record[neighbor_page->num_of_key - 1].key;
			strcpy(target_page->record[0].value, neighbor_page->record[neighbor_page->num_of_key - 1].value);

			parent_page->internal[k_prime_index].key = target_page->record[0].key;
		}

		//target_page가 internal page인 경우
		else {
			page_t* first_child_page = (page_t*)malloc(sizeof(page_t));

			if (first_child_page == NULL) {
				printf("redistribute_page() dynamic allocation error\n");
				free(parent_page);
				unpin(parent_buffer_index);
				return -1;
			}

			int first_child_buffer_index = buffer_read_page(table_id, neighbor_page->internal[neighbor_page->num_of_key - 1].page_num, first_child_page);

			for (i = target_page->num_of_key; i > 0; i--) {
				target_page->internal[i].key = target_page->internal[i - 1].key;
				target_page->internal[i].page_num = target_page->internal[i - 1].page_num;
			}
			target_page->internal[0].page_num = target_page->left_page_num;

			target_page->internal[0].key = k_prime;
			target_page->left_page_num = neighbor_page->internal[neighbor_page->num_of_key - 1].page_num;
			first_child_page->parent_page_num = target_page->page_num;
			parent_page->internal[k_prime_index].key = neighbor_page->internal[neighbor_page->num_of_key - 1].key;

			buffer_write_page(table_id, first_child_page->page_num, first_child_page);
			free(first_child_page);

			unpin(first_child_buffer_index);
		}
	}

	target_page->num_of_key++;
	neighbor_page->num_of_key--;

	buffer_write_page(table_id, target_page->page_num, target_page);
	buffer_write_page(table_id, neighbor_page->page_num, neighbor_page);
	buffer_write_page(table_id, parent_page->page_num, parent_page);

	free(parent_page);

	unpin(parent_buffer_index);

	return 0;
}


int delete_entry(int table_id, page_t* target_page, int64_t key) {

	page_t* header_page = (page_t*)malloc(sizeof(page_t));

	if (header_page == NULL) {
		printf("delete_entry() dynamic allocation error\n");
		return -1;
	}

	int header_buffer_index = buffer_read_page(table_id, 0, header_page);

	remove_entry_from_page(table_id, target_page, key);

	int success;
	int min_keys;
	int neighbor_index;
	int k_prime_index;
	int k_prime;
	int capacity;

	// root 노드에서 지운 경우
	if (target_page->page_num == header_page->root_page_num) {
		success = adjust_root(table_id, target_page);
		free(header_page);
		unpin(header_buffer_index);
		return success;
	}

	min_keys = 1;

	// delayed merge를 구현하기 위해, 만약 지운 page의 key 개수가 1개 이상인 경우, 
	// delete 함수를 끝마치는 방식을 사용
	// 만약 지운 page의 key 개수가 0개이면 그때 merge나, redistribute를 수행
	if (target_page->num_of_key >= min_keys) {
		success = 0;
		free(header_page);
		unpin(header_buffer_index);
		return success;
	}


	page_t* parent_page = (page_t*)malloc(sizeof(page_t));

	if (parent_page == NULL) {
		printf("delete_entry() dynamic allocation error\n");
		free(header_page);
		unpin(header_buffer_index);
		return -1;
	}

	page_t* neighbor_page = (page_t*)malloc(sizeof(page_t));

	if (neighbor_page == NULL) {
		printf("delete_entry() dynamic allocation error\n");
		free(header_page);
		free(parent_page);
		unpin(header_buffer_index);
		return -1;
	}

	int parent_buffer_index = buffer_read_page(table_id, target_page->parent_page_num, parent_page);

	neighbor_index = get_neighbor_index(parent_page, target_page->page_num);

	int neighbor_buffer_index;

	if (neighbor_index == -2) {
		k_prime_index = 0;
		k_prime = parent_page->internal[k_prime_index].key;
		neighbor_buffer_index = buffer_read_page(table_id, parent_page->internal[0].page_num, neighbor_page);
	}

	else if (neighbor_index == -1) {
		k_prime_index = 0;
		k_prime = parent_page->internal[k_prime_index].key;
		neighbor_buffer_index = buffer_read_page(table_id, parent_page->left_page_num, neighbor_page);
	}

	else {
		k_prime_index = neighbor_index + 1;
		k_prime = parent_page->internal[k_prime_index].key;
		neighbor_buffer_index = buffer_read_page(table_id, parent_page->internal[neighbor_index].page_num, neighbor_page);
	}

	if (target_page->is_leaf)
		capacity = cut(LEAF_ORDER);
	else
		capacity = cut(INTERNAL_ORDER - 1);

	// target_page의 키 개수가 0개일때, neighbor_page의 키 개수가 capacity보다 작으면 merge를 수행
	if (neighbor_page->num_of_key + target_page->num_of_key < capacity)
		success = coalesce_page(table_id, target_page, neighbor_page, neighbor_index, k_prime);

	// 그렇지 않은 경우에는 redistribute를 수행
	else
		success = redistribute_page(table_id, target_page, neighbor_page, neighbor_index, k_prime_index, k_prime);

	free(header_page);
	free(parent_page);
	free(neighbor_page);

	unpin(header_buffer_index);
	unpin(parent_buffer_index);
	unpin(neighbor_buffer_index);

	return success;
}


// 해당 키를 찾아 지워주는 함수
// 성공하면 0, 실패하면 -1 반환
int db_delete(int table_id, int64_t key) {

	char* ret_val = (char*)malloc(sizeof(char) * 120);

	if (ret_val == NULL) {
		printf("db_delete() dynamic allocation error\n");
		return -1;
	}

	int exist = db_find(table_id, key, ret_val);

	// 해당 키가 tree에 없는 경우 실패
	if (exist != 0) {
		free(ret_val);
		return -1;
	}

	pagenum_t target_page_num = find_leaf(table_id, key);
	page_t* target_page = (page_t*)malloc(sizeof(page_t));

	if (target_page == NULL) {
		printf("db_delete() dynamic allocation error\n");
		free(ret_val);
		return -1;
	}

	int target_buffer_index = buffer_read_page(table_id, target_page_num, target_page);

	int success = delete_entry(table_id, target_page, key);

	free(target_page);
	free(ret_val);

	unpin(target_buffer_index);

	return success;
}
