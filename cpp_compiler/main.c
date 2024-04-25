#include "main.h"
#define DEBUG
#define WJCL_LINKED_LIST_IMPLEMENTATION
#define WJCL_HASH_MAP_IMPLEMENTATION
#include "../../WJCL/map/wjcl_hash_map.h"
#include "./lib/byter_buffer.h"
#include "./lib/new_value.h"
#include "./lib/java_instruction.h"
#include "./lib/java_class_builder.h"

ByteBuffer constantByteBuff = byteBufferInit();
ByteBuffer methodByteBuff = byteBufferInit();

uint32_t strHash(const char* str) {
    uint32_t hash = 0, seed = 131;
    while (*str)
        hash = hash * seed + (*str++);
    return hash;
}
typedef struct ConstantObject {
    ObjectType type;
    uint64_t value;
} ConstantObject;

bool constantObjectEquals(void* a, void* b) {
    ConstantObject *objA = (ConstantObject*)a, *objB = (ConstantObject*)b;
    if (objA->type != objA->type)
        return false;
    switch (objA->type) {
        case OBJECT_TYPE_STR:
            return strcmp((const char*)objA->value, (const char*)objA->value) == 0;
        default:
            return objA->value == objB->value;
    }
}
uint32_t constantObjectHash(void* key) {
    ConstantObject* obj = (ConstantObject*)key;
    if (obj->type == OBJECT_TYPE_STR)
        return obj->type | strHash((const char*)obj->value);
    else
        return obj->type | (obj->value + (obj->value >> 32));
}

void constantObjectFree(void* key, void* value) {
    ConstantObject* obj = (ConstantObject*)key;
    if (obj->type == OBJECT_TYPE_STR)
        free((char*)obj->value);
}

NodeInfo constantObjectInfo = {
    .equalsFunction = constantObjectEquals,
    .hashFunction = constantObjectHash,
    .freeFlag = WJCL_HASH_MAP_FREE_KEY | WJCL_HASH_MAP_FREE_VALUE,
    .onNodeDelete = constantObjectFree,
};
Map constantObjectMap;

void createMethod(ObjectType returnType, const char* name) {
    uint16_t accessFlags = ACC_PUBLIC | ACC_STATIC;
    char* methodType;
    if (strcmp(name, "main") == 0) {
        methodType = newStr("([Ljava/lang/String;)V");
    } else {
        methodType = newStr("()V");
    }
    // map_putpp(&constantUtf8Map, (void*)name, newInt(constantUtf8Map.size));
    // map_putpp(&constantUtf8Map, (void*)methodType, newInt(constantUtf8Map.size));

    uint8_t code[] = {
        // INS_ldc_w,
        // short2bytes(helloworld),
        // INS_astore_0,
        // INS_getstatic,
        // short2bytes(system_outRef),
        // INS_aload_0,
        // INS_invokevirtual,
        // short2bytes(printlnStrRef),

        // INS_getstatic,
        // short2bytes(system_outRef),
        // INS_ldc_w,
        // short2bytes(num),
        // INS_ldc_w,
        // short2bytes(num),
        // INS_iadd,
        // INS_invokevirtual,
        // short2bytes(printlnIntegerRef),

        INS_return,
    };
    JavaCodeAttribute codeAttribute = {
        .max_stack = 3,
        .max_locals = 1,
        .code_length = sizeof(code),
        .code = code,
        .exception_table_length = 0,
        .attributes_count = 0,
    };
    java_ref nameRef = addUtf8(name, &constantByteBuff);
    java_ref typeRef = addUtf8(methodType, &constantByteBuff);
    JavaAttributeInfo* attributeInfo = newCodeAttribute(&codeAttribute, &constantByteBuff);
    addMethod(ACC_PUBLIC | ACC_STATIC, nameRef, typeRef, &attributeInfo, 1, &methodByteBuff);
    freeAttributeInfo(attributeInfo, true);
}

char* newStrCat(const char* a, const char* b) {
    char* cache = (char*)malloc(strlen(a) + strlen(b) + 1);
    strcpy(cache, a);
    strcat(cache, b);
    return cache;
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
    outputFileName = newStrCat(outputFileName, ".class");
    fout = fopen(outputFileName, "w");
    if (!fout) {
        printf("file `%s` cannot be opened\n", outputFileName);
        exit(1);
    }

    /* Codegen output init */
    java_ref thisClass = 1, SupperClass = 0;
    constantPoolSize = 1;
    methodCount = 0;

    // Build Constants
    thisClass = addClass("Main", &constantByteBuff);
    SupperClass = addClass("java/lang/Object", &constantByteBuff);
    java_ref systemRef = addClass("java/lang/System", &constantByteBuff);
    java_ref printStreamRef = addClass("java/io/PrintStream", &constantByteBuff);

    java_ref out_t = addNameAndType("out", "Ljava/io/PrintStream;", &constantByteBuff);
    java_ref system_outRef = addFieldRef(systemRef, out_t, &constantByteBuff);

    java_ref printlnStrRef = addMethodref(printStreamRef, addNameAndType("println", "(Ljava/lang/String;)V", &constantByteBuff), &constantByteBuff);
    java_ref printlnIntegerRef = addMethodref(printStreamRef, addNameAndType("println", "(I)V", &constantByteBuff), &constantByteBuff);

    yylineno = 1;
    constantObjectMap = map_create(constantObjectInfo);
    yyparse();
    fclose(yyin);
    printf("Total lines: %d\n", yylineno);

    printf("\n");
    printf("constantUtf8Map: %lu\n", constantObjectMap.size);
    map_entries(&constantObjectMap, i, {
        printf("%s %d\n", (char*)i->key, *(int*)i->value);
    });
    map_free(&constantObjectMap);
    printf("\n");

    // Write Header
    uint8_t classFileHeader[] = {
        0xCA, 0xFE, 0xBA, 0xBE,  // Magic Value
        0x00, 0x00, 0x00, 0x34,  // Java Version 8
    };
    fwrite(classFileHeader, 1, sizeof(classFileHeader), fout);
    // Write Constant Pool Data
    fwrite((uint8_t[]){short2bytes(constantPoolSize)}, 1, 2, fout);
    byteBufferWriteToFile(&constantByteBuff, fout);
    // Write Class info
    uint16_t classAccessFlags = ACC_SUPER | ACC_PUBLIC;
    uint8_t classInfo[] = {
        short2bytes(classAccessFlags),  // Access Flags
        short2bytes(thisClass),         // Class in constant pool
        short2bytes(SupperClass),       // Super class in constant pool
        short2bytes(0),                 // Interfaces
        short2bytes(0),                 // Fields
    };
    fwrite(classInfo, 1, sizeof(classInfo), fout);
    // Write Methods info
    fwrite((uint8_t[]){short2bytes(methodCount)}, 1, 2, fout);  // Methods
    byteBufferWriteToFile(&methodByteBuff, fout);
    // Write Attributes info
    fwrite((uint8_t[]){short2bytes(0)}, 1, 2, fout);  // Attributes

    byteBufferFree(&constantByteBuff, false);
    byteBufferFree(&methodByteBuff, false);

    fclose(fout);

    if (compileError)
        remove(outputFileName);
    free(outputFileName);
    yylex_destroy();
    return 0;
}