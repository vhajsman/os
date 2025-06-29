#define __REQ_ALL

#include "memory.h"
#include "multiboot.h"
#include "console.h"
#include "debug.h"
#include "string.h"
#include "paging.h"

void memory_freelist_integrity();

#define SETBIT(i)   bitmap[i / BLOCKS_PER_BUCKET] = bitmap[i / BLOCKS_PER_BUCKET] | (  1 << (i % BLOCKS_PER_BUCKET))
#define CLEARBIT(i) bitmap[i / BLOCKS_PER_BUCKET] = bitmap[i / BLOCKS_PER_BUCKET] & (~(1 << (i % BLOCKS_PER_BUCKET)))
#define ISSET(i)  ((bitmap[i / BLOCKS_PER_BUCKET] >> (i % BLOCKS_PER_BUCKET)) & 0x1)

#define GET_BUCKET32(i) (*((u32*) &bitmap[i / 32]))
#define BLOCK_ALIGN(addr) (((addr) & 0xFFFFF000) + 0x1000)

#define SIZE_MASK       0xFFFFFFFE
#define IS_FREE(size)   ((size) & 1)
#define GET_SIZE(size)  ((size) & SIZE_MASK)

#define _UNSIGNED_ERR (unsigned) -1

struct memory_block* head = nullptr;
struct memory_block* tail = nullptr;
struct memory_block* freelist = nullptr;

void* heap_start;    // Where heap starts (must be page-aligned)
void* heap_end;      // Where heap ends (must be page-aligned)
void* heap_curr;     // Top of heap
void* heap_max;      // Maximum heap_end

int kheap_enabled = 0;

extern u32* end;

u8* bitmap = (u8*) &end;
u32 bitmap_size;
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
    kheap_init(KHEAP_START, KHEAP_MAX_ADDRESS, KHEAP_MAX_ADDRESS);

    if (mbinfo == NULL) {
        debug_message("memory_init(): mbinfo is NULL", "memory", KERNEL_ERROR);
        return;
    }
    
    _h_set(memory_size,         mbinfo->mem_low + mbinfo->mem_high,     10);
    _h_set(memory_blockCount,   memory_size / BLOCK_SIZE,               10);
    _h_set(bitmap_size,         (memory_blockCount + BLOCKS_PER_BUCKET - 1) / BLOCKS_PER_BUCKET,  10);
    if(bitmap_size * BLOCKS_PER_BUCKET < memory_blockCount) {
        debug_append("+1");

        bitmap_size++;
    }

    memset(bitmap, 0x00, bitmap_size);
    _h_set(memory_start, (u8*) BLOCK_ALIGN((u32) (bitmap + bitmap_size)), 16);

    for(u32 i = 0; i < bitmap_size; i++) {
        if(bitmap[i] != 0x00) {
            debug_message("memory_init(): bitmap not empty: byte ", "memory", KERNEL_ERROR);
            debug_number(i, 10);

            return;
        }
    }

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

        if(freelist == NULL) {
            freelist = block;
            head = block;
            tail = block;
        } else {
            tail->next = block;
            block->prev = tail;
            tail = block;
        }

        memory_block_free(i);
        memory_freelist_append(block);

        memory_block_trailing(block);
    }

    struct memory_block* it = freelist;
    for(int i = 0; i < 10 && it != NULL; i++) {
        debug_message("..", "..", KERNEL_MESSAGE);
        debug_number((u32) it, 16);
        debug_append(" size=");
        debug_number(it->size, 10);
        debug_append(" isfree=");
        debug_number(memory_isfree(it), 10);
        debug_append(" used=");
        debug_number(it->used, 10);
        debug_append("\n");

        it = it->next;
    }

    memory_freelist_integrity();

    //while(true) {};
}

#undef _h_set

u32 _placement = (u32) &end;

void memory_block_trailing(struct memory_block* blk) {
    u32 size = blk->size;
    u32 size_real = memory_getRealSize(size);

    u32* trailing = (u32*) ((u8*) blk + sizeof(struct memory_block) + size_real);
    *trailing = size;
}

u32 memory_block_findfree() {
    for(u32 i = 0; i < memory_blockCount; i++) {
        if(!ISSET(i))
            return i;
    }

    return (unsigned) -1;
}

u32 memory_block_alloc() {
    u32 blk = memory_block_findfree();
    if(blk == _UNSIGNED_ERR)
        debug_message("no free block found", "memory", KERNEL_ERROR);

    SETBIT(blk);
    return blk;
}

void memory_block_free(u32 blk) {
    if(blk != _UNSIGNED_ERR)
        CLEARBIT(blk);
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

struct memory_block* memory_bestfit(u32 size) {
    if(!size) {
        debug_message("memory_bestfit(): zero size", "memory", KERNEL_ERROR);
        return NULL;
    }

    if(!freelist) {
        debug_message("memory_bestfit(): not enough memory", "memory", KERNEL_ERROR);
        return NULL;
    }

    struct memory_block* blk_curr = freelist;
    struct memory_block* blk_best = NULL;

    u32 i = 0;
    while(blk_curr) {
        if(i > memory_blockCount)
            break;

        debug_message("memory_bestfit(): checking block ", "memory", KERNEL_MESSAGE);
        debug_number(blk_curr->size, 10); debug_append(" ");
        debug_number(blk_curr->used, 10);

        if(memory_block_doesItFit(blk_curr, size) && (blk_best == NULL || blk_curr->size < blk_best->size))
            // Best fit yet found
            blk_best = blk_curr;

        blk_curr = blk_curr->next;
        i++;
    }

    if(blk_best != NULL) {
        debug_message("memory_bestfit()=", "memory", KERNEL_MESSAGE);
        debug_number((u32) blk_best, 16);
    }

    return blk_best;
}

u32 memory_getRawSize(u32 size) {
    return size & ~1;
}

struct memory_block* memory_block_prev(struct memory_block* blk) {
    if(blk == head)
        return NULL;

    // void* ptr = blk;
    // u32* uptr = ptr - sizeof(u32);
    // u32 prev_size = memory_getRealSize(*uptr);

    u32* uptr = (u32*) ((u8*) blk - sizeof(u32));
    u32 prev_size = memory_getRawSize(*uptr);

    // void* ret = ptr - OVERHEAD - prev_size;
    // return ret;
    
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

    if(free_frame && entry->present) {
        u32 idx_f = (entry->frame * PAGE_SIZE) / PAGE_SIZE;
        if(idx_f < bitmap_size)
            bitmap[idx_f / 8] &= ~(1 << (idx_f % 8));
    }

    memset(entry, 0, sizeof(page_table_entry_t));
    asm volatile("invlpg (%0)" :: "r" (addr_virt) : "memory");
}

void* mmap(page_directory_t* dir, u32 start_va, u32 size, int prot) {
    u32 align_start = PAGE_ALIGN(start_va);
    u32 align_size  = PAGE_ALIGN(size + PAGE_SIZE - 1);

    for(u32 addr = align_start; addr < align_start + align_size; addr += PAGE_SIZE) {
        u32 frame = memory_block_alloc();
        if(frame == _UNSIGNED_ERR) 
            return NULL;

        int attr_kernel =    (prot & PROT_USER)  ? 0 : 1;
        int attr_writeable = (prot & PROT_WRITE) ? 1 : 0;

        paging_allocate(dir, addr, frame, attr_kernel, attr_writeable);
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