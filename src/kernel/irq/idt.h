#ifndef __IDT_H
#define __IDT_H

#include "types.h"

#define NO_IDT_DESCRIPTORS     256

typedef struct {
    u16 base_low;
    u16 segment_selector;
    u8  zero;
    u8  type;
    u16 base_high;
} __attribute__((packed)) IDT;

typedef struct {
    u16 limit;
    u32 base_address;
} __attribute__((packed)) IDT_PTR;

extern void idt_load(u32 idt_ptr); // IDT.ASM

void idt_setEntry(int index, u32 base, u16 seg_sel, u8 flags);
void idt_init();

#endif