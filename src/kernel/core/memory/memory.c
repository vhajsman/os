#define __REQ_ALL

#include "memory.h"
#include "multiboot.h"
#include "console.h"
#include "debug.h"
#include "string.h"
#include "paging.h"

void memory_freelist_integrity();

struct memory_block* head = nullptr;
struct memory_block* tail = nullptr;
struct memory_block* freelist = nullptr;

int kheap_enabled = 0;

extern u32* end;

u8* memory_start;
u32 memory_size;
u32 memory_blockCount;

size_t memory_used = 0;

#define _h_set(__identifier, __value, __base)                   \
    __identifier = __value;                                     \
    debug_message("memory_init(): ", "memory", KERNEL_MESSAGE); \
    debug_append(#__identifier);                                \
    debug_append(" = ");                                        \
    debug_number((int) __identifier, __base);

void memory_init(MULTIBOOT_INFO* mbinfo) {
    if (mbinfo == NULL) {
        debug_message("memory_init(): mbinfo is NULL", "memory", KERNEL_ERROR);
        return;
    }
    
    _h_set(memory_size,         mbinfo->mem_low + mbinfo->mem_high,     10);
    _h_set(memory_start,        (u8*) BLOCK_ALIGN((u32) &end),          10);
    _h_set(memory_blockCount,   memory_size / BLOCK_SIZE,               10);

    void* _heap_start = KHEAP_START;
    void* _heap_end =   (void*) ((u8*) _heap_start + KHEAP_INITIAL_SIZE)
    void* _heap_max =   KHEAP_MAX_ADDRESS;
    
    kheap_init(_heap_start, _heap_end, _heap_max);

    freelist = NULL;
    head = NULL;
    tail = NULL;

    for(u32 i = 0; i < memory_blockCount; i++) {
        struct memory_block* block = (struct memory_block*) (memory_start + i * BLOCK_SIZE);

        block->size = BLOCK_SIZE;
        memory_setfree(&(block->size), 1);

        block->next = NULL;
        block->prev = NULL;
        block->used = 0;

        memory_block_trailing(block);
        memory_freelist_append(block);
        
        if(!head)
            head = block;

        tail = block;
    }

    memory_freelist_integrity();
}

#undef _h_set

u32 _placement = (u32) &end;

void memory_block_trailing(struct memory_block* blk) {
    u32 size = blk->size;
    u32 size_real = memory_getRealSize(size);

    u32* trailing = (u32*) ((u8*) blk + sizeof(struct memory_block) + size_real);
    *trailing = size;
}

void memory_block_orphan(struct memory_block* blk) {
    blk->next = NULL;
    blk->prev = NULL;
}

void kheap_init(void* start, void* end, void* max) {
    heap_start  = start;
    heap_end    = end;
    heap_max    = max;
    heap_curr   = start;

    kheap_enabled = 1;
}

u32 memory_getRealSize(u32 size) {
    return (size % 8 == 0) ? size : ((size + 7) / 8) * 8;
}

int memory_block_istail(struct memory_block* blk) {
    return blk == tail;
}

int memory_block_doesItFit(struct memory_block* blk, u32 size) {
    return blk->size >= size && memory_isfree(blk);
}

void memory_setfree(u32* size, int x) {
    *size = x ? *size | 1 : *size & 0xFFFFFFFE;
}

int memory_isfree(struct memory_block* blk) {
    return blk && blk->size & 1;
}

u32 memory_getRawSize(u32 size) {
    return size & ~1;
}

struct memory_block* memory_block_prev(struct memory_block* blk) {
    if(blk == head)
        return NULL;

    u32* uptr = (u32*) ((u8*) blk - sizeof(u32));
    u32 prev_size = memory_getRawSize(*uptr);
    
    void* ret = (u8*) blk - OVERHEAD - prev_size;
    return (struct memory_block*) ret;
}

struct memory_block* memory_block_next(struct memory_block* blk) {
    if(blk == tail)
        return NULL;

    void *ptr = blk;
    ptr += OVERHEAD + memory_getRealSize(blk->size);

    return ptr;
}

u32 memory_getused() {
    return memory_used;
}

u32 memory_getfree() {
    return memory_size - memory_used;
}

void unmap(page_directory_t* dir, u32 addr_virt, int free_frame) {
    u32 idx_dir = PAGEDIR_INDEX(addr_virt);
    u32 idx_tbl = PAGETBL_INDEX(addr_virt);

    page_table_t* table = dir->ref_tables[idx_dir];
    if(!table)
        return;

    page_table_entry_t* entry = &table->pages[idx_tbl];

    memset(entry, 0, sizeof(page_table_entry_t));
    asm volatile("invlpg (%0)" :: "r" (addr_virt) : "memory");
}

void* mmap(page_directory_t* dir, u32 start_va, u32 size, int prot) {
    u32 align_start = PAGE_ALIGN(start_va);
    u32 align_size  = PAGE_ALIGN(size + PAGE_SIZE - 1);

    for(u32 addr = align_start; addr < align_start + align_size; addr += PAGE_SIZE) {
        void* page = kmalloc(PAGE_SIZE);
        if(!page)
            return NULL;

        u32 frame = (u32) virtual2phys(kpage_dir, page) / PAGE_SIZE;

        int attr_kernel = (prot & PROT_USER)  ? 0 : 1;
        int attr_write  = (prot & PROT_WRITE) ? 1 : 0;

        paging_allocate(dir, addr, frame, attr_kernel, attr_write);
    }

    return (void*) align_start;
}

int munmap(page_directory_t* dir, u32 start_va, u32 size) {
    u32 align_start = PAGE_ALIGN(start_va);
    u32 align_size  = PAGE_ALIGN(size + PAGE_SIZE - 1);

    for(u32 addr = align_start; addr < align_start + align_size; addr += PAGE_SIZE)
        paging_free(dir, addr, 1);

    return 0;
}

int mprotect(page_directory_t* dir, u32 start_va, u32 size, int prot) {
    u32 align_start = PAGE_ALIGN(start_va);
    u32 align_size  = PAGE_ALIGN(size + PAGE_SIZE - 1);

    for(u32 addr = align_start; addr < align_start + align_size; addr += PAGE_SIZE) {
        u32 idx_dir = PAGEDIR_INDEX(addr);
        u32 idx_tbl = PAGETBL_INDEX(addr);

        page_table_t* table = dir->ref_tables[idx_dir];
        if(!table)
            continue;

        page_table_entry_t* entry = &table->pages[idx_tbl];
        if(!entry->present)
            continue;

        entry->rw =   (prot & PROT_WRITE) ? 1 : 0;
        entry->user = (prot & PROT_USER)  ? 1 : 0;

        asm volatile("invlpg (%0)" :: "r" (addr) : "memory");
    }

    return 0;
}


#undef _UNSIGNED_ERR
#undef __REQ_ALL