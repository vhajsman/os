#include "kernel.h"
#include <string.h>
#include "console.h"
#include "debug.h"

char* cpu_brand;

void cpuid(u32 type, u32 *eax, u32 *ebx, u32 *ecx, u32 *edx) {
    asm volatile("cpuid"
                : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
                : "0"(type)); // put the type into eax
}

u32 cpuid_checkBrand(u32* brand) {
    return (strstr((char*) brand, "QEMU") != NULL) ? BRAND_QEMU : BRAND_VBOX;
}

int cpuid_info(int print) {
    u32 brand[12];
    u32 _brand;

    u32 eax, ebx, ecx, edx;
    u32 type;

    static char* eax_str, ebx_str, ecx_str, edx_str;
    char* type_str;

    memset(brand, 0, sizeof(brand));

    cpuid(0x80000002, (u32*) brand + 0x0, (u32*) brand + 0x1, (u32*) brand + 0x2, (u32*) brand + 0x3);
    cpuid(0x80000003, (u32*) brand + 0x4, (u32*) brand + 0x5, (u32*) brand + 0x6, (u32*) brand + 0x7);
    cpuid(0x80000004, (u32*) brand + 0x8, (u32*) brand + 0x9, (u32*) brand + 0xa, (u32*) brand + 0xb);

//    if(print) {
//        //printf("CPUID: Brand: %s\n", brand);
//
//        puts("CPUID: Brand: ");
//        puts(brand);
//        puts("\n");
//    }

    //debug_message("Brand: ", "cpuid", KERNEL_MESSAGE);
    //debug_append(brand);

    debug_message("Brand: ", "cpuid", KERNEL_MESSAGE);

    if(strstr((char*) brand, "QEMU") != NULL) {
        cpu_brand = "QEMU";
    } else {
        cpu_brand = "VBOX";
    }

    debug_append(cpu_brand);


    for(type = 0; type < 4; type++) {
        cpuid(type, &eax, &ebx, &ecx, &edx);

        if(print)
            printf("CPUID: type:0x%x, eax:0x%x, ebx:0x%x, ecx:0x%x, edx:0x%x\n", type, eax, ebx, ecx, edx);

        itoa((char*) eax_str, 16, eax);
        itoa((char*) ebx_str, 16, ebx);
        itoa((char*) ecx_str, 16, ecx);
        itoa((char*) edx_str, 16, edx);

        itoa(type_str, 16, type);

        debug_message("Type: ", "cpuid", KERNEL_MESSAGE);
        debug_append(type_str);

        debug_append(", eax=");
        debug_append((char*) eax_str);
        debug_append(", ebx=");
        debug_append((char*) ebx_str);
        debug_append(", ecx=");
        debug_append((char*) ecx_str);
        debug_append(", edx=");
        debug_append((char*) edx_str);
    }

    // return (strstr(brand, "QEMU") != NULL) ? BRAND_QEMU : BRAND_VBOX;
    // return cpuid_checkBrand(brand);
}
