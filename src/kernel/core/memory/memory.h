#ifndef __MEMORY_H
#define __MEMORY_H

#include "types.h"
#include "multiboot.h"

#ifndef nullptr
#define nullptr ((void*) 0x00)
#endif

// #define MEMORY_DYNAMIC_TOTAL_SIZE       (4 * 1024)
// #define MEMORY_DYNAMIC_NODE_SIZE        sizeof(struct memory_block)

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

/*
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
*/

struct memory_block {
    u8 used;
    u8 kernel;

    unsigned int size;

    struct memory_block* next;
    struct memory_block* prev;
};

// =========================================================
// =================== MEMROY HEAP
// =========================================================

#define KHEAP_START         (void*) 0xC0400000
#define KHEAP_MAX_ADDRESS   (void*) 0xCFFFFFFF
#define KHEAP_INITIAL_SIZE  48 * M
#define HEAP_MIN_SIZE       4 * M

#define PAGE_SIZE 4096
#define OVERHEAD (sizeof(struct memory_block) + sizeof(unsigned int))

#ifdef __REQ_ALL_ADDRESS
    extern void* heap_start;    // Where heap starts (must be page-aligned)
    extern void* heap_end;      // Where heap ends (must be page-aligned)
    extern void* heap_curr;     // Top of heap
    extern void* heap_max;      // Maximum heap_end
#endif

void* malloc(u32 size);
void free(void *ptr);
void* realloc(void *ptr, u32 size);

void memory_init(MULTIBOOT_INFO* mboot_info);

u32 memory_findBlock();
u32 memory_allocateBlk();
void memory_freeBlk(u32 blknum);

// u32 _placement = (u32) &end;

void* kmalloc_cont(u32 sz, int align, u32* phys);
u32 kmalloc_int(u32 sz, int align, u32* phys);
void* kmalloc_align(u32 sz);
u32 kmalloc_p(u32 sz, u32 *phys);
u32 kmalloc_ap(u32 sz, u32 *phys);
void* kmalloc(u32 sz);
void* kcalloc(u32 num, u32 size);
void* krealloc(void* ptr, u32 size);
void kfree(void* ptr);

void kheap_init(void* start, void* end, void* max);

u32 getRealSize(u32 size);

int isEnd(struct memory_block* n);

int doesItFit(struct memory_block* n, u32 size);
void setFree(u32* size, int x);
int isFree(struct memory_block* n);

void removeNodeFromFreelist(struct memory_block* x);
void addNodeToFreelist(struct memory_block* x);

struct memory_block* bestfit(u32 size);
struct memory_block* getPrevBlock(struct memory_block* n);
struct memory_block* getNextBlock(struct memory_block* n);

extern int kheap_enabled;

extern u8* bitmap;
extern u32 bitmap_size;
extern u8* memory_start;
extern u32 memory_size;
extern u32 memory_blockCount;

u32 memory_getUsed();
u32 memory_getFree();
#endif
