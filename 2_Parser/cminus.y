/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
//static char * savedName; /* for use in assignments */
//static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

%}
%nonassoc THEN
%nonassoc IF ELSE WHILE RETURN INT VOID
%nonassoc ID NUM 
%left COMMA
%right ASSIGN
%left EQ NE
%left LT LE GT GE
%left PLUS MINUS
%left TIMES OVER
%left LPAREN RPAREN LBRACE RBRACE
%nonassoc  LCURLY RCURLY SEMI
%nonassoc ERROR 

%% /* Grammar for C-MINUS */

// Rule1
program     : decl_list
                { 
                    savedTree = $1;
                }
            | error
                {
                    savedTree = NULL;
                }
            ;

// Rule2
decl_list   : decl_list decl
                {
                    YYSTYPE t = $1;
                    if(t != NULL) {
                        while(t->sibling != NULL) {
                            t = t->sibling;
                        }

                        t->sibling = $2;
                        $$ = $1;
                    }
                    else {
                        $$ = $2;
                    }
                }
            | decl
                {
                    $$ = $1;
                }
            ;

// Rule3
decl        : var_decl
                {
                    $$ = $1;
                }
            | func_decl
                {
                    $$ = $1;
                }
            ;

// Rule4
var_decl    : type_spec identifier SEMI
                {
                    $$ = $1;
                    $$->attr.name = copyString($2->attr.name);
                    $$->lineno = $2->lineno;
                    free($2);
                    $$->kind.decl = VarK;
                }
            | type_spec identifier LBRACE const_val RBRACE
                {
                    $$ = $1;
                    $$->attr.name = copyString($2->attr.name);
                    $$->lineno = $2->lineno;
                    free($2);
                    $$->kind.decl = VarK;
                    $$->child[0] = $4;
                    if($$->type == Integer) {
                        $$->type = IntegerArr;
                    }
                    else {
                        $$->type = VoidArr;
                    }
                }
            ;

// Rule5
type_spec   : INT
                {
                    $$ = newDeclNode();
                    $$->type = Integer;
                }
            | VOID
                {
                    $$ = newDeclNode();
                    $$->type = Void;  
                }
            ;

// Rule6
func_decl   : type_spec identifier LPAREN params RPAREN comp_stmt
                {
                    $$ = $1;
                    $$->attr.name = copyString($2->attr.name);
                    $$->lineno = $2->lineno;
                    free($2);
                    $$->kind.decl = FuncK;
                    $$->child[0] = $4;
                    $$->child[1] = $6;
                }
            ;

// Rule7
params      : params_list 
                {
                    $$ = $1;
                }
            | VOID
                {
                    $$ = newDeclNode();
                    $$->kind.decl = ParamVoidK;
                }
            ;

// Rule8
params_list : params_list COMMA param
                {
                    YYSTYPE t = $1;
                    if(t != NULL) {
                        while(t->sibling != NULL) {
                            t = t->sibling;
                        }

                        t->sibling = $3;
                        $$ = $1;
                    }
                    else {
                        $$ = $3;
                    }
                }
            | param
                {
                    $$ = $1;
                }
            ;

// Rule9
param       : type_spec identifier
                {
                    $$ = $1;
                    $$->attr.name = copyString($2->attr.name);
                    $$->lineno = $2->lineno;
                    free($2);
                    $$->kind.decl = ParamK;
                }
            | type_spec identifier LBRACE RBRACE
                {
                    $$ = $1;
                    $$->attr.name = copyString($2->attr.name);
                    $$->lineno = $2->lineno;
                    free($2);
                    $$->kind.decl = ParamK;
                    if($$->type == Integer) {
                        $$->type = IntegerArr;
                    }
                    else {
                        $$->type = VoidArr;
                    }

                }
            ;

// Rule10
comp_stmt   : LCURLY locl_decl stmt_list RCURLY
                {
                    $$ = newStmtNode(CompK);
                    $$->child[0] = $2;
                    $$->child[1] = $3;
                }
            ;

// Rule11
locl_decl   : locl_decl var_decl 
                {
                    YYSTYPE t = $1;
                    if(t != NULL) {
                        while(t->sibling != NULL) {
                            t = t->sibling;
                        }

                        t->sibling = $2;
                        $$ = $1;
                    }
                    else {
                        $$ = $2;
                    }
                }
            |
                {
                    $$ = NULL;
                }
            ;

// Rule12
stmt_list   : stmt_list stmt 
                {
                    YYSTYPE t = $1;
                    if(t != NULL) {
                        while(t->sibling != NULL) {
                            t = t->sibling;
                        }
                            t->sibling = $2;
                            $$ = $1;
                    }
                    else {
                        $$ = $2;
                    }
                }
            |
                {
                    $$ = NULL;
                }
            ;

// Rule13
stmt        : exp_stmt
                {
                    $$ = $1;
                }
            | comp_stmt
                {
                    $$ = $1;
                }
            | selec_stmt
                {
                    $$ = $1;
                }
            | iter_stmt
                {
                    $$ = $1;
                }
            | return_stmt
                {
                    $$ = $1;
                }
            ;

// Rule14
exp_stmt    : exp SEMI 
                {
                    $$ = $1;
                }
            | SEMI
                {
                    $$ = NULL;
                }
            ;

// Rule15
selec_stmt  : IF LPAREN exp RPAREN stmt %prec THEN
                {
                    $$ = newStmtNode(IfK);
                    $$->child[0] = $3;
                    $$->child[1] = $5;
                }
            
            | IF LPAREN exp RPAREN stmt ELSE stmt
                {
                    $$ = newStmtNode(IfElseK);
                    $$->child[0] = $3;
                    $$->child[1] = $5;
                    $$->child[2] = $7;
                }
            ;
        
// Rule16
iter_stmt   : WHILE LPAREN exp RPAREN stmt
                {
                    $$ = newStmtNode(WhileK);
                    $$->child[0] = $3;
                    $$->child[1] = $5;
                }
            ;

// Rule17
return_stmt : RETURN SEMI
                {
                    $$ = newStmtNode(ReturnNonK);
                    // $$->type = VOID;
                }
            | RETURN exp SEMI
                {
                    $$ = newStmtNode(ReturnK);
                    $$->child[0] = $2;
                    // $$->type = $2->type;
                }
            ;

// Rule18
exp         : var ASSIGN exp 
                {
                    $$ = newExpNode(AssignK);
                    $$->lineno = $1->lineno;
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                }
            | simple_exp
                {
                    $$ = $1;
                }
            ;
// Rule19
var         : identifier
                {
                    $$ = $1;
                }
            | identifier LBRACE exp RBRACE
                {
                    $$ = $1;
                    $$->child[0] = $3;
                }
            ;

// Rule20
simple_exp  : addtive_exp relop addtive_exp 
                {
                    $$ = $2;
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                }
            | addtive_exp
                {
                    $$ = $1;
                }
            ;

// Rule21
relop       : EQ
                {
                    $$ = newExpNode(OpK);
                    $$->attr.op = EQ;
                }
            | NE
                {
                    $$ = newExpNode(OpK);
                    $$->attr.op = NE;
                }
            | LT
                {
                    $$ = newExpNode(OpK);
                    $$->attr.op = LT;
                }
            | LE
                {
                    $$ = newExpNode(OpK);
                    $$->attr.op = LE;
                }   
            | GT
                {
                    $$ = newExpNode(OpK);
                    $$->attr.op = GT;
                }
            | GE 
                {
                    $$ = newExpNode(OpK);
                    $$->attr.op = GE;
                }
            ;

// Rule22
addtive_exp : addtive_exp addop term 
                {
                    $$ = $2;
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                }
            | term
                {
                    $$ = $1;
                }
            ;

// Rule23
addop       : PLUS
                {
                    $$ = newExpNode(OpK);
                    $$->attr.op = PLUS;
                }
            | MINUS
                {
                    $$ = newExpNode(OpK);
                    $$->attr.op = MINUS;
                }
            ;

// Rule24
term        : term mulop factor 
                {
                    $$ = $2;
                    $$->child[0] = $1;
                    $$->child[1] = $3;
                }
            | factor
                {
                    $$ = $1;
                }
            ;

// Rule25
mulop       : TIMES
                {
                    $$ = newExpNode(OpK);
                    $$->attr.op = TIMES;
                }
            | OVER
                {
                    $$ = newExpNode(OpK);
                    $$->attr.op = OVER;
                }
            ;

// Rule26
factor      : LPAREN exp RPAREN
                {
                    $$ = $2;
                }
            | var
                {
                    $$ = $1;
                }
            | call
                {
                    $$ = $1;
                }
            | const_val
                {
                    $$ = $1;
                }
            ;

// Rule27
call        : identifier LPAREN args RPAREN
                {
                    $$ = $1;
                    $$->attr.name = copyString($1->attr.name);
                    $$->kind.exp = CallK;
                    $$->child[0] = $3;
                }
            ;

// Rule28
args        : arg_list
                {
                    $$ = $1;
                }
            |
                {
                    $$ = NULL;
                }
            ;

// Rule29
arg_list    : arg_list COMMA exp 
                {
                    YYSTYPE t = $1;
                    if(t != NULL) {
                        while(t->sibling != NULL) {
                            t = t->sibling;
                        }
                            t->sibling = $3;
                            $$ = $1;
                    }
                    else {
                        $$ = $3;
                    }
                }
            | exp
                {
                    $$ = $1;
                }
            ;


// Rule30
identifier  : ID 
                {
                    $$ = newExpNode(IdK);
                    $$->attr.name = copyString(tokenString);
                }            
            ;


// Rule31
const_val   : NUM
                {
                    $$ = newExpNode(ConstK);
                    $$->attr.val = atoi(tokenString);
                    $$->type = INT;
                }
            ;
%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

