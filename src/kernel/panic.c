#include "kernel.h"
#include "debug.h"
#include "console.h"
#include "irq/isr.h"
#include "string.h"

#define STRINGIFICATE(X)    #X

#define KERNEL_DEBUG_DUMP_SINGLE_REG(REGS, REG) {   \
    char* regdump;                                  \
    itoa(regdump, 16, REGS->REG);                   \
                                                    \
    debug_append(STRINGIFICATE(REG));               \
    debug_append("=0x");                            \
    debug_append(regdump);                          \
    debug_append(" ");                              \
}

void kernel_panic_dumpreg(REGISTERS* reg, signed int exception) {
    debug_message("Exception caused: ", "Kernel panic", KERNEL_FATAL);
    debug_append(exception_messages[reg->int_no]);

    debug_message("Register dump: ", "Kernel panic", KERNEL_FATAL);

    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, err_code);
    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, eax);
    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, ebx);
    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, ecx);
    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, edx);
    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, edi);
    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, esi);
    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, ebp);
    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, esp);
    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, eip);
    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, cs);
    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, ss);
    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, eflags);
    KERNEL_DEBUG_DUMP_SINGLE_REG(reg, useresp);

    print_registers(reg);

    if(exception >= 0) {
        printf("EXCEPTION: %s\n", exception_messages[reg->int_no]);
    }

}

void kernel_panic(REGISTERS* reg, signed int exception) {

    console_initialize();

    debug_separator("KERNEL PANIC");
    debug_message("Kernel panic.", "Kernel panic", KERNEL_FATAL);

    colorPrint("----- KERNEL PANIC -----\n", 128 + 4);
    printf("Kernel panic.\n");

    kernel_panic_dumpreg(reg, exception);

    puts("\n");
    puts("CPU is now halted. If debugging enabled, check debug log for more information.\n");

    puts("--------------");

    asm("cli");
    while(1) {
    }
}