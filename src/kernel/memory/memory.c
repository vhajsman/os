#include "memory.h"
#include "multiboot.h"
#include "console.h"
#include "debug.h"
#include "string.h"

u32 _mem_low;
u32 _mem_high;

u32 _map_length;
u32 _map_address;

static u8 memory_dynamicArea[MEMORY_DYNAMIC_TOTAL_SIZE];
static struct memory_block* memory_start;

void memory_printInfo() {
    printf("Memory: ---------------------------\n");

    printf("Memory: Low memory  | %d KB\n", _mem_low);
    printf("Memory: High memory | %d KB\n", _mem_high);
    printf("Memory: Map length  | %d\n", _map_length);
    printf("Memory: Map address | 0x%x\n", _map_address);

    printf("Memory: ---------------------------\n");
}

void memory_init(MULTIBOOT_INFO* mboot_info) {
    _mem_low = mboot_info->mem_low;
    _mem_high = mboot_info->mem_high;

    _map_length = mboot_info->mmap_length;
    _map_address = mboot_info->mmap_addr;

    memory_printInfo();

    char buf[20];

    debug_message("Intialization OK.", "PMMU", KERNEL_OK);
    debug_message("Low Memory  KB   |", "PMMU", KERNEL_MESSAGE);  itoa(buf, 10, mboot_info->mem_low); debug_append(buf);
    debug_message("High Memory KB   |", "PMMU", KERNEL_MESSAGE);  itoa(buf, 10, mboot_info->mem_high); debug_append(buf);
    debug_message("Map length Bytes |", "PMMU", KERNEL_MESSAGE);  itoa(buf, 10, mboot_info->mmap_length); debug_append(buf);
    debug_message("Map address      |", "PMMU", KERNEL_MESSAGE);  itoa(buf, 10, mboot_info->mmap_addr); debug_append(buf);

    memory_start = (struct memory_block*) memory_dynamicArea;
    memory_start->size = MEMORY_DYNAMIC_TOTAL_SIZE - MEMORY_DYNAMIC_NODE_SIZE;
    memory_start->next = nullptr;
    memory_start->prev = nullptr;
}


// ====================================================
// ======== MEMORY BLOCK MERGE
// ====================================================

void* memory_merger(struct memory_block* current) {
    struct memory_block* next = current->next;

    if(next == nullptr || next->used)
        return nullptr;

    current->size += current->next->size + MEMORY_DYNAMIC_NODE_SIZE;
    current->next = current->next->next;

    if(current->next != nullptr)
        current->next->prev = current;

    return current;
}

void* memory_mergel(struct memory_block* current) {
    struct memory_block* prev = current->prev;

    if(prev == nullptr || prev->used)
        return nullptr;

    prev->size += current->size + MEMORY_DYNAMIC_NODE_SIZE;
    prev->next = current->next;

    if(current->next != nullptr)
        current->next->prev = prev;
    
    return current;
}


// ====================================================
// ======== MEMORY ALLOCATION
// ====================================================

void* memory_findBest(struct memory_block* mem, size_t size) {
    struct memory_block* best = (struct memory_block*) nullptr;
    struct memory_block* current = mem;

    u32 best_size = MEMORY_DYNAMIC_TOTAL_SIZE + 1;

    while(current) {
        if( (!current->used) && 
            (current->size >= size +MEMORY_DYNAMIC_NODE_SIZE) &&
            (current->size <= best_size)) {
                best = current;
                best_size = current->size;
        }

        current = current->next;
    }

    return best;
}

void* malloc(size_t size) {
    struct memory_block* best = (struct memory_block*) 
        memory_findBest(memory_start, size);
    
    if(best != nullptr) {
        best->size = best->size - size - MEMORY_DYNAMIC_NODE_SIZE;

        struct memory_block* allocate = (struct memory_block*)
            (((u8*) best) + best->size);

        allocate->size = size;
        allocate->used = 1;
        allocate->next = best->next;
        allocate->prev = best;

        if(best->next != nullptr)
            best->next->prev = allocate;

        best->next = allocate;

        return (void*) ((u8*) allocate + MEMORY_DYNAMIC_NODE_SIZE);
    }

    printf("malloc(): not enough memory (for %d bytes)", size);
    return nullptr;
}


// ====================================================
// ======== MEMORY DE-ALLOCATION (FREEING)
// ====================================================

void free(void* ptr) {
    if(ptr == nullptr)
        return;

    struct memory_block* current = (struct memory_block*) 
        ((u8*) ptr - MEMORY_DYNAMIC_NODE_SIZE);
    
    if(current == nullptr)
        return;
    
    current->used = 0;

    current = memory_merger(current);
    memory_mergel(current);
}
