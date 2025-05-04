#ifndef __CPUID_H
#define __CPUID_H

#include "kernel.h"

#define BRAND_QEMU  1
#define BRAND_VBOX  2

void cpuid(u32 type, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx);
int cpuid_info(int print);

#endif
