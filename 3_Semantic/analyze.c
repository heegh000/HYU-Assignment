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

static void scopeEnd(TreeNode * t) {
  if(t->nodekind == StmtK && t->kind.stmt ==CompK) {
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
static void insertNode( TreeNode * t)
{ switch (t->nodekind) { 
    case DeclK:
      switch (t->kind.decl) {
        case VarK:
          if(st_lookup_cur_table(topSymStack(), t->attr.name) == -1) {
            printf("var name: %s, scope: %s\n", t->attr.name, topSymStack()->name);
            st_insert(topSymStack(), t->attr.name, t->type, t->lineno);
          }
          else {
            //Error?
          }
          break;
        case FuncK:
          alreadyCreated = 1;
          st_insert(topSymStack(), t->attr.name, t->type, t->lineno);
          printf("func name: %s, scope: %s\n", t->attr.name, topSymStack()->name);
          SymTable* newSymTab = st_build(topSymStack(), t->attr.name);
          pushSymStack(newSymTab);
          break;
        case ParamK:
          printf("param name: %s, scope: %s\n", t->attr.name, topSymStack()->name);
          st_insert(topSymStack(), t->attr.name, t->type, t->lineno);
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
          break;
        case ReturnNonK:
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp) { 
        case IdK:
        case CallK: ;
          SymTable* symTab = st_lookup(topSymStack(), t->attr.name);
          if(symTab != NULL) {
            st_insert(symTab, t->attr.name, t->type, t->lineno);
          }
          else {
            //Error
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
  SymTable* newSymTab = st_build(topSymStack(), "global");
  pushSymStack(newSymTab);
  traverse(syntaxTree,insertNode,scopeEnd);

  if (TraceAnalyze) { 
    fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message) { 
  fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case ExpK:
      switch (t->kind.exp)
      { case OpK:
          if ((t->child[0]->type != Integer) ||
              (t->child[1]->type != Integer))
            typeError(t,"Op applied to non-integer");
          if ((t->attr.op == EQ) || (t->attr.op == LT))
          //   t->type = Boolean;
          // else
          //   t->type = Integer;
          break;
        case ConstK:
        case IdK:
          t->type = Integer;
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case IfK:
          if (t->child[0]->type == Integer)
            typeError(t->child[0],"if test is not Boolean");
          break;
        case AssignK:
          if (t->child[0]->type != Integer)
            typeError(t->child[0],"assignment of non-integer value");
          break;
        // case WriteK:
        //   if (t->child[0]->type != Integer)
        //     typeError(t->child[0],"write of non-integer value");
        //   break;
        // case RepeatK:
        //   if (t->child[1]->type == Integer)
        //     typeError(t->child[1],"repeat test is not Boolean");
        //   break;
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
