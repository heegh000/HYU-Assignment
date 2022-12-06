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
  struct LineNode_* next;

};


struct SymRec_{ 
  char * name;
  ExpType type;
  int loc;
  SymTable* scope; 
  LineNode* lines;

  int paramNum;
  SymTable* funcScope; 

  SymRec* next;

};

struct SymTable_{  
  char* name;
  int loc;
  SymRec* head;
  SymTable* parent;
  SymTable* next; // for using when printing

};

SymTable* st_build(SymTable*, char*);

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert(SymTable*, TreeNode*, int, SymTable*);
/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup_cur_table(SymTable*, char *, int);
SymRec* st_lookup(SymTable*, char *, int);
  
/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);

#endif
