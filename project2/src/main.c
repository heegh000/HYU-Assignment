#include "bpt.h"

int main() {

	char choice;
	
	open_table("test.txt");

	while(1) {
		scanf("%c", &choice);
		getchar();

		if(choice == 'i') {
			int64_t key;
			char value[120];

			scanf("%ld", &key);
			getchar();
			scanf("%s", value);
			getchar();

			if(db_insert(key, value) != 0)
				printf("insert fail\n");
		}

		else if(choice == 'p') {
			pagenum_t page_num;
			scanf("%ld", &page_num);
			getchar();
			print_page(page_num);
		}
		else if(choice == 'd') {
			int64_t key;
			scanf("%ld", &key);
			getchar();
			if(db_delete(key) != 0)
				printf("delete fail\n");		

		}
		else if(choice == 'q') {
			break;
		}
	}


	return 0;
}
