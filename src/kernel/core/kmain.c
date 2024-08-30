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
#include "modules/interface.h"

void kmain_init_cursor() {
    console_cursor_enable(13, 15);
    console_cursor_move(0, 0);

    console_cursor_hide();
}

#define _kmain_debug_multiboot(var, nbase)              \
    debug_message(#var, "multiboot", KERNEL_MESSAGE);   \
    debug_append(" = ");                                \
    debug_number(mbinfo->var, 10);

void kmain_debug_multiboot(MULTIBOOT_INFO* mbinfo) {
    _kmain_debug_multiboot(flags, 16);
    _kmain_debug_multiboot(mem_low, 10);
    _kmain_debug_multiboot(mem_high, 10);
    _kmain_debug_multiboot(boot_device, 16);
    _kmain_debug_multiboot(cmdline, 16);
    _kmain_debug_multiboot(modules_count, 10);
    _kmain_debug_multiboot(modules_addr, 16);
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
    MULTIBOOT_INFO* mboot_info = (MULTIBOOT_INFO*) addr;

    console_initialize();
    kmain_init_cursor();

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

    printf("CubeBox OS! v 0.0.1 kernel! (test)\n");

    interface_test();

    shell();
}