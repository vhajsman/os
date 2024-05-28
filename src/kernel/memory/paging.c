#include "paging.h"
#include "debug.h"
#include "math.h"

#define __REQ_ALL_ADDRESS
#include "memory.h"

void paging_fault(REGISTERS* reg);

// Defined in kheap.c
// extern void * heap_start, * heap_end, * heap_max, * heap_curr;
// extern int kheap_enabled;

u8* temp_mem;
u8 _paging_enabled;

page_directory_t* kpage_dir;

/*
 * Convert virtual address to physical address
 * If it's the temp page dir, simply subtract 0xC0000000 since we do the page mapping manually in entry.asm
 * Otherwise, search the whole page table.
 * */
void* virtual2phys(page_directory_t* dir, void* virtual_addr) {
    if(!_paging_enabled)
        return (void*) (virtual_addr - LOAD_MEMORY_ADDRESS);

    u32 
    page_dir_idx =      PAGEDIR_INDEX(virtual_addr), 
    page_tbl_idx =      PAGETBL_INDEX(virtual_addr), 
    page_frame_offset = PAGEFRAME_INDEX(virtual_addr);

    if(!dir->ref_tables[page_dir_idx]) {
        debug_message("Page dir entry does not exist", "Paging", KERNEL_FATAL);
        return NULL;
    }

    page_table_t* table = dir->ref_tables[page_dir_idx];

    if(!table->pages[page_tbl_idx].present) {
        debug_message("Page dir entry does not exist", "Paging", KERNEL_FATAL);
        return NULL;
    }

    u32 t = table->pages[page_tbl_idx].frame;
        t = (t << 12) + page_frame_offset;

    return (void*) t;
}

/*
 * A dumb malloc, just to help building the paging data structure for the first 4mb that our kernel uses
 * It only manages memory from the end of pmm bitmap, to 0xC0400000, approximately 2mb.
 * */
void* dumb_kmalloc(u32 size, int align) {
    void * ret = temp_mem;

    if(align && !IS_ALIGN(ret))
        ret = (void*)PAGE_ALIGN(ret);

    temp_mem = temp_mem + size;
    return ret;
}

/*
 * Allocate a frame from pmm, write frame number to the page structure
 * You may notice that we've set the both the PDE and PTE as user-accessible with r/w permissions, because..... we don't care security
 * */
void paging_allocate(page_directory_t* dir, u32 virtual_addr, u32 frame, int is_kernel, int is_writable) {
    IGNORE_UNUSED(is_kernel);
    
    page_table_t* table = NULL;

    if(!dir) {
        debug_message("Page directory empty", "Page", KERNEL_MESSAGE);
        return;
    }

    u32 
    page_dir_idx = PAGEDIR_INDEX(virtual_addr), 
    page_tbl_idx = PAGETBL_INDEX(virtual_addr);
    
    table = dir->ref_tables[page_dir_idx];

    if(!table) {
        if(!kheap_enabled) {
            table = dumb_kmalloc(sizeof(page_table_t), 1);
        } else {
            table = kmalloc_align(sizeof(page_table_t));
        }

        memset(table, 0, sizeof(page_table_t));

        u32 t = (u32) virtual2phys(kpage_dir, table);

        dir->tables[page_dir_idx].frame = t >> 12;
        dir->tables[page_dir_idx].present = 1;
        dir->tables[page_dir_idx].rw = 1;
        dir->tables[page_dir_idx].user = 1;
        dir->tables[page_dir_idx].page_size = 0;

        dir->ref_tables[page_dir_idx] = table;
    }

    if(!table->pages[page_tbl_idx].present) {
        u32 t = frame ? frame : memory_allocateBlk();

        table->pages[page_tbl_idx].frame = t;
        table->pages[page_tbl_idx].present = 1;
        table->pages[page_tbl_idx].rw = 1;
        table->pages[page_tbl_idx].user = 1;
    }
}

/*
 * Allocate a set of pages specified by the region
 * */
void paging_allocateRegion(page_directory_t* dir, u32 start_va, u32 end_va, int iden_map, int is_kernel, int is_writable) {
    u32 start   = start_va  & 0xfffff000;
    u32 end     = end_va    & 0xfffff000;

    while(start <= end) {
        if(iden_map) {
            paging_allocate(dir, start, start / PAGE_SIZE, is_kernel, is_writable);
        } else {
            paging_allocate(dir, start, 0, is_kernel, is_writable);
        }

        start = start + PAGE_SIZE;
    }
}

/*
 * Find the corresponding page table entry, and set frame to 0
 * Also, clear corresponding bit in pmm bitmap
 * @parameter free:
 *      0: only clear the page table entry, don't actually free the frame
 *      1 : free the frame
 * */
void paging_free(page_directory_t * dir, u32 virtual_addr, int _free) {
    if(dir == TEMP_PAGE_DIRECTORY) 
        return;

    u32 
    page_dir_idx = PAGEDIR_INDEX(virtual_addr), 
    page_tbl_idx = PAGETBL_INDEX(virtual_addr);

    if(!dir->ref_tables[page_dir_idx]) {
        debug_message("Free page: page dir entry does not exist", "Page", KERNEL_ERROR);
        return;
    }

    page_table_t* table = dir->ref_tables[page_dir_idx];

    if(!table->pages[page_tbl_idx].present) {
        debug_message("Free page: page dir entry does not exist", "Page", KERNEL_ERROR);
        return;
    }

    if(_free)
        free(table->pages[page_tbl_idx].frame);

    table->pages[page_tbl_idx].present = 0;
    table->pages[page_tbl_idx].frame = 0;
}

/*
 * Free all frames within the region
 * */
void paging_freeRegion(page_directory_t* dir, u32 start_va, u32 end_va, int _free) {
    u32 start   = start_va  & 0xfffff000;
    u32 end     = end_va    & 0xfffff000;

    while(start <= end) {
        paging_free(dir, start, 1);
        start = start + PAGE_SIZE;
    }

    IGNORE_UNUSED(_free);
}

void paging_init() {
    temp_mem = bitmap + bitmap_size;

    // Allocate a page directory and set it to all zeros
    kpage_dir = dumb_kmalloc(sizeof(page_directory_t), 1);
    memset(kpage_dir, 0, sizeof(page_directory_t));

    // Now, map 4mb begining from 0xC0000000 to 0xC0400000
    // (should corresponding to first 1024 physical blocks)
    u32 i = LOAD_MEMORY_ADDRESS;
    while(i < LOAD_MEMORY_ADDRESS + 4 * M) {
        paging_allocate(kpage_dir, i, 0, 1, 1);
        i = i + PAGE_SIZE;
    }
    
    i = LOAD_MEMORY_ADDRESS + 4 * M;
    while(i < LOAD_MEMORY_ADDRESS + 4 * M + KHEAP_INITIAL_SIZE) {
        paging_allocate(kpage_dir, i, 0, 1, 1);
        i = i + PAGE_SIZE;
    }

    isr_registerInterruptHandler(14, paging_fault);
    paging_switchdir(kpage_dir, 0);

    paging_enable();
    
    paging_allocateRegion(kpage_dir, 0x0, 0x10000, 1, 1, 1);
}

/*
 * Switch page directory,
 * phys : Is the address given physical or virtual ?
 * */
void paging_switchdir(page_directory_t* page_dir, u32 phys) {
    u32 t = phys    ? (u32) page_dir 
                    : (u32) virtual2phys(TEMP_PAGE_DIRECTORY, page_dir);
    
    // if(!phys)
    //     t = (uint32_t)virtual2phys(TEMP_PAGE_DIRECTORY, page_dir);
    // else
    //     t = (uint32_t)page_dir;

    asm volatile("mov %0, %%cr3" :: "r"(t));
}

/*
 * Enable paging, turn off PSE bit first as it was turned on by entry.asm when kernel was loading
 * Then enable PG Bit in cr0
 * */
void paging_enable() {
    u32 cr0, cr4;

    asm volatile("mov %%cr4, %0" : "=r"(cr4));
    CLEAR_PSEBIT(cr4);
    asm volatile("mov %0, %%cr4" :: "r"(cr4));

    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    SET_PGBIT(cr0);
    asm volatile("mov %0, %%cr0" :: "r"(cr0));

    _paging_enabled = 1;
}

void * ksbrk(int size) {
    void * runner = NULL;
    void * new_boundary;
    void * old_heap_curr;

restart_sbrk:
    old_heap_curr = heap_curr;
    
    if(size == 0)
        goto ret;

    if(size > 0) {
        new_boundary = heap_curr + (u32) size;
        
        if(new_boundary <= heap_end) {
            goto update_boundary;
        } else if(new_boundary > heap_max) {
            return NULL;
        }

        if(new_boundary > heap_end) {
            // expand the heap by getting more pages
            runner = heap_end;
            while(runner < new_boundary) {
                paging_allocate(kpage_dir, (u32) runner, 0, 1, 1);
                runner = runner +  PAGE_SIZE;
            }

            // Put away the page table first, then sbrk user-requested data again
            if(old_heap_curr != heap_curr)
                goto restart_sbrk;
            
            heap_end = runner;
            goto update_boundary;
        }
    }
    
    if(size < 0){
        // Free as many pages as possible, then update heap_end, heap_curr and return old_heap_curr
        new_boundary = (void*) ((u32) heap_curr - (u32) abs(size));

        if(new_boundary < heap_start + HEAP_MIN_SIZE)
            new_boundary = heap_start + HEAP_MIN_SIZE;

        runner = heap_end - PAGE_SIZE;
        while(runner > new_boundary) {
            paging_free(kpage_dir, (u32) runner, 1);
            runner = runner - PAGE_SIZE;
        }

        heap_end = runner + PAGE_SIZE;
        goto update_boundary;
    }

update_boundary:
    heap_curr = new_boundary;

ret:
    return old_heap_curr;
}

/*
 * Copy a page table
 * */
page_table_t* paging_copyTable(page_directory_t* src_page_dir, page_directory_t* dst_page_dir, u32 page_dir_idx, page_table_t* src) {
    page_table_t* table = (page_table_t*) kmalloc_align(sizeof(page_table_t));

    for(int i = 0; i < 1024; i++) {
        if(!table->pages[i].frame)
            continue;
        
        u32 src_virtual_address = (page_dir_idx << 22) | (i << 12) | (0);  // Source frame's virtual address
        u32 dst_virtual_address = src_virtual_address;                     // Destination frame's virtual address
        u32 tmp_virtual_address = 0;                                       // Temporary virtual address in current virtual address space

        // Allocate a frame in destination page table
        paging_allocate(dst_page_dir, dst_virtual_address, 0, 0, 1);
        
        // Point tmp_virtual_address and dst_virtual_address both to the same frame
        paging_allocate(src_page_dir, tmp_virtual_address, (u32) virtual2phys(dst_page_dir, (void*)dst_virtual_address), 0, 1);
        
        if (src->pages[i].present) table->pages[i].present =  1;
        if (src->pages[i].rw)      table->pages[i].rw =       1;
        if (src->pages[i].user)    table->pages[i].user =     1;
        if (src->pages[i].accessed)table->pages[i].accessed = 1;
        if (src->pages[i].dirty)   table->pages[i].dirty =    1;

        memcpy((void*) tmp_virtual_address, (void*) src_virtual_address, PAGE_SIZE);
        
        // Unlink frame
        paging_free(src_page_dir, tmp_virtual_address, 0);
    }

    return table;
}


/*
 * Copy a page directory
 * */
void paging_copyDir(page_directory_t* dst, page_directory_t* src) {
    for(u32 i = 0; i < 1024; i++) {
        if(kpage_dir->ref_tables[i] == src->ref_tables[i]) {
            dst->tables[i] = src->tables[i];
            dst->ref_tables[i] = src->ref_tables[i];
        } else {
            // For non-kernel pages, copy the pages

            dst->ref_tables[i] = paging_copyTable(src, dst, i, src->ref_tables[i]);
            u32 phys = (u32) virtual2phys(src, dst->ref_tables[i]);

            dst->tables[i].frame = phys >> 12;
            dst->tables[i].user = 1;
            dst->tables[i].rw = 1;
            dst->tables[i].present = 1;
        }
    }
}

/* Print out useful information when a page fault occur
 * */
void paging_fault(REGISTERS* reg) {
    asm volatile("sti");

    debug_message("Page fault detected", "Memory", KERNEL_FATAL);
    kernel_panic(reg, -1);
} 

#undef __REQ_ALL_ADDRESS
