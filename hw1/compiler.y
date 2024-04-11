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
%token SHR SHL BAN BOR BNT BXO ADD SUB MUL DIV REM NOT GTR LES GEQ LEQ EQL NEQ LAN LOR
%token VAL_ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN REM_ASSIGN BAN_ASSIGN BOR_ASSIGN BXO_ASSIGN SHR_ASSIGN SHL_ASSIGN INC_ASSIGN DEC_ASSIGN
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

%left VAL_ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN REM_ASSIGN BAN_ASSIGN BOR_ASSIGN BXO_ASSIGN SHR_ASSIGN SHL_ASSIGN
%left LOR
%left LAN
%left BOR
%left BXO
%left BAN
%left EQL NEQ
%left LES LEQ GTR GEQ
%left SHL SHR
%left ADD SUB
%left MUL DIV REM
%right NOT BNT INC_ASSIGN DEC_ASSIGN ')'

/* %nonassoc IFX
%nonassoc ELSE */

/* Yacc will start at this nonterminal */
%start Program

%%
/* Grammar section */

Program
    : { pushScope(); } GlobalStmtList { dumpScope(); }
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
    : VARIABLE_T IDENT { funcLineNo = yylineno; } '(' FunctionParameterStmtList ')' { createFunction($<var_type>1, $<s_var>2); } '{' StmtList '}' { dumpScope(); }
;
FunctionParameterStmtList 
    : FunctionParameterStmtList ',' FunctionParameterStmt
    | FunctionParameterStmt
    | /* Empty function parameter */
;
FunctionParameterStmt
    : VARIABLE_T IDENT { pushFunParm($<var_type>1, $<s_var>2, VAR_FLAG_DEFAULT); }
    | VARIABLE_T MUL IDENT { pushFunParm($<var_type>1, $<s_var>3, VAR_FLAG_POINTER); } // Pointer
    | VARIABLE_T IDENT '[' ']' { pushFunParm($<var_type>1, $<s_var>3, VAR_FLAG_ARRAY); } // Array
;

/* Scope */
ScopeStmt
    : '{' { pushScope(); } StmtList '}' { dumpScope(); }
    | '{' { pushScope(); } '}' { dumpScope(); }
;
StmtList 
    : StmtList Stmt
    | Stmt
;
Stmt
    : ScopeStmt
    | ';'
    | FOR '(' ForVariableStmt ';' { forBegin(); } Expression ';' { forConditionEnd(&$<object_val>6); } Expression ')'
        { forHeaderEnd(); } ScopeStmt { forEnd(); }
    | COUT CoutParmListStmt ';' { stdoutPrint(); }
    | VARIABLE_T { variableIdentType = $<var_type>1; } VariableIdentListStmt ';'
    | Expression ';'
    | RETURN Expression ';' { printf("RETURN\n"); }
    | BREAK ';' { printf("BREAK\n"); }
    | CONTINUE ';' { printf("CONTINUE\n"); }
;

CoutParmListStmt
    : CoutParmListStmt SHL Expression { pushFunInParm(&$<object_val>3); }
    | SHL Expression { pushFunInParm(&$<object_val>2); }
;
/* Create variable */
VariableIdentListStmt
    : VariableIdentListStmt ',' VariableIdentStmt
    | VariableIdentStmt
;
VariableIdentStmt
    : IDENT VAL_ASSIGN Expression { if(!createVariable(variableIdentType, $<s_var>1, VAR_FLAG_DEFAULT)) yyerrorf("Failed to create variable '%s'\n", $<s_var>1); }
    | IDENT { if(!createVariable(variableIdentType, $<s_var>1, VAR_FLAG_DEFAULT)) yyerrorf("Failed to create variable '%s'\n", $<s_var>1); }
;

ForVariableStmt
    : VARIABLE_T IDENT VAL_ASSIGN Expression { if(!createVariable($<var_type>1, $<s_var>2, VAR_FLAG_DEFAULT)) yyerrorf("Failed to create variable '%s'\n", $<s_var>2); }
    |
;

Expression
    : Expression ADD Expression { if(objectExpression('+', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression SUB Expression { if(objectExpression('-', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression MUL Expression { if(objectExpression('*', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression DIV Expression { if(objectExpression('/', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression REM Expression { if(objectExpression('%', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    
    | Expression SHR Expression { if(objectExpBinary('>', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression SHL Expression { if(objectExpBinary('<', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression BAN Expression { if(objectExpBinary('&', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression BOR Expression { if(objectExpBinary('|', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression BXO Expression { if(objectExpBinary('^', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }

    | Expression GTR Expression { if(objectExpBoolean('>', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression LES Expression { if(objectExpBoolean('<', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression GEQ Expression { if(objectExpBoolean('.'/*>=*/, &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }
    | Expression LEQ Expression { if(objectExpBoolean(','/*<=*/, &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }     
    | Expression EQL Expression { if(objectExpBoolean('=', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }     
    | Expression NEQ Expression { if(objectExpBoolean('!', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }     
    | Expression LAN Expression { if(objectExpBoolean('&', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }     
    | Expression LOR Expression { if(objectExpBoolean('|', &$<object_val>1, &$<object_val>3, &$$)) YYABORT; }     

    | Expression VAL_ASSIGN Expression { if(objectValueAssign(&$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not assign\n", $<object_val>1.symbol->name); }
    | Expression ADD_ASSIGN Expression { if(objectExpAssign('+', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not ADD assign\n", $<object_val>1.symbol->name); }
    | Expression SUB_ASSIGN Expression { if(objectExpAssign('-', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not SUB assign\n", $<object_val>1.symbol->name); }
    | Expression MUL_ASSIGN Expression { if(objectExpAssign('*', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not MUL assign\n", $<object_val>1.symbol->name); }
    | Expression DIV_ASSIGN Expression { if(objectExpAssign('/', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not DIV assign\n", $<object_val>1.symbol->name); }
    | Expression REM_ASSIGN Expression { if(objectExpAssign('%', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not REM assign\n", $<object_val>1.symbol->name); }
    | Expression SHR_ASSIGN Expression { if(objectExpAssign('>', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not SHR assign\n", $<object_val>1.symbol->name); }
    | Expression SHL_ASSIGN Expression { if(objectExpAssign('<', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not SHL assign\n", $<object_val>1.symbol->name); }
    | Expression BAN_ASSIGN Expression { if(objectExpAssign('&', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not BAN assign\n", $<object_val>1.symbol->name); }
    | Expression BOR_ASSIGN Expression { if(objectExpAssign('|', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not BOR assign\n", $<object_val>1.symbol->name); }
    | Expression INC_ASSIGN { if(objectIncAssign(&$<object_val>1, &$$)) YYABORT; }
    | Expression DEC_ASSIGN { if(objectDecAssign(&$<object_val>1, &$$)) YYABORT; }
    | BNT Expression { if(objectNotBinaryExpression(&$<object_val>1, &$$)) YYABORT; }
    | NOT Expression { if(objectNotExpression(&$<object_val>2, &$$)) YYABORT; }
    | SUB Expression { if(objectNegExpression(&$<object_val>2, &$$)) YYABORT; }
    | '(' Expression ')' { $$ = $<object_val>2; }
    | '(' VARIABLE_T ')' Expression { if(objectCast($<var_type>2, &$<object_val>4, &$$)) YYABORT; }
    | ValueStmt
;

ValueStmt
    : BOOL_LIT { $$ = (Object){OBJECT_TYPE_BOOL, (*(uint8_t*)&$<b_var>1), NULL}; printf("BOOL_LIT %s\n", $<b_var>1 ? "TRUE" : "FALSE"); }
    | FLOAT_LIT { $$ = (Object){OBJECT_TYPE_FLOAT, (*(uint32_t*)&$<f_var>1), NULL}; printf("FLOAT_LIT %f\n", $<f_var>1); }
    | INT_LIT { $$ = (Object){OBJECT_TYPE_INT, (*(uint32_t*)&$<i_var>1), NULL}; printf("INT_LIT %d\n", $<i_var>1); }
    | STR_LIT { $$ = (Object){OBJECT_TYPE_STR, (*(uint64_t*)&$<s_var>1), NULL}; printf("STR_LIT \"%s\"\n", $<s_var>1); }
    | IDENT { Object* o = findVariable($<s_var>1);
        if(!o) yyerrorf("variable '%s' not declared\n", $<s_var>1);
        $$ = *o;
    }
;

%%
/* C code section */