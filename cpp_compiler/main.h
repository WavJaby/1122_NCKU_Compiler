#ifndef MAIN_H
#define MAIN_H
#include "compiler_common.h"

int yyparse();
int yylex();
int yylex_destroy();
uint32_t yylineno;
uint32_t yyleng;

bool compileError;

void createMethod(ObjectType returnType, const char* name);

#endif