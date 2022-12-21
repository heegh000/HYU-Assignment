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

SymTable* peekSymStack() {
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
static void nullProc(TreeNode * t) { 
  if (t==NULL) {
    return;
  }
  else {
    return;
  }
}

static void scopeEnd(TreeNode * t) {
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

static void printError(TreeNode * t, int error) {
  switch (error) {
    case 0:
      fprintf(listing, "Error: Undeclared function \"%s\" is called at line %d\n", t->attr.name, t->lineno);
      break;
    case 1:
      fprintf(listing, "Error: Undeclared variable \"%s\" is used at line %d\n", t->attr.name, t->lineno);
      break;
    case 2:
      fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d\n", t->attr.name, t->lineno);
      break;
    case 3:
      fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). Indices should be integer\n", t->lineno, t->attr.name);
      break;
    case 4:
      fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). Indexing can only be allowed for int[] variables\n", t->lineno, t->attr.name);
      break;
    case 5:
      fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
      break;
    case 6:
      fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
      break;
    case 7:
      fprintf(listing, "Error: Invalid operation at line %d\n", t->lineno);
      break;
    case 8:
      fprintf(listing, "Error: Invalid assignment at line %d\n", t->lineno);
      break;
    case 9:
      fprintf(listing, "Error: Invalid condition at line %d\n", t->lineno);
      break;
    case 10:
      fprintf(listing, "Error: Invalid return at line %d\n", t->lineno);
      break;
    default:
      fprintf(listing, "Error: Unknown Error\n");
      break;
  }
  Error = TRUE;
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
          if(st_lookup_target_table(peekSymStack(), t->attr.name, VARIABLE) == NULL) {
            st_insert(peekSymStack(), t, -1);
          }
          else {
            printError(t, 2);
          }
          break;
        case FuncK:
          if(st_lookup_target_table(peekSymStack(), t->attr.name, 0) == NULL) {
            alreadyCreated = TRUE;

            funcName = t->attr.name;

            SymTable* newSymTab = st_build(peekSymStack(), t->attr.name);
            
            TreeNode* param = t->child[0]; 
            int paramNum = 0;
            if(param->kind.decl != ParamVoidK) {
              while(param != NULL) {
                paramNum++;
                param = param->sibling;
              }
            }
            printf("ASDSAD: %s, %d\n", t->attr.name, paramNum);

            st_insert(newSymTab, t, paramNum);
            pushSymStack(newSymTab);
          }
          else {
            printError(t, 2);
          }
          break;
        case ParamK:
          if(st_lookup_target_table(peekSymStack(), t->attr.name, VARIABLE) == NULL) {
            st_insert(peekSymStack(), t, -1);
          }
          else {
            printError(t, 2);
          }
          break;
        case ParamVoidK:
          break;
        default:
          break;
      }

      break;

    case StmtK:
      switch (t->kind.stmt) { 
        case CompK:
          if(!alreadyCreated)  {
            SymTable* newSymTab = st_build(peekSymStack(), NULL);
            pushSymStack(newSymTab);
          }
          alreadyCreated = FALSE;
          break;
        case IfK:
        case IfElseK:
        case WhileK:
          break;
        case ReturnK:
          t->attr.name = funcName;
          break;
        case ReturnNonK:
          t->attr.name = funcName;
          break;
        default:
          break;
      }

      break;

    case ExpK:
      switch (t->kind.exp) { 
        case VarAccessK: ;
          rec = st_lookup(peekSymStack(), t->attr.name, VARIABLE);
          t->curScope = peekSymStack();
          if(rec != NULL) {
            st_insert(rec->scope, t, -1);
          }
          else {
            t->type = Undefined;
          }
          break;
      
        case CallK: ;
          rec = st_lookup_target_table(globalTab, t->attr.name, FUNCTION);
          t->curScope = peekSymStack();
          if(rec != NULL) {
            st_insert(globalTab, t, -1);
          }
          else {
            t->type = Undefined;
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
  SymTable* newSymTab = st_build(peekSymStack(), "global");
  globalTab = newSymTab;
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
  inputFunc->curScope = peekSymStack();
  SymTable* inputSymTab = st_build(peekSymStack(), inputFunc->attr.name);

  

  outputFunc->sibling = NULL;
  outputFunc->lineno = 0;
  outputFunc->attr.name = "output";
  outputFunc->type = Void;
  outputFunc->curScope = peekSymStack();


  SymTable* outputSymTab = st_build(peekSymStack(), outputFunc->attr.name);
  SymRec* outputParam = (SymRec*) malloc(sizeof(SymRec));
  outputParam->name = "value";
  outputParam->type = Integer;
  outputParam->loc = 0;
  outputParam->scope = outputSymTab;
  outputParam->paramNum = -1;
  outputParam->next = NULL;

  outputSymTab->head = outputParam;
  st_insert(inputSymTab, inputFunc, 0);
  st_insert(outputSymTab, outputFunc, 1);

  traverse(syntaxTree,insertNode, scopeEnd);

  if (TraceAnalyze) { 
    fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
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
            printError(t, 6);
          }
          else if(t->type == IntegerArr) {
            if(t->child[0]->type != Integer) {
              printError(t, 3);
            }
          }
          break;
        case FuncK:
          break;
        case ParamK:
          if(t->type == Void) {
            printError(t, 6);
          }
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
          rec = st_lookup_target_table(globalTab, t->attr.name, FUNCTION);
          if(rec->type != t->child[0]->type) {
            printError(t, 10);
          }
          break;
        case ReturnNonK:
          rec = st_lookup_target_table(globalTab, t->attr.name, FUNCTION);
          if(rec->type != Void) {
            printError(t, 10);
          }
          break;
        default:
          break;
      }
      
      break;

    case ExpK:
      switch (t->kind.exp) { 
        case VarAccessK: ;
          //There is no variable looking for
          if(t->type == Undefined) {
            printError(t, 1);
          }
          //There is the variable looking for
          else {
            rec = st_lookup(t->curScope, t->attr.name, VARIABLE);
            
            //Access integer
            if(t->child[0] == NULL) { 
              t->type = rec->type;
            } 
            //Access integer array
            else {
              //the variable is not array
              if(rec->type != IntegerArr) {
                printError(t, 4);
                t->type = Integer; //7p 1번 예시
              }
              //index is not integer
              else if(t->child[0]->type != Integer) {
                  printError(t,3);
                  t->type = Integer; //7p 1번 예시
              }
              else {
                t->type = Integer;
              }
            }
          }
          break;
        case CallK: ;
          if(t->type == Undefined) {
            printError(t, 0);
          }
          else {
            rec = st_lookup_target_table(globalTab, t->attr.name, FUNCTION);
            TreeNode* arg = t->child[0];

            //There is no argument
            if(arg == NULL) {
              if(rec->paramNum == 0) {
                t->type = rec->type;
              }
              else {
                printError(t, 5);
                t->type = rec->type; // 7p 1번 예시
              }
            }

            else {
              int argsNum = 0;

              while (arg != NULL) {
                argsNum ++;
                arg = arg->sibling;
              }

              //# of Argument is not matched
              if(argsNum != rec->paramNum) {
                printError(t, 5);
                t->type = rec->type; // 7p 1번 예시
              }

              else {
                int isMatched = TRUE;
                arg = t->child[0];

                SymRec* param = rec->scope->head;

                while(arg != NULL) {
                  if(param->type != arg->type) {
                    isMatched = FALSE;
                  }
                  param = param->next;
                  arg = arg->sibling;
                }

                if(isMatched) {
                  t->type = rec->type;
                }
                else {
                  printError(t, 5);
                  t->type = rec->type; // 7p 1번 예시
                }

              }
            }
          }
          break;
        case AssignK:
          if(t->child[0]->type != t->child[1]->type) {
            printError(t, 8);
          }
          else {
            t->type = t->child[0]->type;
          }
          break;
        case OpK:
          if(t->child[0]->type != Integer || t->child[1]->type != Integer) {
            printError(t, 7);
            //t->type = Integer; //메일 질문 4번에 의한 주석처리
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
