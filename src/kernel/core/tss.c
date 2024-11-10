#include "tss.h"
#include "stack.h"

struct kernel_tss_struct kernel_tss;

void tss_init() {
    kernel_tss.ss0          = 0x10;
    kernel_tss.esp0         = KERNEL_STACK_TOP;
    kernel_tss.iomap_base   = sizeof(struct kernel_tss_struct);
}

void tss_load() {
    asm volatile("ltr %%ax" :: "a"(0x28));
}
