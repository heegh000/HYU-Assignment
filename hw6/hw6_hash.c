#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KEY_SIZE 10
#define TABLE_SIZE 13
#define empty(e) ((strlen(e.key) == 0))
#define equal(e1, e2) (!strcmp(e1.key, e2.key))

typedef struct Element {
	char key[KEY_SIZE];
} Element;

Element hashTable[TABLE_SIZE];

void initTable(Element ht[]) {
	int i;
	for ( i = 0; i < TABLE_SIZE; i++)
		ht[i].key[0] = '\0';
}

int transform(char* key) {
	int i;
	int number = 0;
	int size = strlen(key);
	for(i = 0; i < size; i++)
		number = number + key[i];
	return number;
}

int hashFunction(char* key) {
	return transform(key) % TABLE_SIZE;
}

void addHashTable(Element item, Element ht[]) { 
	int i, hashValue;
	i = hashValue = hashFunction(item.key);

	while(!empty(ht[i])) {
		if(equal(item, ht[i])) {
			printf("중복 삽입 에러\n");
			return;
		}
		
		i = (i+1) % TABLE_SIZE;
		if(i == hashValue) {
			printf("모든 버킷 조사\n");
			return;
		}
	}

	strcpy(ht[i].key, item.key);
}

void hashSearch(Element item, Element ht[]) {
	int i, hashValue;
	i = hashValue = hashFunction(item.key);

	while(!empty(ht[i])) {
		if(equal(item, ht[i])) {
                        printf("테이블에 찾는 값이 있습니다.\n");
                        return;
                }

                i = (i+1) % TABLE_SIZE;
                if(i == hashValue) {
                        printf("테이블에 찾는 값이 없습니다.\n");
                        return;
                }
	}
	printf("테이블에 찾는 값이 없습니다.\n");

}

void printHashTable(Element ht[]) {
	int i;
	for(i = 0; i < TABLE_SIZE; i++)
		printf("[%d] %s\n", i, ht[i].key);
}

int main() {
	Element temp;
	int op;

	while(1) {
		printf("연산 입력(0: 추가, 1: 탐색, 2: 종료) = ");
		scanf("%d", &op);

		if(op == 2) break;

		printf("키 입력: ");
		scanf("%s", temp.key);

		if(op == 0)
			addHashTable(temp, hashTable);
		else if (op == 1)
			hashSearch(temp, hashTable);
		printHashTable(hashTable);
	}

	return 0;
}
