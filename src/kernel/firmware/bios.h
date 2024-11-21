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
    bios_byte  jump[3];
    bios_byte  oem[8];
    bios_word  sectorSize;
    bios_byte  sectorsPerCluster;
    bios_word  reserved;
    bios_byte  tableCount;
    bios_word  entryCount;
    bios_word  sectorsPerVolume;
    bios_byte  mediaDescriptorType;
    bios_word  sectorsPerTable;
    bios_word  sectorsPerTrack;
    bios_word  headsCount;
    bios_dword hiddenCount;
    bios_dword largeCount;
};



#endif
