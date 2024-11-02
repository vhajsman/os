#ifndef __MULTIBOOT_H
#define __MULTIBOOT_H

#include "types.h"

#define MULTIBOOT_MAGIC_HEADER      0x1BADB002
#define MULTIBOOT_BOOTLOADER_MAGIC  0x2BADB002

typedef struct {
    u32 magic;
    u32 flags;
    u32 checksum;
    u32 header_addr;
    u32 load_addr;
    u32 load_end_addr;
    u32 bss_end_addr;
    u32 entry_addr;
} MULTIBOOT_HEADER;

typedef struct {
    u32 tabsize;
    u32 strsize;
    u32 addr;
    u32 reserved;
} AOUT_SYMBOL_TABLE;

typedef struct {
    u32 num;
    u32 size;
    u32 addr;
    u32 shndx;
} ELF_SECTION_HEADER_TABLE;

typedef struct {
    u32 flags;

    // * available low-high memory from BIOS, present if flags[0] is set(MEMINFO in entry.asm)
    u32 mem_low;
    u32 mem_high;

    // * "root" partition, present if flags[1] is set(BOOTDEVICE in entry.asm)
    u32 boot_device;

    u32 cmdline;

    u32 modules_count;
    u32 modules_addr;

    union {
        AOUT_SYMBOL_TABLE aout_sym;
        ELF_SECTION_HEADER_TABLE elf_sec;
    } u;

    u32 mmap_length;
    u32 mmap_addr;

    u32 drives_length;
    u32 drives_addr;

    // * ROM configuration table, present if flags[8] is set(CONFIGT in entry.asm)
    u32 config_table;

    u32 boot_loader_name;

    u32 apm_table;

    u32 vbe_control_info;
    u32 vbe_mode_info;
    u16 vbe_mode;
    u16 vbe_interface_seg;
    u16 vbe_interface_off;
    u16 vbe_interface_len;

    u64 framebuffer_addr;
    u32 framebuffer_pitch;
    u32 framebuffer_width;
    u32 framebuffer_height;
    u8 framebuffer_bpp;
    u8 framebuffer_type;  // ? indexed = 0, RGB = 1, EGA = 2

} MULTIBOOT_INFO;


typedef enum {
    MULTIBOOT_MEMORY_AVAILABLE = 1,
    MULTIBOOT_MEMORY_RESERVED,
    MULTIBOOT_MEMORY_ACPI_RECLAIMABLE,
    MULTIBOOT_MEMORY_NVS,
    MULTIBOOT_MEMORY_BADRAM
} MULTIBOOT_MEMORY_TYPE;

typedef struct {
    u32 size;
    u32 addr_low;
    u32 addr_high;
    u32 len_low;
    u32 len_high;
    MULTIBOOT_MEMORY_TYPE type;
} MULTIBOOT_MEMORY_MAP;

typedef struct multiboot_module {
    u32 mod_start;
    u32 mod_end;
    u32 string;
    u32 reserved;
} multiboot_module_t;

#ifdef __mboot_info__needed_explicite
// ? mboot_info is defined in kmain.c
extern MULTIBOOT_INFO* mboot_info;
#endif

#endif
