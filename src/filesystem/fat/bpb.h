// ? BPB = BIOS Parameter Block

#ifndef __BPB_H
#define __BPB_H

#include "types.h"

struct fat_bpb {
    union {
        u8  oemName_bytes[8];
        u64 oemName;
    };

    u16 bytesPerSector;
    u8  sectorsPerCluster;
    u16 reservedSectors;
    
    u8  tablesCount;
    u16 entryCount;
    u16 sectorsCount;

    u8  media;

    u16 sectorsPerTable;
    u16 sectorsPerTrack;
    u16 headsPerCylinder;

    u32 hiddenSectors;
    u32 longSectors;
};

struct fat_bpbext {
    u32 sectorsPerTable32;

    u16 flags;
    u16 version;

    u32 rootCluster;
    u16 infoCluster;

    u16 backupBoot;
    u16 reserved[6];
};

typedef struct fat_bpb  fat_bpb_t;
typedef struct fat_bpb* fat_bpbptr_t;

typedef struct fat_bpbext  fat_bpbext_t;
typedef struct fat_bpbext* fat_bpbextptr_t;

struct fat_bootsect {
    u8 ignore[3];

    struct fat_bpb      bpb;
    struct fat_bpbext   bpbext;

    u8 filler[448];
};

typedef struct fat_bootsect     bootSect_t;
typedef struct fat_bootsect*    bootSectptr_t;

void fat_readBoot(bootSectptr_t ptr);

#endif
