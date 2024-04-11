#ifndef COMPILER_COMMON_H
#define COMPILER_COMMON_H

#include <stdbool.h>
#include <stdint.h>

typedef enum _objectType {
    OBJECT_TYPE_UNDEFINED,
    OBJECT_TYPE_AUTO,
    OBJECT_TYPE_VOID,
    OBJECT_TYPE_CHAR,
    OBJECT_TYPE_INT,
    OBJECT_TYPE_LONG,
    OBJECT_TYPE_FLOAT,
    OBJECT_TYPE_DOUBLE,
    OBJECT_TYPE_BOOL,
    OBJECT_TYPE_STR,
    OBJECT_TYPE_FUNCTION,
    OBJECT_TYPE_ARRAY,
} ObjectType;

typedef struct _symbolData {
    char* name;
    uint32_t index;
    int8_t mutable;
    uint64_t addr;
    uint32_t lineno;
    char* func_sig;
    uint8_t func_var;
    bool write;
} SymbolData;

typedef struct _object {
    ObjectType type;
    uint64_t value;
    int ptrOffset;
    SymbolData* symbol;
} Object;

extern int yylineno;

#endif /* COMPILER_COMMON_H */