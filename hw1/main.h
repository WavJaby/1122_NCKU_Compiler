#ifndef MAIN_H
#define MAIN_H
#include "compiler_common.h"

extern FILE* yyin;
extern bool compileError;
int yyparse();
int yylex();
int yylex_destroy();

extern ObjectType variableIdentType;

#define VAR_FLAG_DEFAULT 0

void pushScope();
void dumpScope();

void functionParmPush(ObjectType variableType, bool array, char* variableName, int parmFlag);
void functionCreate(ObjectType variableType, bool array, char* funcName);
void functionArgNew();
void functionArgPush(Object* obj);
void functionCall(char* funcName, Object* out);

void arrayCreate(Object* out);

Object* findVariable(char* variableName);
Object* createVariable(ObjectType variableType, bool array, char* variableName, Object* value, int variableFlag);

// Expressions
bool objectExpression(char op, Object* a, Object* b, Object* out);
bool objectExpBinary(char op, Object* a, Object* b, Object* out);
bool objectExpBoolean(char op, Object* a, Object* b, Object* out);
bool objectNotBinaryExpression(Object* a, Object* out);
bool objectNotBooleanExpression(Object* a, Object* out);
bool objectNegExpression(Object* a, Object* out);
bool objectIncAssign(Object* a, Object* out);
bool objectDecAssign(Object* a, Object* out);
bool objectCast(ObjectType variableType, Object* a, Object* out);
bool objectExpAssign(char op, Object* dest, Object* val, Object* out);
bool objectValueAssign(Object* dest, Object* val, Object* out);
bool objectArrayGet(Object* obj, Object* index, Object* out);

void stdoutPrint();
void returnObject(Object* obj);

bool forInit();
bool forConditionEnd(Object* result);
bool forHeaderEnd();
bool foreachHeaderEnd(Object* obj);
bool forEnd();

#endif