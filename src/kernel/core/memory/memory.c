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

#define _UNSIGNED_ERR (unsigned) -1

struct memory_block* memory_block_prev(struct memory_block* blk);
struct memory_block* memory_block_next(struct memory_block* blk);
void memory_freelist_remove(struct memory_block* blk);
void memory_freelist_append(struct memory_block* blk);
u32 memory_getRealSize(u32 size);
int memory_block_istail(struct memory_block* blk);
int memory_block_doesItFit(struct memory_block* blk, u32 size);
void memory_setfree(u32* size, int x);
int memory_isfree(struct memory_block* blk);


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

u32 _placement = (u32) &end;

#ifdef _BUILD_INSIDERS
#define __malloc_message(message, level)    debug_message(message, "kmalloc", level);
#define __malloc_append(message)            debug_append(message);
#define __malloc_number(number, base)       debug_number(number, base);
#else
#define __malloc_message(message, level)
#define __malloc_append(message)
#define __malloc_number(number, base)
#endif

void* kmalloc_cont(u32 size, int align, u32* addr_phys) {
    if(align == 1 && (_placement & 0xFFFFF000)) {
        // Align placement addr
        _placement &= 0xFFFFF000;
        _placement += 0x1000;
    }

    if(addr_phys != NULL)
        *addr_phys = _placement;

    u32 temp = _placement;
    _placement += size;

    return (void*) temp;
}

u32 kmalloc_int(u32 size, int align, u32* addr_phys) {
    if(heap_start != NULL) {
        if(align)
            size =+ BLOCK_SIZE;

        void* addr = kmalloc(size);
        u32 addr_align = ((u32) addr & 0xFFFFF000) + 0x1000;

        if(addr_phys != 0) {
            u32 t = align ? addr_align : (u32) addr;
            *addr_phys = (u32) virtual2phys(kpage_dir, (void*) t);
        }

        return align ? addr_align : (u32) addr;
    }

    if(align == 1 && (_placement & 0xFFFFF000)) {
        _placement &= 0xFFFFF000;
        _placement += 0x1000;
    }

    if(addr_phys)
        *addr_phys = _placement;

    u32 temp = _placement;
    _placement += size;

    return temp;
}

void* kmalloc_align(u32 size) {
    return (void*) kmalloc_int(size, 1, 0);
}

u32 kmalloc_p(u32 size, u32* addr_phys) {
    return kmalloc_int(size, 0, addr_phys);
}

u32 kmalloc_ap(u32 size, u32* addr_phys) {
    return kmalloc_int(size, 1, addr_phys);
}

void* kmalloc_k(u32 size) {
    return (void*) kmalloc_int(size, 0, 0);
}

void* kcalloc(u32 num, u32 size) {
    void* ptr = kmalloc(num * size);
    memset(ptr, 0, num * size);

    return ptr;
}

#ifdef _BUILD_INSIDERS
#define __realloc_message(message, level)    debug_message(message, "krealloc", level);
#define __relloc_append(message)             debug_append(message);
#define __relloc_number(number, base)        debug_number(number, base);
#else
#define __relloc_message(message, level)
#define __relloc_append(message)
#define __relloc_number(number, base)
#endif

#ifdef _BUILD_INSIDERS
#define _h_set(__identifier, __value, __base)                   \
    __identifier = __value;                                     \
    debug_message("krealloc(): ", "memory", KERNEL_MESSAGE);    \
    debug_append(#__identifier);                                \
    debug_append(" = ");                                        \
    debug_number((int) __identifier, __base);
#else
#define _h_set(__identifier, __value, __base)                   \
    __identifier = __value;
#endif

void* krealloc(void* ptr, u32 size) {
    u32* trailing = NULL;

    if(!ptr)
        return kmalloc(size);

    if(size == 0 && ptr != NULL) {
        __realloc_message("size == 0 && ptr != NULL", KERNEL_MESSAGE);

        kfree(ptr);
        return NULL;
    }

    u32 _h_set(size_rnd, ((size + 15) / 16) * 16, 10);
    u32 _h_set(size_blk, size_rnd + OVERHEAD, 10);

    struct memory_block* nptr       = ptr - sizeof(struct memory_block);
    struct memory_block* blk_next   = memory_block_next(nptr);
    struct memory_block* blk_prev   = memory_block_prev(nptr);

    if(nptr->size == size)
        return ptr;

    if(nptr->size < size) {
        if( tail != nptr && 
            memory_isfree(blk_next) && 
            (memory_getRealSize(nptr->size) + OVERHEAD + memory_getRealSize(blk_next->size)) >= size_rnd) {
                memory_freelist_remove(blk_next);

                nptr->size = memory_getRealSize(nptr->size) + OVERHEAD + memory_getRealSize(blk_next->size);
                memory_setfree(&(nptr->size), 0);

                trailing = (void*) nptr + sizeof(struct memory_block) + memory_getRealSize(nptr->size);
                *trailing = nptr->size;
                
                if(tail == blk_next)
                    tail = nptr;

                return nptr + 1;
        }

        if( head != nptr && 
            memory_isfree(blk_prev) &&
            (memory_getRealSize(nptr->size) + OVERHEAD + memory_getRealSize(blk_prev->size)) >= size_rnd) {
                u32 size_org = memory_getRealSize(nptr->size);
                
                memory_freelist_remove(blk_prev);

                blk_prev->size = size_org + OVERHEAD + memory_getRealSize(blk_prev->size);
                memory_setfree(&(blk_prev->size), 0);

                trailing = (void*) blk_prev + sizeof(struct memory_block) + memory_getRealSize(blk_prev->size);
                *trailing = blk_prev->size;

                if(tail == nptr)
                    tail = blk_prev;

                memcpy(blk_prev + 1, ptr, size_org);
                return blk_prev + 1;
        }

        void* nplace = kmalloc(size);
        memcpy(nplace, ptr, memory_getRealSize(nptr->size));

        kfree(ptr);

        return nplace;
    }

    u32 rest = memory_getRealSize(nptr->size) + OVERHEAD - size_blk;
    if(rest < 8 + OVERHEAD)
        return ptr;

    nptr->size = size_blk - OVERHEAD;
    memory_setfree(&(nptr->size), 0);

    trailing = (void*) nptr + sizeof(struct memory_block) + memory_getRealSize(nptr->size);
    *trailing = nptr->size;

    struct memory_block* blk_split = (void*) trailing + sizeof(u32);
    
    if(blk_next && memory_isfree(blk_next)) {
        blk_split->size = rest + memory_getRealSize(blk_next->size);
        memory_setfree(&(blk_split->size), 1);

        trailing = (void*) blk_split + sizeof(struct memory_block) + memory_getRealSize(blk_split->size);
        *trailing = blk_split->size;

        memory_freelist_remove(blk_next);

        if(tail == blk_next)
            tail = blk_split;

        memory_freelist_append(blk_split);
        return ptr;
    }

    blk_split->size = rest - OVERHEAD;
    memory_setfree(&(blk_split->size), 0);

    trailing = (void*) blk_split + sizeof(struct memory_block) + memory_getRealSize(blk_split->size);
    *trailing = blk_split->size;

    memory_freelist_append(blk_split);
    return ptr;
}

#ifdef _h_set
#undef _h_set
#endif

void kfree(void* ptr) {
    struct memory_block* blk_curr = ptr - sizeof(struct memory_block);
    struct memory_block* blk_prev = memory_block_prev(blk_curr);
    struct memory_block* blk_next = memory_block_next(blk_curr);

    u32 size = memory_getRealSize(blk_curr->size) + OVERHEAD;
    memory_used -= size;

    if(memory_isfree(blk_prev) && memory_isfree(blk_next)) {
        blk_prev->size =
            memory_getRealSize(blk_prev->size) + 2 * OVERHEAD +
            memory_getRealSize(blk_curr->size) +
            memory_getRealSize(blk_next->size);

        memory_setfree(&(blk_prev->size), 1);

        u32* trailing = (void*) blk_prev + sizeof(struct memory_block) + memory_getRealSize(blk_prev->size);
        *trailing = blk_prev->size;

        if(tail == blk_next)
            tail = blk_prev;

        memory_freelist_remove(blk_next);
        return;
    }

    if(memory_isfree(blk_prev)) {
        blk_prev->size = memory_getRealSize(blk_prev->size) + OVERHEAD + memory_getRealSize(blk_curr->size);
        memory_setfree(&(blk_prev->size), 1);

        u32* trailing = (void*) blk_prev + sizeof(struct memory_block) + memory_getRealSize(blk_prev->size);
        *trailing = blk_prev->size;

        if(tail == blk_curr)
            tail = blk_prev;

        return;
    }

    if(memory_isfree(blk_next)) {
        blk_curr->size = memory_getRealSize(blk_curr->size) + OVERHEAD + memory_getRealSize(blk_next->size);
        memory_setfree(&(blk_curr->size), 1);

        u32* trailing = (void*) blk_curr + sizeof(struct memory_block) + memory_getRealSize(blk_curr->size);
        *trailing = blk_curr->size;

        if(tail == blk_next)
            tail = blk_curr;

        memory_freelist_remove(blk_next);
        memory_freelist_append(blk_curr);

        return;
    }

    memory_setfree(&(blk_curr->size), 1);

    u32* trailing = (void*) blk_curr + sizeof(struct memory_block) + memory_getRealSize(blk_curr->size);
    *trailing = blk_curr->size;

    memory_freelist_append(blk_curr);
}

void kheap_init(void* start, void* end, void* max) {
    heap_start  = start;
    heap_end    = end;
    heap_max    = max;
    heap_curr   = start;

    kheap_enabled = 1;
}

u32 memory_getRealSize(u32 size) {
    // return (size >> 1) << 1;
    return (size % 8 == 0) ? size : ((size + 7) / 8) * 8;
}

int memory_block_istail(struct memory_block* blk) {
    return blk == tail;
}

int memory_block_doesItFit(struct memory_block* blk, u32 size) {
    // return blk->size >= memory_getRealSize(size) && memory_isfree(blk);
    return blk->size >= size && memory_isfree(blk);
}

void memory_setfree(u32* size, int x) {
    *size = x ? *size | 1 : *size & 0xFFFFFFFE;
}

int memory_isfree(struct memory_block* blk) {
    // return blk ? blk->size & 1 : 0;
    return blk && blk->size & 1;
}

void memory_freelist_remove(struct memory_block* blk) {
    if(!blk)
        return;

    if(blk->prev) {
        blk->prev->next = blk->next;
    } else {
        freelist = blk->next;
    }

    if(blk->next) {
        blk->next->prev = blk->prev;
    }

    memory_block_orphan(blk);
}

void memory_freelist_append(struct memory_block* blk) {
    if(!blk) {
        debug_message("memory_freelist_append(): append null block!", "memory", KERNEL_ERROR);
        return;
    }

    struct memory_block* it = freelist;
    while(it) {
        if(it == blk) {
            debug_message("memory_freelist_append(): block already in freelist!", "memory", KERNEL_ERROR);
            return;
        }

        it = it->next;
    }

    if(blk->used)
        debug_message("memory_freelist_append(): append used block!", "memory", KERNEL_WARNING);

    blk->used = 0;
    blk->prev = NULL;
    blk->next = freelist;

    if(freelist)
        freelist->prev = blk;

    freelist = blk;
}

void memory_freelist_integrity() {
    struct memory_block* slow = freelist;
    struct memory_block* fast = freelist;

    while (slow && fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;

        if (slow == fast) {
            debug_message("Cycle detected in freelist!", "memory", KERNEL_ERROR);
            return;
        }
    }

    debug_message("No cycles in freelist detected.", "memory", KERNEL_MESSAGE);
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

struct memory_block* memory_block_prev(struct memory_block* blk) {
    if(blk == head)
        return NULL;

    void* ptr = blk;
    u32* uptr = ptr - sizeof(u32);
    u32 prev_size = memory_getRealSize(*uptr);

    void* ret = ptr - OVERHEAD - prev_size;
    return ret;
}

struct memory_block* memory_block_next(struct memory_block* blk) {
    if(blk == tail)
        return NULL;

    void *ptr = blk;
    ptr += OVERHEAD + memory_getRealSize(blk->size);

    return ptr;
}

#define _h_set(__identifier, __value, __base)                   \
    __identifier = __value;                                     \
    debug_message("memory_init(): ", "memory", KERNEL_MESSAGE); \
    debug_append(#__identifier);                                \
    debug_append(" = ");                                        \
    debug_number((int) __identifier, __base);

void* kmalloc(u32 size) {
    void* result = NULL;
    u32* trailing = NULL;

    if(size == 0)
        goto done;

    __malloc_message("running kmalloc() size=", KERNEL_MESSAGE);
    __malloc_number(size, 10);

    u32 _h_set(size_rnd, ((size + 15) / 16) * 16, 10);
    u32 _h_set(size_blk, size_rnd + OVERHEAD, 10);

    struct memory_block* blk_best = memory_bestfit(size_rnd);

    if(blk_best) {
        void* ptr = (void*) blk_best;
        void* blk_save_next = memory_block_next(blk_best);

        u32 size_chunk = memory_getRealSize(blk_best->size) + OVERHEAD;
        u32 size_rest = size_chunk - size_blk;
        u32 size_w = (size_rest < 8 + OVERHEAD) ? size_chunk : size_blk;

        blk_best->size = size_w - OVERHEAD;
        memory_setfree(&(blk_best->size), 0);

        void* base = ptr;
        trailing = ptr + size_w - sizeof(u32);
        *trailing = blk_best->size;
        ptr = (void*) (trailing + 1);

        memory_used += size_blk;

        if(size_rest < 8 + OVERHEAD) {
            __malloc_message("size_rest is smaller than OVERHEAD+8", KERNEL_MESSAGE);
            goto noSplit;
        }

        if(size_rest >= 8) {
            __malloc_message("size_rest >= 8", KERNEL_MESSAGE);

            if(base != tail && memory_isfree(blk_save_next)) {
                void* blk_next_void = blk_save_next;
                struct memory_block* blk_next = blk_next_void;
    
                memory_freelist_remove(blk_next);
    
                struct memory_block* t = ptr;
                t->size = size_rest - OVERHEAD + memory_getRealSize(blk_next->size) + OVERHEAD;
                memory_setfree(&(t->size), 1);

                ptr += sizeof(struct memory_block) + memory_getRealSize(t->size);
                trailing = ptr;
                *trailing = t->size;

                if(blk_next == tail) {
                    tail = t;

                    int size_reclaim = memory_getRealSize(t->size) + OVERHEAD;
                    ksbrk(-size_reclaim);

                    goto noSplit;
                }

                memory_freelist_append(t);
            } else {
                struct memory_block* putback = ptr;
                putback->size = size_rest - OVERHEAD;
                memory_setfree(&(putback->size), 1);

                trailing = ptr + sizeof(struct memory_block) + memory_getRealSize(putback->size);
                *trailing = putback->size;

                if(base == tail) {
                    tail = putback;

                    int size_reclaim = memory_getRealSize(putback->size) + OVERHEAD;
                    ksbrk(-size_reclaim);

                    goto noSplit;
                }
            }
        }
noSplit:
    __malloc_message("goto nosplit", KERNEL_MESSAGE);

    memory_freelist_remove(base);

    result = base + sizeof(struct memory_block);
    goto done;

    } else {
        u32 size_real = size_blk;
        struct memory_block* ret = ksbrk(size_real);

        ASSERT(ret != NULL && "kmalloc(): not enough memory in heap\n");

        memory_used += size_real;

        if(!head)
            head = ret;

        void* ptr  = ret;
        void* save = ret;
        tail = ptr;

        ret->size = size_blk - OVERHEAD;
        memory_setfree(&(ret->size), 0);

        ptr += size_blk - sizeof(u32);

        trailing = ptr;
        *trailing = ret->size;

        result = save + sizeof(struct memory_block);
        goto done;
    }

done:
    __malloc_message("malloc(", KERNEL_MESSAGE);
    __malloc_number(size, 10);
    __malloc_append(")");

    __malloc_append(" = ");
    __malloc_number((unsigned int) &result, 16);

    if(blk_best != NULL)
        blk_best->used=1;

    return result;
}

#undef _h_set

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