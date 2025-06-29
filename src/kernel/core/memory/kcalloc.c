#include "memory.h"

void* kcalloc(u32 num, u32 size) {
    void* ptr = kmalloc(num * size);
    memset(ptr, 0, num * size);

    return ptr;
}