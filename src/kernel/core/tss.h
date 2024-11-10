#ifndef _TSS_H
#define _TSS_H

#include "types.h"

struct kernel_tss_struct {
    u32 prev;
    u32 esp0, ss0, esp1, ss1, esp2, ss2, cr3;
    u32 eip, eflags;
    u32 eax, ecx, edx, ebx;
    u32 esp, ebp, esi, edi;
    u32 es, cs, ss, ds, fs, gs;
    u32 ldt;
    u16 trap;
    u16 iomap_base;
};

extern struct kernel_tss_struct kernel_tss;

void tss_init();
void tss_load();

#endif
