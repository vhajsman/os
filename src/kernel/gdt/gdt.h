#ifndef __GDT_H
#define __GDT_H

#include "types.h"

#define NO_GDT_DESCRIPTORS 8

typedef struct {
    u16     segment_limit;
    u16     base_low;
    u8      base_middle;
    u8      access;
    u8      granularity;
    u8      base_high;
} __attribute__((packed)) GDT;

typedef struct {
    u16     limit;
    u32     base_address;
} __attribute__((packed)) GDT_PTR;

extern void gdt_load(u32 gdt_ptr); // GDT.ASM

void gdt_setEntry(int index, u32 base, u32 limit, u8 access, u8 gran);
void gdt_init();

#endif