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

#define _kmain_debug_multiboot(var, nbase)              \
    debug_message(#var, "multiboot", KERNEL_MESSAGE);   \
    debug_append(" = ");                                \
    debug_number(mbinfo->var, 10);

#define _kmain_debug_multiboot_module(var, nbase)           \
    debug_message("  --> ", "multiboot", KERNEL_MESSAGE);   \
    debug_append(#var);                                     \
    debug_append(" = ");                                    \
    debug_number(_mod->var, nbase);

void kernel_debug_multiboot_module(MULTIBOOT_INFO* mbinfo) {
    multiboot_module_t* _mod = (multiboot_module_t*) mbinfo->modules_addr;

    _kmain_debug_multiboot_module(mod_start, 16);
    _kmain_debug_multiboot_module(mod_end, 16);
    _kmain_debug_multiboot_module(string, 16);
    _kmain_debug_multiboot_module(reserved, 16);
}

void kmain_debug_multiboot(MULTIBOOT_INFO* mbinfo) {
    _kmain_debug_multiboot(flags, 16);
    _kmain_debug_multiboot(mem_low, 10);
    _kmain_debug_multiboot(mem_high, 10);
    _kmain_debug_multiboot(boot_device, 16);
    _kmain_debug_multiboot(cmdline, 16);
    _kmain_debug_multiboot(modules_count, 10);
    _kmain_debug_multiboot(modules_addr, 16);

    kernel_debug_multiboot_module(mbinfo);
    
    _kmain_debug_multiboot(mmap_length, 10);
    _kmain_debug_multiboot(mmap_addr, 16);
    _kmain_debug_multiboot(drives_length, 10);
    _kmain_debug_multiboot(drives_addr, 16);
    _kmain_debug_multiboot(config_table, 16);
    _kmain_debug_multiboot(boot_loader_name, 16);
    _kmain_debug_multiboot(apm_table, 16);
    _kmain_debug_multiboot(vbe_control_info, 16);
    _kmain_debug_multiboot(vbe_mode_info, 16);
    _kmain_debug_multiboot(vbe_mode, 16);
    _kmain_debug_multiboot(vbe_interface_seg, 16);
    _kmain_debug_multiboot(vbe_interface_off, 16);
    _kmain_debug_multiboot(vbe_interface_len, 10);
    _kmain_debug_multiboot(framebuffer_addr, 16);
    _kmain_debug_multiboot(framebuffer_pitch, 10);
    _kmain_debug_multiboot(framebuffer_width, 10);
    _kmain_debug_multiboot(framebuffer_height, 10);
    _kmain_debug_multiboot(framebuffer_bpp, 16);
    _kmain_debug_multiboot(framebuffer_type, 16);
}


void kmain(unsigned long magic, unsigned long addr) {
    IGNORE_UNUSED(magic);
    IGNORE_UNUSED(addr);

    MULTIBOOT_INFO* mboot_info = (MULTIBOOT_INFO*) addr;

    console_initialize();
    kmain_init_cursor();

    printf("CubeBox OS! v 0.0.1 kernel! (test)\n");
    putc('\n');

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

    kmain_debug_multiboot(mboot_info);
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        puts("Invalid magic number.\n");
        debug_message("Invalid magic number (not multiboot 1?)", "kmain", KERNEL_ERROR);

        debug_message("Magic number: ", "kmain", KERNEL_MESSAGE);
        debug_number(magic, 16);

        return;
    }

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
