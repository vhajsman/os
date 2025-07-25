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
#include "hid/mouse.h"
#include "hook.h"
#include "video/fb.h"
#include "video/vga.h"
#include "time/rtc.h"
#include "device.h"
#include "util/fstab.h"

#include "random.h"
struct rng_ctx kernel_seed_ctx;

rng_seed_t kernel_seed_v = {
    0x12, 0x34, 0x56, 0x78,
    0x9A, 0xBC, 0xDE, 0xF0,
    0x01, 0x23, 0x45, 0x67,
    0x89, 0xAB, 0xCD, 0xEF,
    0xFE, 0xDC, 0xBA, 0x98,
    0x76, 0x54, 0x32, 0x10,
    0x00, 0x11, 0x22, 0x33,
    0x44, 0x55, 0x66, 0x77
};


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

int kmain_automount(void* context) {
    IGNORE_UNUSED(context);

    device_debug();

    char buffer[4096];
    int read = initrd_read("etc/fstab", buffer, 4096);

    if(read == 0) {
        debug_message("failed to read fstab", "kmain", KERNEL_ERROR);
        return 300;
    }

    debug_message("attemping automount...", "kmain", KERNEL_ERROR);
    return fstab_mount(buffer);
}

int kmain_seed(void* context) {
    IGNORE_UNUSED(context);

    #ifdef _OPT_SEED_ENTROPY_RTC
        struct rtc_time time;
        rtc_getTime(&time);

        kernel_seed_ctx.seed[0] = time.sec;
        kernel_seed_ctx.seed[1] = time.min;
        kernel_seed_ctx.seed[2] = time.hrs;
        kernel_seed_ctx.seed[3] = time.day;
        kernel_seed_ctx.seed[4] = time.mon;
        kernel_seed_ctx.seed[5] = (u8) (time.year & 0xFF);
        kernel_seed_ctx.seed[6] = (u8)((time.year >> 8) & 0xFF);
    #endif
        
    for(int i = 12; i < _RNG_SEED_BYTES; i++)
        kernel_seed_ctx.seed[i] = (u8)((kernel_seed_ctx.seed[i - 12] ^ kernel_seed_ctx.seed[i - 6]) + i);
    
    rng_init(&kernel_seed_ctx, kernel_seed_v);
        
    debug_message("RNG testing", "RNG", KERNEL_MESSAGE);
    u8 t[4];
    for(int i = 0; i < 10; i++) {
        debug_message("  --> ", "RNG", KERNEL_MESSAGE);

        rng(&kernel_seed_ctx, t, 4);

        u32 val; 
        memcpy(&val, t , 4);
        debug_number(val, 10);
    }

    return 0;
}

void kmain_setuphooks() {
    kmain_hooks.name = "startup";
    kmain_hooks.count = 0;

    hook_register(&kmain_hooks, kmain_initperipherial, "peripherial init", NULL);
    hook_register(&kmain_hooks, kmain_cpuid, "CPUID", NULL);
    hook_register(&kmain_hooks, kmain_initrd, "initrd", NULL);
    hook_register(&kmain_hooks, kmain_seed, "get seed for RNG", NULL);
}


void kmain(unsigned long magic, unsigned long addr) {
    mboot_info = (MULTIBOOT_INFO*) addr;

    asm volatile("cli");

    console_initialize();
    kmain_init_cursor();
    gdt_init();
    idt_init();

    asm volatile("sti");

    pit_init(1000); 

    // DO NOT initialize debugging on your own if not insiders build
    #ifdef _BUILD_INSIDERS
    serial_init();

    debug_setPort(COM1);
    debug_setVerbose(0);

    debug_separator("START OF THE DEBUG LOG");
    debug_message("Debug enabled.", "Kernel main", KERNEL_MESSAGE);
    debug_message("CubeBox OS! v 0.0.1 kernel!", 0, KERNEL_IMPORTANT);
    #endif

    #ifdef _BUILD_INSIDERS
        debug_message("insiders build - testing-purpose functionality included.", 0, KERNEL_MESSAGE);
    #endif

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

    ide_init(0x1f0, 0x3f6, 0x170, 0x376, 0x000);
    //floppy_init();

    putc('\n');
    printf("CubeBox OS! v 0.0.1 kernel! (test)\n");

    #ifdef _BUILD_INSIDERS
        printf("insiders build - testing-purpose functionality included.\n");
        printf("Insiders build functions could possibly put your computer on security risk! Test purposes only!\n");
    #endif

    //ahci_init();
    mouse_init();
    
    kmain_automount(NULL);
    fs_mounts_debug();

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
