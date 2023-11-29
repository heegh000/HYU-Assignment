/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

typedef struct LineNode_ LineNode;
typedef struct SymRec_ SymRec;
typedef struct SymTable_ SymTable;

struct LineNode_{ 
  int lineno;
  LineNode* next;
};

struct SymRec_{ 
  char * name;
  ExpType type;
  int loc;
  // If var, a scope where the var is defined, 
  // If func, a scope of func 
  SymTable* scope;
  LineNode* lines;

  int paramNum;

  SymRec* next;
};

struct SymTable_{  
  char* name;
  int loc;
  SymRec* head;
  SymTable* parent;
  SymTable* next; // for using when printing
};


SymTable* globalTab;

SymTable* st_build(SymTable*, char*);
void st_insert(SymTable*, TreeNode*, int);
SymRec* st_lookup_target_table(SymTable*, char *, int);
SymRec* st_lookup(SymTable*, char *, int);
  
void printSymTab(FILE * listing);

#endif
