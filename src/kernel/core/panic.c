#include "kernel.h"
#include "debug.h"
#include "console.h"
#include "irq/isr.h"
#include "string.h"

// For blinking keyboard leds
#include "hid/kbd.h"
#include "devices/pit.h"

#define KERNEL_DEBUG_DUMP_SINGLE_REG(REGS, REG) {   \
    char regdump[10];                               \
    itoa(regdump, 16, REGS->REG);                   \
                                                    \
    debug_append("\n                 --> ");        \
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

#ifndef __KERNEL_PANIC_STACK_TRACE_DEPTH
#define __KERNEL_PANIC_STACK_TRACE_DEPTH 12
#endif

void kernel_panic(REGISTERS* reg, signed int exception) {
    debug_separator("KERNEL PANIC");
    debug_message("Kernel panic.", "Kernel panic", KERNEL_FATAL);

    colorPrint("----- KERNEL PANIC -----\n", 128 + 4);
    printf("Kernel panic.\n");

    if(reg != NULL)
        kernel_panic_dumpreg(reg, exception);


    // --- stack trace ---
    debug_message("kernel stack trace\n", "Kernel panic", KERNEL_FATAL);
    void _print(unsigned char* data) {
        puts((const char*) data);
        debug_append((const char*) data);
    }

    puts("\nKERNEL STACK TRACE:\n");
    debug_dumpStackTrace(__KERNEL_PANIC_STACK_TRACE_DEPTH, &_print);

    puts("--------------");

    asm("cli");
    while(1) {
        // asm("hlt");
    }
}
