#include "memory.h"

void kfree(void* ptr) {
    struct memory_block* blk_curr = ptr - sizeof(struct memory_block);
    struct memory_block* blk_prev = memory_block_prev(blk_curr);
    struct memory_block* blk_next = memory_block_next(blk_curr);

    u32 size = memory_getRealSize(blk_curr->size) + OVERHEAD;
    memory_used -= size;

    blk_curr->used = 0;

    bool prev_free = memory_isfree(blk_prev);
    bool next_free = memory_isfree(blk_next);

    if(prev_free) {
        memory_freelist_remove(blk_prev);
        memory_freelist_remove(blk_curr);
    }

    if(next_free) 
        memory_freelist_remove(blk_next);

    if(prev_free && next_free) {
        blk_prev->size =
            memory_getRealSize(blk_prev->size) + 2 * OVERHEAD +
            memory_getRealSize(blk_curr->size) +
            memory_getRealSize(blk_next->size);

        memory_setfree(&(blk_prev->size), 1);

        // u32* trailing = (void*) blk_prev + sizeof(struct memory_block) + memory_getRealSize(blk_prev->size);
        // *trailing = blk_prev->size;
        memory_block_trailing(blk_prev);

        if(tail == blk_next)
            tail = blk_prev;

        memory_freelist_remove(blk_next);
        return;
    }

    if(prev_free) {
        blk_prev->size = memory_getRealSize(blk_prev->size) + OVERHEAD + memory_getRealSize(blk_curr->size);
        memory_setfree(&(blk_prev->size), 1);

        // u32* trailing = (void*) blk_prev + sizeof(struct memory_block) + memory_getRealSize(blk_prev->size);
        // *trailing = blk_prev->size;
        memory_block_trailing(blk_prev);

        if(tail == blk_curr)
            tail = blk_prev;

        memory_freelist_remove(blk_prev);
        return;
    }

    if(next_free) {
        blk_curr->size = memory_getRealSize(blk_curr->size) + OVERHEAD + memory_getRealSize(blk_next->size);
        memory_setfree(&(blk_curr->size), 1);

        // u32* trailing = (void*) blk_curr + sizeof(struct memory_block) + memory_getRealSize(blk_curr->size);
        // *trailing = blk_curr->size;
        memory_block_trailing(blk_curr);

        if(tail == blk_next)
            tail = blk_curr;

        memory_freelist_remove(blk_next);
        memory_freelist_append(blk_curr);

        return;
    }

    memory_setfree(&(blk_curr->size), 1);

    // u32* trailing = (void*) blk_curr + sizeof(struct memory_block) + memory_getRealSize(blk_curr->size);
    // *trailing = blk_curr->size;
    memory_block_trailing(blk_curr);

    memory_freelist_append(blk_curr);
}