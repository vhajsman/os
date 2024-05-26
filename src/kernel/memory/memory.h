#ifndef __MEMORY_H
#define __MEMORY_H

#include "types.h"
#include "multiboot.h"

#ifndef nullptr
#define nullptr ((void*) 0x00)
#endif

#define MEMORY_DYNAMIC_TOTAL_SIZE       (4 * 1024)
#define MEMORY_DYNAMIC_NODE_SIZE        sizeof(struct memory_block)

#define BLOCK_SIZE 4096
#define BLOCKS_PER_BUCKET 8

#define SETBIT(i)   \
    bitmap[i / BLOCKS_PER_BUCKET] = bitmap[i / BLOCKS_PER_BUCKET] | \
                                    (1 << (i % BLOCKS_PER_BUCKET))
#define CLEARBIT(i) \
    bitmap[i / BLOCKS_PER_BUCKET] = bitmap[i / BLOCKS_PER_BUCKET] & \
                                    (~(1 << (i % BLOCKS_PER_BUCKET)))

#define ISSET(i) \
    ((bitmap[i / BLOCKS_PER_BUCKET] >> (i % BLOCKS_PER_BUCKET)) & 0x1)

#define GET_BUCKET32(i) \
    (*((u32*) &bitmap[i / 32]))

#define BLOCK_ALIGN(addr) \
    (((addr) & 0xFFFFF000) + 0x1000)

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
