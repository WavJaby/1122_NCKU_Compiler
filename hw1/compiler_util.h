#ifndef COMPILER_UTIL_H
#define COMPILER_UTIL_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

extern int yylineno;
extern int yycolumn;
extern int yyoffset;
extern int yyleng;
extern char* yyInputFileName;
extern bool compileError;
extern FILE* yyin;

#define ERROR_PREFIX "%s:%d:%d: error: "

#define yyerrorf(format, ...)                                                                         \
    {                                                                                                 \
        printf(ERROR_PREFIX format, yyInputFileName, yylineno, yycolumn - yyleng + 1, ##__VA_ARGS__); \
        compileError = true;                                                                          \
        YYABORT;                                                                                      \
    }

void yyerror(char const* msg) {
    printf(ERROR_PREFIX " %s\n", yyInputFileName, yylineno, yycolumn - yyleng + 1, msg);
    char cache[64 + 1];
    cache[64] = 0;
    printf("%d\n", yyoffset);
    fseek(yyin, 0, SEEK_SET);
    fread(cache, 1, 64, yyin);
    // for (int i = 0; cache[i]; i++)
    //     if (cache[i] == '\r' || cache[i] == '\n') {
    //         cache[i] = 0;
    //         break;
    //     }
    printf("%6d |%s\n       |%*.s^\n", yylineno, cache - yycolumn + yyleng, yycolumn - yyleng, "");
    compileError = true;
}

#endif