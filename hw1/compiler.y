/* Definition section */
%{
    #include "compiler_common.h"
    #include "compiler_util.h"
    #include "main.h"

    int yydebug = 1;

    ObjectType variableIdentType;
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
%type <object_val> ExpressionAssign
%type <object_val> ValueStmt
%type <object_val> IdentStmt
%type <object_val> FunctionCallStmt
%type <b_var> VariableArrayStmt

%left ','
%right VAL_ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN REM_ASSIGN BAN_ASSIGN BOR_ASSIGN BXO_ASSIGN SHR_ASSIGN SHL_ASSIGN
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
%left NOT BNT '(' ')'
%left INC_ASSIGN DEC_ASSIGN '[' ']' '{' '}'

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
    : VARIABLE_T IDENT VariableArrayStmt { funcLineNo = yylineno; } '(' FunctionParameterStmtList ')' 
        { functionCreate($<var_type>1, $<b_var>3, $<s_var>2); } '{' StmtList '}' { dumpScope(); }
;
FunctionParameterStmtList 
    : FunctionParameterStmtList ',' FunctionParameterStmt
    | FunctionParameterStmt
    | /* Empty function parameter */
;
FunctionParameterStmt
    : VARIABLE_T IDENT VariableArrayStmt { functionParmPush($<var_type>1, $<b_var>3, $<s_var>2, VAR_FLAG_DEFAULT); }
;
FunctionCallStmt
    : IDENT { functionArgNew(); } '(' FunctionArgumentStmtList ')' { functionCall($<s_var>1, &$$); }
;
FunctionArgumentStmtList 
    : FunctionArgumentStmtList ',' Expression { functionArgPush(&$<object_val>3); }
    | Expression { functionArgPush(&$<object_val>1); }
    | /* Empty function argument */
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
    | FOR { printf("FOR\n"); } '(' ForInitStmt ';' { forInit(); } ForConditionStmt ';' { forConditionEnd(&$<object_val>6); } ForUpdationStmt ')'
        { forHeaderEnd(); } ScopeStmt { forEnd(); }
    | WHILE { printf("WHILE\n"); } '(' Expression ')' ScopeStmt
    | IfStmt

    | COUT { functionArgNew(); } CoutParmListStmt ';' { stdoutPrint(); }
    | VariableCreateStmt ';'
    | ExpressionAssign ';'
    | RETURN Expression ';' { printf("RETURN\n"); }
    | BREAK ';' { printf("BREAK\n"); }
    | CONTINUE ';' { printf("CONTINUE\n"); }
;

/* Cout */
CoutParmListStmt
    : CoutParmListStmt SHL Expression { functionArgPush(&$<object_val>3); }
    | SHL Expression { functionArgPush(&$<object_val>2); }
;

/* Create variable */
VariableCreateStmt
    : VARIABLE_T { variableIdentType = $<var_type>1; } VariableIdentListStmt
;
VariableIdentListStmt
    : VariableIdentListStmt ',' VariableIdentStmt
    | VariableIdentStmt
;
VariableIdentStmt
    : IDENT VariableArrayStmt VAL_ASSIGN Expression 
        { if(!createVariable(variableIdentType, $<b_var>2, $<s_var>1, VAR_FLAG_DEFAULT)) yyerrorf("Failed to create variable '%s'\n", $<s_var>1); }
    | IDENT VariableArrayStmt 
        { if(!createVariable(variableIdentType, $<b_var>2, $<s_var>1, VAR_FLAG_DEFAULT)) yyerrorf("Failed to create variable '%s'\n", $<s_var>1); }
;
VariableArrayStmt
    : { $$ = false; }
    | '[' ']' { $$ = true; }
    | '[' Expression ']' { $$ = true; }
;

/* For Statement */
ForInitStmt
    : VariableCreateStmt
    |
;
ForConditionStmt
    : Expression
    |
;
ForUpdationStmt
    : ForUpdationStmt ',' Expression
    | Expression
    |
;

/* If Statement */
IfStmt
    : IF '(' Expression ')' { printf("IF\n"); } ScopeStmt ElseStmt
;
ElseStmt
    : /* No "else" stament*/
    | ELSE { printf("ELSE\n"); } IfStmt
    | ELSE { printf("ELSE\n"); } ScopeStmt
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

    | ExpressionAssign

    | BNT Expression { if(objectNotBinaryExpression(&$<object_val>2, &$$)) YYABORT; }
    | NOT Expression { if(objectNotExpression(&$<object_val>2, &$$)) YYABORT; }
    | SUB Expression { if(objectNegExpression(&$<object_val>2, &$$)) YYABORT; }
    | '(' Expression ')' { $$ = $<object_val>2; }
    | '(' VARIABLE_T ')' Expression { if(objectCast($<var_type>2, &$<object_val>4, &$$)) YYABORT; }
    | FunctionCallStmt
    | ValueStmt
;

ExpressionAssign
    : IdentStmt VAL_ASSIGN Expression { if(objectValueAssign(&$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not assign\n", $<object_val>1.symbol->name); }
    | IdentStmt ADD_ASSIGN Expression { if(objectExpAssign('+', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not ADD assign\n", $<object_val>1.symbol->name); }
    | IdentStmt SUB_ASSIGN Expression { if(objectExpAssign('-', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not SUB assign\n", $<object_val>1.symbol->name); }
    | IdentStmt MUL_ASSIGN Expression { if(objectExpAssign('*', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not MUL assign\n", $<object_val>1.symbol->name); }
    | IdentStmt DIV_ASSIGN Expression { if(objectExpAssign('/', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not DIV assign\n", $<object_val>1.symbol->name); }
    | IdentStmt REM_ASSIGN Expression { if(objectExpAssign('%', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not REM assign\n", $<object_val>1.symbol->name); }
    | IdentStmt SHR_ASSIGN Expression { if(objectExpAssign('>', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not SHR assign\n", $<object_val>1.symbol->name); }
    | IdentStmt SHL_ASSIGN Expression { if(objectExpAssign('<', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not SHL assign\n", $<object_val>1.symbol->name); }
    | IdentStmt BAN_ASSIGN Expression { if(objectExpAssign('&', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not BAN assign\n", $<object_val>1.symbol->name); }
    | IdentStmt BOR_ASSIGN Expression { if(objectExpAssign('|', &$<object_val>1, &$<object_val>3, &$$)) yyerrorf("'%s' can not BOR assign\n", $<object_val>1.symbol->name); }
    | IdentStmt INC_ASSIGN { if(objectIncAssign(&$<object_val>1, &$$)) YYABORT; }
    | IdentStmt DEC_ASSIGN { if(objectDecAssign(&$<object_val>1, &$$)) YYABORT; }
;

ValueStmt
    : BOOL_LIT { $$ = (Object){OBJECT_TYPE_BOOL, (*(uint8_t*)&$<b_var>1), false, NULL}; printf("BOOL_LIT %s\n", $<b_var>1 ? "TRUE" : "FALSE"); }
    | FLOAT_LIT { $$ = (Object){OBJECT_TYPE_FLOAT, (*(uint32_t*)&$<f_var>1), false, NULL}; printf("FLOAT_LIT %f\n", $<f_var>1); }
    | INT_LIT { $$ = (Object){OBJECT_TYPE_INT, (*(uint32_t*)&$<i_var>1), false, NULL}; printf("INT_LIT %d\n", $<i_var>1); }
    | STR_LIT { $$ = (Object){OBJECT_TYPE_STR, (*(uint64_t*)&$<s_var>1), false, NULL}; printf("STR_LIT \"%s\"\n", $<s_var>1); }
    | IdentStmt
    | /*Array*/ { functionArgNew(); } '{' FunctionArgumentStmtList '}' { arrayCreate(&$$); }
;

IdentStmt
    : IDENT {
        Object* o = findVariable($<s_var>1);
        if(!o) yyerrorf("variable '%s' not declared\n", $<s_var>1);
        $$ = *o;
    }
    | IDENT '[' Expression ']' {
        Object* o = findVariable($<s_var>1);
        if(!o) yyerrorf("variable '%s' not declared\n", $<s_var>1);
        if(objectArrayGet(o, &$<object_val>3, &$$)) YYABORT; 
    }
;

%%
/* C code section */