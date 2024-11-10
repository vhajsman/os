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
#include "ata/ata.h"
#include "initrd.h"
#include "modules/interface.h"
#include "fs/fs.h"
#include "fs/tar.h"
#include "ahci.h"
#include "hid/mouse.h"
#include "hook.h"
#include "video/fb.h"
#include "video/vga.h"
#include "time/rtc.h"
#include "floppy.h"

MULTIBOOT_INFO* mboot_info;

struct kernel_hook_list kmain_hooks;

struct kernel_interface _kmain_rootfs;
struct kernel_interface_callflags c;

fs_node_t* kmain_rootfs_mount(const char* tar_data);
void kernel_rootfs_unmount(struct kernel_interface_callflags* callflags);
void kernel_rootfs_init(struct kernel_interface_callflags* callflags);


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


int kmain_initperipherial(void* context) {
    IGNORE_UNUSED(context);

    kbd_init();
    kbd_enable();

    rtc_init();

    return 0;
}

int kmain_cpuid(void* context) {
    IGNORE_UNUSED(context);

    cpuid_info(0);

    return 0;
}

int kmain_initrd(void* context) {
    IGNORE_UNUSED(context);

    _kmain_rootfs.name_short = "rootfs";
    _kmain_rootfs.name_friendly = "root filesystem";
    _kmain_rootfs.init = &kernel_rootfs_init;
    _kmain_rootfs.deinit = &kernel_rootfs_unmount;
    c.flags = 0;

    kernel_interface_load(-1, &_kmain_rootfs);
    kernel_interface_init(kernel_interface_findByName("rootfs"), &c);

    initrd_load(mboot_info);

    puts("\n");

    return 0;
}

void kmain_setuphooks() {
    kmain_hooks.name = "startup";
    kmain_hooks.count = 0;

    hook_register(&kmain_hooks, kmain_initperipherial, "peripherial init", NULL);
    hook_register(&kmain_hooks, kmain_cpuid, "CPUID", NULL);
    hook_register(&kmain_hooks, kmain_initrd, "initrd", NULL);
}


void kmain(unsigned long magic, unsigned long addr) {
    mboot_info = (MULTIBOOT_INFO*) addr;

    console_initialize();
    kmain_init_cursor();
    gdt_init();
    idt_init();

    pit_init(1000); 
    serial_init();

    debug_setPort(COM1);
    debug_setVerbose(0);

    debug_separator("START OF THE DEBUG LOG");
    debug_message("Debug enabled.", "Kernel main", KERNEL_MESSAGE);
    debug_message("CubeBox OS! v 0.0.1 kernel! (test)", 0, KERNEL_IMPORTANT);

    memory_init(mboot_info);

    kmain_setuphooks();
    hook_call(&kmain_hooks);

    kmain_debug_multiboot(mboot_info);
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        puts("Invalid magic number.\n");
        debug_message("Invalid magic number (not multiboot 1?)", "kmain", KERNEL_ERROR);

        debug_message("Magic number: ", "kmain", KERNEL_MESSAGE);
        debug_number(magic, 16);

        return;
    }

    pci_init();

    ide_init(0x1f0, 0x3f6, 0x170, 0x376, 0xf0);
    //floppy_init();

    putc('\n');
    printf("CubeBox OS! v 0.0.1 kernel! (test)\n");

    //ahci_init();
    mouse_init();

    //framebuffer_init();

    shell();
}

fs_node_t* kmain_rootfs_mount(const char* tar_data) {
    debug_message("Tarball data pointer: ", "kernel", KERNEL_MESSAGE);
    debug_number((u32) tar_data, 16);

    fs_root = (fs_node_t*) malloc(sizeof(fs_node_t));
    memset(fs_root, 0, sizeof(fs_node_t));

    strcpy(fs_root->name, "rootfs");
    fs_root->flags = FS_DIRECTORY;
    fs_root->readdir = tar_readdir;
    fs_root->finddir = tar_finddir;
    fs_root->inode = 0;
    fs_root->length = 0;

    fs_root->impl = (u32) tar_data;

    return fs_root;
}

void kernel_rootfs_init(struct kernel_interface_callflags* callflags) {
    fs_node_t* _rootfs_dist = kmain_rootfs_mount((const char*) initrd_data);
    callflags->exit_code = _rootfs_dist == NULL ? 3 : 0;
}

void kernel_rootfs_unmount(struct kernel_interface_callflags* callflags) {
    callflags->exit_code = 0;
}
