#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "main.h"
#define DEBUG_OUT
#define WJCL_LINKED_LIST_IMPLEMENTATION
#define WJCL_HASH_MAP_IMPLEMENTATION
#include "../../WJCL/map/wjcl_hash_map.h"
#include "../../WJCL/list/wjcl_linked_list.h"
#include "object_map_entries.h"
#include "value_operation.h"

#define debug printf("%s:%d: ############### debug\n", __FILE__, __LINE__)

#define caseL(case_, code) \
    case case_:            \
        code;              \
        break

#define checkOp(val, failed) \
    if (val->type == OBJECT_TYPE_BOOL || val->type == OBJECT_TYPE_FLOAT || val->type == OBJECT_TYPE_DOUBLE || val->type == OBJECT_TYPE_STR) failed

#define iload(var) code("iload %" PRId64 " ; %s", (var)->symbol->addr, (var)->symbol->name)
#define lload(var) code("lload %" PRId64 " ; %s", (var)->symbol->addr, (var)->symbol->name)
#define fload(var) code("fload %" PRId64 " ; %s", (var)->symbol->addr, (var)->symbol->name)
#define dload(var) code("dload %" PRId64 " ; %s", (var)->symbol->addr, (var)->symbol->name)
#define aload(var) code("aload %" PRId64 " ; %s", (var)->symbol->addr, (var)->symbol->name)

#define istore(var) code("istore %" PRId64 " ; %s", (var)->symbol->addr, (var)->symbol->name)
#define lstore(var) code("lstore %" PRId64 " ; %s", (var)->symbol->addr, (var)->symbol->name)
#define fstore(var) code("fstore %" PRId64 " ; %s", (var)->symbol->addr, (var)->symbol->name)
#define dstore(var) code("dstore %" PRId64 " ; %s", (var)->symbol->addr, (var)->symbol->name)
#define astore(var) code("astore %" PRId64 " ; %s", (var)->symbol->addr, (var)->symbol->name)

#define ldz(val) code("ldc %d", getBool(val))
#define ldb(val) code("ldc %d", getByte(val))
#define ldc(val) code("ldc %d", getChar(val))
#define lds(val) code("ldc %d", getShort(val))
#define ldi(val) code("ldc %d", getInt(val))
#define ldl(val) code("ldc %" PRId64, getLong(val))
#define ldf(val) code("ldc %.7f", getFloat(val))
#define ldd(val) code("ldc %lf", getDouble(val))
#define ldt(val) code("ldc \"%s\"", getString(val))

#define loadIfNotInStack(val, failed)                                              \
    if (!val->flag & VAR_FLAG_IN_STACK)                                            \
        switch (val->type) {                                                       \
            caseL(OBJECT_TYPE_SHORT, if (val->symbol) iload(val); else lds(val));  \
            caseL(OBJECT_TYPE_INT, if (val->symbol) iload(val); else ldi(val));    \
            caseL(OBJECT_TYPE_LONG, if (val->symbol) lload(val); else ldl(val));   \
            caseL(OBJECT_TYPE_FLOAT, if (val->symbol) fload(val); else ldf(val));  \
            caseL(OBJECT_TYPE_DOUBLE, if (val->symbol) dload(val); else ldd(val)); \
            caseL(OBJECT_TYPE_STR, if (val->symbol) aload(val); else ldt(val));    \
        case OBJECT_TYPE_BOOL:                                                     \
        case OBJECT_TYPE_BYTE:                                                     \
        case OBJECT_TYPE_CHAR:                                                     \
            if (val->symbol) iload(val);                                           \
            else ldc(val);                                                         \
            break;                                                                 \
        default:                                                                   \
            failed;                                                                \
        }

#define storeTo(var, failed)                    \
    switch (var->type) {                        \
        caseL(OBJECT_TYPE_LONG, lstore(var));   \
        caseL(OBJECT_TYPE_FLOAT, fstore(var));  \
        caseL(OBJECT_TYPE_DOUBLE, dstore(var)); \
        caseL(OBJECT_TYPE_STR, astore(var));    \
    case OBJECT_TYPE_BOOL:                      \
    case OBJECT_TYPE_BYTE:                      \
    case OBJECT_TYPE_CHAR:                      \
    case OBJECT_TYPE_SHORT:                     \
    case OBJECT_TYPE_INT:                       \
        istore(var);                            \
        break;                                  \
    default:                                    \
        failed;                                 \
    }

#define asmOperation(type, int, long, float, double, string, failed) \
    switch (type) {                                                  \
        caseL(OBJECT_TYPE_LONG, codeRaw(long));                      \
        caseL(OBJECT_TYPE_FLOAT, codeRaw(float));                    \
        caseL(OBJECT_TYPE_DOUBLE, codeRaw(double));                  \
        caseL(OBJECT_TYPE_STR, codeRaw(string));                     \
    case OBJECT_TYPE_BOOL:                                           \
    case OBJECT_TYPE_BYTE:                                           \
    case OBJECT_TYPE_CHAR:                                           \
    case OBJECT_TYPE_SHORT:                                          \
    case OBJECT_TYPE_INT:                                            \
        codeRaw(int);                                                \
        break;                                                       \
    default:                                                         \
        failed;                                                      \
    }

const char* objectTypeName[] = {
    [OBJECT_TYPE_UNDEFINED] = "undefined",
    [OBJECT_TYPE_VOID] = "void",
    [OBJECT_TYPE_BOOL] = "bool",
    [OBJECT_TYPE_BYTE] = "byte",
    [OBJECT_TYPE_CHAR] = "char",
    [OBJECT_TYPE_SHORT] = "short",
    [OBJECT_TYPE_INT] = "int",
    [OBJECT_TYPE_LONG] = "long",
    [OBJECT_TYPE_FLOAT] = "float",
    [OBJECT_TYPE_DOUBLE] = "double",
    [OBJECT_TYPE_STR] = "string",
};
const char* objectJavaTypeName[] = {
    [OBJECT_TYPE_UNDEFINED] = "V",
    [OBJECT_TYPE_VOID] = "V",
    [OBJECT_TYPE_BOOL] = "Z",
    [OBJECT_TYPE_BYTE] = "B",
    [OBJECT_TYPE_CHAR] = "C",
    [OBJECT_TYPE_SHORT] = "S",
    [OBJECT_TYPE_INT] = "I",
    [OBJECT_TYPE_LONG] = "J",
    [OBJECT_TYPE_FLOAT] = "F",
    [OBJECT_TYPE_DOUBLE] = "D",
    [OBJECT_TYPE_STR] = "Ljava/lang/String;",
};
const int objectJavaTypeNameLen[] = {
    [OBJECT_TYPE_UNDEFINED] = 1,
    [OBJECT_TYPE_VOID] = 1,
    [OBJECT_TYPE_BOOL] = 1,
    [OBJECT_TYPE_BYTE] = 1,
    [OBJECT_TYPE_CHAR] = 1,
    [OBJECT_TYPE_SHORT] = 1,
    [OBJECT_TYPE_INT] = 1,
    [OBJECT_TYPE_LONG] = 1,
    [OBJECT_TYPE_FLOAT] = 1,
    [OBJECT_TYPE_DOUBLE] = 1,
    [OBJECT_TYPE_STR] = 18,
};
const uint8_t objectTypeSize[] = {
    [OBJECT_TYPE_UNDEFINED] = 0,
    [OBJECT_TYPE_VOID] = 0,
    [OBJECT_TYPE_BOOL] = 1,
    [OBJECT_TYPE_BYTE] = 1,
    [OBJECT_TYPE_CHAR] = 1,
    [OBJECT_TYPE_SHORT] = 2,
    [OBJECT_TYPE_INT] = 4,
    [OBJECT_TYPE_LONG] = 8,
    [OBJECT_TYPE_FLOAT] = 4,
    [OBJECT_TYPE_DOUBLE] = 8,
    [OBJECT_TYPE_STR] = 0,
};

char* yyInputFileName;
bool compileError;

int indent = 0;
int scopeLevel = 0;
int funcLineNo = 0;
int variableAddress = 0;

Map staticVar;                                          // Map<char*, Object*>
LinkedList scopeListStack = linkedList_create();        // LinkedList<Map<char*, Object*>*>
LinkedList scopeReturnStack = linkedList_create();      // LinkedList<Object*>
LinkedList funcParm = linkedList_create();              // LinkedList<Object*>
LinkedList funcArgStack = linkedList_create();          // LinkedList<LinkedList<Object*>>
LinkedList foreachAutoTypeStack = linkedList_create();  // LinkedList<Object*>

void pushScope(Object* obj) {
    printf("> Create symbol table (scope level %d)\n", ++scopeLevel);
    linkedList_addPtr(&scopeListStack, map_new(objectInfo));
    linkedList_addPtr(&scopeReturnStack, obj);
}

void dumpScope() {
    printf("\n> Dump symbol table (scope level: %d)\n", scopeLevel);

    printf("Index     Name                Type      Addr      Lineno    Func_sig  \n");
    Map* scope = (Map*)scopeListStack.last->value;
    Object** sorted = (Object**)malloc(sizeof(Object*) * scope->size);
    map_entries(scope, i, {
        Object* obj = (Object*)i->value;
        sorted[obj->symbol->index] = obj;
    });
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

    // Pop scope stack
    map_free(scope);
    free(scope);
    linkedList_removeNode(&scopeListStack, scopeListStack.last);
    --scopeLevel;

    // Generate code
    if (scopeLevel == 0) {
        Object* scope = (Object*)scopeReturnStack.last->value;
        if (scope)
            codeRaw("    return");
        else
            codeRaw("    return");
        codeRaw(".end method");

        linkedList_removeNode(&scopeReturnStack, scopeReturnStack.last);
    }
}

static inline Object* newObject(ObjectType variableType, bool array, uint64_t value, SymbolData* symbol) {
    Object* obj = malloc(sizeof(Object));
    obj->type = variableType;
    obj->value = value;
    obj->array = array;
    obj->flag = 0;
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
    else if (value) {
        loadIfNotInStack(value, return NULL);
        storeTo(obj, return NULL);
    }

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
    pushScope(funcObj);
    // Add function
    char* sig = malloc(64);
    int sigIndex = 1;
    sig[0] = '(';
    linkedList_foreachPtr(&funcParm, Object * obj, {
        if (obj->array) sig[sigIndex++] = '[';
        // Append parameter type name
        strcpy(sig + sigIndex, objectJavaTypeName[obj->type]);
        sigIndex += objectJavaTypeNameLen[obj->type];
        // Create variable
        SymbolData* symbol = obj->symbol;
        symbol->index = ((Map*)scopeListStack.last->value)->size;
        map_putpp(scopeListStack.last->value, symbol->name, obj);
        printf("> Insert `%s` (addr: %ld) to scope level %u\n", symbol->name, symbol->addr, scopeLevel);
    });
    // Function with no parameter
    if (funcParm.length == 0) sig[sigIndex++] = 'V';
    sig[sigIndex++] = ')';
    // Function return type
    if (strncmp("main", funcName, 4)) {
        strcpy(sig + sigIndex, objectJavaTypeName[returnType]);
        sigIndex += objectJavaTypeNameLen[returnType];
    } else sig[sigIndex++] = 'V';  // main function return void
    // Realloc string length
    sig = realloc(sig, sigIndex + 1);
    sig[sigIndex] = 0;
    // Create function description
    SymbolData* symbol = funcObj->symbol = newSymbol(funcName, funcIndex, -1, yylineno, sig);
    linkedList_free(&funcParm);

    --scopeLevel;
    code(".method public static %s%s", funcName, sig);
    code(".limit stack %d", 100);
    code(".limit locals %d", 100);
    ++scopeLevel;
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
    if (!a->symbol && !b->symbol) {
        return valueOperation(op, a, b, out);
    }

    if (a->type == OBJECT_TYPE_STR || a->type == OBJECT_TYPE_STR)
        return true;

    uint8_t aPri = objectTypePriority[a->type], bPri = objectTypePriority[b->type];
    if (aPri < bPri) out->type = b->type;
    else out->type = a->type;
    out->flag = VAR_FLAG_IN_STACK;

    loadIfNotInStack(a, return true);
    loadIfNotInStack(b, return true);
    // b load before a
    if (b->flag & VAR_FLAG_IN_STACK && !(a->flag & VAR_FLAG_IN_STACK))
        codeRaw("swap");

    switch (op) {
    case '+':
        printf("ADD\n");
        asmOperation(out->type, "iadd", "ladd", "fadd", "dadd", , return true);
        break;
    case '-':
        printf("SUB\n");
        asmOperation(out->type, "isub", "lsub", "fsub", "dsub", , return true);
        break;
    case '*':
        printf("MUL\n");
        asmOperation(out->type, "imul", "lmul", "fmul", "dmul", , return true);
        break;
    case '/':
        printf("DIV\n");
        asmOperation(out->type, "idiv", "ldiv", "fdiv", "ddiv", , return true);
        break;
    case '%':
        printf("REM\n");
        checkOp(a, return true);
        checkOp(b, return true);
        asmOperation(out->type, "irem", "lrem", , , , return true);
        break;
    default:
        return true;
    }

    return false;
}

bool objectExpBinary(char op, Object* a, Object* b, Object* out) {
    if (!a->symbol && !b->symbol) {
        return binaryOperation(op, a, b, out);
    }

    if (a->type == OBJECT_TYPE_STR || a->type == OBJECT_TYPE_STR)
        return true;

    uint8_t aPri = objectTypePriority[a->type], bPri = objectTypePriority[b->type];
    if (aPri < bPri) out->type = b->type;
    else out->type = a->type;
    out->flag = VAR_FLAG_IN_STACK;

    loadIfNotInStack(a, return true);
    loadIfNotInStack(b, return true);
    // b load before a
    if (b->flag & VAR_FLAG_IN_STACK && !(a->flag & VAR_FLAG_IN_STACK))
        codeRaw("swap");

    switch (op) {
    case '>':
        printf("SHR\n");
        checkOp(a, return true);
        checkOp(b, return true);
        asmOperation(out->type, "ishr", "lshr", , , , return true);
        break;
    case '<':
        printf("SHL\n");
        checkOp(a, return true);
        checkOp(b, return true);
        asmOperation(out->type, "ishl", "lshl", , , , return true);
        break;
    case '&':
        printf("BAN\n");
        checkOp(a, return true);
        checkOp(b, return true);
        asmOperation(out->type, "iand", "land", , , , return true);
        break;
    case '|':
        printf("BOR\n");
        checkOp(a, return true);
        checkOp(b, return true);
        asmOperation(out->type, "ior", "lor", , , , return true);
        break;
    case '^':
        printf("BXO\n");
        checkOp(a, return true);
        checkOp(b, return true);
        asmOperation(out->type, "ixor", "lxor", , , , return true);
        break;
    default:
        return true;
    }

    return false;
}

bool objectExpBoolean(char op, Object* a, Object* b, Object* out) {
    if (!a->symbol && !b->symbol) {
        return booleanOperation(op, a, b, out);
    }

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

    if (!a->symbol) {
        out->symbol = NULL;
        int8_t b_val;
        int16_t s_val;
        int32_t i_val;
        int64_t l_val;
        switch (out->type) {
        case OBJECT_TYPE_BYTE:
        case OBJECT_TYPE_CHAR:
            b_val = ~getByte(a);
            out->value = asVal(b_val);
            printf("%%b=%d\n", b_val);
            break;
        case OBJECT_TYPE_SHORT:
            s_val = ~getShort(a);
            out->value = asVal(s_val);
            printf("%%s=%d\n", s_val);
            break;
        case OBJECT_TYPE_INT:
            i_val = ~getInt(a);
            out->value = asVal(i_val);
            printf("%%i=%d\n", i_val);
            break;
        case OBJECT_TYPE_LONG:
            l_val = ~getLong(a);
            out->value = asVal(l_val);
            printf("%%l=%ld\n", l_val);
            break;
        default:
            return true;
        }
    }
    return false;
}

bool objectNegExpression(Object* a, Object* out) {
    printf("NEG\n");
    out->type = a->type;

    if (!a->symbol) {
        out->symbol = NULL;
        int8_t b_val;
        int16_t s_val;
        int32_t i_val;
        int64_t l_val;
        float f_val;
        double d_val;
        switch (out->type) {
        case OBJECT_TYPE_BYTE:
        case OBJECT_TYPE_CHAR:
            b_val = 0 - getByte(a);
            out->value = asVal(b_val);
            printf("%%b=%d\n", b_val);
            break;
        case OBJECT_TYPE_SHORT:
            s_val = 0 - getShort(a);
            out->value = asVal(s_val);
            printf("%%s=%d\n", s_val);
            break;
        case OBJECT_TYPE_INT:
            i_val = 0 - getInt(a);
            out->value = asVal(i_val);
            printf("%%i=%d\n", i_val);
            break;
        case OBJECT_TYPE_LONG:
            l_val = 0 - getLong(a);
            out->value = asVal(l_val);
            printf("%%l=%ld\n", l_val);
            break;
        case OBJECT_TYPE_FLOAT:
            f_val = 0 - getFloat(a);
            out->value = asVal(f_val);
            printf("%%f=%.7f\n", f_val);
            break;
        case OBJECT_TYPE_DOUBLE:
            d_val = 0 - getDouble(a);
            out->value = asVal(d_val);
            printf("%%d=%lf\n", d_val);
            break;
        default:
            return true;
        }
    }
    return false;
}

bool objectNotBooleanExpression(Object* a, Object* out) {
    printf("NOT\n");
    if (a->type != OBJECT_TYPE_BOOL)
        return true;

    out->type = OBJECT_TYPE_BOOL;
    out->value = !a->value;
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
    // load value
    loadIfNotInStack(dest, return true);
    loadIfNotInStack(val, return true);

    if (val->type == OBJECT_TYPE_STR || dest->type == OBJECT_TYPE_STR)
        return true;

    switch (op) {
    case '+':
        printf("ADD_ASSIGN\n");
        asmOperation(dest->type, "iadd", "ladd", "fadd", "dadd", , return true);
        break;
    case '-':
        printf("SUB_ASSIGN\n");
        asmOperation(dest->type, "isub", "lsub", "fsub", "dsub", , return true);
        break;
    case '*':
        printf("MUL_ASSIGN\n");
        asmOperation(dest->type, "imul", "lmul", "fmul", "dmul", , return true);
        break;
    case '/':
        printf("DIV_ASSIGN\n");
        asmOperation(dest->type, "idiv", "ldiv", "fdiv", "ddiv", , return true);
        break;
    case '%':
        printf("REM_ASSIGN\n");
        checkOp(dest, return true);
        checkOp(val, return true);
        asmOperation(dest->type, "irem", "lrem", , , , return true);
        break;
    case '>':  // >>=
        printf("SHR_ASSIGN\n");
        checkOp(dest, return true);
        checkOp(val, return true);
        asmOperation(dest->type, "ishr", "lshr", , , , return true);
        break;
    case '<':  // <<=
        printf("SHL_ASSIGN\n");
        checkOp(dest, return true);
        checkOp(val, return true);
        asmOperation(dest->type, "ishl", "lshl", , , , return true);
        break;
    case '&':
        printf("BAN_ASSIGN\n");
        checkOp(dest, return true);
        checkOp(val, return true);
        asmOperation(dest->type, "iand", "land", , , , return true);
        break;
    case '|':
        printf("BOR_ASSIGN\n");
        checkOp(dest, return true);
        checkOp(val, return true);
        asmOperation(dest->type, "ior", "lor", , , , return true);
        break;
    case '^':
        printf("BXO_ASSIGN\n");
        checkOp(dest, return true);
        checkOp(val, return true);
        asmOperation(dest->type, "ixor", "lxor", , , , return true);
        break;
    default:
        return true;
    }

    storeTo(dest, return true);

    out->type = dest->type;
    return false;
}

bool objectValueAssign(Object* dest, Object* val, Object* out) {
    printf("VAL_ASSIGN\n");
    out->type = dest->type;

    // load value
    loadIfNotInStack(val, return true);
    storeTo(dest, return true);
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
    Object* obj;
    linkedList_foreachPtr(funcArgs, obj, {
        printf(" %s", objectTypeName[obj->type]);

        codeRaw("getstatic java/lang/System/out Ljava/io/PrintStream;");
        // Newline
        if (obj->symbol && obj->symbol->addr == -1) {
            codeRaw("invokevirtual java/io/PrintStream/println()V");
            continue;
        }
        loadIfNotInStack(obj, );

        code("invokevirtual java/io/PrintStream/print(%s)V", objectJavaTypeName[obj->type]);
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
    char* outputFileName = NULL;
    if (argc == 3) {
        yyin = fopen(yyInputFileName = argv[1], "r");
        yyout = fopen(outputFileName = argv[2], "w");
    } else if (argc == 2) {
        yyin = fopen(yyInputFileName = argv[1], "r");
        yyout = stdout;
    } else {
        printf("require input file");
        exit(1);
    }
    if (!yyin) {
        printf("file `%s` doesn't exists or cannot be opened\n", yyInputFileName);
        exit(1);
    }
    if (!yyout) {
        printf("file `%s` doesn't exists or cannot be opened\n", outputFileName);
        exit(1);
    }

    // Start parsing
    staticVar = (Map)map_create(objectInfo);
    Object endl = {OBJECT_TYPE_STR, false, 0, 0, &(SymbolData){"endl", 0, -1}};
    map_putpp(&staticVar, "endl", &endl);

    codeRaw(".class public Main");
    codeRaw(".super java/lang/Object\n");
    scopeLevel = -1;

    yyparse();
    printf("Total lines: %d\n", yylineno);
    fclose(yyin);

    yylex_destroy();
    return 0;
}