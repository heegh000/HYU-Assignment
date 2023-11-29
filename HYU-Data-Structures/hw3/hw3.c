#include <stdio.h>
#include <stdlib.h>

typedef struct polyNode {
	int coef;
	int expon;
	polyNode* next;
}polyNode;

int findPolynomial(polyNode* pHeadNode, int expon);
void showPolynomial(polyNode* pHeadNode);

void addPolynomial(polyNode** pHeadNode, int coef, int expon) {
	
	polyNode* copyHead = *pHeadNode;
	polyNode* addNode = NULL;

	if((*pHeadNode) == NULL) {
		(*pHeadNode) = (polyNode*)malloc(sizeof(polyNode));
		(*pHeadNode)->coef = 0;
		(*pHeadNode)->expon = 0;
		(*pHeadNode)->next = NULL;
		addPolynomial(pHeadNode, coef, expon);
	}
	
	if(findPolynomial((*pHeadNode), expon) == 1) 
		return;


	while((*pHeadNode)->next!= NULL && ((*pHeadNode)->next)->expon > expon) {
		(*pHeadNode) = (*pHeadNode)->next;
	}


	addNode = (polyNode*)malloc(sizeof(polyNode));
	addNode->coef = coef;
	addNode->expon = expon;
	addNode->next = (*pHeadNode)->next;

	(*pHeadNode)->next = addNode;

	*pHeadNode = copyHead;
}

void removePolynomial(polyNode** pHeadNode, int expon) {

	if((*pHeadNode) == NULL) {
		return;		
	}

	if(findPolynomial((*pHeadNode), expon) == 0) {
		printf("there is no x^%d\n",expon);
		return;
	}

	polyNode* pHeadNode_ = (*pHeadNode)->next;
        polyNode* delNode = pHeadNode_;




	if( pHeadNode_->expon == expon ) {
		delNode = pHeadNode_;
		(*pHeadNode)->next = pHeadNode_->next;
		free(delNode);
		return;
	}
	
	while(delNode->next != NULL) {
		delNode = delNode->next;
		
		if(delNode->expon == expon)
			break;
		
		pHeadNode_ = pHeadNode_->next;
	}
	
	pHeadNode_->next = delNode->next;
	free(delNode);
	

}

polyNode* multiplication(polyNode* A, polyNode* B) {
	
	if(A == NULL || B == NULL)
		return NULL;
				
	polyNode* C = (polyNode*)malloc(sizeof(polyNode));
	polyNode* C_start = C;
	polyNode* A_start = A;

	while(B->next != NULL) {
		
		B = B->next;
		A = A_start;
		
		while(A->next != NULL) {
				
			A = A->next;

			if(findPolynomial(C, A->expon + B->expon)) {
				polyNode* C_find = C_start;
				
				C_find = C_find->next;
				
				while(C_find->next != NULL) {
								
					if(C_find->expon == A->expon + B->expon)
						break;
			
					C_find = C_find->next; 
				} 

				C_find->coef = C_find->coef + (A->coef * B->coef);
				
				if(C_find->coef == 0)
					removePolynomial(&C_find, C_find->expon);
			}
			
			else {
				addPolynomial(&C_start, A->coef * B->coef, A->expon + B->expon);
			}
		
		}
	}

	return C;
}

void showPolynomial(polyNode* pHeadNode) {
	
	if(pHeadNode !=NULL && pHeadNode->next != NULL) {
		printf("\n");
		polyNode* pHeadNode_ = pHeadNode->next;
		while(1) {
			printf("%d*X^%d",pHeadNode_->coef, pHeadNode_->expon);
			pHeadNode_ = pHeadNode_->next;

			if(pHeadNode_ == NULL) 
				break;
			printf(" + ");
		}
	}	
	
	printf("\n======================\n");

}

void clearPolynomial(polyNode** pHeadNode) {

	polyNode* delNode = (*pHeadNode);
	
	if((*pHeadNode) == NULL) {
		return;
	}


	while((*pHeadNode) != NULL) { 
		(*pHeadNode) = (*pHeadNode)->next;
		free(delNode);
		delNode = (*pHeadNode);
	}

}

int findPolynomial(polyNode* pHeadNode, int expon) {
	
	if(pHeadNode == NULL) {
		return 0;
	}

	polyNode* pHeadNode_ = pHeadNode->next;

	while(pHeadNode_ != NULL) {
		if(pHeadNode_->expon == expon)
			return 1;

		pHeadNode_ = pHeadNode_->next;
	}

	return 0;

	
}

int main()
{
	int mode, coef, expon;
	char poly;

	polyNode *A, *B, *C;
	A = B = C = NULL;

	do
	{
		printf("\nSelect the mode\n");
		printf("=================\n");
		printf("1: add polynomial\n");
		printf("2: remove polynomial\n");
		printf("3: Multiplication\n");
		printf("4: show polynomial\n");
		printf("5: clear polynomial\n");
		printf("-1: exit the program\n");
		printf("==================\n");
		printf("mode: ");
		scanf("%d", &mode);

		switch (mode)
		{
		case 1:
			printf("\nChoose the polynomial to add ('A'or'B')\n");
			printf("Which polynomial?: ");
			scanf(" %c", &poly);
			if (poly != 'A' && poly != 'B')
				printf("Please choose right polynomial (A or B)\n");
			else
			{
				//계수와 차수를 입력 받아서 polynomial에 추가하는 함수 실행
				//이미 존재하는 차수를 입력 받을 경우 추가할 수 없음
				printf("input the coef: ");
				scanf("%d", &coef);
				printf("input the expon: ");
				scanf("%d", &expon);

				if (poly == 'A')
					addPolynomial(&A, coef, expon);
				else
					addPolynomial(&B, coef, expon);
			}
			break;
		case 2:
			printf("\nChoose the polynomial to remove ('A'or'B')\n");
			printf("Which polynomial?: ");
			scanf(" %c", &poly);
			if (poly != 'A' && poly != 'B')
				printf("Please choose right polynomial (A or B)\n");
			else
			{
				printf("input the expon: ");
				scanf("%d", &expon);
				//차수를 입력 받아서 polynomial에서 제거하는 함수 실행
				if (poly == 'A')
					removePolynomial(&A, expon);
				else
					removePolynomial(&B, expon);
			}
			break;
		case 3:
			printf("\nMultiplication with A and B\n");
			//C = A * B 연산을 수행
			C = multiplication(A, B);
			printf("C: ");
			//C의 결과를 출력
			showPolynomial(C);
			break;
		case 4:
			//A,B polynomial 각각 출력
			printf("A: ");
			showPolynomial(A);
			printf("B: ");
			showPolynomial(B);
			break;
		case 5:
			printf("\nChoose the polynomial to clear ('A'or'B')\n");
			printf("Which polynomial?: ");
			scanf(" %c", &poly);
			if (poly != 'A' && poly != 'B')
				printf("Please choose right polynomial (A or B)\n");
			else
				//해당 polynomial를 초기화 시키는 함수 실행
			{
				if (poly == 'A')
					clearPolynomial(&A);
				else
					clearPolynomial(&B);
			}
			break;
		default:
			break;
		}
	} while (mode != -1);
	return 0;
}
