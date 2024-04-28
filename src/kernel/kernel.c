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

    static MULTIBOOT_INFO *mboot_info;

    console_initialize();

    printf("CubeBox OS! v 0.0.1 kernel! (test)\n");

    gdt_init();
    idt_init();
    
    kbd_init();
    kbd_enable();

    serial_init();

    pit_init();

    debug_setPort(COM1);
    debug_setVerbose(0);

    debug_separator("START OF THE DEBUG LOG");
    debug_message("Debug enabled.", "Kernel main", KERNEL_MESSAGE);
    debug_message("CubeBox OS! v 0.0.1 kernel! (test)", 0, KERNEL_IMPORTANT);

    cpuid_info(1);
    memory_init(mboot_info);

    shell();
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

