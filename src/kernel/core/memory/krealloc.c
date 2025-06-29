#include "memory.h"

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

                // trailing = (void*) nptr + sizeof(struct memory_block) + memory_getRealSize(nptr->size);
                // *trailing = nptr->size;
                memory_block_trailing(nptr);
                
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

                // trailing = (void*) blk_prev + sizeof(struct memory_block) + memory_getRealSize(blk_prev->size);
                // *trailing = blk_prev->size;
                memory_block_trailing(blk_prev);

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

    // trailing = (void*) nptr + sizeof(struct memory_block) + memory_getRealSize(nptr->size);
    // *trailing = nptr->size;
    memory_block_trailing(nptr);

    struct memory_block* blk_split = (void*) trailing + sizeof(u32);
    
    if(blk_next && memory_isfree(blk_next)) {
        blk_split->size = rest + memory_getRealSize(blk_next->size);
        memory_setfree(&(blk_split->size), 1);

        // trailing = (void*) blk_split + sizeof(struct memory_block) + memory_getRealSize(blk_split->size);
        // *trailing = blk_split->size;
        memory_block_trailing(nptr);

        memory_freelist_remove(blk_next);

        if(tail == blk_next)
            tail = blk_split;

        memory_freelist_append(blk_split);
        return ptr;
    }

    blk_split->size = rest - OVERHEAD;
    memory_setfree(&(blk_split->size), 0);

    // trailing = (void*) blk_split + sizeof(struct memory_block) + memory_getRealSize(blk_split->size);
    // *trailing = blk_split->size;
    memory_block_trailing(blk_split);

    memory_freelist_append(blk_split);
    return ptr;
}

#ifdef _h_set
#undef _h_set
#endif