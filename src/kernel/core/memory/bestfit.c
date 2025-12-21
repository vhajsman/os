#include "memory.h"

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

//        debug_message("memory_bestfit(): checking block ", "memory", KERNEL_MESSAGE);
//        debug_number(blk_curr->size, 10); debug_append(" ");
//        debug_number(blk_curr->used, 10);

        if(memory_block_doesItFit(blk_curr, size) && (blk_best == NULL || blk_curr->size < blk_best->size))
            // Best fit yet found
            blk_best = blk_curr;

        blk_curr = blk_curr->next;
        i++;
    }

//    if(blk_best != NULL) {
//        debug_message("memory_bestfit()=", "memory", KERNEL_MESSAGE);
//        debug_number((u32) blk_best, 16);
//    }

    return blk_best;
}
