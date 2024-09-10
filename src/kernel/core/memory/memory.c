#include "memory.h"
#include "multiboot.h"
#include "console.h"
#include "debug.h"
#include "string.h"
#include "paging.h"

extern u32* end;

u8* bitmap = (u8*) &end;
u32 bitmap_size;
u8* memory_start;
u32 memory_size;
u32 memory_blockCount;

void memory_init(MULTIBOOT_INFO* mboot_info) {
    // _mem_low = mboot_info->mem_low;
    // _mem_high = mboot_info->mem_high;
    // _map_length = mboot_info->mmap_length;
    // _map_address = mboot_info->mmap_addr;

    memory_size = mboot_info->mem_low + mboot_info->mem_high;
    memory_blockCount = memory_size / BLOCK_SIZE;

    bitmap_size = memory_blockCount / BLOCKS_PER_BUCKET;

    if(bitmap_size * BLOCKS_PER_BUCKET < memory_blockCount)
        bitmap_size++;

    memset(bitmap, 0x00, bitmap_size);

    memory_start = (u8*) BLOCK_ALIGN(((u32) (bitmap + bitmap_size)));

    __debug_messagen("Total memory:     ", "Memory", KERNEL_MESSAGE, memory_size, 10);
    __debug_messagen("Total Blocks:     ", "Memory", KERNEL_MESSAGE, memory_blockCount, 10);
    __debug_messagen("Bitmap address:   ", "Memory", KERNEL_MESSAGE, &bitmap, 16);
    __debug_messagen("Bitmap size:      ", "Memory", KERNEL_MESSAGE, bitmap_size, 10);
    __debug_messagen("Memory starts @ ",   "Memory", KERNEL_MESSAGE, &memory_start, 16);

    for(u32 i = 0; i < bitmap_size; i ++) {
        if(bitmap[i] != 0) {
            debug_message("Bitmap is not empty", "Memory", KERNEL_WARNING);
            return;
        }
    }
}

u32 memory_findBlock() {
    for(u32 i = 0; i < memory_blockCount; i ++) {
        if(!ISSET(i))
            return i;
    }

    puts("no free block found.\n");
    debug_message("No free block found. Out of memory.", "Memory", KERNEL_ERROR);

    return 0;
}

u32 memory_allocateBlk() {
    u32 freeblk = memory_findBlock();
    SETBIT(freeblk);

    return freeblk;
}

void memory_freeBlk(u32 blknum) {
    CLEARBIT(blknum);
}

// =========================================================
// =================== MEMROY HEAP
// =========================================================

struct memory_block* head = nullptr;
struct memory_block* tail = nullptr;
struct memory_block* freelist = nullptr;

void* heap_start;    // Where heap starts (must be page-aligned)
void* heap_end;      // Where heap ends (must be page-aligned)
void* heap_curr;     // Top of heap
void* heap_max;      // Maximum heap_end

int kheap_enabled = 0;

u32 _placement = (u32) &end;

void* kmalloc_cont(u32 sz, int align, u32* phys) {
    if (align == 1 && (_placement & 0xFFFFF000) ) {
        // Align the placement address

        _placement &= 0xFFFFF000;
        _placement += 0x1000;
    }

    if (phys)
        *phys = _placement;
    
    u32 tmp = _placement;
    _placement += sz;

    return (void*) tmp;
}

u32 kmalloc_int(u32 sz, int align, u32* phys) {
    if (heap_start != NULL) {
        if(align) 
            sz = sz + 4096;

        void* addr = malloc(sz);
        u32 align_addr = ((u32) addr & 0xFFFFF000) + 0x1000;

        if (phys != 0) {
            u32 t = (u32) addr;
            
            if(align)
                t = align_addr;

             *phys = (u32) virtual2phys(kpage_dir, (void*) t);
        }
        
        if(align)
            return align_addr;

        return (u32) addr;
    } else {
        if (align == 1 && (_placement & 0xFFFFF000)) {
            _placement &= 0xFFFFF000;
            _placement += 0x1000;
        }

        if (phys) {
            *phys = _placement;
        }

        u32 tmp = _placement;
        _placement += sz;

        return tmp;
    }
}

void* kmalloc_align(u32 sz) {
    return (void*) kmalloc_int(sz, 1, 0);
}

u32 kmalloc_p(u32 sz, u32 *phys) {
    return kmalloc_int(sz, 0, phys);
}

u32 kmalloc_ap(u32 sz, u32 *phys) {
    return kmalloc_int(sz, 1, phys);
}

void* kmalloc(u32 sz) {
    return (void*) kmalloc_int(sz, 0, 0);
}

void* kcalloc(u32 num, u32 size) {
    void * ptr = malloc(num * size);
    memset(ptr, 0, num*size);

    return ptr;
}

void* krealloc(void* ptr, u32 size) {
    return realloc(ptr, size);
}

void kfree(void* ptr) {
    free(ptr);
}

void kheap_init(void* start, void* end, void* max) {
    heap_start = start;
    heap_end = end;
    heap_max = max;
    heap_curr = start;

    kheap_enabled = 1;
}

u32 getRealSize(u32 size) {
    return (size >> 1) << 1;
}

int isEnd(struct memory_block* n) {
    return n == tail;
}

int doesItFit(struct memory_block* n, u32 size) {
    return n->size >= getRealSize(size) && isFree(n);
}

void setFree(u32* size, int x) {
    if(x) {
        *size = *size | 1;
        return;
    }

    *size = *size & 0xFFFFFFFE;
}

int isFree(struct memory_block* n) {
    if(!n) 
        return 0;

    return (n->size & 0x1);
}

void removeNodeFromFreelist(struct memory_block* x) {
    if(!x) 
        return;

    if(x->prev) {
        x->prev->next = x->next;

        if(x->next)
            x->next->prev = x->prev;
    } else {
        freelist = x->next;

        if(freelist)
            freelist->prev = NULL;
    }
}

void addNodeToFreelist(struct memory_block* x) {
    if(!x) 
        return;

    x->next = freelist;

    if(freelist)
        freelist->prev = x;

    freelist = x;
    freelist->prev = NULL;
}

struct memory_block* bestfit(u32 size) {
    if(!freelist) 
        return NULL;

    struct memory_block* curr = freelist;
    struct memory_block* currBest = NULL;

    while(curr) {
        if(doesItFit(curr, size)) {
            if(currBest == NULL || curr->size < currBest->size)
                currBest = curr;
        }

        curr = curr ->next;
    }

    return currBest;;
}

struct memory_block* getPrevBlock(struct memory_block* n) {
    if(n == head) 
        return NULL;
    
    void* ptr = n;
    u32* uptr = ptr - sizeof(u32);
    u32  prev_size = getRealSize(*uptr);

    void* ret = ptr - OVERHEAD - prev_size;
    return ret;
}

struct memory_block* getNextBlock(struct memory_block* n) {
    if(n == tail) 
        return NULL;

    void* ptr = n;
    ptr = ptr + OVERHEAD + getRealSize(n->size);

    return ptr;
}

void *malloc(u32 size) {
    if(size == 0) 
        return NULL;
        
    u32 roundedSize = ((size + 15) / 16) * 16;
    u32 blockSize = roundedSize + OVERHEAD;

    struct memory_block* best;
    best = bestfit(roundedSize);

    u32* trailingSize = NULL;
    
    if(best) {
        void* ptr = (void*) best;
        void* saveNextBlock = getNextBlock(best);

        u32 chunkSize = getRealSize(best->size) + OVERHEAD;
        u32 rest = chunkSize - blockSize;
        u32 whichSize = (rest < 8 + OVERHEAD) ? chunkSize : blockSize;

        best->size = whichSize - OVERHEAD;

        setFree(&(best->size), 0);

        void * base = ptr;
        trailingSize = ptr + whichSize - sizeof(u32);
        *trailingSize = best->size;

        ptr = (void*)(trailingSize + 1);

        if(rest < 8 + OVERHEAD) 
            goto noSplit;
            
        if(rest >= 8) {
            if(base != tail && isFree(saveNextBlock)) {
                void* nextblock = saveNextBlock;
                struct memory_block* n_nextblock = nextblock;
                
                removeNodeFromFreelist(n_nextblock);
                
                struct memory_block* t = ptr;

                t->size = rest - OVERHEAD + getRealSize(n_nextblock->size) + OVERHEAD;
                setFree(&(t->size), 1);

                ptr = ptr + sizeof(struct memory_block) + getRealSize(t->size);
                trailingSize = ptr;
                *trailingSize = t->size;

                if(nextblock == tail){
                    tail = t;

                    //int reclaimSize = getRealSize(t->size) + OVERHEAD;
                    //ksbrk(-reclaimSize);
                    //goto noSplit;
                }
                
                addNodeToFreelist(t);
            } else {
                struct memory_block* putThisBack = ptr;
                
                putThisBack->size = rest - OVERHEAD;
                setFree(&(putThisBack->size), 1);

                trailingSize = ptr + sizeof(struct memory_block) + getRealSize(putThisBack->size);
                *trailingSize = putThisBack->size;

                if(base == tail){
                    tail = putThisBack;

                    //int reclaimSize = getRealSize(putThisBack->size) + OVERHEAD;
                    //ksbrk(-reclaimSize);
                    //goto noSplit;
                }

                addNodeToFreelist(putThisBack);
            }
        }
noSplit:
        removeNodeFromFreelist(base);
        return base + sizeof(struct memory_block);
        
    } else {
        u32 realsize = blockSize;
        struct memory_block* ret = ksbrk(realsize);
        
        ASSERT(ret != NULL && "Heap is running out of space\n");
        
        if(!head) 
            head = ret;
        
        void * ptr = ret;
        void * save = ret;
        tail = ptr;

        ret->size = blockSize - OVERHEAD;
        setFree(&(ret->size), 0);

        ptr = ptr + blockSize - sizeof(u32);

        trailingSize = ptr;
        *trailingSize = ret->size;
        
        return save + sizeof(struct memory_block);
    }
}

void free(void *ptr) {
    struct memory_block* curr = ptr - sizeof(struct memory_block);
    struct memory_block* prev = getPrevBlock(curr);
    struct memory_block* next = getNextBlock(curr);

    if(isFree(prev) && isFree(next)) {
        prev->size = getRealSize(prev->size) + 2 * OVERHEAD + 
                     getRealSize(curr->size) + 
                     getRealSize(next->size);

        setFree(&(prev->size), 1);

        u32* trailingSize = (void*) prev + sizeof(struct memory_block) + getRealSize(prev->size);
        *trailingSize = prev->size;
        
        if(tail == next) 
            tail = prev;

        removeNodeFromFreelist(next);

    } else if(isFree(prev)) {
        prev->size = getRealSize(prev->size) + OVERHEAD + getRealSize(curr->size);
        setFree(&(prev->size), 1);

        u32 * trailingSize = (void*) prev + sizeof(struct memory_block) + getRealSize(prev->size);
        *trailingSize = prev->size;

        if(tail == curr) 
            tail = prev;

    } else if(isFree(next)) {
        curr->size = getRealSize(curr->size) + OVERHEAD + getRealSize(next->size);
        setFree(&(curr->size), 1);

        u32 * trailingSize = (void*) curr + sizeof(struct memory_block) + getRealSize(curr->size);
        *trailingSize = curr->size;

        if(tail == next) 
            tail = curr;

        removeNodeFromFreelist(next);
        addNodeToFreelist(curr);

    } else {
        setFree(&(curr->size), 1);

        u32* trailingSize = (void*) curr + sizeof(struct memory_block) + getRealSize(curr->size);
        *trailingSize = curr->size;

        addNodeToFreelist(curr);
    }
}

void* realloc(void *ptr, u32 size) {
    u32* trailingSize = NULL;

    if(!ptr) 
        return malloc(size);

    if(size == 0 && ptr != NULL) {
        free(ptr);
        return NULL;
    }

    u32 roundedSize = ((size + 15) / 16) * 16;
    u32 blockSize = roundedSize + OVERHEAD;

    struct memory_block *nextBlock, *prevBlock;
    struct memory_block* nptr = ptr - sizeof(struct memory_block);

    nextBlock = getNextBlock(nptr);
    prevBlock = getPrevBlock(nptr);

    if(nptr->size == size) 
        return ptr;

    if(nptr->size < size) {
        if(tail != nptr && isFree(nextBlock) && (getRealSize(nptr->size) + OVERHEAD + getRealSize(nextBlock->size)) >= roundedSize) {
            removeNodeFromFreelist(nextBlock);

            nptr->size = getRealSize(nptr->size) + OVERHEAD + getRealSize(nextBlock->size);
            setFree(&(nptr->size), 0);

            trailingSize = (void*)nptr + sizeof(struct memory_block) + getRealSize(nptr->size);
            *trailingSize = nptr->size;

            if(tail == nextBlock)
                tail = nptr;

            return nptr + 1;

        } else if(head != nptr && isFree(prevBlock) && (getRealSize(nptr->size) + OVERHEAD + getRealSize(prevBlock->size)) >= roundedSize) {
            u32 originalSize = getRealSize(nptr->size);
            
            removeNodeFromFreelist(prevBlock);
            
            prevBlock->size = originalSize + OVERHEAD + getRealSize(prevBlock->size);
            setFree(&(prevBlock->size), 0);

            trailingSize = (void*)prevBlock + sizeof(struct memory_block) + getRealSize(prevBlock->size);
            *trailingSize = prevBlock->size;

            if(tail == nptr)
                tail = prevBlock;

            memcpy(prevBlock+1, ptr, originalSize);
            return prevBlock + 1;
        }

        void* newplace = malloc(size);
        memcpy(newplace, ptr, getRealSize(nptr->size));

        free(ptr);
        return newplace;
    } else {
        u32 rest = getRealSize(nptr->size) + OVERHEAD - blockSize;
        
        if(rest < 8 + OVERHEAD) 
            return ptr;

        nptr->size = blockSize - OVERHEAD;
        setFree(&(nptr->size), 0);

        trailingSize = (void*)nptr + sizeof(struct memory_block) + getRealSize(nptr->size);
        *trailingSize = nptr->size;
        
        /*
           if(tail == nptr) {
           ksbrk(-reclaimSize);
           return ptr;
           }
        */

        struct memory_block* splitBlock = (void*) trailingSize + sizeof(u32);

        if(nextBlock && isFree(nextBlock)) {
            splitBlock->size = rest + getRealSize(nextBlock->size);
            setFree(&(splitBlock->size), 1);

            trailingSize = (void*)splitBlock + sizeof(struct memory_block) + getRealSize(splitBlock->size);
            *trailingSize = splitBlock->size;

            removeNodeFromFreelist(nextBlock);

            if(tail == nextBlock)
                tail = splitBlock;
            
            addNodeToFreelist(splitBlock);

            return ptr;
        }

        splitBlock->size = rest - OVERHEAD;
        setFree(&(splitBlock->size), 1);

        trailingSize = (void*) splitBlock + sizeof(struct memory_block) + getRealSize(splitBlock->size);
        *trailingSize = splitBlock->size;
        
        addNodeToFreelist(splitBlock);

        return ptr;
    }
}
