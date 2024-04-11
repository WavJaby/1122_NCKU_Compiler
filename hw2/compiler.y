/* Definition section */
%{
    #include "compiler_common.h"
    #include "compiler_util.h"
    #include "main.h"

    int yydebug = 1;
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
%token SHR SHL BAN BOR BNT BXO ADD SUB MUL DIV MOD NOT GTR LES GEQ LEQ EQL NEQ LAN LOR
%token VAL_ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN REM_ASSIGN SHR_ASSIGN SHL_ASSIGN BAN_ASSIGN BOR_ASSIGN INC_ASSIGN DEC_ASSIGN
%token IF ELSE FOR WHILE RETURN BREAK CONTINUE

/* Token with return, which need to sepcify type */
%token <var_type> VARIABLE_T
%token <b_var> BOOL_LIT
%token <i_var> INT_LIT
%token <f_var> FLOAT_LIT
%token <s_var> STR_LIT
%token <s_var> IDENT

/* Nonterminal with return, which need to sepcify type */
%type <object_val> Expression
%type <object_val> ValueStmt
/* %type <object_val> IdentStmt */

%left VAL_ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN REM_ASSIGN SHR_ASSIGN SHL_ASSIGN BAN_ASSIGN BOR_ASSIGN
%left LOR
%left LAN
%left GTR LES EQL NEQ LEQ GEQ
%left LSHIFT RSHIFT
%left ADD SUB
%left MUL DIV MOD
%left INC_ASSIGN DEC_ASSIGN 

/* %nonassoc IFX
%nonassoc ELSE */

/* Yacc will start at this nonterminal */
%start Program

%%
/* Grammar section */

Program
    : GlobalStmtList
    | /* Empty file */
;

GlobalStmtList 
    : GlobalStmtList GlobalStmt
    | GlobalStmt
;

GlobalStmt
    : DefineVariableStmt
    | FunctionDefStmt
;

DefineVariableStmt
    : VARIABLE_T IDENT VAL_ASSIGN Expression ';'
;

/* Function */
FunctionDefStmt
    : VARIABLE_T IDENT '(' FunctionParameterStmtList ')' ScopeStmt
;
FunctionParameterStmtList 
    : FunctionParameterStmtList ',' FunctionParameterStmt
    | FunctionParameterStmt
    | /* Empty function parameter */
;
FunctionParameterStmt
    : VARIABLE_T IDENT { pushFunVar($<var_type>1, $<s_var>2, false); }
    | VARIABLE_T MUL IDENT { pushFunVar($<var_type>1, $<s_var>3, true); } // Pointer
    | VARIABLE_T IDENT '[' ']' { pushFunVar($<var_type>1, $<s_var>3, true); } // Array
;

/* Scope */
ScopeStmt
    : '{' StmtList '}'
    | '{' '}'
;
StmtList 
    : StmtList Stmt
    | Stmt
;
Stmt
    : ScopeStmt
    | ';'
    | FOR '(' ForVariableStmt ';' { forBegin(); } Expression ';' { forConditionEnd(&$<object_val>6); } VariableAssignStmt ')'
        { forHeaderEnd(); } ScopeStmt { forEnd(); }
    | COUT CoutParameterStmt ';'
    | VariableAssignStmt ';'
    | RETURN Expression ';'
    | BREAK ';'
    | CONTINUE ';'
;

CoutParameterStmt
    : CoutParameterStmt SHL Expression
    | SHL Expression
;

VariableAssignStmt
    : Expression VAL_ASSIGN Expression { 
        if(objectValueAssign(&$<object_val>1, &$<object_val>3)) yyerrorf("'%s' is not variable\n", $<object_val>1.symbol->name);
    }
    // +=
    | Expression ADD_ASSIGN Expression {
        if(objectAddAssign(&$<object_val>1, &$<object_val>3)) yyerrorf("'%s' can not add assign\n", $<object_val>1.symbol->name);
    }
    // -=
    | Expression SUB_ASSIGN Expression {
        if(objectSubAssign(&$<object_val>1, &$<object_val>3)) yyerrorf("'%s' can not sub assign\n", $<object_val>1.symbol->name);
    }
;

ForVariableStmt
    : VARIABLE_T IDENT VAL_ASSIGN Expression { if(!createVariable($<var_type>1, $<s_var>2, &$<object_val>4)) yyerrorf("Failed to create variable '%s'\n", $<s_var>2); }
    | Expression VAL_ASSIGN Expression { if(objectValueAssign(&$<object_val>1, &$<object_val>3)) yyerrorf("'%s' is not variable\n", $<object_val>1.symbol->name); }
    |
;

Expression
    : Expression ADD Expression { if(objectAdd(&$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression SUB Expression { if(objectSub(&$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression MUL Expression { if(objectMul(&$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression DIV Expression { if(objectDiv(&$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression LES Expression { if(objectLes(&$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression GTR Expression { if(objectGtr(&$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression INC_ASSIGN { if(objectIncAssign(&$<object_val>1, &$$)) YYABORT; }
    | Expression DEC_ASSIGN { if(objectDecAssign(&$<object_val>1, &$$)) YYABORT; }
    | MUL Expression { if(getPointerValue(&$<object_val>2, &$$)) YYABORT; }
    | '(' Expression ')' { $$ = $<object_val>2; }
    | ValueStmt
;

ValueStmt
    : BOOL_LIT { $$ = (Object){OBJECT_TYPE_BOOL, (*(uint8_t*)&$<b_var>1), 0, NULL}; }
    | FLOAT_LIT { $$ = (Object){OBJECT_TYPE_FLOAT, (*(uint32_t*)&$<f_var>1), 0, NULL}; }
    | INT_LIT { $$ = (Object){OBJECT_TYPE_INT, (*(uint32_t*)&$<i_var>1), 0, NULL}; }
    | STR_LIT { $$ = (Object){OBJECT_TYPE_STR, (*(uint64_t*)&$<s_var>1), 0, NULL}; }
    | IDENT { Object* o = findVariable($<s_var>1);
        if(!o) yyerrorf("variable '%s' not declared\n", $<s_var>1);
        $$ = *o;
    }
    | IDENT '[' INT_LIT ']' { Object* o = findVariable($<s_var>1);
        if(!o) yyerrorf("variable '%s' not declared\n", $<s_var>1);
        $$ = (Object){o->type, VAR_FLAG_PTR_VALUE, $<i_var>3, o->symbol};
    }
;

%%
/* C code section */