/* Definition section */
%{
    #define YY_NO_UNPUT
    #define YY_NO_INPUT
    #define printToken(token, value) (value ? printf("%-8s \t %s\n", token, (char*)value) : printf("%s\n", token))

    FILE* fout;
%}

/* Define regular expression label */
letter [a-zA-Z_]
digit [0-9]
variable {letter}+({letter}|{digit})*
inumber {digit}+
fnumber ({digit}*\.{digit}+)

%option yylineno
%x CMT_CON
%x DSTR_CON
%x STR_CON

/* Rules section */
%%

"/*"                { BEGIN(CMT_CON); yymore(); }
<CMT_CON>(.|\r?\n)  { yymore(); }
<CMT_CON>"*/"       { BEGIN(INITIAL); printToken("MULTI_COMMENT", yytext); }
"//".*              { printToken("COMMENT", yytext); }

\"                  { BEGIN(DSTR_CON); yymore(); }
<DSTR_CON>\"        { BEGIN(INITIAL); printToken("STRING_LIT", yytext); }
<DSTR_CON>(\\\"|[^\"])* { yymore(); }

'                   { BEGIN(STR_CON); yymore(); }
<STR_CON>'          { BEGIN(INITIAL); printToken("CHAR_LIT", yytext); }
<STR_CON>(\\'|[^'\n])* { yymore(); }

">>"        { printToken("RSHIFT", NULL); }
"<<"        { printToken("LSHIFT", NULL); }
"&"         { printToken("'&'", NULL); }
"|"         { printToken("'|'", NULL); }
'~'         { printToken("'~'", NULL); }

"+"         { printToken("'+'", NULL); }
"-"         { printToken("'-'", NULL); }
"*"         { printToken("'*'", NULL); }
"/"         { printToken("'/'", NULL); }
"%"         { printToken("'%'", NULL); }
"!"         { printToken("'!'", NULL); }

">"         { printToken("'>'", NULL); }
"<"         { printToken("'<'", NULL); }
">="        { printToken("GEQ", NULL); }
"<="        { printToken("LEQ", NULL); }
"=="        { printToken("EQL", NULL); }
"!="        { printToken("NEQ", NULL); }
"&&"        { printToken("LAND", NULL); }
"||"        { printToken("LOR", NULL); }

"="         { printToken("ASSIGN", NULL); }
"+="        { printToken("ADD_ASSIGN", NULL); }
"-="        { printToken("SUB_ASSIGN", NULL); }
"*="        { printToken("MUL_ASSIGN", NULL); }
"/="        { printToken("DIV_ASSIGN", NULL); }
"%="        { printToken("REM_ASSIGN", NULL); }

"("         { printToken("'('", NULL); }
")"         { printToken("')'", NULL); }
"["         { printToken("'['", NULL); }
"]"         { printToken("']'", NULL); }
"{"         { printToken("'{'", NULL); }
"}"         { printToken("'}'", NULL); }

":"         { printToken("':'", NULL); }
";"         { printToken("';'", NULL); }
","         { printToken("','", NULL); }

"cout"      { printToken("COUT", NULL); }

"void"      { printToken("VOID_TYPE", NULL); }
"char"      { printToken("CHAR_TYPE", NULL); }
"int"       { printToken("INT_TYPE", NULL); }
"long"      { printToken("LONG_TYPE", NULL); }
"float"     { printToken("FLOAT_TYPE", NULL); }
"double"    { printToken("DOUBLE_TYPE", NULL); }
"bool"      { printToken("BOOL_TYPE", NULL); }
"string"    { printToken("STRING_TYPE", NULL); }

"if"        { printToken("IF", NULL); }
"else"      { printToken("ELSE", NULL); }
"for"       { printToken("FOR", NULL); }
"while"     { printToken("WHILE", NULL); }
"return"    { printToken("RETURN", NULL); }
"break"     { printToken("BREAK", NULL); }

"true"      { printToken("BOOL_LIT", yytext); }
"false"     { printToken("BOOL_LIT", yytext); }
{inumber}   { printToken("INT_LIT", yytext); }
{fnumber}   { printToken("FLOAT_LIT", yytext); }
{variable}  { printToken("VARIABLE", yytext); }

<<EOF>>     { static int once = 0; if (once++) {yyterminate();} }
[ \t\r\n]+  {;}


%%
/*  C Code section */
int yywrap(void) {
    return 1;
}


int main(int argc, char* argv[]) {
    char *outputFileName = NULL, *inputFileName = NULL;
    if (argc == 3) {
        yyin = fopen(inputFileName = argv[1], "r");
        outputFileName = argv[2];
    } else if (argc == 2) {
        yyin = stdin;
        outputFileName = argv[1];
    } else {
        yyin = stdin;
        outputFileName = "output";
    }
    if (!yyin) {
        printf("file `%s` doesn't exists or cannot be opened\n", inputFileName);
        exit(1);
    }
    /* fout = fopen(outputFileName, "w");
    if (!fout) {
        printf("file `%s` cannot be opened\n", outputFileName);
        exit(1);
    } */

    yylex();
    printf("\nFinish scanning,\ntotal line: %d\n", yylineno);
    /* printf("comment line: %d\n", comment_num); */
    fclose(yyin);
    /* fclose(fout); */
    return 0;
}