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
#include "pci.h"
#include "ata/ide.h"
#include "initrd.h"

bool _kernel_exitOk = 0;

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

void kmain_init_cursor() {
    console_cursor_enable(13, 15);
    console_cursor_move(0, 0);

    console_cursor_hide();
}

void kmain(unsigned long magic, unsigned long addr) {
    IGNORE_UNUSED(magic);
    IGNORE_UNUSED(addr);

    MULTIBOOT_INFO* mboot_info = (MULTIBOOT_INFO*) addr;

    console_initialize();
    kmain_init_cursor();

    printf("CubeBox OS! v 0.0.1 kernel! (test)\n");

    gdt_init();
    idt_init();
    
    pit_init(1000);
    
    kbd_init();
    kbd_enable();

    serial_init();


    debug_setPort(COM1);
    debug_setVerbose(0);

    debug_separator("START OF THE DEBUG LOG");
    debug_message("Debug enabled.", "Kernel main", KERNEL_MESSAGE);
    debug_message("CubeBox OS! v 0.0.1 kernel! (test)", 0, KERNEL_IMPORTANT);

    cpuid_info(0);
    memory_init(mboot_info);

    initrd_load(mboot_info);

    pci_init();

    ide_init(0x1f0, 0x3f6, 0x170, 0x376, 0xf0);

    shell();
}

extern char* _levelsStrings[10];

u8 _levelColors[10] = {
    7, 7, 5, 14, 4, 4, 8, 10, 4, 1
};

void kout(enum kernel_statusLevels lvl, char* interface, char* message, char* query) {
    // --- APPEND MESSAGE TO DEBUG LOG ---
    debug_message(message, interface, lvl);

    if(query != NULL) {
        debug_append(" ");
        debug_append(query);
    }

    // --- PRINT MESSAGE ON SCREEN ---
    
    if(message == NULL) // ? Nothing to output to screen
        return;

    u8 color = _levelColors[lvl];
    colorPrint(_levelsStrings[lvl], color);
    putc(' ');

    if(interface != NULL) {
        putc('@');
        colorPrint(interface, color);
        putc(':');
        putc(' ');
    }

    colorPrint(message, color);
    
    if(query != NULL) {
        putc(' ');
        colorPrint(query, 15);
    }

    putc('\n');
}

void kernel_exit() {
    asm("cli");

    if(_kernel_exitOk)
        return;

    kout(KERNEL_FATAL, "kernel", "Kernel exited unexceptedly (crashed), call panic.", NULL);
    kernel_panic((REGISTERS*) 0, 6);
}
