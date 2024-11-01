#ifndef __KERNEL_H
#define __KERNEL_H

#include "types.h"
#include "debug.h"
#include "fs/fs.h"
#include "initrd.h"
#include "fs/tar.h"
#include "modules/interface.h"

#ifndef IGNORE_UNUSED
#define IGNORE_UNUSED(X)    (void)(X)
#endif

#define BRAND_QEMU  1
#define BRAND_VBOX  2

#ifndef ALIGN
#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#endif

// #define PANIC(msg) panic(msg, __FILE__, __LINE__)

#ifndef ASSERT
#define ASSERT(b)
#endif

#define LOAD_MEMORY_ADDRESS 0xC0000000

#ifndef NULL
#define NULL ((void*)0)
#endif

#define TRUE 1
#define FALSE 0

#define K 1024
#define M (1024*K)
#define G (1024*M)


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

typedef struct {
    u16 di;
    u16 si;
    u16 bp;
    u16 sp;
    u16 bx;
    u16 dx;
    u16 cx;
    u16 ax;

    u16 ds;
    u16 es;
    u16 fs;
    u16 gs;
    u16 ss;
    u16 eflags;
} REGISTERS_16;

BOOL is_echo(char *b);

void shutdown();

// _____ CPUID.C _____
void cpuid(u32 type, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);
int cpuid_info(int print);

void kernel_panic(REGISTERS* reg, signed int exception);

void kout(enum kernel_statusLevels lvl, char* interface, char* message, char* query);


#endif