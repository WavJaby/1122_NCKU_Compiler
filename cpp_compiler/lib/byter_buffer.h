
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct ByteBuffer {
    uint8_t* buf;
    size_t len;
} ByteBuffer;

#define byteBufferInit() \
    { NULL, 0 }
#define byteBufferNew() calloc(1, sizeof(ByteBuffer))

void byteBufferWriteI16(uint16_t data, ByteBuffer* byteBuffer) {
    byteBuffer->buf = (uint8_t*)realloc(byteBuffer->buf, byteBuffer->len + 2);
    *(byteBuffer->buf + byteBuffer->len) = (data >> 8) & 0xFF;
    *(byteBuffer->buf + byteBuffer->len + 1) = data & 0xFF;
    byteBuffer->len += 2;
}

void byteBufferWrite(uint8_t* arr, size_t size, ByteBuffer* byteBuffer) {
    byteBuffer->buf = (uint8_t*)realloc(byteBuffer->buf, byteBuffer->len + size);
    memcpy(byteBuffer->buf + byteBuffer->len, arr, size);
    byteBuffer->len += size;
}

void byteBufferWriteToFile(ByteBuffer* byteBuffer, FILE* file) {
    fwrite(byteBuffer->buf, 1, byteBuffer->len, file);
}

void byteBufferClear(ByteBuffer* byteBuffer) {
    byteBuffer->buf = (uint8_t*)realloc(byteBuffer->buf, 0);
    byteBuffer->len = 0;
}

void byteBufferFree(ByteBuffer* byteBuffer, bool self) {
    free(byteBuffer->buf);
    if (self) free(byteBuffer);
}