#include "bpt.h"

extern int num_of_buffer;
extern char* pathname_arr[11];

int main() {
	int i = 0;
	while(1) {

		char choice[100]; 
		
		scanf("%s",choice);

		if(strcmp(choice, "0") == 0) {

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

		else if(strcmp(choice, "init") == 0) {
			
			int buffer_num;
			scanf("%d", &buffer_num);
		
			init_db(buffer_num);

		}

		else if(strcmp(choice, "1") == 0) {
			int table_id = 0;
			int64_t key = 0;
			char value[120] = {0, };

			scanf("%d %ld %s", &table_id, &key, value);

			if(db_insert(table_id, key, value) == -1)
				printf("insert fail\n");
			else if(i % 10000 == 0)
				printf("insert %d success\n", i);
			i++;

		}

		else if(strcmp(choice, "3") == 0) {
			int table_id = 0;
			int64_t key = 0;

			scanf("%d %ld", &table_id, &key);

			if(db_delete(table_id, key) == -1)
				printf("delete fail\n");
			else
				printf("delete success\n");
		}


		else if(strcmp(choice, "5") == 0) {
			int table_id1 = 0;
			int table_id2 = 0;
			char result[200];

			scanf("%d %d %s", &table_id1, &table_id2, result);

			join_table(table_id1, table_id2, result);
		
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

		else if(strcmp(choice, "quit") == 0)
			break;


	}

	shutdown_db();

	return 0;
}
