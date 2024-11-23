#ifndef __BIOS_H
#define __BIOS_H

#include "types.h"

typedef u8  bios_byte;
typedef u16 bios_word;
typedef u32 bios_dword;

#define BIOS_BPB_JUMP       0x00
#define BIOS_BPB_OEM        0x03
#define BIOS_BPB_BPS        0x0B
#define BIOS_BPB_SPC        0x0D
#define BIOS_BPB_RESERVERD  0x0E
#define BIOS_BPB_FAT_COUNT  0x10
#define BIOS_BPB_ENTRIES    0x11
#define BIOS_BPB_SECTORS    0x13
#define BIOS_BPB_MDT        0x15 // Media descriptor type
#define BIOS_BPB_SPFAT      0x16
#define BIOS_BPB_SPT        0x18
#define BIOS_BPB_HEADS      0x1A
#define BIOS_BPB_HIDDEN     0x1C
#define BIOS_BPB_LARGESECT  0x20

struct bios_bpb {
    bios_word   bytes_per_sector;
    bios_byte   sectors_per_cluster;
    bios_word   reserved_sectors;
    bios_byte   fat_count;
    bios_word   root_entry_count;
    bios_word   total_sectors_16;
    bios_byte   media_type;
    bios_word   fat_size_16;
    bios_word   sectors_per_track;
    bios_word   num_heads;
    bios_dword  hidden_sectors;
    bios_dword  total_sectors_32;

    // FAT32
    bios_dword  fat_size_32;
    bios_word   ext_flags;
    bios_word   fs_version;
    bios_dword  root_cluster;
    bios_word   fs_info;
    bios_word   backup_boot_sector;
    bios_byte   reserved[12];
} __attribute__((packed));

#endif
