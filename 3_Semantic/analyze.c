/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"


typedef struct StackNode_ StackNode;

struct StackNode_{
  SymTable* symTab;
  StackNode* next; 
};

StackNode* symStack;
int alreadyCreated;
char* funcName;


void pushSymStack(SymTable* symTab) {
  StackNode* newNode = (StackNode*) malloc(sizeof(StackNode));
  newNode->symTab= symTab;
  newNode->next = symStack;
  symStack = newNode;
}

SymTable* popSymStack() {
  if(symStack != NULL) {
    SymTable* retTab = symStack->symTab;

    StackNode* temp = symStack;
    symStack = symStack->next;
    free(temp);

    return retTab;
  }
  return NULL;
}

SymTable* topSymStack() {
  if(symStack == NULL) {
    return NULL;
  }
  else {
    return symStack->symTab;
  }
}


/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t, void (* preProc) (TreeNode *), void (* postProc) (TreeNode *) ) { 
  if (t != NULL) { 
    preProc(t);
    //{ 
    int i;
    for (i=0; i < MAXCHILDREN; i++) {
      traverse(t->child[i],preProc,postProc);
    }
    //}
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) {
    return;
  }
  else {
    return;
  }
}

static void insertEnd(TreeNode * t) {
  if(t->nodekind == DeclK && t->kind.decl == FuncK) {
    funcName = NULL;
  }
  else if(t->nodekind == StmtK && t->kind.stmt ==CompK) {
    popSymStack();
  }
  else {
    return ;
  }
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t) { 

  SymRec* rec;

  switch (t->nodekind) { 
    case DeclK:
      switch (t->kind.decl) {
        case VarK:
          if(st_lookup_cur_table(topSymStack(), t->attr.name, -1) == -1) {
            printf("%s %d\n", t->attr.name, t->type);
            st_insert(topSymStack(), t, -1, NULL);
          }
          else {
            //Error?
          }
          break;
        case FuncK:
          if(st_lookup_cur_table(topSymStack(), t->attr.name, 0) == -1) {
            alreadyCreated = 1;
            funcName = t->attr.name;

            SymTable* newSymTab = st_build(topSymStack(), t->attr.name);
            
            TreeNode* param = t->child[0]; 
            int paramNum = 0;

            while(param != NULL) {
              paramNum++;
              param = param->sibling;
            }

            st_insert(topSymStack(), t, paramNum, newSymTab);
            pushSymStack(newSymTab);
          }
          else {
            //Error?
          }
          break;
        case ParamK:
          if(st_lookup_cur_table(topSymStack(), t->attr.name, -1) == -1) {
            st_insert(topSymStack(), t, -1, NULL);
          }
          else {
            //Error?
          }
          break;
        case ParamVoidK:
          break;
        default:
          break;
      }
    case StmtK:
      switch (t->kind.stmt) { 
        case CompK:
          if(!alreadyCreated)  {
            alreadyCreated = 0;
            SymTable* newSymTab = st_build(topSymStack(), t->attr.name);
            pushSymStack(newSymTab);
          }
          break;
        case IfK:
          break;
        case IfElseK:
          break;
        case WhileK:
          break;
        case ReturnK:
          t->attr.name = funcName;
          t->curTop = topSymStack();
          break;
        case ReturnNonK:
          t->attr.name = funcName;
          t->curTop = topSymStack();
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp) { 
        case VarAccessK: ;
          rec = st_lookup(topSymStack(), t->attr.name, -1);
          t->curTop = topSymStack();
          if(rec != NULL) {
            st_insert(rec->scope, t, -1, NULL);
          }
          break;
        case CallK: ;
          rec = st_lookup(topSymStack(), t->attr.name, 0);
          t->curTop = topSymStack();
          if(rec != NULL) {
            st_insert(rec->scope, t, -1, NULL);
          }
          break;
        case AssignK:
          break;
        case OpK:
          break;
        case ConstK:
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree) { 

  // Make Gloable Scope
  SymTable* newSymTab = st_build(topSymStack(), "global");
  pushSymStack(newSymTab);

  // Make Built-in function

  TreeNode* inputFunc = (TreeNode*) malloc(sizeof(TreeNode));

  if(inputFunc == NULL) {
    fprintf(listing, "Out of memory error at malloc input function\n");
  }

  TreeNode* outputFunc = (TreeNode*) malloc(sizeof(TreeNode));

  if(outputFunc == NULL) {
    fprintf(listing, "Out of memory error at ouput function\n");
  }

  for(int i = 0; i < MAXCHILDREN; i++) {
    inputFunc->child[i] = NULL;
    outputFunc->child[i] = NULL;
  }

  inputFunc->sibling = NULL;
  inputFunc->lineno = 0;
  inputFunc->attr.name = "input";
  inputFunc->type = Integer;
  inputFunc->curTop = topSymStack();
  SymTable* inputSymTab = st_build(topSymStack(), inputFunc->attr.name);

  

  outputFunc->sibling = NULL;
  outputFunc->lineno = 0;
  outputFunc->attr.name = "output";
  outputFunc->type = Void;
  outputFunc->curTop = topSymStack();


  SymTable* outputSymTab = st_build(topSymStack(), outputFunc->attr.name);
  SymRec* outputParam = (SymRec*) malloc(sizeof(SymRec));
  outputParam->name = "value";
  outputParam->type = Integer;
  outputParam->loc = 0;
  outputParam->scope = outputSymTab;
  outputParam->paramNum = 0;
  outputParam->funcScope = NULL;
  outputParam->next = NULL;

  outputSymTab->head = outputParam;
  st_insert(topSymStack(), inputFunc, 0, inputSymTab);
  st_insert(topSymStack(), outputFunc, 1, outputSymTab);

  traverse(syntaxTree,insertNode,insertEnd);

  if (TraceAnalyze) { 
    fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void printError(TreeNode * t, int error) {
  switch (error) {
    case 0:
      fprintf(listing, "Error: undeclared function \"%s\" is called at line %d\n", t->attr.name, t->lineno);
      break;
    case 1:
      fprintf(listing, "Error: undeclared variable \"%s\" is used at line %d\n", t->attr.name, t->lineno);
      break;
    case 2:
      fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
      break;
    case 3:
      fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indicies should be integer\n", t->lineno, t->attr.name);
      break;
    case 4:
      fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indexing can only allowed for int[] variables\n", t->lineno, t->attr.name);
      break;
    case 5:
      fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
      break;
    case 6:
      fprintf(listing, "Error: Invalid return at line %d\n", t->lineno);
      break;
    case 7:
      fprintf(listing, "Error: invalid assignment at line %d\n", t->lineno);
      break;
    case 8:
      fprintf(listing, "Error: invalid operation at line %d\n", t->lineno);
      break;
    case 9:
      fprintf(listing, "Error: invalid condition at line %d\n", t->lineno);
      break;
    case 10:
      fprintf(listing, "Error: redeclared\n");
      break;
    default:
      fprintf(listing, "Error: Unknown Error\n");
      break;
  }
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t) { 
  SymRec* rec;

  switch (t->nodekind) { 
    case DeclK:
      switch (t->kind.decl) {
        case VarK:
          if(t->type == Void) {
            printError(t, 2);
          }
          else if(t->type == IntegerArr) {
            if(t->child[0]->type != Integer) {
              printError(t, 3);
            }
          }
          break;
        case FuncK:
        case ParamK: 
        case ParamVoidK:
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt) { 
        case CompK:
          break;
        case IfK:
        case IfElseK:
        case WhileK:
          if(t->child[0]->type != Integer) {
            printError(t, 9);
          }
          break;
        case ReturnK:
          rec = st_lookup(t->curTop, t->attr.name, 0);
          if(rec->type != t->child[0]->type) {
            printError(t, 6);
          }
          break;
        case ReturnNonK:
          rec = st_lookup(t->curTop, t->attr.name, 0);
          if(rec->type != Void) {
            printError(t, 6);
          }

          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp) { 
        case VarAccessK: ;
          rec = st_lookup(t->curTop, t->attr.name, -1);
          if(rec == NULL) {
            printError(t, 1);
          }
          else {
            if(t->child[0] == NULL) { 
              t->type = rec->type;
            } 
            else {
              if(rec->type != IntegerArr) {
                printError(t, 4);
              }
              else if(t->child[0]->type != Integer) {
                  printError(t,3);
              }
              else {
                t->type = Integer;
              }
            }
          }
          break;
        case CallK: ;
          rec = st_lookup(t->curTop, t->attr.name, 0);
          if(rec == NULL) {
            printError(t, 0);
          }
          else {
            TreeNode* arg = t->child[0];

            if(arg == NULL) {
              if(rec->paramNum == 0) {
                t->type = rec->type;
              }
              else {
                printError(t, 5);
              }
            }

            else {
              int argsNum = 0;

              while (arg != NULL) {
                argsNum ++;
                arg = arg->sibling;
              }

              if(argsNum != rec->paramNum) {
                printError(t, 5);
              }
              else {
                int isMatched = 1;
                arg = t->child[0];
                SymRec* param = rec->funcScope->head;
                while(arg != NULL) {
                  if(param->type != arg->type) {
                    isMatched = 0 ;
                  }
                  param = param->next;
                  arg = arg->sibling;
                }
                if(!isMatched) {
                  printError(t, 5);
                }
                else {
                  t->type = rec->type;
                }

              }
            }

          }
          break;
        case AssignK:
          if(t->child[0]->type != t->child[1]->type) {
            printError(t, 7);
          }
          else {
            t->type = t->child[0]->type;
          }
          break;
        case OpK:
          if(t->child[0]->type != Integer || t->child[1]->type != Integer) {
            printError(t, 8);
          }
          else {
            t->type = Integer;
          }
          break;

        case ConstK:
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree) { 
  traverse(syntaxTree,nullProc,checkNode);
}
