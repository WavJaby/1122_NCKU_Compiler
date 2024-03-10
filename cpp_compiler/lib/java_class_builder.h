
typedef uint16_t java_ref;
typedef struct JavaCodeAttribute {
    uint16_t max_stack;
    uint16_t max_locals;
    uint32_t code_length;
    uint8_t* code;
    uint16_t exception_table_length;
    // {   u2 start_pc;
    //     u2 end_pc;
    //     u2 handler_pc;
    //     u2 catch_type;
    // } exception_table[]
    // uint8_t* exception_table;
    uint16_t attributes_count;
    // AttributeInfoBuilder* attributes;
} JavaCodeAttribute;

typedef struct JavaAttributeInfo {
    java_ref attribute_name_index;
    uint32_t attribute_length;
    uint8_t* info;
} JavaAttributeInfo;

typedef struct JavaMethodInfo {
    uint16_t access_flags;
    java_ref name_index;
    java_ref descriptor_index;
    uint16_t attributes_count;
    JavaAttributeInfo* attributes;
} JavaMethodInfo;

FILE* yyin;
FILE* fout;  // Compile output file
uint16_t constantPoolSize, methodCount;

#define CONSTANT_Class 7
#define CONSTANT_Fieldref 9
#define CONSTANT_Methodref 10
#define CONSTANT_InterfaceMethodref 11
#define CONSTANT_String 8
#define CONSTANT_Integer 3
#define CONSTANT_Float 4
#define CONSTANT_Long 5
#define CONSTANT_Double 6
#define CONSTANT_NameAndType 12
#define CONSTANT_Utf8 1
#define CONSTANT_MethodHandle 15
#define CONSTANT_MethodType 16
#define CONSTANT_InvokeDynamic 18

#define ACC_PUBLIC 0x0001
#define ACC_PRIVATE 0x0002
#define ACC_PROTECTED 0x0004
#define ACC_STATIC 0x0008
#define ACC_FINAL 0x0010 /*( Declared final, can not be extended )*/
#define ACC_SUPER 0x0020 /*( Not final, can be extended )*/
#define ACC_VOLATILE 0x0040
#define ACC_TRANSIENT 0x0080 /*( Not written or read by a persistent object manager)*/
#define ACC_INTERFACE 0x0200
#define ACC_ABSTRACT 0x0400
#define ACC_SYNTHETIC 0x1000 /*( Not present in source code. Generated )*/
#define ACC_ANNOTATION 0x2000
#define ACC_ENUM 0x4000

#define short2bytes(value) (((value) >> 8) & 0xFF), ((value) & 0xFF)
#define int2bytes(value) (((value) >> 24) & 0xFF), (((value) >> 16) & 0xFF), (((value) >> 8) & 0xFF), ((value) & 0xFF)

java_ref addUtf8(const char* string, ByteBuffer* buf) {
    size_t len = strlen(string);
    // Value string header
    byteBufferWrite((uint8_t[]){CONSTANT_Utf8, 0, len}, 3, buf);
    // Value string data
    byteBufferWrite((uint8_t*)string, len, buf);
#ifdef DEBUG
    printf("%5hu = Utf8               #%s\n", constantPoolSize, string);
#endif
    return constantPoolSize++;
}

java_ref addClass(const char* className, ByteBuffer* buf) {
    byteBufferWrite((uint8_t[]){CONSTANT_Class, short2bytes(constantPoolSize + 1)}, 3, buf);
#ifdef DEBUG
    printf("%5hu = Class              #%hu\n", constantPoolSize, constantPoolSize + 1);
#endif
    java_ref thisRef = constantPoolSize++;
    addUtf8(className, buf);
    return thisRef;
}

java_ref addInteger(int value, ByteBuffer* buf) {
    byteBufferWrite((uint8_t[]){CONSTANT_Integer, value >> 24, value >> 16, value >> 8, value}, 5, buf);
#ifdef DEBUG
    printf("%5hu = String             #%d\n", constantPoolSize, value);
#endif
    return constantPoolSize++;
}

java_ref addString(const char* value, ByteBuffer* buf) {
    byteBufferWrite((uint8_t[]){CONSTANT_String, short2bytes(constantPoolSize + 1)}, 3, buf);
#ifdef DEBUG
    printf("%5hu = String             #%hu\n", constantPoolSize, constantPoolSize + 1);
#endif
    java_ref thisRef = constantPoolSize++;
    addUtf8(value, buf);
    return thisRef;
}

java_ref addFieldRef(java_ref classRef, java_ref nameAndTypeRef, ByteBuffer* buf) {
    byteBufferWrite((uint8_t[]){CONSTANT_Fieldref, short2bytes(classRef), short2bytes(nameAndTypeRef)}, 5, buf);
#ifdef DEBUG
    printf("%5hu = Fieldref           #%hu.#%hu\n", constantPoolSize, classRef, nameAndTypeRef);
#endif
    return constantPoolSize++;
}

java_ref addMethodref(java_ref classRef, java_ref nameAndTypeRef, ByteBuffer* buf) {
    byteBufferWrite((uint8_t[]){CONSTANT_Methodref, short2bytes(classRef), short2bytes(nameAndTypeRef)}, 5, buf);
#ifdef DEBUG
    printf("%5hu = Methodref          #%hu.#%hu\n", constantPoolSize, classRef, nameAndTypeRef);
#endif
    return constantPoolSize++;
}

java_ref addNameAndType(const char* name, const char* type, ByteBuffer* buf) {
    byteBufferWrite((uint8_t[]){CONSTANT_NameAndType, short2bytes(constantPoolSize + 1), short2bytes(constantPoolSize + 2)}, 5, buf);
#ifdef DEBUG
    printf("%5hu = NameAndType        #%hu:#%hu\n", constantPoolSize, constantPoolSize + 1, constantPoolSize + 2);
#endif
    java_ref thisRef = constantPoolSize++;
    addUtf8(name, buf);
    addUtf8(type, buf);
    return thisRef;
}

void addMethod(uint16_t accessFlags, java_ref nameRef, java_ref typeRef, JavaAttributeInfo** attributes, uint16_t attributeCount, ByteBuffer* buf) {
#ifdef DEBUG
    printf("%s\n", __func__);
#endif
    byteBufferWrite((uint8_t[]){
                        short2bytes(accessFlags),
                        short2bytes(nameRef),
                        short2bytes(typeRef),
                        short2bytes(attributeCount),
                    },
                    8, buf);
    for (size_t i = 0; i < attributeCount; i++) {
        JavaAttributeInfo* attribute = attributes[i];
        byteBufferWrite((uint8_t[]){
                            short2bytes(attribute->attribute_name_index),
                            int2bytes(attribute->attribute_length),
                        },
                        6, buf);
        byteBufferWrite(attribute->info, attribute->attribute_length, buf);
    }
    methodCount++;
}

JavaAttributeInfo* newCodeAttribute(JavaCodeAttribute* codeAttribute, ByteBuffer* constantByteBuff) {
#ifdef DEBUG
    printf("%s\n", __func__);
#endif
    JavaAttributeInfo* attributeInfo = malloc(sizeof(JavaAttributeInfo));
    attributeInfo->attribute_name_index = addUtf8("Code", constantByteBuff);
    ByteBuffer infoBuffer = byteBufferInit();
    byteBufferWrite((uint8_t[]){
                        short2bytes(codeAttribute->max_stack),
                        short2bytes(codeAttribute->max_locals),
                        int2bytes(codeAttribute->code_length),
                    },
                    8, &infoBuffer);
    byteBufferWrite(codeAttribute->code, codeAttribute->code_length, &infoBuffer);
    byteBufferWriteI16(codeAttribute->exception_table_length, &infoBuffer);
    byteBufferWriteI16(codeAttribute->attributes_count, &infoBuffer);
    attributeInfo->attribute_length = infoBuffer.len;
    attributeInfo->info = infoBuffer.buf;
    return attributeInfo;
}

void freeAttributeInfo(JavaAttributeInfo* attributeInfo, bool self) {
    free(attributeInfo->info);
    if (self) free(attributeInfo);
}