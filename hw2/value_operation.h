#include "compiler_common.h"

#define valueOp(op, out, a, b) \
    switch (op) {              \
    case '+':                  \
        out = a + b;           \
        break;                 \
    case '-':                  \
        out = a - b;           \
        break;                 \
    case '*':                  \
        out = a * b;           \
        break;                 \
    case '/':                  \
        out = a / b;           \
        break;                 \
    case '%':                  \
        out = a % b;           \
        break;                 \
    default:                   \
        return true;           \
    }                          \
    break;

#define valueFloatOp(op, out, a, b) \
    switch (op) {                   \
    case '+':                       \
        out = a + b;                \
        break;                      \
    case '-':                       \
        out = a - b;                \
        break;                      \
    case '*':                       \
        out = a * b;                \
        break;                      \
    case '/':                       \
        out = a / b;                \
        break;                      \
    default:                        \
        return true;                \
    }                               \
    break;

#define valueBinaryOp(op, out, a, b) \
    switch (op) {                    \
    case '>':                        \
        out = a >> b;                \
        break;                       \
    case '<':                        \
        out = a << b;                \
        break;                       \
    case '&':                        \
        out = a & b;                 \
        break;                       \
    case '|':                        \
        out = a | b;                 \
        break;                       \
    case '^':                        \
        out = a ^ b;                 \
        break;                       \
    default:                         \
        return true;                 \
    }                                \
    break;

#define valueAssignOp(op, out, a) \
    switch (op) {                 \
    case '+':                     \
        out += a;                 \
        break;                    \
    case '-':                     \
        out -= a;                 \
        break;                    \
    case '*':                     \
        out *= a;                 \
        break;                    \
    case '/':                     \
        out /= a;                 \
        break;                    \
    case '%':                     \
        out %= a;                 \
        break;                    \
    case '>':                     \
        out >>= a;                \
        break;                    \
    case '<':                     \
        out <<= a;                \
        break;                    \
    case '&':                     \
        out &= a;                 \
        break;                    \
    case '|':                     \
        out |= a;                 \
        break;                    \
    case '^':                     \
        out ^= a;                 \
        break;                    \
    default:                      \
        return true;              \
    }                             \
    break;

#define valueAssignFloatOp(op, out, a) \
    switch (op) {                      \
    case '+':                          \
        out += a;                      \
        break;                         \
    case '-':                          \
        out -= a;                      \
        break;                         \
    case '*':                          \
        out *= a;                      \
        break;                         \
    case '/':                          \
        out /= a;                      \
        break;                         \
    default:                           \
        return true;                   \
    }                                  \
    break;

#define valueBooleanCmp(op, out, a, b) \
    switch (op) {                      \
    case '>':                          \
        out = a > b;                   \
        break;                         \
    case '<':                          \
        out = a < b;                   \
        break;                         \
    case '.' /*>=*/:                   \
        out = a >= b;                  \
        break;                         \
    case ',' /*<=*/:                   \
        out = a <= b;                  \
        break;                         \
    case '=':                          \
        out = a == b;                  \
        break;                         \
    case '!':                          \
        out = a != b;                  \
        break;                         \
    default:                           \
        return true;                   \
    }                                  \
    break;

#define valueBoolean(op, out, a, b) \
    switch (op) {                   \
    case '=':                       \
        out = a == b;               \
        break;                      \
    case '!':                       \
        out = a != b;               \
        break;                      \
    case '&':                       \
        out = a && b;               \
        break;                      \
    case '|':                       \
        out = a || b;               \
        break;                      \
    default:                        \
        return true;                \
    }                               \
    break;

#define sw(op, b_val, s_val, i_val, l_val, f_val, d_val, a, b) \
    switch (b->type) {                                         \
    case OBJECT_TYPE_BOOL:                                     \
    case OBJECT_TYPE_BYTE:                                     \
    case OBJECT_TYPE_CHAR:                                     \
        valueOp(op, b_val, a, getByte(b));                     \
    case OBJECT_TYPE_SHORT:                                    \
        valueOp(op, s_val, a, getShort(b));                    \
    case OBJECT_TYPE_INT:                                      \
        valueOp(op, i_val, a, getInt(b));                      \
    case OBJECT_TYPE_LONG:                                     \
        valueOp(op, l_val, a, getLong(b));                     \
    case OBJECT_TYPE_FLOAT:                                    \
        valueFloatOp(op, f_val, a, getFloat(b));               \
    case OBJECT_TYPE_DOUBLE:                                   \
        valueFloatOp(op, d_val, a, getDouble(b));              \
    default:                                                   \
        return true;                                           \
    }                                                          \
    break;

#define swFloat(op, b_val, s_val, i_val, l_val, f_val, d_val, a, b) \
    switch (b->type) {                                              \
    case OBJECT_TYPE_BOOL:                                          \
    case OBJECT_TYPE_BYTE:                                          \
    case OBJECT_TYPE_CHAR:                                          \
        valueFloatOp(op, b_val, a, getByte(b));                     \
    case OBJECT_TYPE_SHORT:                                         \
        valueFloatOp(op, s_val, a, getShort(b));                    \
    case OBJECT_TYPE_INT:                                           \
        valueFloatOp(op, i_val, a, getInt(b));                      \
    case OBJECT_TYPE_LONG:                                          \
        valueFloatOp(op, l_val, a, getLong(b));                     \
    case OBJECT_TYPE_FLOAT:                                         \
        valueFloatOp(op, f_val, a, getFloat(b));                    \
    case OBJECT_TYPE_DOUBLE:                                        \
        valueFloatOp(op, d_val, a, getDouble(b));                   \
    default:                                                        \
        return true;                                                \
    }                                                               \
    break;

#define swBinary(op, b_val, s_val, i_val, l_val, a, b) \
    switch (b->type) {                                 \
    case OBJECT_TYPE_BOOL:                             \
    case OBJECT_TYPE_BYTE:                             \
    case OBJECT_TYPE_CHAR:                             \
        valueBinaryOp(op, b_val, a, getByte(b));       \
    case OBJECT_TYPE_SHORT:                            \
        valueBinaryOp(op, s_val, a, getShort(b));      \
    case OBJECT_TYPE_INT:                              \
        valueBinaryOp(op, i_val, a, getInt(b));        \
    case OBJECT_TYPE_LONG:                             \
        valueBinaryOp(op, l_val, a, getLong(b));       \
    default:                                           \
        return true;                                   \
    }                                                  \
    break;

#define swBoolean(op, b_val, a, b)                   \
    switch (b->type) {                               \
    case OBJECT_TYPE_BYTE:                           \
    case OBJECT_TYPE_CHAR:                           \
        valueBooleanCmp(op, b_val, a, getByte(b));   \
    case OBJECT_TYPE_SHORT:                          \
        valueBooleanCmp(op, b_val, a, getShort(b));  \
    case OBJECT_TYPE_INT:                            \
        valueBooleanCmp(op, b_val, a, getInt(b));    \
    case OBJECT_TYPE_LONG:                           \
        valueBooleanCmp(op, b_val, a, getLong(b));   \
    case OBJECT_TYPE_FLOAT:                          \
        valueBooleanCmp(op, b_val, a, getFloat(b));  \
    case OBJECT_TYPE_DOUBLE:                         \
        valueBooleanCmp(op, b_val, a, getDouble(b)); \
    default:                                         \
        return true;                                 \
    }                                                \
    break;

const uint8_t objectTypePriority[] = {
    [OBJECT_TYPE_UNDEFINED] = 0,
    [OBJECT_TYPE_VOID] = 0,
    [OBJECT_TYPE_BOOL] = 1,
    [OBJECT_TYPE_BYTE] = 2,
    [OBJECT_TYPE_CHAR] = 3,
    [OBJECT_TYPE_SHORT] = 4,
    [OBJECT_TYPE_INT] = 5,
    [OBJECT_TYPE_LONG] = 6,
    [OBJECT_TYPE_FLOAT] = 7,
    [OBJECT_TYPE_DOUBLE] = 8,
    [OBJECT_TYPE_STR] = 0,
};

bool valueOperation(char op, Object* a, Object* b, Object* out) {
    out->symbol = NULL;
    out->flag = 0;

    uint8_t aPri = objectTypePriority[a->type], bPri = objectTypePriority[b->type];
    if (aPri < bPri) out->type = b->type;
    else out->type = a->type;

    int8_t b_val;
    int16_t s_val;
    int32_t i_val;
    int64_t l_val;
    float f_val;
    double d_val;

    switch (a->type) {
    case OBJECT_TYPE_BOOL:
    case OBJECT_TYPE_BYTE:
    case OBJECT_TYPE_CHAR:
        sw(op, b_val, s_val, i_val, l_val, f_val, d_val, getByte(a), b);
    case OBJECT_TYPE_SHORT:
        sw(op, s_val, s_val, i_val, l_val, f_val, d_val, getShort(a), b);
    case OBJECT_TYPE_INT:
        sw(op, i_val, i_val, i_val, l_val, f_val, d_val, getInt(a), b);
    case OBJECT_TYPE_LONG:
        sw(op, l_val, l_val, l_val, l_val, f_val, d_val, getLong(a), b);
    case OBJECT_TYPE_FLOAT:
        swFloat(op, f_val, f_val, f_val, f_val, f_val, d_val, getFloat(a), b);
    case OBJECT_TYPE_DOUBLE:
        swFloat(op, d_val, d_val, d_val, d_val, d_val, d_val, getDouble(a), b);
    default:
        return true;
    }

    switch (out->type) {
    case OBJECT_TYPE_BOOL:
    case OBJECT_TYPE_BYTE:
    case OBJECT_TYPE_CHAR:
        out->value = asVal(b_val);
        printf("%%b=%d\n", b_val);
        break;
    case OBJECT_TYPE_SHORT:
        out->value = asVal(s_val);
        printf("%%s=%d\n", s_val);
        break;
    case OBJECT_TYPE_INT:
        out->value = asVal(i_val);
        printf("%%i=%d\n", i_val);
        break;
    case OBJECT_TYPE_LONG:
        out->value = asVal(l_val);
        printf("%%l=%ld\n", l_val);
        break;
    case OBJECT_TYPE_FLOAT:
        out->value = asVal(f_val);
        printf("%%f=%.6f\n", f_val);
        break;
    case OBJECT_TYPE_DOUBLE:
        out->value = asVal(d_val);
        printf("%%d=%lf\n", d_val);
        break;
    default:
        return true;
    }
    return false;
}

bool binaryOperation(char op, Object* a, Object* b, Object* out) {
    uint8_t aPri = objectTypePriority[a->type], bPri = objectTypePriority[b->type];
    if (aPri < bPri) out->type = b->type;
    else out->type = a->type;

    out->symbol = NULL;
    out->flag = 0;
    out->array = 0;

    int8_t b_val;
    int16_t s_val;
    int32_t i_val;
    int64_t l_val;
    switch (a->type) {
    case OBJECT_TYPE_BOOL:
    case OBJECT_TYPE_BYTE:
    case OBJECT_TYPE_CHAR:
        swBinary(op, b_val, s_val, i_val, l_val, getByte(a), b);
    case OBJECT_TYPE_SHORT:
        swBinary(op, s_val, s_val, i_val, l_val, getShort(a), b);
    case OBJECT_TYPE_INT:
        swBinary(op, i_val, i_val, i_val, l_val, getInt(a), b);
    case OBJECT_TYPE_LONG:
        swBinary(op, l_val, l_val, l_val, l_val, getLong(a), b);
    default:
        return true;
    }

    switch (out->type) {
    case OBJECT_TYPE_BOOL:
    case OBJECT_TYPE_BYTE:
    case OBJECT_TYPE_CHAR:
        out->value = asVal(b_val);
        printf("%%b=%d\n", b_val);
        break;
    case OBJECT_TYPE_SHORT:
        out->value = asVal(s_val);
        printf("%%s=%d\n", s_val);
        break;
    case OBJECT_TYPE_INT:
        out->value = asVal(i_val);
        printf("%%i=%d\n", i_val);
        break;
    case OBJECT_TYPE_LONG:
        out->value = asVal(l_val);
        printf("%%l=%ld\n", l_val);
        break;
    default:
        return true;
    }
    return false;
}

bool booleanOperation(char op, Object* a, Object* b, Object* out) {
    out->symbol = NULL;
    out->flag = 0;
    out->type = OBJECT_TYPE_BOOL;
    out->array = 0;

    bool b_val;
    switch (a->type) {
    case OBJECT_TYPE_BOOL:
        if (b->type != OBJECT_TYPE_BOOL)
            return true;
        valueBoolean(op, b_val, getBool(a), getBool(b));
    case OBJECT_TYPE_BYTE:
    case OBJECT_TYPE_CHAR:
        swBoolean(op, b_val, getByte(a), b);
    case OBJECT_TYPE_SHORT:
        swBoolean(op, b_val, getShort(a), b);
    case OBJECT_TYPE_INT:
        swBoolean(op, b_val, getInt(a), b);
    case OBJECT_TYPE_LONG:
        swBoolean(op, b_val, getLong(a), b);
    case OBJECT_TYPE_FLOAT:
        swBoolean(op, b_val, getFloat(a), b);
    case OBJECT_TYPE_DOUBLE:
        swBoolean(op, b_val, getDouble(a), b);
    default:
        return true;
    }
    printf("%%b=%s\n", b_val ? "true" : "false");
    out->value = asVal(b_val);

    return false;
}

bool valueOperationNeg(Object* a, Object* out) {
    out->symbol = NULL;
    out->flag = 0;
    out->type = a->type;
    out->array = 0;

    int8_t b_val;
    int16_t s_val;
    int32_t i_val;
    int64_t l_val;
    float f_val;
    double d_val;
    switch (out->type) {
    case OBJECT_TYPE_BOOL:
    case OBJECT_TYPE_BYTE:
    case OBJECT_TYPE_CHAR:
        b_val = -getByte(a);
        out->value = asVal(b_val);
        printf("%%b=%d\n", b_val);
        break;
    case OBJECT_TYPE_SHORT:
        s_val = -getShort(a);
        out->value = asVal(s_val);
        printf("%%s=%d\n", s_val);
        break;
    case OBJECT_TYPE_INT:
        i_val = -getInt(a);
        out->value = asVal(i_val);
        printf("%%i=%d\n", i_val);
        break;
    case OBJECT_TYPE_LONG:
        l_val = -getLong(a);
        out->value = asVal(l_val);
        printf("%%l=%ld\n", l_val);
        break;
    case OBJECT_TYPE_FLOAT:
        f_val = -getFloat(a);
        out->value = asVal(f_val);
        printf("%%f=%.7f\n", f_val);
        break;
    case OBJECT_TYPE_DOUBLE:
        d_val = -getDouble(a);
        out->value = asVal(d_val);
        printf("%%d=%lf\n", d_val);
        break;
    default:
        return true;
    }
    return false;
}

bool valueOperationBinaryNot(Object* a, Object* out) {
    out->symbol = NULL;
    out->flag = 0;
    out->type = a->type;
    out->array = 0;
    
    int8_t b_val;
    int16_t s_val;
    int32_t i_val;
    int64_t l_val;
    switch (out->type) {
    case OBJECT_TYPE_BOOL:
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
    return false;
}
