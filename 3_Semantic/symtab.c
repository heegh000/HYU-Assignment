/****************************************************/
/* File: symTab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "globals.h"

// for using when printing
SymTable* tabHead = NULL;
SymTable* tabTail = NULL;

SymTable* st_build(SymTable* parent, char* name) {
  SymTable* symTab = (SymTable*) malloc(sizeof(SymTable));
  symTab->name = name;
  symTab->loc = 0;
  symTab->parent = parent;

  if(tabTail == NULL) {
    tabHead = symTab;
    tabTail = symTab;
  }
  else {
    tabTail->next = symTab;
    tabTail = tabTail->next;
  }

  return symTab;
}


void st_insert(SymTable* symTab, char* name, ExpType type, int lineno) {
  SymRec* rec = symTab->head;
  SymRec* prevRec = NULL;

  while ((rec != NULL) && (strcmp(name, rec->name) != 0)) {
    prevRec = rec;
    rec = rec->next;
  }

  /* variable not yet in table */
  if (rec == NULL) { 
    rec = (SymRec*) malloc(sizeof(SymRec));
    rec->name = name;
    rec->loc = symTab->loc++;
    rec->scope = symTab;
    rec->lines = (LineNode*) malloc(sizeof(LineNode));
    rec->lines->lineno = lineno;
    rec->next = NULL;

     if(prevRec == NULL) {
       symTab->head = rec;
     }
     else {
       prevRec->next = rec;
     }

    // l = (BucketList) malloc(sizeof(struct BucketListRec));
    // l->name = name;
    // l->lines = (LineList) malloc(sizeof(struct LineListRec));
    // l->lines->lineno = lineno;
    // l->memloc = loc;
    // l->lines->next = NULL;
    // l->next = hashTable[h];
    // hashTable[h] = l; 
  }
  /* found in table, so just add line number */
  else { 
    LineNode* lineNode = rec->lines;
    while (lineNode->next != NULL) {
      lineNode = lineNode->next;
    } 
    lineNode->next = (LineNode*) malloc(sizeof(LineNode));
    lineNode->next->lineno = lineno;
    lineNode->next->next = NULL;
  }
}


/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup_cur_table (SymTable* symTab, char * name ) { 
  SymRec* rec = symTab->head;
  while ((rec != NULL) && (strcmp(name, rec->name) != 0)) {
    rec = rec->next;
  }
  
  if (rec == NULL) {
    return -1;
  }
  else {
    return rec->loc;
  }
}

SymTable* st_lookup (SymTable* symTab, char* name )  {
  SymRec* rec = symTab->head;
  while ((rec != NULL) && (strcmp(name, rec->name) != 0)) {
    rec = rec->next;
  }
  
  if (rec == NULL) {
    if(symTab->parent == NULL) {
      return NULL;
    }
    else {
      return st_lookup(symTab->parent, name);
    }
  }
  else {
    return rec->scope;
  }
}


/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE* listing) { 
  if(tabHead != NULL) {

    SymTable* tab = tabHead;
    SymRec* rec;
    LineNode* lineNode;

    while (tab != NULL){
      rec = tab->head;
      
      if(tab->name == NULL) {
        fprintf(listing,"Symbol Table\n");
      }
      else {
        fprintf(listing,"Symbol Table %s\n", tab->name);
      }

      fprintf(listing,"Variable Name  Location   Line Numbers\n");
      fprintf(listing,"-------------  --------   ------------\n");
      while (rec != NULL) {
        fprintf(listing,"%-14s", rec->name);
        fprintf(listing,"%-8d  ",rec->loc);
        lineNode = rec->lines;

        while (lineNode != NULL) { 
            fprintf(listing,"%4d ", lineNode->lineno);
            lineNode = lineNode->next;
        }
        fprintf(listing,"\n");
        rec = rec->next;
      }
      tab = tab->next;
    }
    


  //   // int i;
  //   // fprintf(listing,"Variable Name  Location   Line Numbers\n");
  //   // fprintf(listing,"-------------  --------   ------------\n");
    
  //   // for (i=0;i<SIZE;++i) { 
  //   //   if (hashTable[i] != NULL) { 
  //   //     BucketList l = hashTable[i];

  //   //     while (l != NULL) { 
  //   //       LineList t = l->lines;
  //   //       fprintf(listing,"%-14s ",l->name);
  //   //       fprintf(listing,"%-8d  ",l->memloc);
          
  //   //       while (t != NULL) { 
  //   //         fprintf(listing,"%4d ",t->lineno);
  //   //         t = t->next;
  //   //       }
          
  //   //       fprintf(listing,"\n");
  //   //       l = l->next;
  //   //     }
  //   //   }
  //   }
  // }

  }
}
