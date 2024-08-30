#include "gdt.h"

GDT g_gdt[NO_GDT_DESCRIPTORS];
GDT_PTR g_gdt_ptr;

void gdt_setEntry(int index, u32 base, u32 limit, u8 access, u8 gran) {
    GDT *this = &g_gdt[index];

    this->segment_limit =   limit & 0xFFFF;
    this->base_low =        base  & 0xFFFF;
    this->base_middle =     (base >> 16) & 0xFF;
    this->access =          access;;

    this->granularity = (limit >> 16) & 0x0F;
    this->granularity = this->granularity | (gran & 0xF0);

    this->base_high = (base >> 24 & 0xFF);
}

// initialize GDT
void gdt_init() {
    g_gdt_ptr.limit = sizeof(g_gdt) - 1;
    g_gdt_ptr.base_address = (u32) g_gdt;

    
    gdt_setEntry(0, 0, 0, 0, 0);                   // NULL segment
    gdt_setEntry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);    // code segment
    gdt_setEntry(2, 0, 0xFFFFFFFF, 0x92, 0xCF);    // data segment
    gdt_setEntry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);    // user code segment
    gdt_setEntry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);    // user data segment

    gdt_load((u32) &g_gdt_ptr);
}