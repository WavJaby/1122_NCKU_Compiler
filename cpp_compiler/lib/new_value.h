
static inline void* newInt(int value) {
    void* ptr = malloc(sizeof(int));
    *(int*)ptr = value;
    return ptr;
}

static inline void* newStr(const char* value) {
    return strdup(value);
}