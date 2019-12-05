
#include "bpt.h"
#define INPUT 10000
#define PAGENUM 10000
#include <time.h>

void* func(void* arg)
{
	
	char value[120];
	int dead = 0;
	int tid;
	value[0] = 'b';
	tid = begin_trx();
	int who = 0;
	printf("tid: %d\n",tid);

		dead =db_find(1, 0, value, tid);

		dead =db_update(1, 0, value, tid);
		
		printf("end %d\n", tid);
		end_trx(tid);

}
int main() {
	pthread_t threads[3];
	int a = 100;
	int status;
	
	srand(time(NULL));
	int random;
	int who;
	int num;
	char* input_file = "chasanggod.dat";
	char* input_file2 = "chasanggod";
	char* input_file3 = "cc";	
	char input2[120];
	char str;
	input2[0] = 'a';
	//OPEN
	init_db(1000);
	open_table(input_file);
	open_table(input_file2);
	open_table(input_file3);
	
	//INSERT
	for(int i = 0; i < INPUT; i++)
	{

		printf("insert %d\n", i);
		db_insert(1, i, input2);
		db_insert(2, i, input2); 

	}


	printf("0");
	printf("\n\n\n");
	

	for(int i = 0; i < 3; i++)
	{
		if(pthread_create(&threads[i], 0, func, (void*)&i) <0)
		{
			perror("no");
			exit(0);
		}
	}
	for(int i = 0; i < 3; i++)
	{
		
		pthread_join(threads[i], NULL);
	}
	printf("end");
	shutdown_db();
	return EXIT_SUCCESS;
}

