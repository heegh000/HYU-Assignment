#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LEVEL 3
#define MIN_DATA -9999

//SkipNode 구조체 선언
typedef struct SkipNode {
   int data;
   int level;
   struct SkipNode* next[MAX_LEVEL];
}SkipNode;

//SkipNode 관련 함수
void insertSkipNode(SkipNode* pHeadNode, int data)
{
   int level = 1, i, pos = MAX_LEVEL;
   SkipNode* pTmpNode[MAX_LEVEL];
   SkipNode* pNewNode;

   for (i = 0; i < MAX_LEVEL; i++) {
      pTmpNode[i] = pHeadNode;
   }

   for (i = MAX_LEVEL - 1; i >= 0; i--) {
      while (pTmpNode[i]->next[i] != NULL) {
         if ((pTmpNode[i]->next[i])->data > data)
            break;
         pTmpNode[i] = pTmpNode[i]->next[i];
      }
   }
   while (rand() % 2) {
      level++;
      if (level >= MAX_LEVEL) {
         break;
      }
   }
   pNewNode = (SkipNode*)malloc(sizeof(SkipNode));
   pNewNode->level = level;
   pNewNode->data = data;
   for (i = 0; i < MAX_LEVEL; i++) {
      pNewNode->next[i] = NULL;
   }
   for (i = pNewNode->level - 1; i >= 0; i--) {
      pNewNode->next[i] = pTmpNode[i]->next[i];
      pTmpNode[i]->next[i] = pNewNode;
   }
   printf("Insert [%d] with level [%d]\n", data, level);
}

void showSkipNode(SkipNode* pHeadNode)
{
   SkipNode* pTmpNode = NULL;
   int i, j;

   //Level 별로 끝에 도달할 때까지 데이터 출력
   printf("---------------------------------\n");
   for (i = MAX_LEVEL; i > 0; i--)
   {
      pTmpNode = pHeadNode->next[0];
      printf("Level %d:\t", i);
      while (pTmpNode != NULL)
      {
         if (pTmpNode->level >= i)
            printf("%d-----", pTmpNode->data);
         else
            printf("------");
         pTmpNode = pTmpNode->next[0];
      }
      printf("NULL\n");
   }
   printf("---------------------------------\n");
}

void searchSkipNode(SkipNode* pHeadNode, int data)
{
   int pos = MAX_LEVEL - 1;
   SkipNode* pTmpNode = pHeadNode->next[pos];

   //현재 Level에 속한 노드가 없거나 data가 찾고자 하는 data 보다 클 경우 Level 감소
   while (pTmpNode == NULL || pTmpNode->data > data)
      pTmpNode = pHeadNode->next[--pos];

   printf("Search [%d] : ", data);
   while (pTmpNode->data != data)
   {
      //탐색을 하다가 Node가 끝나거나 찾고자 하는 data가 없을 경우 Level 감소
      if (pTmpNode->next[pos] == NULL || pTmpNode->next[pos]->data > data)
         pos -= 1;
      else
      {
         printf("%d, ", pTmpNode->data);
         pTmpNode = pTmpNode->next[pos];
         if (pTmpNode->data == data)
         {
            printf("%d\n", pTmpNode->data);
            return;
         }
      }
   }
   printf(",,, there is no %d\n", data);
   printf("---------------------------------\n");
}

int main() {
   //랜덤함수 시드 초기화
   srand(time(NULL));

   //Skip List 생성
   SkipNode* SkipList = (SkipNode*)malloc(sizeof(SkipNode));
   SkipList->level = MAX_LEVEL;
   SkipList->data = MIN_DATA;
   for (int i = 0; i < SkipList->level; i++)
      SkipList->next[i] = NULL;

   insertSkipNode(SkipList, 3);
   insertSkipNode(SkipList, 9);
   showSkipNode(SkipList);

   insertSkipNode(SkipList, 1);
   insertSkipNode(SkipList, 4);
   showSkipNode(SkipList);

   searchSkipNode(SkipList, 4);

   insertSkipNode(SkipList, 5);
   insertSkipNode(SkipList, 7);
   showSkipNode(SkipList);

   insertSkipNode(SkipList, 6);
   insertSkipNode(SkipList, 8);
   showSkipNode(SkipList);

   insertSkipNode(SkipList, 2);
   insertSkipNode(SkipList, 10);
   showSkipNode(SkipList);

   searchSkipNode(SkipList, 7);

}
