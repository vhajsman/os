#ifndef __MEMORY_H
#define __MEMORY_H

#include "types.h"
#include "multiboot.h"
#include "stdlib.h"
#include "memory/paging.h"

#ifndef nullptr
#define nullptr ((void*) 0x00)
#endif

#define BLOCK_SIZE 4096
#define BLOCKS_PER_BUCKET 32

#define PROT_EXEC  0x1
#define PROT_WRITE 0x2
#define PROT_READ  0x4
#define PROT_NONE  0x0
#define PROT_USER  0x8

void memory_init(MULTIBOOT_INFO* mboot_info);
void kheap_init(void* start, void* end, void* max);

struct memory_block {
    u8 used;
    u8 kernel;

    unsigned int size;

    struct memory_block* next;
    struct memory_block* prev;
};

u32 memory_block_find();
u32 memory_block_alloc();
void memory_block_free(u32 blknum);

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

/**
 * @brief allocate contigual memory block
 * 
 * @param sz block size
 * @param align tells if block should be aligned
 * @param phys where to save result address of allocated block
 * @return void* 
 */
void* kmalloc_cont(u32 sz, int align, u32* phys);

/**
 * @brief allocates memory block
 * 
 * @param sz block size
 * @param align tells if block should be aligned
 * @param phys where to save result address of allocated block
 * @return u32 
 */
u32 kmalloc_int(u32 sz, int align, u32* phys);

/**
 * @brief allocates aligned memory block
 * 
 * @param sz block size
 * @return void* 
 */
void* kmalloc_align(u32 sz);

u32 kmalloc_p(u32 sz, u32 *phys);
u32 kmalloc_ap(u32 sz, u32 *phys);

/**
 * @brief allocates memory block and returns pointer to its address
 * 
 * @param sz block size
 * @return void* 
 */
void* kmalloc(u32 sz);

/**
 * @brief allocates memory for array and returns its address
 * 
 * @param num array element count
 * @param size array element size
 * @return void* 
 */
void* kcalloc(u32 num, u32 size);

/**
 * @brief re-allocates (resizes) allocated memory block and returns its new address
 * 
 * @param ptr memory block array
 * @param size new size
 * @return void* 
 */
void* krealloc(void* ptr, u32 size);

/**
 * @brief frees allocated memory block
 * 
 * @param ptr memory block pointer to be freed
 */
void kfree(void* ptr);

/**
 * @brief calculates and returns the nearest higher or equal multiple of 8 for a given size
 * 
 * @param size size to be rounded
 * @return u32 
 */
u32 memory_getRealSize(u32 size);

int doesItFit(struct memory_block* n, u32 size);

// void setFree(u32* size, int x);
// int isFree(struct memory_block* n);

void memory_freelist_remove(struct memory_block* x);
void memory_freelist_append(struct memory_block* x);

struct memory_block* memory_bestfit(u32 size);
struct memory_block* memory_block_prev(struct memory_block* n);
struct memory_block* memory_block_next(struct memory_block* n);

#ifdef __REQ_ALL
    extern int kheap_enabled;
    
    extern u8* bitmap;
    extern u32 bitmap_size;
    extern u8* memory_start;
    extern u32 memory_size;
    extern u32 memory_blockCount;
#endif


/**
 * @brief returns total amount of allocated memory
 * 
 * @return u32 
 */
u32 memory_getUsed();

/**
 * @brief returns total amount of un-allocated memory
 * 
 * @return u32 
 */
u32 memory_getFree();

void unmap(page_directory_t* dir, u32 addr_virt, int free_frame);
void* mmap(page_directory_t* dir, u32 start_va, u32 size, int prot);
int munmap(page_directory_t* dir, u32 start_va, u32 size);

int mprotect(page_directory_t* dir, u32 start_va, u32 size, int prot);

#endif
