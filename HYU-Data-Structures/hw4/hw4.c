#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FALSE -1
#define TRUE 1

//List의 기본 구성 단위인 Node
typedef struct Node {
	int data;
	struct Node* nextNode;
}Node;

typedef struct LinkedList {
	int curCount;      //현재 List에 들어있는 Node의 갯수
	Node headNode; //List의 시작 Node
}LinkedList;

int addNode(LinkedList* pList, int pos, int data) {
	int i = 0;
	Node* pNewNode = NULL; 
	Node* pTmpNode = NULL;


	if(pList == NULL) {
		printf("addNode() error1 \n");
		return FALSE;
	}

	if(pos < 0 || pos > pList->curCount) {
		printf("addNode() error2 : 추가 범위 초과 \n");
		return FALSE;
	}

	pNewNode = (Node*)malloc(sizeof(Node));
	
	if(!pNewNode) {
		printf("addNode() error3 \n");
		return FALSE;
	}

	pNewNode->data = data;
	pNewNode->nextNode = NULL;


	pTmpNode = &(pList->headNode);
	for(i = 0; i < pos; i++)
		pTmpNode = pTmpNode->nextNode;
	
	pNewNode->nextNode = pTmpNode->nextNode;
	pTmpNode->nextNode = pNewNode;
	pList->curCount++;
	
	return TRUE;
}

int removeNode(LinkedList* pList, int pos) {
	
	int i = 0;
	Node* pDelNode = NULL;
	Node* pTmpNode = NULL;

	if(pList == NULL) {
		printf("removeNode() error1 \n");
		return FALSE;
	}
	
	if(pos < 0 || pos > pList->curCount) {
		printf("removeNode() error2 \n");
		return FALSE;
	}

	pTmpNode = &(pList->headNode);
	
	for(i = 0; i < pos; i++)
		pTmpNode = pTmpNode->nextNode;

	pDelNode = pTmpNode->nextNode;
	pTmpNode->nextNode = pDelNode->nextNode;
	
	free(pDelNode);
	pList->curCount--;
	return TRUE;
}

void showNode(LinkedList* pList) {
	int i = 0;
	Node* pNode = NULL;

	if(pList == NULL) {
		printf("showNode() error1 \n");
		return; 
	}

	printf("현재 노드의 개수 : %d\n", pList->curCount);
	
	pNode = pList->headNode.nextNode;

	while(pNode != NULL) {
		printf("[%d] \n", pNode->data);
		pNode = pNode->nextNode;
	}

	printf("-----------------------\n");
}
	


int isEmpty(LinkedList* pList) {
	if(pList == NULL) {
		printf("isEmpty() error\n");
		return FALSE;
	}

	if(pList->headNode.nextNode != NULL)
		return TRUE;
	else
		return FALSE;
}

int findPos(LinkedList* pList, int data) {
	int pos = 0;
	Node *pNode = NULL;

	if(pList == NULL) {
		printf("findPos() error\n");
		return FALSE;
	}

	pNode = pList->headNode.nextNode;

	while( pNode != NULL) {
		if(pNode->data == data)
			return pos;
				
		pNode = pNode->nextNode;
		pos++;
	}

	return FALSE;
}

void makeEmpty(LinkedList* pList) {
	Node* pDummyNode = NULL;
	Node* pTmpNode = NULL;
	
	if(pList != NULL) {
		pTmpNode = pList->headNode.nextNode;

		while(pTmpNode != NULL) {
			pDummyNode = pTmpNode;
			pTmpNode = pTmpNode->nextNode;
			free(pDummyNode);
			pList->curCount--;
		}
		
		pList->headNode.nextNode = NULL;
	}
}

//Stack 구조체 선언 (Linked List)
typedef struct StackNode {
	Node* data;
	struct StackNode* next;
}StackNode;

int isEmpty(StackNode* top);

//Stack 관련 함수
void pushLinkedStack(StackNode** top, Node* data) {

	StackNode *pNode = NULL;

	pNode = (StackNode*) malloc(sizeof(StackNode));
	pNode->data = data;
	pNode->next = NULL;

	if(isEmpty(*top)) {
		*top = pNode;
	}
	else {
		pNode->next = *top;
		*top = pNode;
	}

}

StackNode* popLinkedStack(StackNode** top) {
	StackNode *pNode =NULL;

	if(isEmpty(*top)) {
		printf("the Stack is empty\n");
		return NULL;
	}

	pNode = *top;
	*top = pNode->next;
	
	return pNode;
}

void showLinkedStack(StackNode* top) {
	StackNode *pNode = NULL;
	
	if(isEmpty(top)) {
		printf("the Stack is empty\n");
		return;
	}

	pNode = top;
	
	printf("==========Show Stack=========\n");
	
	while(pNode != NULL) {
		printf("[%d]\n", pNode->data->data);
		pNode = pNode->next;
	}

	printf("=============================\n");
}


StackNode* topLinkedStack(StackNode* top) {
	StackNode *pNode = NULL;
	
	if(!(isEmpty(top)))
		pNode = top;

	return pNode;
}

void deleteLinkedStack(StackNode** top) {
	StackNode *pNode = NULL;
	StackNode *pDelNode = NULL;
	pNode = *top;

	while(pNode != NULL) {
		pDelNode = pNode;
		pNode = pNode->next;
		free(pDelNode);
	}

	*top = NULL;
}
int isEmpty(StackNode* top) {
	if(top == NULL)
		return TRUE;
	else 
		return 0;
}

void reverseList(LinkedList* pList, StackNode** top) {
	Node* pNode = NULL;
	StackNode* sNode = NULL;
	
	printf("Reverse Linked List!\n");

	pNode = (pList->headNode).nextNode;
	
	while(pNode!= NULL) {
		pushLinkedStack(top, pNode);	
		pNode = pNode->nextNode;
	}

	sNode = popLinkedStack(top);
	(pList->headNode).nextNode = sNode->data;
	while(!isEmpty(*top)){
		sNode->data->nextNode = (*top)->data;
		sNode = popLinkedStack(top);
	}	
	sNode->data->nextNode = NULL;
}

int main()  {

	int pos;
	LinkedList* linkedList = (LinkedList*)malloc(sizeof(LinkedList));
	linkedList->curCount = 0;
	linkedList->headNode.nextNode = NULL;

	StackNode* top = NULL;
	StackNode* pNode;

	addNode(linkedList, 0, 10);
	addNode(linkedList, 5, 100);
	addNode(linkedList, 1, 20);
	addNode(linkedList, 2, 30);
	addNode(linkedList, 1, 50);
	addNode(linkedList, 1, 70);
	addNode(linkedList, 1, 40);

	showNode(linkedList);
	
	reverseList(linkedList, &top);
	
	showNode(linkedList);
	
	//removeNode(linkedList, 1);
	//showNode(linkedList);
	
	//pos = findPos(linkedList,30);
	//printf("the location of node with data '30': %d\n", pos);

	makeEmpty(linkedList);
	showNode(linkedList);
	return 0;
}	
