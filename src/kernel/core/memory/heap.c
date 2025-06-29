#include "memory.h"

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