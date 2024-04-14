#include "kernel.h"
#include <string.h>
#include "console.h"

void cpuid(u32 type, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx) {
    asm volatile("cpuid"
                : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                : "0"(type)); // put the type into eax
}

int cpuid_info(int print) {
    u32 brand[12];
    u32 eax, ebx, ecx, edx;
    u32 type;

    memset(brand, 0, sizeof(brand));
    cpuid(0x80000002, (u32*) brand + 0x0, (u32*) brand + 0x1, (u32*) brand + 0x2, (u32*) brand + 0x3);
    cpuid(0x80000003, (u32*) brand + 0x4, (u32*) brand + 0x5, (u32*) brand + 0x6, (u32*) brand + 0x7);
    cpuid(0x80000004, (u32*) brand + 0x8, (u32*) brand + 0x9, (u32*) brand + 0xa, (u32*) brand + 0xb);

    if (print) {
        printf("CPUID: Brand: %s\n", brand);

        for(type = 0; type < 4; type++) {
            cpuid(type, &eax, &ebx, &ecx, &edx);
            printf("CPUID: type:0x%x, eax:0x%x, ebx:0x%x, ecx:0x%x, edx:0x%x\n", type, eax, ebx, ecx, edx);
        }
    }

    return (strstr(brand, "QEMU") != NULL) ? BRAND_QEMU : BRAND_VBOX;
}
