#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#define DEBUG_OUT
#define WJCL_LINKED_LIST_IMPLEMENTATION
#define WJCL_HASH_MAP_IMPLEMENTATION
#include "../../WJCL/map/wjcl_hash_map.h"
#include "../../WJCL/list/wjcl_linked_list.h"
#include "object_map_entries.h"

#define debug printf("%s:%d: ############### debug\n", __FILE__, __LINE__)

#define toupper(_char) (_char - (char)32)

const char* objectTypeName[] = {
    [OBJECT_TYPE_UNDEFINED] = "undefined",
    [OBJECT_TYPE_VOID] = "void",
    [OBJECT_TYPE_INT] = "int",
    [OBJECT_TYPE_FLOAT] = "float",
    [OBJECT_TYPE_BOOL] = "bool",
    [OBJECT_TYPE_STR] = "string",
    [OBJECT_TYPE_FUNCTION] = "function",
};

char* yyInputFileName;
bool compileError;

int indent = 0;
int scopeLevel = -1;
int funcLineNo = 0;
int variableAddress = 0;
ObjectType variableIdentType;

Map staticVar;
LinkedList scopeList = linkedList_create();
LinkedList funcParm = linkedList_create();

void pushScope() {
    printf("> Create symbol table (scope level %d)\n", ++scopeLevel);
    linkedList_addPtr(&scopeList, map_new(objectInfo));
}

void dumpScope() {
    printf("\n> Dump symbol table (scope level: %d)\n", scopeLevel);

    printf("Index     Name                Type      Addr      Lineno    Func_sig  \n");
    Map* scope = (Map*)scopeList.last->value;
    Object** sorted = (Object**)malloc(sizeof(Object*) * scope->size);
    map_entries(scope, i, {
        Object* obj = (Object*)i->value;
        sorted[obj->symbol->index] = obj;
    });
    for (size_t i = 0; i < scope->size; i++) {
        Object* obj = (Object*)sorted[i];
        SymbolData* symbolData = obj->symbol;
        printf("%-10d%-20s%-10s%-10ld%-10d%-10s\n",
               symbolData->index, symbolData->name, objectTypeName[obj->type],
               symbolData->addr, symbolData->lineno, symbolData->func_sig);
    }

    map_free((Map*)scopeList.last->value);
    free(scopeList.last->value);
    linkedList_removeNode(&scopeList, scopeList.last);
    --scopeLevel;
}

Object* createVariable(ObjectType variableType, char* variableName, int variableFlag) {
    Object* obj = malloc(sizeof(Object));
    obj->type = variableType;
    obj->value = variableFlag;
    SymbolData* symbol = obj->symbol = malloc(sizeof(SymbolData));
    symbol->name = (char*)variableName;
    symbol->index = ((Map*)scopeList.last->value)->size;
    symbol->lineno = yylineno;
    symbol->addr = variableAddress++;
    symbol->func_sig = "-";
    map_putpp(scopeList.last->value, (void*)variableName, obj);
    printf("> Insert `%s` (addr: %ld) to scope level %u\n", variableName, symbol->addr, scopeLevel);
    return obj;
}

void pushFunParm(ObjectType variableType, char* variableName, int variableFlag) {
    Object* obj = malloc(sizeof(Object));
    obj->type = variableType;
    obj->value = variableFlag;
    SymbolData* symbol = obj->symbol = malloc(sizeof(SymbolData));
    symbol->name = (char*)variableName;
    symbol->lineno = yylineno;
    symbol->addr = variableAddress++;
    symbol->func_sig = "-";

    linkedList_addPtr(&funcParm, obj);
}

void createFunction(ObjectType variableType, char* funcName) {
    printf("func: %s\n", funcName);
    Object* funcObj = (Object*)malloc(sizeof(Object));
    funcObj->type = OBJECT_TYPE_FUNCTION;
    funcObj->value = 0;
    map_putpp(scopeList.last->value, (void*)funcName, funcObj);
    printf("> Insert `%s` (addr: -1) to scope level %u\n", funcName, scopeLevel);
    pushScope();
    // Add variables
    char* sig = malloc(64);
    int sigIndex = 1;
    sig[0] = '(';
    linkedList_foreachPtr(&funcParm, Object*, obj, {
        SymbolData* symbol = obj->symbol;
        if (obj->type == OBJECT_TYPE_STR) {
            if (obj->value & VAR_FLAG_ARRAY) sig[sigIndex++] = '[';
            strcpy(sig + sigIndex, "Ljava/lang/String;");
            sigIndex += 18;
        } else
            sig[sigIndex++] = toupper(*objectTypeName[obj->type]);
        symbol->index = ((Map*)scopeList.last->value)->size;
        map_putpp(scopeList.last->value, symbol->name, obj);
        printf("> Insert `%s` (addr: %ld) to scope level %u\n", symbol->name, symbol->addr, scopeLevel);
    });
    if (funcParm.length == 0)
        sig[sigIndex++] = 'V';
    sig[sigIndex++] = ')';
    sig[sigIndex++] = toupper(*objectTypeName[variableType]);
    sig = realloc(sig, sigIndex + 1);
    sig[sigIndex] = 0;
    // Create function description
    SymbolData* symbol = funcObj->symbol = (SymbolData*)malloc(sizeof(SymbolData));
    symbol->name = funcName;
    symbol->func_sig = sig;
    symbol->lineno = funcLineNo;
    symbol->addr = -1;
    linkedList_free(&funcParm);
}

void debugPrintInst(char instc, Object* a, Object* b, Object* out) {
    return;
}

bool objectExpression(char op, Object* dest, Object* val, Object* out) {
    switch (op) {
    case '+':
        printf("ADD\n");
        break;
    case '-':
        printf("SUB\n");
        break;
    case '*':
        printf("MUL\n");
        break;
    case '/':
        printf("DIV\n");
        break;
    case '%':
        printf("REM\n");
        break;
    default:
        return true;
    }
    return false;
}

bool objectExpBinary(char op, Object* a, Object* b, Object* out) {
    switch (op) {
    case '>':
        printf("SHR\n");
        break;
    case '<':
        printf("SHL\n");
        break;
    case '&':
        printf("BAN\n");
        break;
    case '|':
        printf("BOR\n");
        break;
    case '^':
        printf("BXO\n");
        break;
    default:
        return true;
    }
    return false;
}

bool objectExpBoolean(char op, Object* a, Object* b, Object* out) {
    switch (op) {
    case '>':
        printf("GTR\n");
        break;
    case '<':
        printf("LES\n");
        break;
    case '.':
        printf("GEQ\n");
        break;
    case ',':
        printf("LEQ\n");
        break;
    case '=':
        printf("EQL\n");
        break;
    case '!':
        printf("NEQ\n");
        break;
    case '&':
        printf("LAN\n");
        break;
    case '|':
        printf("LOR\n");
        break;
    default:
        return true;
    }
    return false;
}

bool objectExpAssign(char op, Object* dest, Object* val, Object* out) {
    switch (op) {
    case '+':
        printf("ADD_ASSIGN\n");
        break;
    case '-':
        printf("SUB_ASSIGN\n");
        break;
    case '*':
        printf("MUL_ASSIGN\n");
        break;
    case '/':
        printf("DIV_ASSIGN\n");
        break;
    case '%':
        printf("REM_ASSIGN\n");
        break;
    case '>':
        printf("SHR_ASSIGN\n");
        break;
    case '<':
        printf("SHL_ASSIGN\n");
        break;
    case '&':
        printf("BAN_ASSIGN\n");
        break;
    case '|':
        printf("BOR_ASSIGN\n");
        break;
    default:
        return true;
    }
    return false;
}

bool objectValueAssign(Object* dest, Object* val, Object* out) {
    printf("EQL_ASSIGN\n");
    return false;
}

bool objectNotBinaryExpression(Object* dest, Object* out) {
    printf("BNT\n");
    return false;
}

bool objectNegExpression(Object* dest, Object* out) {
    printf("NEG\n");
    return false;
}
bool objectNotExpression(Object* dest, Object* out) {
    printf("NOT\n");
    return false;
}

// ++
bool objectIncAssign(Object* a, Object* out) {
    printf("INC_ASSIGN\n");
    return false;
}

// --
bool objectDecAssign(Object* a, Object* out) {
    printf("DEC_ASSIGN\n");
    return false;
}

bool objectCast(ObjectType variableType, Object* dest, Object* out) {
    printf("Cast to %s\n", objectTypeName[variableType]);
    return false;
}

Object* findVariable(char* variableName) {
    Object* variable = NULL;
    linkedList_foreachPtr(&scopeList, Map*, scope, {
        variable = map_get(scope, variableName);
        if (variable)
            break;
    });
    if (!variable)
        variable = (Object*)map_get(&staticVar, (void*)variableName);
    if (!variable)
        return NULL;
    printf("IDENT (name=%s, address=%ld)\n", variableName, variable->symbol->addr);
    return variable;
}

void pushFunInParm(Object* variable) {
    if (!variable) return;
    Object* cache = (Object*)malloc(sizeof(Object));
    memcpy(cache, variable, sizeof(Object));
    linkedList_addPtr(&funcParm, cache);
}

void stdoutPrint() {
    printf("cout");
    linkedList_foreachPtr(&funcParm, Object*, obj, {
        printf(" %s", objectTypeName[obj->type]);
    });
    linkedList_free(&funcParm);
    printf("\n");
}

bool forBegin() {
    return false;
}

bool forConditionEnd(Object* result) {
    return false;
}

bool forHeaderEnd() {
    return false;
}

bool forEnd() {
    return false;
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        yyin = fopen(yyInputFileName = argv[1], "r");
    } else {
        yyin = stdin;
    }
    if (!yyin) {
        printf("file `%s` doesn't exists or cannot be opened\n", yyInputFileName);
        exit(1);
    }

    // Start parsing
    staticVar = map_create(objectInfo);
    Object endl = {OBJECT_TYPE_STR, 0, &(SymbolData){"endl", 0, -1}};
    map_putpp(&staticVar, "endl", &endl);

    yyparse();
    printf("Total lines: %d\n", yylineno);
    fclose(yyin);

    yylex_destroy();
    return 0;
}