/* Definition section */
%{
    #include "compiler_common.h"
    #include "main.h"
    
    int yydebug = 1;

    void yyerror (char const *s) {
        printf("error:%d:%d %s\n", yylineno, yyleng, s);
        compileError = true;
    }

%}

/* Variable or self-defined structure */
%union {
    ObjectType var_type;

    bool b_var;
    int i_var;
    float f_var;
    char *s_var;

    Object object_val;
}

/* Token without return */
%token COUT
%token GEQ LEQ EQL NEQ LOR LAND
%token ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN REM_ASSIGN
%token IF ELSE FOR WHILE LOOP
%token RETURN BREAK
%token ARROW AS IN RSHIFT LSHIFT

/* Token with return, which need to sepcify type */
%token <var_type> VARIABLE_T
%token <b_var> BOOL_LIT
%token <i_var> INT_LIT
%token <f_var> FLOAT_LIT
%token <s_var> STRING_LIT
%token <s_var> VARIABLE

/* Nonterminal with return, which need to sepcify type */
%type <object_val> Expression
%type <object_val> ValueStmt

%left LOR
%left LAND
%left '>' '<' EQL NEQ LEQ GEQ
%left LSHIFT RSHIFT
%left '+' '-'
%left '*' '/' '%'

/* %nonassoc IFX
%nonassoc ELSE */

/* Yacc will start at this nonterminal */
%start Program

%%
/* Grammar section */

Program
    : StmtList
    | // Empty file
;

StmtList 
    : StmtList Stmt
    | Stmt
;

Stmt
    : ScopeStmt
    | VARIABLE_T VARIABLE '(' FunctionVariableStmtList ')' { createMethod($<var_type>1, $<s_var>2); } ScopeStmt
    | VARIABLE '(' FunctionVariableStmtList ')' ';'
    | VARIABLE_T VARIABLE ASSIGN Expression ';'
    | COUT LSHIFT STRING_LIT ';'
    | ';'
;

ScopeStmt
    : '{' StmtList '}'
    | '{' '}'
;

FunctionVariableStmtList 
    : FunctionVariableStmtList ',' FunctionVariableStmt
    | FunctionVariableStmt
    |
;
FunctionVariableStmt
    : VARIABLE_T VARIABLE
    | VARIABLE_T VARIABLE '[' ']'
;

Expression
    : ValueStmt
;

ValueStmt
    : BOOL_LIT { $$ = (Object){OBJECT_TYPE_BOOLEAN, (uint64_t)$<b_var>1}; printf("bool %s\n", $<b_var>1?"true":"false"); }
    | FLOAT_LIT { $$ = (Object){OBJECT_TYPE_FLOAT, *((uint64_t*)&$<f_var>1)}; printf("FLOAT_LIT %f\n", $<f_var>1); }
    | INT_LIT { $$ = (Object){OBJECT_TYPE_INT, (uint64_t)$<i_var>1}; printf("INT_LIT %d\n", $<i_var>1); }
    | STRING_LIT { $$ = (Object){OBJECT_TYPE_STRING, (uint64_t)$<s_var>1}; printf("STRING_LIT \"%s\"\n", $<s_var>1); }
;

%%
/* C code section */