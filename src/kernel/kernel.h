#ifndef __KERNEL_H
#define __KERNEL_H

#include "types.h"

#define IGNORE_UNUSED(X)    (void)(X)

#define BRAND_QEMU  1
#define BRAND_VBOX  2

extern u8 __kernel_section_start;
extern u8 __kernel_section_end;
extern u8 __kernel_text_section_start;
extern u8 __kernel_text_section_end;
extern u8 __kernel_data_section_start;
extern u8 __kernel_data_section_end;
extern u8 __kernel_rodata_section_start;
extern u8 __kernel_rodata_section_end;
extern u8 __kernel_bss_section_start;
extern u8 __kernel_bss_section_end;

typedef struct {
    u32 ds;
    u32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u32 int_no, err_code;
    u32 eip, cs, eflags, useresp, ss;
} REGISTERS;

BOOL is_echo(char *b);

void shutdown();

// _____ CPUID.C _____
void cpuid(u32 type, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);
int cpuid_info(int print);

void kernel_panic(REGISTERS* reg, signed int exception);

void kout(enum kernel_statusLevels lvl, char* interface, char* message, char* query);

#endif