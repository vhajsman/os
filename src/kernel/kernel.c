#include "kernel.h"
#include "console.h"
#include "string.h"
#include "gdt/gdt.h"
#include "irq/idt.h"
#include "hid/kbd.h"
#include "ioport.h"
#include "shell/shell.h"
#include "com/serial.h"
#include "time/timer.h"
#include "memory/memory.h"
#include "debug.h"
// #include "storage/fdc.h"

/*
void mboot_print(MULTIBOOT_INFO* mboot_info) {
    u32 i;
    printf("magic: 0x%x\n", magic);
    if(magic == MULTIBOOT_BOOTLOADER_MAGIC) {
        mboot_info = (MULTIBOOT_INFO *)addr;
        printf("  flags: 0x%x\n", mboot_info->flags);
        printf("  mem_low: 0x%x KB\n", mboot_info->mem_low);
        printf("  mem_high: 0x%x KB\n", mboot_info->mem_high);
        printf("  boot_device: 0x%x\n", mboot_info->boot_device);
        printf("  cmdline: %s\n", (char *)mboot_info->cmdline);
        printf("  modules_count: %d\n", mboot_info->modules_count);
        printf("  modules_addr: 0x%x\n", mboot_info->modules_addr);
        printf("  mmap_length: %d\n", mboot_info->mmap_length);
        printf("  mmap_addr: 0x%x\n", mboot_info->mmap_addr);
        printf("  memory map:-\n");
        for (i = 0; i < mboot_info->mmap_length; i += sizeof(MULTIBOOT_MEMORY_MAP)) {
            MULTIBOOT_MEMORY_MAP *mmap = (MULTIBOOT_MEMORY_MAP *)(mboot_info->mmap_addr + i);

            printf("    size: %d, addr: 0x%x%x, len: %d%d, type: %d\n", 
                    mmap->size, mmap->addr_low, mmap->addr_high, mmap->len_low, mmap->len_high, mmap->type);

            if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                
            }
        }
        printf("  boot_loader_name: %s\n", (char *)mboot_info->boot_loader_name);
        printf("  vbe_control_info: 0x%x\n", mboot_info->vbe_control_info);
        printf("  vbe_mode_info: 0x%x\n", mboot_info->vbe_mode_info);
        printf("  framebuffer_addr: 0x%x\n", mboot_info->framebuffer_addr);
        printf("  framebuffer_width: %d\n", mboot_info->framebuffer_width);
        printf("  framebuffer_height: %d\n", mboot_info->framebuffer_height);
        printf("  framebuffer_type: %d\n", mboot_info->framebuffer_type);
    } else {
        printf("invalid multiboot magic number\n");
    }
}
*/

void shutdown() {
    int brand = cpuid_info(0);
    
    if (brand == BRAND_QEMU) {
        outports(0x604, 0x2000);
        return;
    }
    
    outports(0x4004, 0x3400);
}

#include "multiboot.h"

void kbdEvHandler(char c) {
    putc(c);
}


void kmain(unsigned long magic, unsigned long addr) {
    IGNORE_UNUSED(magic);
    IGNORE_UNUSED(addr);

    MULTIBOOT_INFO *mboot_info;
    IGNORE_UNUSED(mboot_info);

    console_initialize(/*VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK*/);
    cursor_disable();

    gdt_init();
    idt_init();
    
    kbd_init();
    kbd_enable();

    printf("CubeBox OS! v 0.0.1 kernel! (test)\n");

    serial_init();
    serial_write(COM1, "Serial communication test: COM1\n\r");
    serial_write(COM2, "Serial communication test: COM2\n\r");

    timer_install();

    debug_setPort(COM1);
    debug_setVerbose(0);

    debug_separator("START OF THE DEBUG LOG");
    debug_message("Debug enabled.", "Kernel main", KERNEL_MESSAGE);

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

