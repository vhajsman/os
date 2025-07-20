#include "memory.h"

static u32 nextFreeFrame = 0x100000;

u32 memory_block_alloc() {
    /*
    for(u32 i = 0; i < memory_blockCount; i++) {
        if((GET_BUCKET32(i) & (1 << (i % 32))) == 0) {
            GET_BUCKET32(i) |= (1 << (i % 32));
            return i * BLOCK_SIZE;
        }
    }
    
    return (u32) -1;
    */ 

    u32 frame = nextFreeFrame;
    nextFreeFrame += BLOCK_SIZE;

    return frame;
}
