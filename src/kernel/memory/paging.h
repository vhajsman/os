#ifndef __PAGING_H
#define __PAGING_H

#include "kernel.h"
#include "string.h"

#define PAGE_SIZE 4096

#define IS_ALIGN(addr)      \
    ((((u32) (addr)) | 0xFFFFF000) == 0)
#define PAGE_ALIGN(addr)    \
    ((((u32) (addr)) & 0xFFFFF000) + 0x1000)

#define PAGEDIR_INDEX(vaddr) \
    (((u32) vaddr) >> 22)
#define PAGETBL_INDEX(vaddr) \
    ((((u32) vaddr) >>12) & 0x3ff)
#define PAGEFRAME_INDEX(vaddr) \
    (((u32) vaddr) & 0xfff)

#define SET_PGBIT(cr0) \
    (cr0 = cr0 | 0x80000000)
#define CLEAR_PSEBIT(cr4) \
    (cr4 = cr4 & 0xffffffef)

#define PAGING_ERR_PRESENT     0x1
#define PAGING_ERR_RW          0x2
#define PAGING_ERR_USER        0x4
#define PAGING_ERR_RESERVED    0x8
#define PAGING_ERR_INST        0x10

// extern u8* bitmap;
// extern u32  bitmap_size;

typedef struct paging_dir_entry {
    unsigned int present    : 1;
    unsigned int rw         : 1;
    unsigned int user       : 1;
    unsigned int w_through  : 1;
    unsigned int cache      : 1;
    unsigned int access     : 1;
    unsigned int reserved   : 1;
    unsigned int page_size  : 1;
    unsigned int global     : 1;
    unsigned int available  : 3;
    unsigned int frame      : 20;
} page_dir_entry_t;

typedef struct page_table_entry {
    unsigned int present    : 1;
    unsigned int rw         : 1;
    unsigned int user       : 1;
    unsigned int reserved   : 2;
    unsigned int accessed   : 1;
    unsigned int dirty      : 1;
    unsigned int reserved2  : 2;
    unsigned int available  : 3;
    unsigned int frame      : 20;
} page_table_entry_t;

typedef struct page_table {
    page_table_entry_t pages[1024];
} page_table_t;

typedef struct page_directory {
    page_dir_entry_t tables[1024];
    page_table_t* ref_tables[1024];
} page_directory_t;

// Defined in entry.asm
// extern page_directory_t* TEMP_PAGE_DIRECTORY;

extern page_directory_t* kpage_dir;
extern page_directory_t* TEMP_PAGE_DIRECTORY;

/*
 * Convert virtual address to physical address
 * If it's the temp page dir, simply subtract 0xC0000000 since we do the page mapping manually in entry.asm
 * Otherwise, search the whole page table.
 * */
void* virtual2phys(page_directory_t* dir, void* virtual_addr);

/*
 * A dumb malloc, just to help building the paging data structure for the first 4mb that our kernel uses
 * It only manages memory from the end of pmm bitmap, to 0xC0400000, approximately 2mb.
 * */
void* dumb_kmalloc(u32 size, int align);

/*
 * Allocate a frame from pmm, write frame number to the page structure
 * You may notice that we've set the both the PDE and PTE as user-accessible with r/w permissions, because..... we don't care security
 * */
void paging_allocate(page_directory_t* dir, u32 virtual_addr, u32 frame, int is_kernel, int is_writable);

/*
 * Allocate a set of pages specified by the region
 * */
void paging_allocateRegion(page_directory_t* dir, u32 start_va, u32 end_va, int iden_map, int is_kernel, int is_writable);

/*
 * Find the corresponding page table entry, and set frame to 0
 * Also, clear corresponding bit in pmm bitmap
 * @parameter free:
 *      0: only clear the page table entry, don't actually free the frame
 *      1 : free the frame
 * */
void paging_free(page_directory_t * dir, u32 virtual_addr, int _free);

/*
 * Free all frames within the region
 * */
void paging_freeRegion(page_directory_t* dir, u32 start_va, u32 end_va, int free);

void paging_init();

/*
 * Switch page directory,
 * phys : Is the address given physical or virtual ?
 * */
void paging_switchdir(page_directory_t* page_dir, u32 phys);

/*
 * Enable paging, turn off PSE bit first as it was turned on by entry.asm when kernel was loading
 * Then enable PG Bit in cr0
 * */
void paging_enable();

void * ksbrk(int size);

/*
 * Copy a page table
 * */
page_table_t* paging_copyTable(page_directory_t* src_page_dir, page_directory_t* dst_page_dir, u32 page_dir_idx, page_table_t* src);

/*
 * Copy a page directory
 * */
void paging_copyDir(page_directory_t* dst, page_directory_t* src);

#endif
