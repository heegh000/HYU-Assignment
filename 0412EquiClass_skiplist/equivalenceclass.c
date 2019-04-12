#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 24
#define FALSE 0
#define TRUE 1

//구조체 선언
typedef struct node *nodePointer;
typedef struct node {
   int data;
   nodePointer link;
}node;

int main() {
   int out[MAX_SIZE];
   nodePointer seq[MAX_SIZE];
   nodePointer x, y, top;
   int i, j, n;

   printf("Enter the size (<= %d) ", MAX_SIZE);
   scanf("%d", &n);
   for (i = 0; i < n; i++) {

      out[i] = TRUE;
      seq[i] = NULL;
   }

   printf("Enter a pair of numbers (-1 -1 to quit): ");
   scanf("%d%d", &i, &j);
   while (i >= 0)
   {
      x = (nodePointer)malloc(sizeof(node));
      x->data = j;
      x->link = seq[i];
      seq[i] = x;

      x = (nodePointer)malloc(sizeof(node));
      x->data = i;
      x->link = seq[j];
      seq[j] = x;

      printf("Enter a pair of numbers (-1 -1 to quit): ");
      scanf("%d%d", &i, &j);
   }

   for (i = 0; i < n; i++)
      if (out[i]) {
         printf("\nNew class: %5d", i);

         //클래스 i를 탐색
         out[i] = FALSE;
         x = seq[i];
         top = NULL;
         for (;;) {
            while (x) {
               j = x->data;
               if (out[j]) {
                  printf("%5d", j);
                  out[j] = FALSE;
                  y = x->link;
                  x->link = top;
                  top = x;
                  x = y;
               }
               else
                  x = x->link;
            }
            //top이 FALSE이면 전부 탐색한것
            if (!top)
               break;
            //위치를 저장해뒀던 top으로 이동
            x = seq[top->data];
            top = top->link;
         }
      }
   printf("\n");

}
