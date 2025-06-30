#include "memory.h"

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
            size += BLOCK_SIZE;

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
                    ASSERT(ksbrk(-size_reclaim) && "kmalloc: ksbrk() failed");

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
                    ASSERT(ksbrk(-size_reclaim) != NULL && "kmalloc: ksbrk() failed\n");

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

        ASSERT(ret != NULL && "kmalloc(): ksbrk() failed\n");

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
    __malloc_number((unsigned int) result, 16);

    if(blk_best != NULL)
        blk_best->used=1;

    return result;
}

#undef _h_set