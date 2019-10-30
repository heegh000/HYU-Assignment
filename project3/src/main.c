#include "bpt.h"

extern int num_of_buffer;
extern char* pathname_arr[11];

int main() {
	int buffer_num;
	printf("Enter buffer_num: ");
	scanf("%d", &buffer_num);
	init_db(buffer_num);

	while(1) {

		char choice[100]; 
		
		scanf("%s",choice);

		if(strcmp(choice, "o") == 0) {

			char path[100];
			int table_id = 0;
			scanf("%s", path);

			table_id = open_table(path);

			if(table_id == -1) {
				printf("open fail\n");
				continue;
			}
			printf("%d table open\n", table_id);
		}

		else if(strcmp(choice, "i") == 0) {
			int table_id = 0;
			int64_t key = 0;
			char value[120] = {0, };

			scanf("%d %ld %s", &table_id, &key, value);

			if(db_insert(table_id, key, value) == -1)
				printf("insert fail\n");
			else 
				printf("insert success\n");


		}

		else if(strcmp(choice, "d") == 0) {
			int table_id = 0;
			int64_t key = 0;

			scanf("%d %ld", &table_id, &key);

			if(db_delete(table_id, key) == -1)
				printf("delete fail\n");
			else
				printf("delete success\n");
		}

		else if(strcmp(choice, "pp") == 0) {
			pagenum_t page_num;
			
			int table_id;	
		
			scanf("%d %ld", &table_id, &page_num);

			print_page(table_id, page_num);
		}

		else if(strcmp(choice, "pb") == 0) {
			for(int i = 0; i < num_of_buffer; i++) 
				print_buffer(i);
		}

		else if(strcmp(choice, "c") == 0) {
			int table_id = 0;

			scanf("%d", &table_id);

			close_table(table_id);
		}

		else if(strcmp(choice, "q") == 0)
			break;


	}

	shutdown_db();

	return 0;
}
