#include "kernel.h"
#include "console.h"
#include "string.h"
#include "gdt/gdt.h"
#include "irq/idt.h"
#include "irq/isr.h"
#include "hid/kbd.h"
#include "ioport.h"
#include "shell/shell.h"
#include "com/serial.h"
#include "time/timer.h"
#include "memory/memory.h"
#include "multiboot.h"
#include "debug.h"

void shutdown() {
    int brand = cpuid_info(0);
    
    if (brand == BRAND_QEMU) {
        outports(0x604, 0x2000);
        return;
    }
    
    outports(0x4004, 0x3400);
}

void kbdEvHandler(char c) {
    putc(c);
}

void kmain(unsigned long magic, unsigned long addr) {
    IGNORE_UNUSED(magic);
    IGNORE_UNUSED(addr);

    MULTIBOOT_INFO *mboot_info;
    IGNORE_UNUSED(mboot_info);

    console_initialize();
    cursor_disable();

    printf("CubeBox OS! v 0.0.1 kernel! (test)\n");

    gdt_init();
    idt_init();
    
    kbd_init();
    kbd_enable();

    serial_init();

    timer_install();

    debug_setPort(COM1);
    debug_setVerbose(0);

    debug_separator("START OF THE DEBUG LOG");
    debug_message("Debug enabled.", "Kernel main", KERNEL_MESSAGE);
    debug_message("CubeBox OS! v 0.0.1 kernel! (test)", 0, KERNEL_IMPORTANT);

    cpuid_info(1);
    memory_init(mboot_info);

    shell();
}

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
    asm("cli");

    console_initialize();

    debug_separator("KERNEL PANIC");
    debug_message("Kernel panic.", "Kernel panic", KERNEL_FATAL);

    colorPrint("----- KERNEL PANIC -----\n", 128 + 4);
    printf("Kernel panic.\n");

    kernel_panic_dumpreg(reg, exception);

    puts("\n");
    puts("CPU is now halted. If debugging enabled, check debug log for more information.\n");

    puts("--------------");

    while(1) {
    }
}

 
#define STACK_CHK_GUARD 0xe2dee396
u32 __stack_chk_guard = STACK_CHK_GUARD;
 
__attribute__((noreturn))
void __stack_chk_fail(void) {
    while(1);
}

__attribute__((noreturn))
void __stack_chk_fail_local(void) {
    while(1);
}

