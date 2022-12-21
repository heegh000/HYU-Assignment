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

  if(symTab == NULL) {
    fprintf(listing, "Out of memory error at building symbol table %s", name);
  }

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


void st_insert(SymTable* symTab, TreeNode* astNode, int paramNum) {

  SymTable* targetTab;
  if(paramNum == VARIABLE) {
    targetTab = symTab;
  }
  else {
    targetTab = globalTab;
  }

  SymRec* rec = targetTab->head;
  SymRec* prevRec = NULL;

  while ((rec != NULL) && (strcmp(astNode->attr.name, rec->name) != 0)) {
    prevRec = rec;
    rec = rec->next;
  }

  if (rec == NULL) { 
    rec = (SymRec*) malloc(sizeof(SymRec));
    rec->name = astNode->attr.name;
    rec->type = astNode->type;
    rec->loc = targetTab->loc++;
    rec->scope = symTab;
    rec->lines = (LineNode*) malloc(sizeof(LineNode));
    rec->lines->lineno = astNode->lineno;
    rec->next = NULL;

    rec->paramNum = paramNum;

    if(prevRec == NULL) {
      targetTab->head = rec;
    }
    else {
      prevRec->next = rec;
    }
  }

  else { 
    LineNode* lineNode = rec->lines;
    while (lineNode->next != NULL) {
      lineNode = lineNode->next;
    } 
    lineNode->next = (LineNode*) malloc(sizeof(LineNode));
    lineNode->next->lineno = astNode->lineno;
    lineNode->next->next = NULL;
  }
}


SymRec* st_lookup_target_table (SymTable* symTab, char * name, int kind) { 
  SymRec* rec = symTab->head;

  if(kind == VARIABLE) {
    while ((rec != NULL) && ( (strcmp(name, rec->name) != 0) || ( strcmp(name, rec->name) == 0 && (rec->paramNum != VARIABLE) ) )) {
      rec = rec->next;
    }
  } 
  else {
    while ((rec != NULL) && ( (strcmp(name, rec->name) != 0) || ( strcmp(name, rec->name) == 0 && (rec->paramNum == VARIABLE) ) )) {
      rec = rec->next;
    }
  }

    
  if (rec == NULL) {
    return NULL;
  }
  else {
    return rec;
  }
}

SymRec* st_lookup (SymTable* symTab, char* name, int kind)  {
  SymRec* rec = symTab->head;

  if(kind == VARIABLE) {
    while ((rec != NULL) && ( (strcmp(name, rec->name) != 0) || ( strcmp(name, rec->name) == 0 && (rec->paramNum != VARIABLE) ) )) {
      rec = rec->next;
    }
  } 
  else {
    while ((rec != NULL) && ( (strcmp(name, rec->name) != 0) || ( strcmp(name, rec->name) == 0 && (rec->paramNum == VARIABLE) ) )) {
      rec = rec->next;
    }
  }


  if (rec == NULL) {
    if(symTab->parent == NULL) {
      return NULL;
    }
    else {
      return st_lookup(symTab->parent, name, kind);
    }
  }
  else {
    return rec;
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

      fprintf(listing,"Variable Name   type   Location   kind   Line Numbers\n");
      fprintf(listing,"-------------   ----   --------   ----   ------------\n");
      while (rec != NULL) {
        fprintf(listing,"%-16s", rec->name);

        if(rec->type == Integer) {
          fprintf(listing, "%-8s", "int");
        }
        else if(rec->type == Void) {
          fprintf(listing, "%-8s", "void");
        }
        else if(rec->type == IntegerArr) {
          fprintf(listing, "%-8s", "intArr");
        }
        fprintf(listing,"%-8d  ",rec->loc);
        lineNode = rec->lines;
        
        if(rec->paramNum == VARIABLE) {
          fprintf(listing,"%4s", "var");
        }
        else {
          fprintf(listing,"%4s", "func");
        }

        while (lineNode != NULL) { 
            fprintf(listing,"%4d ", lineNode->lineno);
            lineNode = lineNode->next;
        }



        fprintf(listing,"\n");
        rec = rec->next;
      }
      
      fprintf(listing,"\n\n");

      tab = tab->next;
    }

  }
}
