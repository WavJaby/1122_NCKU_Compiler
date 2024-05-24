/* Definition section */
%{
    #include "compiler_util.h"
    #include "main.h"

    int yydebug = 1;

    ObjectType variableIdentType;
%}

/* Variable or self-defined structure */
%union {
    ObjectType var_type;

    bool b_var;
    char c_var;
    int32_t i_var;
    int64_t l_var;
    float f_var;
    double d_var;
    char *s_var;

    Object obj_val;

    // LinkList<Object*>
    LinkedList* array_subscript;
}

/* Token without return */
%token COUT
%token SHR SHL BAN BOR BNT BXO ADD SUB MUL DIV REM NOT GTR LES GEQ LEQ EQL NEQ LAN LOR
%token VAL_ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN REM_ASSIGN BAN_ASSIGN BOR_ASSIGN BXO_ASSIGN SHR_ASSIGN SHL_ASSIGN INC_ASSIGN DEC_ASSIGN
%token IF ELSE FOR WHILE RETURN BREAK CONTINUE

/* Token with return, which need to sepcify type */
%token <var_type> VARIABLE_T
%token <b_var> BOOL_LIT
%token <c_var> CHAR_LIT
%token <i_var> INT_LIT
%token <l_var> LONG_LIT
%token <f_var> FLOAT_LIT
%token <d_var> DOUBLE_LIT
%token <s_var> STR_LIT
%token <s_var> IDENT

/* Nonterminal with return, which need to sepcify type */
%type <obj_val> Expression
%type <obj_val> ExpressionValue
%type <obj_val> ExpressionAssign
%type <obj_val> ValueStmt
%type <obj_val> IdentStmt
%type <obj_val> FunctionCallStmt
%type <array_subscript> ArraySubscriptStmtList
%type <array_subscript> VariableArrayStmt

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
%right NOT BNT '(' ')'
%left INC_ASSIGN DEC_ASSIGN '[' ']' '{' '}'

%nonassoc THEN
%nonassoc ELSE

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
    : VARIABLE_T IDENT VariableArrayStmt { functionLocalsBegin(); } '(' FunctionParameterStmtList ')' 
        { functionBegin($<var_type>1, $<array_subscript>3, $<s_var>2); } '{' StmtList '}' { if(functionEnd($<var_type>1)) yyerrorf("Function '%s' must return \n", $<s_var>2); }
;
FunctionParameterStmtList
    : FunctionParameterStmtList ',' FunctionParameterStmt
    | FunctionParameterStmt
    | /* Empty function parameter */
;
FunctionParameterStmt
    : VARIABLE_T IDENT VariableArrayStmt { functionParmPush($<var_type>1, $<array_subscript>3, $<s_var>2); }
;
FunctionCallStmt
    : IDENT { functionArgsBegin(); } '(' FunctionArgumentStmtList ')' { functionCall($<s_var>1, &$$); }
;
FunctionArgumentStmtList 
    : FunctionArgumentStmtList ',' Expression { functionArgPush(&$<obj_val>3); }
    | Expression { functionArgPush(&$<obj_val>1); }
    | /* Empty function argument */
;

/* Scope */
ScopeStmt
    : '{' { pushScope(); } StmtList '}' { dumpScope(); }
    | '{' { pushScope(); } '}' { dumpScope(); }
    | BodyStmt
;
ManualScopeStmt
    : '{' StmtList '}'
    | '{' '}'
    | BodyStmt
;
StmtList 
    : StmtList ScopeStmt
    | ScopeStmt
;
BodyStmt
    : ';'
    | FOR { forBegin(); } '(' ForHeaderStmt ')' ManualScopeStmt { forEnd(); }
    | WHILE { whileBegin(); } '(' Expression ')' { whileBodyBegin(); } ScopeStmt { whileEnd(); }
    | IfStmt

    | VariableCreateStmt ';'
    | ExpressionAssign ';'
    | FunctionCallStmt ';'
    | COUT CoutParmListStmt ';'
    | RETURN Expression ';' { returnObject(&$<obj_val>2); }
    | RETURN ';' { returnObject(NULL); }
    | BREAK ';' { breakLoop(); }
    | CONTINUE ';' { printf("CONTINUE\n"); }
;

/* Cout */
CoutParmListStmt
    : CoutParmListStmt SHL Expression { stdoutPrint(&$<obj_val>3); }
    | SHL Expression { stdoutPrint(&$<obj_val>2); }
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
    : IDENT VariableArrayStmt { if(initVariable(variableIdentType, $<array_subscript>2, $<s_var>1)) yyerrorf("Failed to create variable '%s'\n", $<s_var>1); } VAL_ASSIGN Expression 
        { if(!createVariable(variableIdentType, $<array_subscript>2, $<s_var>1, &$<obj_val>5)) yyerrorf("Failed to create variable '%s'\n", $<s_var>1); }
    | IDENT VariableArrayStmt 
        { if(initVariable(variableIdentType, $<array_subscript>2, $<s_var>1) || !createVariable(variableIdentType, $<array_subscript>2, $<s_var>1, NULL)) yyerrorf("Failed to create variable '%s'\n", $<s_var>1); }
;
VariableArrayStmt
    : { $$ = NULL; }
    | '[' ']' { $$ = arraySubscriptBegin(NULL); }
    | ArraySubscriptStmtList
;

/* For Statement */
ForHeaderStmt
    : ForInitStmt ';' 
        { forInitEnd(); } ForConditionStmt ';' 
        { forConditionEnd(NULL); } ForUpdationStmt { forHeaderEnd(); }
    | VariableCreateStmt { forInitEnd(); } ':' 
        Expression { foreachHeaderEnd(&$<obj_val>4); }
;
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
IfHeader
    : IF '(' Expression ')' { ifBegin(&$<obj_val>3); } ScopeStmt
;
IfStmt
    : IfHeader %prec THEN { ifOnlyEnd(); }
    | IfHeader ELSE { elseBegin(); } ScopeStmt { elseEnd(); }
;

Expression
    : Expression ADD Expression { if(objectExpression('+', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't ADD\n"); }
    | Expression SUB Expression { if(objectExpression('-', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't SUB\n"); }
    | Expression MUL Expression { if(objectExpression('*', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't MUL\n"); }
    | Expression DIV Expression { if(objectExpression('/', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't DIV\n"); }
    | Expression REM Expression { if(objectExpression('%', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't REM\n"); }
    
    | Expression SHR Expression { if(objectExpBinary('>', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't SHR\n"); }
    | Expression SHL Expression { if(objectExpBinary('<', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't SHL\n"); }
    | Expression BAN Expression { if(objectExpBinary('&', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't BAN\n"); }
    | Expression BOR Expression { if(objectExpBinary('|', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't BOR\n"); }
    | Expression BXO Expression { if(objectExpBinary('^', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't BXO\n"); }

    | Expression GTR Expression { if(objectExpBoolean('>', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't GTR\n"); }
    | Expression LES Expression { if(objectExpBoolean('<', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't LES\n"); }
    | Expression GEQ Expression { if(objectExpBoolean('.'/*>=*/, &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't GEQ\n"); }
    | Expression LEQ Expression { if(objectExpBoolean(','/*<=*/, &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't LEQ\n"); }
    | Expression EQL Expression { if(objectExpBoolean('=', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't EQL\n"); }
    | Expression NEQ Expression { if(objectExpBoolean('!', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't NEQ\n"); }
    | Expression LAN Expression { if(objectExpBoolean('&', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't LAN\n"); }
    | Expression LOR Expression { if(objectExpBoolean('|', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("can't LOR\n"); }

    | ExpressionAssign

    | ExpressionValue
;

ExpressionValue
    : BNT ExpressionValue { if(objectNotBinaryExpression(&$<obj_val>2, &$$)) YYABORT; }
    | NOT ExpressionValue { if(objectNotBooleanExpression(&$<obj_val>2, &$$)) YYABORT; }
    | SUB ExpressionValue { if(objectNegExpression(&$<obj_val>2, &$$)) YYABORT; }
    | '(' VARIABLE_T ')' Expression { if(objectCast($<var_type>2, &$<obj_val>4, &$$)) YYABORT; }
    | '(' Expression ')' { $$ = $<obj_val>2; }
    | FunctionCallStmt
    | ValueStmt
;

ExpressionAssign
    : IdentStmt VAL_ASSIGN Expression { if(objectValueAssign(&$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("'%s' can't assign\n", $<obj_val>1.symbol->name); }
    | IdentStmt ADD_ASSIGN Expression { if(objectExpAssign('+', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("'%s' can't ADD assign\n", $<obj_val>1.symbol->name); }
    | IdentStmt SUB_ASSIGN Expression { if(objectExpAssign('-', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("'%s' can't SUB assign\n", $<obj_val>1.symbol->name); }
    | IdentStmt MUL_ASSIGN Expression { if(objectExpAssign('*', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("'%s' can't MUL assign\n", $<obj_val>1.symbol->name); }
    | IdentStmt DIV_ASSIGN Expression { if(objectExpAssign('/', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("'%s' can't DIV assign\n", $<obj_val>1.symbol->name); }
    | IdentStmt REM_ASSIGN Expression { if(objectExpAssign('%', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("'%s' can't REM assign\n", $<obj_val>1.symbol->name); }
    | IdentStmt SHR_ASSIGN Expression { if(objectExpAssign('>', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("'%s' can't SHR assign\n", $<obj_val>1.symbol->name); }
    | IdentStmt SHL_ASSIGN Expression { if(objectExpAssign('<', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("'%s' can't SHL assign\n", $<obj_val>1.symbol->name); }
    | IdentStmt BAN_ASSIGN Expression { if(objectExpAssign('&', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("'%s' can't BAN assign\n", $<obj_val>1.symbol->name); }
    | IdentStmt BOR_ASSIGN Expression { if(objectExpAssign('|', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("'%s' can't BOR assign\n", $<obj_val>1.symbol->name); }
    | IdentStmt BXO_ASSIGN Expression { if(objectExpAssign('^', &$<obj_val>1, &$<obj_val>3, &$$)) yyerrorf("'%s' can't BXO assign\n", $<obj_val>1.symbol->name); }
    | IdentStmt INC_ASSIGN { if(objectIncAssign(&$<obj_val>1, &$$)) yyerrorf("%s can't INC_ASSIGN\n", $<obj_val>1.symbol->name); }
    | IdentStmt DEC_ASSIGN { if(objectDecAssign(&$<obj_val>1, &$$)) yyerrorf("%s can't DEC_ASSIGN\n", $<obj_val>1.symbol->name); }
;

ValueStmt
    : BOOL_LIT { $$ = (Object){OBJECT_TYPE_BOOL, false, (*(uint64_t*)&$<b_var>1), 0, NULL}; printf("BOOL_LIT %s\n", $<b_var>1 ? "TRUE" : "FALSE"); }
    | FLOAT_LIT { $$ = (Object){OBJECT_TYPE_FLOAT, false, (*(uint64_t*)&$<f_var>1), 0, NULL}; printf("FLOAT_LIT %.6f\n", $<f_var>1); }
    | DOUBLE_LIT { $$ = (Object){OBJECT_TYPE_DOUBLE, false, (*(uint64_t*)&$<d_var>1), 0, NULL}; printf("DOUBLE_LIT %lf\n", $<d_var>1); }
    | INT_LIT { $$ = (Object){OBJECT_TYPE_INT, false, (*(uint64_t*)&$<i_var>1), 0, NULL}; printf("INT_LIT %d\n", $<i_var>1); }
    | STR_LIT { $$ = (Object){OBJECT_TYPE_STR, false, (*(uint64_t*)&$<s_var>1), 0, NULL}; printf("STR_LIT \"%s\"\n", $<s_var>1); }
    | CHAR_LIT { $$ = (Object){OBJECT_TYPE_CHAR, false, (*(uint64_t*)&$<c_var>1), 0, NULL}; printf("CHAR_LIT '%c'\n", $<c_var>1); }
    | /*Array*/ { functionArgsBegin(); } '{' FunctionArgumentStmtList '}' { if(arrayCreate(&$$)) yyerrorf("Failed to create array %s", "\n"); }
    | IdentStmt
;

IdentStmt
    : IDENT {
        Object* o = findVariable($<s_var>1);
        if(!o) yyerrorf("variable '%s' not declared\n", $<s_var>1);
        $$ = *o;
    }
    | IDENT ArraySubscriptStmtList {
        Object* o = findVariable($<s_var>1);
        if(!o) yyerrorf("variable '%s' not declared\n", $<s_var>1);
        if(objectArrayGet(o, $<array_subscript>2, &$$)) yyerrorf("variable array '%s' failed to read\n", $<s_var>1); 
    }
;

ArraySubscriptStmtList
    : ArraySubscriptStmtList '[' Expression ']' { arraySubscriptPush($$ = $<array_subscript>1, &$<obj_val>3); }
    | '[' Expression ']' { $$ = arraySubscriptBegin(&$<obj_val>2); }
;

%%
/* C code section */