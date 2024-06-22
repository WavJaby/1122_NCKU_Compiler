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
    [OBJECT_TYPE_CHAR] = "char",
    [OBJECT_TYPE_SHORT] = "short",
    [OBJECT_TYPE_INT] = "int",
    [OBJECT_TYPE_LONG] = "long",
    [OBJECT_TYPE_FLOAT] = "float",
    [OBJECT_TYPE_DOUBLE] = "double",
    [OBJECT_TYPE_BOOL] = "bool",
    [OBJECT_TYPE_STR] = "string",
};
const uint8_t objectTypeSize[] = {
    [OBJECT_TYPE_UNDEFINED] = 0,
    [OBJECT_TYPE_VOID] = 0,
    [OBJECT_TYPE_CHAR] = 1,
    [OBJECT_TYPE_SHORT] = 2,
    [OBJECT_TYPE_INT] = 4,
    [OBJECT_TYPE_LONG] = 8,
    [OBJECT_TYPE_FLOAT] = 4,
    [OBJECT_TYPE_DOUBLE] = 8,
    [OBJECT_TYPE_BOOL] = 1,
    [OBJECT_TYPE_STR] = 0,
};
const uint8_t objectTypePriority[] = {
    [OBJECT_TYPE_UNDEFINED] = 0,
    [OBJECT_TYPE_VOID] = 0,
    [OBJECT_TYPE_CHAR] = 2,
    [OBJECT_TYPE_SHORT] = 3,
    [OBJECT_TYPE_INT] = 4,
    [OBJECT_TYPE_LONG] = 5,
    [OBJECT_TYPE_FLOAT] = 6,
    [OBJECT_TYPE_DOUBLE] = 7,
    [OBJECT_TYPE_BOOL] = 1,
    [OBJECT_TYPE_STR] = 0,
};

char* yyInputFileName;
bool compileError;

int indent = 0;
int scopeLevel = -1;
int funcLineNo = 0;
int variableAddress = 0;

Map staticVar;                                          // Map<char*, Object*>
LinkedList scopeListStack = linkedList_create();        // LinkedList<Map<char*, Object*>*>
LinkedList funcParm = linkedList_create();              // LinkedList<Object*>
LinkedList funcArgStack = linkedList_create();          // LinkedList<LinkedList<Object*>>
LinkedList foreachAutoTypeStack = linkedList_create();  // LinkedList<Object*>

void pushScope() {
    printf("> Create symbol table (scope level %d)\n", ++scopeLevel);
    linkedList_addPtr(&scopeListStack, map_new(objectInfo));
}

void dumpScope() {
    printf("\n> Dump symbol table (scope level: %d)\n", scopeLevel);

    printf("Index     Name                Type      Addr      Lineno    Func_sig  \n");
    Map* scope = (Map*)scopeListStack.last->value;
    Object** sorted = (Object**)malloc(sizeof(Object*) * scope->size);
    map_entries(scope, i) {
        Object* obj = (Object*)i->value;
        sorted[obj->symbol->index] = obj;
    }
    for (size_t i = 0; i < scope->size; i++) {
        Object* obj = (Object*)sorted[i];
        SymbolData* symbolData = obj->symbol;
        char* typeName = symbolData->func_sig ? "function" : (char*)objectTypeName[obj->type];
        char* funcSig = symbolData->func_sig ? symbolData->func_sig : "-";
        // if (obj->array) {
        //     size_t len = strlen(typeName);
        //     char* cache = malloc(len + 3);
        //     memcpy(cache, typeName, len);
        //     memcpy(cache + len, "[]", 3);
        //     typeName = cache;
        // }
        printf("%-10d%-20s%-10s%-10ld%-10d%-10s\n",
               symbolData->index, symbolData->name, typeName,
               symbolData->addr, symbolData->lineno, funcSig);
        // if (obj->array) free(typeName);
    }

    map_free((Map*)scopeListStack.last->value);
    free(scopeListStack.last->value);
    linkedList_removeNode(&scopeListStack, scopeListStack.last);
    --scopeLevel;
}

static inline Object* newObject(ObjectType variableType, bool array, uint64_t value, SymbolData* symbol) {
    Object* obj = malloc(sizeof(Object));
    obj->type = variableType;
    obj->value = value;
    obj->array = array;
    obj->symbol = symbol;
    return obj;
}

static inline SymbolData* newSymbol(char* name, int32_t index, int64_t addr, int32_t lineno, char* funcSig) {
    SymbolData* symbol = malloc(sizeof(SymbolData));
    symbol->name = name;
    symbol->index = index;
    symbol->addr = addr;
    symbol->lineno = lineno;
    symbol->func_sig = funcSig;
    return symbol;
}

Object* createVariable(ObjectType variableType, bool array, char* variableName, Object* value, int variableFlag) {
    int index = ((Map*)scopeListStack.last->value)->size;
    if (variableType == OBJECT_TYPE_AUTO && value)
        variableType = value->type;
    Object* obj = newObject(variableType, array, variableFlag,
                            newSymbol(variableName, index, variableAddress++, yylineno, NULL));
    map_putpp(scopeListStack.last->value, (void*)variableName, obj);
    printf("> Insert `%s` (addr: %ld) to scope level %u\n", variableName, obj->symbol->addr, scopeLevel);
    // Auto type in for each loop, need assign later
    if (variableType == OBJECT_TYPE_AUTO && !value)
        linkedList_addPtr(&foreachAutoTypeStack, obj);
    return obj;
}

void functionParmPush(ObjectType variableType, bool array, char* variableName, int variableFlag) {
    Object* obj = newObject(variableType, array, variableFlag,
                            newSymbol(variableName, -1, variableAddress++, yylineno, NULL));
    linkedList_addPtr(&funcParm, obj);
}

void functionCreate(ObjectType returnType, bool array, char* funcName) {
    printf("func: %s\n", funcName);
    Object* funcObj = newObject(returnType, array, 0, NULL);
    int funcIndex = ((Map*)scopeListStack.last->value)->size;
    map_putpp(scopeListStack.last->value, (void*)funcName, funcObj);
    printf("> Insert `%s` (addr: -1) to scope level %u\n", funcName, scopeLevel);
    pushScope();
    // Add variables
    char* sig = malloc(64);
    int sigIndex = 1;
    sig[0] = '(';
    linkedList_foreachPtr(&funcParm, Object * obj, {
        SymbolData* symbol = obj->symbol;
        if (obj->type == OBJECT_TYPE_STR) {
            if (obj->array) sig[sigIndex++] = '[';
            strcpy(sig + sigIndex, "Ljava/lang/String;");
            sigIndex += 18;
        } else
            sig[sigIndex++] = toupper(*objectTypeName[obj->type]);
        symbol->index = ((Map*)scopeListStack.last->value)->size;
        map_putpp(scopeListStack.last->value, symbol->name, obj);
        printf("> Insert `%s` (addr: %ld) to scope level %u\n", symbol->name, symbol->addr, scopeLevel);
    });
    if (funcParm.length == 0)
        sig[sigIndex++] = 'V';
    sig[sigIndex++] = ')';
    sig[sigIndex++] = toupper(*objectTypeName[returnType]);
    sig = realloc(sig, sigIndex + 1);
    sig[sigIndex] = 0;
    // Create function description
    SymbolData* symbol = funcObj->symbol = newSymbol(funcName, funcIndex, -1, yylineno, sig);
    linkedList_free(&funcParm);
}

void functionArgNew() {
    LinkedList* funcArg = linkedList_addPtr(&funcArgStack, linkedList_new());
}

void functionArgPush(Object* obj) {
    Object* cache = (Object*)malloc(sizeof(Object));
    memcpy(cache, obj, sizeof(Object));
    linkedList_addPtr(funcArgStack.last->value, cache);
}

void functionCall(char* funcName, Object* out) {
    Object* funcObj = findVariable(funcName);
    if (!funcObj || !funcObj->symbol) return;
    SymbolData* symbol = funcObj->symbol;

    printf("call: %s%s\n", funcName, symbol->func_sig);
    out->type = funcObj->type;

    linkedList_free(funcArgStack.last->value);
    linkedList_removeNode(&funcArgStack, funcArgStack.last);
}

void arrayCreate(Object* out) {
    // LinkedList<Object*>
    LinkedList* elements = (LinkedList*)funcArgStack.last->value;

    printf("create array: %lu\n", elements->length);
    linkedList_free(funcArgStack.last->value);
    linkedList_removeNode(&funcArgStack, funcArgStack.last);
}

bool objectExpression(char op, Object* a, Object* b, Object* out) {
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

    uint8_t aPri = objectTypePriority[a->type], bPri = objectTypePriority[b->type];
    if (!aPri || !bPri)
        return true;

    if (aPri < bPri)
        out->type = b->type;
    else
        out->type = a->type;

    return false;
}

bool objectExpBinary(char op, Object* a, Object* b, Object* out) {
    switch (op) {
    case '>':  // >>
        printf("SHR\n");
        break;
    case '<':  // <<
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

    uint8_t aPri = objectTypePriority[a->type], bPri = objectTypePriority[b->type];
    if (!aPri || !bPri)
        return true;

    if (aPri < bPri)
        out->type = b->type;
    else
        out->type = a->type;
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
    case '.':  // >=
        printf("GEQ\n");
        break;
    case ',':  // <=
        printf("LEQ\n");
        break;
    case '=':
        printf("EQL\n");
        break;
    case '!':  // !=
        printf("NEQ\n");
        break;
    case '&':  // &&
        printf("LAN\n");
        break;
    case '|':  // ||
        printf("LOR\n");
        break;
    default:
        return true;
    }
    out->type = OBJECT_TYPE_BOOL;
    return false;
}

bool objectNotBinaryExpression(Object* a, Object* out) {
    printf("BNT\n");
    out->type = a->type;
    return false;
}

bool objectNegExpression(Object* a, Object* out) {
    printf("NEG\n");
    out->type = a->type;
    return false;
}

bool objectNotBooleanExpression(Object* a, Object* out) {
    printf("NOT\n");
    out->type = OBJECT_TYPE_BOOL;
    return false;
}

bool objectCast(ObjectType variableType, Object* a, Object* out) {
    printf("Cast to %s\n", objectTypeName[variableType]);
    out->type = variableType;
    return false;
}

bool objectArrayGet(Object* obj, Object* index, Object* out) {
    if (!obj->array) return true;
    out->type = obj->type;
    return false;
}

// ++
bool objectIncAssign(Object* a, Object* out) {
    printf("INC_ASSIGN\n");
    out->type = a->type;
    return false;
}

// --
bool objectDecAssign(Object* a, Object* out) {
    printf("DEC_ASSIGN\n");
    out->type = a->type;
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
    case '>':  // >>=
        printf("SHR_ASSIGN\n");
        break;
    case '<':  // <<=
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
    out->type = dest->type;
    return false;
}

bool objectValueAssign(Object* dest, Object* val, Object* out) {
    printf("VAL_ASSIGN\n");
    out->type = dest->type;
    return false;
}

Object* findVariable(char* variableName) {
    Object* variable = NULL;

    LinkedListNode* node = scopeListStack.last;
    Map* scope = NULL;
    while (node) {
        scope = node->value;
        variable = map_get(scope, variableName);
        if (variable)
            break;
        node = node->previous;
    }
    if (!variable)
        variable = (Object*)map_get(&staticVar, (void*)variableName);
    if (!variable)
        return NULL;
    printf("IDENT (name=%s, address=%ld)\n", variableName, variable->symbol->addr);
    return variable;
}

void stdoutPrint() {
    LinkedList* funcArgs = funcArgStack.last->value;
    printf("cout");
    linkedList_foreachPtr(funcArgs, Object * obj, {
        printf(" %s", objectTypeName[obj->type]);
    });
    linkedList_free(funcArgs);
    linkedList_removeNode(&funcArgStack, funcArgStack.last);
    printf("\n");
}

bool forInit() {
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

bool foreachHeaderEnd(Object* obj) {
    if (foreachAutoTypeStack.length) {
        linkedList_foreachPtr(&foreachAutoTypeStack, Object * variable, {
            variable->type = obj->type;
        });
    }
    linkedList_free(&foreachAutoTypeStack);
}

void returnObject(Object* obj) {
    // printf("RETURN %s\n", objectTypeName[obj->type]);
    printf("RETURN\n");
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
    staticVar = (Map)map_create(objectInfo);
    Object endl = {OBJECT_TYPE_STR, false, 0, &(SymbolData){"endl", 0, -1}};
    map_putpp(&staticVar, "endl", &endl);

    yyparse();
    printf("Total lines: %d\n", yylineno);
    fclose(yyin);

    yylex_destroy();
    return 0;
}