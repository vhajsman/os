#ifndef __MEMORY_H
#define __MEMORY_H

#include "types.h"
#include "multiboot.h"

#ifndef nullptr
#define nullptr ((void*) 0x00)
#endif

#define MEMORY_DYNAMIC_TOTAL_SIZE       (4 * 1024)
#define MEMORY_DYNAMIC_NODE_SIZE        sizeof(struct memory_block)

struct memory_block {
    u8 used;
    u8 kernel;

    u32 size;

    struct memory_block* next;
    struct memory_block* prev;
};

void memory_init(MULTIBOOT_INFO* mboot_info);

void* memory_merger(struct memory_block* current);
void* memory_mergel(struct memory_block* current);

void* malloc(size_t size);
void free(void* ptr);

#endif
