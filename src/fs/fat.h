#include "fs.h"
#include "device.h"
#include "fs.h"

#define FAT_ATTR_READONLY       (u8) 0x01
#define FAT_ATTR_HIDDEN         (u8) 0x02
#define FAT_ATTR_SYSTEM         (u8) 0x04
#define FAT_ATTR_VOLUME_ID      (u8) 0x08
#define FAT_ATTR_DIRECTORY      (u8) 0x10
#define FAT_ATTR_ARCHIVE        (u8) 0x20

#define FAT_ATTR_LONG_NAME      \
    (FAT_ATTR_READONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOLUME_ID)

// FAT32 BIOS Parameter Block (BPB)
typedef struct __attribute__((packed)) {
                                // size description
                                // -----------------------------------
    u8      jump_boot[3];       // 3    jump over disk format information
    char    oem_name[8];        // 8    OEM identifier
    u16     bps;                // 2    bytes per sector
    u8      spc;                // 1    sectors per cluster
    u16     sectors_reserved;   // 2    reserved sector count
    u8      num_fats;           // 1    FAT count
    u16     rootentry_count;    // 2    0 for FAT32
    u16     sectors_total_16;   // 2    total sector count
    u8      media;              // 1    media descriptor type       https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system#BPB20_OFS_0Ah
    u16     fat_size_16;        // 2    0 for FAT32
    u16     spt;                // 2    sectors per track
    u16     num_heads;          // 2    storage media heads count
    u32     sectors_hidden;     // 4    hidden sector count
    u32     sectors_total_32;   // 4    large sector count
    // ----- FAT 32 -----       // ----- FAT 32 -----
    u32     fat_size_32;        // 4    sectors per FAT
    u16     ext_flags;          // 2    flags
    u16     fs_version;         // 2    FAT version number
    u32     root_cluster;       // 4    usually 2
    u16     fs_info;            // 2    sector number of FSInfo structure
    u16     bootsect_backup;    // 2    sector number of backup boot sector
    u8      reserved[12];       // 12   reserverd, all 0 if formated
    u8      drive_number;       // 1    drive number
    u8      reserved1;          // 1    reserved
    u32     volume_id;          // 4    volume serial number
    char    volume_label[12];   // 12   volume label
    char    fs_type[8];         // 8    always "FAT32  "
    char    boot_code[420];     // 420  boot code
    u16     boot_signature;     // 2    boot partition signature
} fat32_bpb_t;

// FAT32 BIOS FSInfo structure
typedef struct __attribute__((packed)) {
                                // size description
                                // -----------------------------------
    u32     signature_lead;     // 4    lead signature (0x41615252 for a valid FSInfo structure)
    char    reserved[480];      // 480  reserved. never used.
    u32     signature;          // 4    another signature (0x61417272)
    u32     cluster_free;       // 4    last known free cluster count (0xFFFFFFFF if unknown)
    u32     cluster_scan;       // 4    indicates first cluster for scan (0xFFFFFFFF if no hint)
    u8      reserved1[12];      // 12   reserved. never used.
    u32     signature_trail;    // 4    trail signature (0xAA550000)
} fat32_fsinfo_t;

typedef struct {
    u16 bps;                // bytes per sector
    u8  spc;                // sector per cluster
    u16 reserved_sectors;   // reserved sector count
    u8  num_fats;           // FAT count
    u32 fat_size;           // FAT size
    u32 fat_start;          // FAT start sector
    u32 heap_start;         // cluster heap start sector
    u32 root;               // root cluster

    device_t* dev;
} fat32_kernel_info_t;

typedef struct {
    fat32_fsinfo_t* fsinfo;
    u32 cluster_start;
} fat32_file_t;

typedef struct __attribute__((packed)) {
    char name[11];
    u8 attr;
    u8 reserved;
    u8 ctime_tenths;
    u16 ctime;
    u16 cdate;
    u16 adate;
    u16 first_cluster_high;
    u16 mtime;
    u16 mdate;
    u16 first_cluster_low;
    u32 filesize;
} fat32_dirent_t;

