#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define WJCL_LINKED_LIST_IMPLEMENTATION
#include "main.h"
#define WJCL_HASH_MAP_IMPLEMENTATION
#include "../../WJCL/map/wjcl_hash_map.h"
#include "object_map_entries.h"
#include "value_operation.h"

#define debug printf("%s:%d: ############### debug\n", __FILE__, __LINE__)

#define lower(_char) (_char + (char)32)

#define caseL(case_, code) \
    case case_:            \
        code;              \
        break

#define checkOp(val, failed) \
    if (val->type == OBJECT_TYPE_BOOL || val->type == OBJECT_TYPE_FLOAT || val->type == OBJECT_TYPE_DOUBLE || val->type == OBJECT_TYPE_STR) failed

#define _expressionBasicCheck(a) \
    a->type == OBJECT_TYPE_UNDEFINED || a->type == OBJECT_TYPE_VOID || a->type == OBJECT_TYPE_AUTO || a->type == OBJECT_TYPE_STR || a->array
#define expressionBasicCheck(a, b) if (_expressionBasicCheck(a) || _expressionBasicCheck(b))
#define expressionBasicCheck1(a) if (_expressionBasicCheck(a))

#define precalculateExpressionCheck1(a) \
    if (!a->symbol && !(a->flag & VAR_FLAG_IN_STACK))
#define precalculateExpressionCheck(a, b) \
    if (!a->symbol && !b->symbol && !(a->flag & VAR_FLAG_IN_STACK) && !(b->flag & VAR_FLAG_IN_STACK))

#define typeComparison(a, b, out)                                                       \
    if (objectTypePriority[a->type] < objectTypePriority[b->type]) out->type = b->type; \
    else out->type = a->type;

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
#define ldl(val) code("ldc_w %" PRId64, getLong(val))
#define ldf(val) code("ldc %.6f", getFloat(val))
#define ldd(val) code("ldc_w %lf", getDouble(val))
#define ldt(val) code("ldc \"%s\"", getString(val))

#define booleanResultOpFmt " BT%d ;==========\niconst_0\ngoto BE%d\nBT%d:\niconst_1\nBE%d: ;##########\n"
#define booleanResultOp booleanOpLabelCount, booleanOpLabelCount, booleanOpLabelCount, booleanOpLabelCount

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

#define asmOperationRaw(type, int, long, float, double, string, failed) \
    switch (type) {                                                     \
        caseL(OBJECT_TYPE_LONG, codeRaw(long));                         \
        caseL(OBJECT_TYPE_FLOAT, codeRaw(float));                       \
        caseL(OBJECT_TYPE_DOUBLE, codeRaw(double));                     \
        caseL(OBJECT_TYPE_STR, codeRaw(string));                        \
    case OBJECT_TYPE_BOOL:                                              \
    case OBJECT_TYPE_BYTE:                                              \
    case OBJECT_TYPE_CHAR:                                              \
    case OBJECT_TYPE_SHORT:                                             \
    case OBJECT_TYPE_INT:                                               \
        codeRaw(int);                                                   \
        break;                                                          \
    default:                                                            \
        failed;                                                         \
    }

#define asmOperation(type, int, long, float, double, string, failed) \
    switch (type) {                                                  \
        caseL(OBJECT_TYPE_LONG, long);                               \
        caseL(OBJECT_TYPE_FLOAT, float);                             \
        caseL(OBJECT_TYPE_DOUBLE, double);                           \
        caseL(OBJECT_TYPE_STR, string);                              \
    case OBJECT_TYPE_BOOL:                                           \
    case OBJECT_TYPE_BYTE:                                           \
    case OBJECT_TYPE_CHAR:                                           \
    case OBJECT_TYPE_SHORT:                                          \
    case OBJECT_TYPE_INT:                                            \
        int;                                                         \
        break;                                                       \
    default:                                                         \
        failed;                                                      \
    }

Object valueOne = {OBJECT_TYPE_INT, false, 1, 0, NULL};
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

int scopeLevel = 0;

Map staticVar;                                          // Map<char*, Object*>
LinkedList scopeListStack = linkedList_create();        // LinkedList<Map<char*, Object*>*>
LinkedList scopeReturnStack = linkedList_create();      // LinkedList<Object*>
LinkedList funcParm = linkedList_create();              // LinkedList<Object*>
LinkedList funcArgStack = linkedList_create();          // LinkedList<LinkedList<Object*>>
LinkedList foreachAutoTypeStack = linkedList_create();  // LinkedList<Object*>
LinkedList functionLocalsStack = linkedList_create();   // LinkedList<int>

// Stacks for tracking label count
LinkedList ifStack = linkedList_create();     // LinkedList<int>
LinkedList whileStack = linkedList_create();  // LinkedList<int>
LinkedList forStack = linkedList_create();    // LinkedList<int>

int booleanOpLabelCount = 0, ifLabelCount = 0, whileLabelCount = 0, forLabelCount = 0;
int currentLoopType = 0;
#define LOOP_TYPE_FOR 1
#define LOOP_TYPE_WHILE 2

void pushScope() {
    printf("> Create symbol table (scope level %d)\n", ++scopeLevel);
    linkedList_addPtr(&scopeListStack, map_new(objectInfo));
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
}

static inline Object* newObject(ObjectType variableType, int array, uint64_t value, SymbolData* symbol) {
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

bool initVariable(ObjectType variableType, LinkedList* arraySubscripts, char* variableName) {
    if (!arraySubscripts)
        return false;
    printf("init variable\n");

    // Create array
    Object* elementCount;
    char cache[1024];
    int i = 0;
    linkedList_foreachPtr(arraySubscripts, elementCount, {
        if (elementCount->type != OBJECT_TYPE_CHAR && elementCount->type != OBJECT_TYPE_BYTE &&
            elementCount->type != OBJECT_TYPE_SHORT && elementCount->type != OBJECT_TYPE_INT)
            return true;
        cache[i++] = '[';
        loadIfNotInStack(elementCount, return true);
    });
    // Multi dim array
    if (arraySubscripts->length > 1) {
        strcpy(cache + i, objectJavaTypeName[variableType]);
        code("multianewarray %s %ld", cache, arraySubscripts->length);
    } else
        code("newarray %s", objectTypeName[variableType]);

    return false;
}

Object* createVariable(ObjectType variableType, LinkedList* arraySubscripts, char* variableName, Object* value) {
    int index = ((Map*)scopeListStack.last->value)->size;
    if (variableType == OBJECT_TYPE_AUTO && value)
        variableType = value->type;

    int arrayDim = arraySubscripts ? arraySubscripts->length : 0;
    int* localAddress = functionLocalsStack.last->value;
    Object* obj = newObject(variableType, arrayDim, 0,
                            newSymbol(variableName, index, (*localAddress)++, yylineno, NULL));
    arraySubscriptEnd(arraySubscripts);

    map_putpp(scopeListStack.last->value, (void*)variableName, obj);
    printf("> Insert `%s` (addr: %ld) to scope level %u\n", variableName, obj->symbol->addr, scopeLevel);

    // Auto type in for each loop, need assign later
    if (variableType == OBJECT_TYPE_AUTO && !value)
        linkedList_addPtr(&foreachAutoTypeStack, obj);
    else if (obj->array)
        // Store array
        astore(obj);
    else if (value) {
        if (value->array) {
            Object* index;
            linkedList_foreachPtr(value->arraySubscript, index, {
                aload(value);
                loadIfNotInStack(index, return NULL);
                codeRaw("iaload");
            });
            arraySubscriptEnd(value->arraySubscript);
        } else
            // Store value
            loadIfNotInStack(value, return NULL);
        storeTo(obj, return NULL);
    }

    return obj;
}

void functionLocalsBegin() {
    linkedList_add(&functionLocalsStack, 0);
}

void functionParmPush(ObjectType returnType, LinkedList* arraySubscripts, char* variableName) {
    int arrayDim = arraySubscripts ? arraySubscripts->length : 0;
    int* localAddress = functionLocalsStack.last->value;
    Object* obj = newObject(returnType, arrayDim, 0,
                            newSymbol(variableName, -1, (*localAddress)++, yylineno, NULL));
    arraySubscriptEnd(arraySubscripts);
    linkedList_addPtr(&funcParm, obj);
}

void functionBegin(ObjectType returnType, LinkedList* arraySubscripts, char* funcName) {
    printf("func: %s\n", funcName);
    int arrayDim = arraySubscripts ? arraySubscripts->length : 0;
    Object* funcObj = newObject(returnType, arrayDim, 0, NULL);
    arraySubscriptEnd(arraySubscripts);
    int funcIndex = ((Map*)scopeListStack.last->value)->size;
    map_putpp(scopeListStack.last->value, (void*)funcName, funcObj);
    printf("> Insert `%s` (addr: -1) to scope level %u\n", funcName, scopeLevel);
    pushScope();

    // main function return void, TODO: remove this later
    if (strncmp("main", funcName, 4) == 0)
        funcObj->type = OBJECT_TYPE_VOID;
    linkedList_addPtr(&scopeReturnStack, funcObj);

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
    strcpy(sig + sigIndex, objectJavaTypeName[funcObj->type]);
    sigIndex += objectJavaTypeNameLen[funcObj->type];
    // Realloc string length
    sig = realloc(sig, sigIndex + 1);
    sig[sigIndex] = 0;
    // Create function description
    SymbolData* symbol = funcObj->symbol = newSymbol(funcName, funcIndex, -1, yylineno, sig);
    linkedList_free(&funcParm);

    --scopeLevel;
    code("\n.method public static %s%s", funcName, sig);
    code(".limit stack %d", 100);
    code(".limit locals %d", 100);
    ++scopeLevel;
}

bool functionEnd(ObjectType returnType) {
    // TODO: remove this later
    Object* lastReturn = (Object*)scopeReturnStack.last->value;
    if (lastReturn->symbol && strncmp("main", lastReturn->symbol->name, 4) == 0)
        returnType = OBJECT_TYPE_VOID;

    if (returnType == OBJECT_TYPE_VOID) {
        codeRaw("return");
    } else {
        Object* scope = (Object*)scopeReturnStack.last->value;
        if (!scope)
            return true;
        if (scope->type != returnType)
            return true;
    }
    linkedList_removeNode(&scopeReturnStack, scopeReturnStack.last);
    linkedList_removeNode(&functionLocalsStack, functionLocalsStack.last);
    dumpScope();
    codeRaw(".end method");
    return false;
}

void functionArgsBegin() {
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
    out->array = 0;
    out->flag = VAR_FLAG_IN_STACK;
    out->symbol = NULL;

    // Load args
    LinkedList* funcArgs = (LinkedList*)funcArgStack.last->value;
    Object* obj;
    linkedList_foreachPtr(funcArgs, obj, {
        loadIfNotInStack(obj, return);
    });
    // Call function
    code("invokestatic Main/%s%s", funcName, symbol->func_sig);

    linkedList_free(funcArgStack.last->value);
    linkedList_removeNode(&funcArgStack, funcArgStack.last);
}

bool objectExpression(char op, Object* a, Object* b, Object* out) {
    expressionBasicCheck(a, b) return true;
    precalculateExpressionCheck(a, b) return valueOperation(op, a, b, out);
    typeComparison(a, b, out);

    out->flag = VAR_FLAG_IN_STACK;
    out->symbol = NULL;

    loadIfNotInStack(a, return true);
    loadIfNotInStack(b, return true);
    // b load before a
    if (b->flag & VAR_FLAG_IN_STACK && !(a->flag & VAR_FLAG_IN_STACK))
        codeRaw("swap");

    switch (op) {
    case '+':
        printf("ADD\n");
        asmOperationRaw(out->type, "iadd", "ladd", "fadd", "dadd", , return true);
        break;
    case '-':
        printf("SUB\n");
        asmOperationRaw(out->type, "isub", "lsub", "fsub", "dsub", , return true);
        break;
    case '*':
        printf("MUL\n");
        asmOperationRaw(out->type, "imul", "lmul", "fmul", "dmul", , return true);
        break;
    case '/':
        printf("DIV\n");
        asmOperationRaw(out->type, "idiv", "ldiv", "fdiv", "ddiv", , return true);
        break;
    case '%':
        printf("REM\n");
        checkOp(a, return true);
        checkOp(b, return true);
        asmOperationRaw(out->type, "irem", "lrem", , , , return true);
        break;
    default:
        return true;
    }

    return false;
}

bool objectExpBinary(char op, Object* a, Object* b, Object* out) {
    expressionBasicCheck(a, b) return true;
    precalculateExpressionCheck(a, b) return binaryOperation(op, a, b, out);
    typeComparison(a, b, out);

    out->flag = VAR_FLAG_IN_STACK;
    out->symbol = NULL;

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
        asmOperationRaw(out->type, "ishr", "lshr", , , , return true);
        break;
    case '<':
        printf("SHL\n");
        checkOp(a, return true);
        checkOp(b, return true);
        asmOperationRaw(out->type, "ishl", "lshl", , , , return true);
        break;
    case '&':
        printf("BAN\n");
        checkOp(a, return true);
        checkOp(b, return true);
        asmOperationRaw(out->type, "iand", "land", , , , return true);
        break;
    case '|':
        printf("BOR\n");
        checkOp(a, return true);
        checkOp(b, return true);
        asmOperationRaw(out->type, "ior", "lor", , , , return true);
        break;
    case '^':
        printf("BXO\n");
        checkOp(a, return true);
        checkOp(b, return true);
        asmOperationRaw(out->type, "ixor", "lxor", , , , return true);
        break;
    default:
        return true;
    }

    return false;
}

bool objectExpBoolean(char op, Object* a, Object* b, Object* out) {
    expressionBasicCheck(a, b) return true;
    precalculateExpressionCheck(a, b) return booleanOperation(op, a, b, out);

    loadIfNotInStack(a, return true);
    loadIfNotInStack(b, return true);
    // b load before a
    if (b->flag & VAR_FLAG_IN_STACK && !(a->flag & VAR_FLAG_IN_STACK))
        codeRaw("swap");

    out->type = OBJECT_TYPE_BOOL;
    out->flag = VAR_FLAG_IN_STACK;
    out->symbol = NULL;

    switch (op) {
    case '>':
        printf("GTR\n");
        code("if_icmpgt" booleanResultOpFmt, booleanResultOp);
        break;
    case '<':
        printf("LES\n");
        code("if_icmplt" booleanResultOpFmt, booleanResultOp);
        break;
    case '.':  // >=
        printf("GEQ\n");
        code("if_icmpge" booleanResultOpFmt, booleanResultOp);
        break;
    case ',':  // <=
        printf("LEQ\n");
        code("if_icmple" booleanResultOpFmt, booleanResultOp);
        break;
    case '=':  // ==
        printf("EQL\n");
        code("if_icmpeq" booleanResultOpFmt, booleanResultOp);
        break;
    case '!':  // !=
        printf("NEQ\n");
        code("if_icmpne" booleanResultOpFmt, booleanResultOp);
        break;
    case '&':  // &&
        printf("LAN\n");
        code("ifeq BFP%d ;==========\nifeq BF%d\niconst_1\ngoto BE%d\nBFP%d:\npop\nBF%d:\niconst_0\nBE%d: ;##########\n",
             booleanResultOp, booleanOpLabelCount, booleanOpLabelCount);
        break;
    case '|':  // ||
        printf("LOR\n");
        code("ifeq BTP%d ;==========\nifeq BT%d\niconst_0\ngoto BE%d\nBTP%d:\npop\nBT%d:\niconst_1\nBE%d: ;##########\n",
             booleanResultOp, booleanOpLabelCount, booleanOpLabelCount);
        break;
    default:
        return true;
    }
    ++booleanOpLabelCount;

    return false;
}

// ~a
bool objectNotBinaryExpression(Object* a, Object* out) {
    expressionBasicCheck1(a) return true;
    precalculateExpressionCheck1(a) return valueOperationBinaryNot(a, out);

    printf("BNT\n");
    out->type = a->type;
    out->flag = VAR_FLAG_IN_STACK;
    out->symbol = NULL;
    return true;

    return false;
}

// -a
bool objectNegExpression(Object* a, Object* out) {
    expressionBasicCheck1(a) return true;
    precalculateExpressionCheck1(a) return valueOperationNeg(a, out);

    printf("NEG\n");

    out->type = a->type;
    out->flag = VAR_FLAG_IN_STACK;
    out->symbol = NULL;

    loadIfNotInStack(a, return true);
    asmOperationRaw(a->type, "inge", "lnge", "fnge", "dnge", , return true);

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

// (int) a
bool objectCast(ObjectType variableType, Object* a, Object* out) {
    printf("Cast to %s\n", objectTypeName[variableType]);
    out->type = variableType;
    out->flag = VAR_FLAG_IN_STACK;
    out->symbol = a->symbol;
    loadIfNotInStack(a, return true);

    if (a->type == OBJECT_TYPE_BOOL || a->type == OBJECT_TYPE_STR ||
        variableType == OBJECT_TYPE_BOOL || variableType == OBJECT_TYPE_STR)
        return true;
    code("%c2%c", lower(*objectJavaTypeName[a->type]), lower(*objectJavaTypeName[variableType]));
    return false;
}

// a += 123
bool objectExpAssign(char op, Object* dest, Object* val, Object* out) {
    // load value
    loadIfNotInStack(dest, return true);
    loadIfNotInStack(val, return true);

    if (val->type == OBJECT_TYPE_STR || dest->type == OBJECT_TYPE_STR)
        return true;

    switch (op) {
    case '+':
        printf("ADD_ASSIGN\n");
        asmOperationRaw(dest->type, "iadd", "ladd", "fadd", "dadd", , return true);
        break;
    case '-':
        printf("SUB_ASSIGN\n");
        asmOperationRaw(dest->type, "isub", "lsub", "fsub", "dsub", , return true);
        break;
    case '*':
        printf("MUL_ASSIGN\n");
        asmOperationRaw(dest->type, "imul", "lmul", "fmul", "dmul", , return true);
        break;
    case '/':
        printf("DIV_ASSIGN\n");
        asmOperationRaw(dest->type, "idiv", "ldiv", "fdiv", "ddiv", , return true);
        break;
    case '%':
        printf("REM_ASSIGN\n");
        checkOp(dest, return true);
        checkOp(val, return true);
        asmOperationRaw(dest->type, "irem", "lrem", , , , return true);
        break;
    case '>':  // >>=
        printf("SHR_ASSIGN\n");
        checkOp(dest, return true);
        checkOp(val, return true);
        asmOperationRaw(dest->type, "ishr", "lshr", , , , return true);
        break;
    case '<':  // <<=
        printf("SHL_ASSIGN\n");
        checkOp(dest, return true);
        checkOp(val, return true);
        asmOperationRaw(dest->type, "ishl", "lshl", , , , return true);
        break;
    case '&':
        printf("BAN_ASSIGN\n");
        checkOp(dest, return true);
        checkOp(val, return true);
        asmOperationRaw(dest->type, "iand", "land", , , , return true);
        break;
    case '|':
        printf("BOR_ASSIGN\n");
        checkOp(dest, return true);
        checkOp(val, return true);
        asmOperationRaw(dest->type, "ior", "lor", , , , return true);
        break;
    case '^':
        printf("BXO_ASSIGN\n");
        checkOp(dest, return true);
        checkOp(val, return true);
        asmOperationRaw(dest->type, "ixor", "lxor", , , , return true);
        break;
    default:
        return true;
    }

    storeTo(dest, return true);

    out->type = dest->type;
    out->symbol = NULL;
    return false;
}

// a = 123
bool objectValueAssign(Object* dest, Object* val, Object* out) {
    printf("VAL_ASSIGN\n");

    if (val->type != dest->type) {
        if (val->type == OBJECT_TYPE_BOOL || val->type == OBJECT_TYPE_STR ||
            dest->type == OBJECT_TYPE_BOOL || dest->type == OBJECT_TYPE_STR)
            return true;
        code("%c2%c", lower(*objectJavaTypeName[val->type]), lower(*objectJavaTypeName[dest->type]));
    }

    out->type = dest->type;
    out->symbol = NULL;

    // Assign array
    if (dest->array) {
        Object* index;
        int i = dest->arraySubscript->length;
        // Save value first
        if (val->flag & VAR_FLAG_IN_STACK) {
            int64_t localAddress = *(int*)functionLocalsStack.last->value + 1;
            code("istore %" PRId64 " ; ##tmp", localAddress);
            val = &(Object){val->type, 0, 0, 0, &(SymbolData){"##tmp", 0, localAddress}};
        }

        aload(dest);
        linkedList_foreachPtr(dest->arraySubscript, index, {
            // Load array index
            loadIfNotInStack(index, return true);
            // Multi dim array
            if (i-- > 1) {
                codeRaw("aaload");
            } else {
                loadIfNotInStack(val, return true);
                codeRaw("iastore");
            }
        });
        arraySubscriptEnd(dest->arraySubscript);
    } else {
        loadIfNotInStack(val, return true);
        storeTo(dest, return true);
    }

    return false;
}

// ++
bool objectIncAssign(Object* a, Object* out) {
    printf("INC_ASSIGN\n");
    if (a->type == OBJECT_TYPE_BOOL) return true;

    // loadIfNotInStack(a, return true);
    out->type = a->type;
    out->flag = VAR_FLAG_IN_STACK;
    out->symbol = NULL;

    asmOperation(a->type,
                 code("iinc %" PRId64 " 1", a->symbol->addr),
                 code("linc %" PRId64 " 1", a->symbol->addr),
                 code("finc %" PRId64 " 1", a->symbol->addr),
                 code("dinc %" PRId64 " 1", a->symbol->addr),
                 , return true);

    // return objectExpAssign('+', a, &valueOne, a);
    return false;
}

// --
bool objectDecAssign(Object* a, Object* out) {
    printf("DEC_ASSIGN\n");
    if (a->type == OBJECT_TYPE_BOOL) return true;

    // loadIfNotInStack(a, return true);
    out->type = a->type;
    out->flag = VAR_FLAG_IN_STACK;
    out->symbol = NULL;

    asmOperation(a->type,
                 code("iinc %" PRId64 " -1", a->symbol->addr),
                 code("linc %" PRId64 " -1", a->symbol->addr),
                 code("finc %" PRId64 " -1", a->symbol->addr),
                 code("dinc %" PRId64 " -1", a->symbol->addr),
                 , return true);

    // return objectExpAssign('-', a, &valueOne, a);
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

bool stdoutPrint(Object* obj) {
    printf("print %s\n", objectTypeName[obj->type]);

    codeRaw("getstatic java/lang/System/out Ljava/io/PrintStream;");
    // Newline
    if (obj->symbol && obj->symbol->addr == -1) {
        codeRaw("invokevirtual java/io/PrintStream/println()V");
        return false;
    }
    // Load array
    if (obj->array) {
        Object* index;
        int i = obj->arraySubscript->length;
        aload(obj);
        linkedList_foreachPtr(obj->arraySubscript, index, {
            // Load array index
            loadIfNotInStack(index, return true);
            // Multi dim array
            if (i-- > 1)
                codeRaw("aaload");
            else
                codeRaw("iaload");
        });
        arraySubscriptEnd(obj->arraySubscript);
    } else
        loadIfNotInStack(obj, return true);
    if (obj->flag & VAR_FLAG_IN_STACK)
        codeRaw("swap");
    code("invokevirtual java/io/PrintStream/print(%s)V", objectJavaTypeName[obj->type]);
    return true;
}

bool ifBegin(Object* a) {
    printf("IF_BEGIN\n");

    loadIfNotInStack(a, return true);
    code("ifeq IFE_%d", ifLabelCount);
    code("IF_%d:", ifLabelCount);
    linkedList_add(&ifStack, ifLabelCount);
    ++ifLabelCount;
    return false;
}

bool ifEnd() {
    printf("IF_END\n");
    int count = *(int*)ifStack.last->value;
    code("IFE_%d:", count);
    return false;
}

bool ifOnlyEnd() {
    printf("IF_END\n");
    int count = *(int*)ifStack.last->value;
    code("IFE_%d:", count);
    linkedList_removeNode(&ifStack, ifStack.last);
    return false;
}

bool elseBegin() {
    printf("ELSE_BEGIN\n");
    int count = *(int*)ifStack.last->value;
    code("goto ELSEE_%d", count);
    code("IFE_%d:", count);
    code("ELSE_%d:", count);
    return false;
}

bool elseEnd() {
    printf("ELSE_END\n");
    int count = *(int*)ifStack.last->value;
    code("ELSEE_%d:", count);
    linkedList_removeNode(&ifStack, ifStack.last);
}

bool whileBegin() {
    printf("WHILE_BEGIN\n");
    code("WHILE%d:", whileLabelCount);

    linkedList_add(&whileStack, whileLabelCount);
    ++whileLabelCount;
    return false;
}

bool whileBodyBegin() {
    int count = *(int*)whileStack.last->value;
    code("ifeq WHILE%d_", count);
    currentLoopType = LOOP_TYPE_WHILE;

    return false;
}

bool whileEnd() {
    printf("WHILE_END\n");

    int count = *(int*)whileStack.last->value;
    code("goto WHILE%d", count);
    code("WHILE%d_:", count);
    currentLoopType = 0;

    linkedList_removeNode(&whileStack, whileStack.last);
    return false;
}

bool forBegin() {
    printf("FOR_BEGIN\n");
    pushScope();
    return false;
}

bool forInitEnd() {
    printf("FOR_INIT_END\n");
    // For condition start
    code("FORC%d:", forLabelCount);

    linkedList_add(&forStack, forLabelCount);
    ++forLabelCount;
    return false;
}

bool forConditionEnd(Object* result) {
    printf("FOR_CONDITION_END\n");
    int count = *(int*)forStack.last->value;
    // Check condition
    code("ifeq FORE%d", count);
    // Go body if true
    code("goto FORB%d", count);

    // For hader start
    code("FORH%d:", count);
    return false;
}

bool forHeaderEnd() {
    printf("FOR_HEADER_END\n");
    int count = *(int*)forStack.last->value;
    // After header instruction, check condition
    code("goto FORC%d", count);

    // For body start
    code("FORB%d:", count);
    currentLoopType = LOOP_TYPE_FOR;

    return false;
}

bool forEnd() {
    int count = *(int*)forStack.last->value;
    // Go do header instruction
    code("goto FORH%d", count);

    code("FORE%d:", count);
    currentLoopType = 0;

    linkedList_removeNode(&forStack, forStack.last);
    dumpScope();
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

bool returnObject(Object* obj) {
    ObjectType type = obj ? obj->type : OBJECT_TYPE_VOID;
    printf("RETURN %s\n", objectJavaTypeName[type]);

    // TODO: remove this later
    Object* lastReturn = (Object*)scopeReturnStack.last->value;
    if (lastReturn->symbol && strncmp("main", lastReturn->symbol->name, 4) == 0)
        return false;

    if (type == OBJECT_TYPE_VOID) {
        codeRaw("return");
        return false;
    }

    // Set return object;
    scopeReturnStack.last->value = obj;
    asmOperationRaw(type, "ireturn", "lreturn", "freturn", "dreturn", "areturn", return true);
    return false;
}

bool breakLoop() {
    printf("BREAK\n");
    if (currentLoopType == LOOP_TYPE_WHILE) {
        code("goto WHILE%d_", *(int*)whileStack.last->value);
    } else if (currentLoopType == LOOP_TYPE_FOR) {
        code("goto FORE%d", *(int*)forStack.last->value);
    }
    return false;
}

bool arrayCreate(Object* out) {
    // LinkedList<Object*>
    LinkedList* elements = (LinkedList*)funcArgStack.last->value;
    printf("create array: %lu\n", elements->length);

    Object* obj;
    // Check array element types
    ObjectType type = OBJECT_TYPE_UNDEFINED;
    linkedList_foreachPtr(elements, obj, {
        if (type == OBJECT_TYPE_UNDEFINED)
            type = obj->type;
        else if (type != obj->type)
            return true;
    });

    // Load elements
    int index = 0;
    linkedList_foreachPtr(elements, obj, {
        codeRaw("dup");
        // TODO: support double
        code("ldc %d", index++);
        loadIfNotInStack(obj, return true);
        codeRaw("iastore");
    });
    out->type = type;
    out->array = 1;
    out->flag = VAR_FLAG_IN_STACK;
    out->symbol = NULL;

    linkedList_free(funcArgStack.last->value);
    linkedList_removeNode(&funcArgStack, funcArgStack.last);
    return false;
}

bool objectArrayGet(Object* arr, LinkedList* arraySubscripts, Object* out) {
    if (!arr->array) return true;
    out->type = arr->type;
    out->array = arr->array;
    out->flag = 0;
    out->symbol = arr->symbol;
    out->arraySubscript = arraySubscripts;

    // Object* index;
    // linkedList_foreachPtr(out->arraySubscript, index, {
    //     printf("[%d] ", getInt(index));
    // });
    // printf("\n");

    return false;
}

LinkedList* arraySubscriptBegin(Object* index) {
    LinkedList* linkedlist = linkedList_new();
    arraySubscriptPush(linkedlist, index);
    return linkedlist;
}

bool arraySubscriptPush(LinkedList* arraySubscripts, Object* index) {
    if (index) {
        Object* clone = (Object*)malloc(sizeof(Object));
        memcpy(clone, index, sizeof(Object));
        index = clone;
    }
    linkedList_addPtr(arraySubscripts, index);
    return false;
}

bool arraySubscriptEnd(LinkedList* arraySubscripts) {
    linkedList_free(arraySubscripts);
    return false;
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
    codeRaw(".super java/lang/Object");
    scopeLevel = -1;

    yyparse();
    printf("Total lines: %d\n", yylineno);
    fclose(yyin);

    yylex_destroy();
    return 0;
}