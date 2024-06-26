#ifndef __FLOPPY_H
#define __FLOPPY_H

#include "types.h"

#define FLOPPY_SECTOR_DTL_128   0
#define FLOPPY_SECTOR_DTL_256   1
#define FLOPPY_SECTOR_DTL_512   2
#define FLOPPY_SECTOR_DTL_1024  3


// =========================================================
// ===== I/O MAPPING
// =========================================================

#define FLOPPY_DOR  0x3F2
#define FLOPPY_MSR  0x3F4
#define FLOPPY_FIFO 0x3F5
#define FLOPPY_CCR  0x3F7

#define FLOPPY_DOR_MASK_DRIVE0          0
#define FLOPPY_DOR_MASK_DRIVE1          1
#define FLOPPY_DOR_MASK_DRIVE2          2
#define FLOPPY_DOR_MASK_DRIVE3          3
#define FLOPPY_DOR_MASK_RESET           4
#define FLOPPY_DOR_MASK_DMA             8
#define FLOPPY_DOR_MASK_MOTOR0          16
#define FLOPPY_DOR_MASK_MOTOR1          32
#define FLOPPY_DOR_MASK_MOTOR2          64
#define FLOPPY_DOR_MASK_MOTOR3          128

#define FLOPPY_MSR_MASK_DRIVE0_POS_MODE 1
#define FLOPPY_MSR_MASK_DRIVE1_POS_MODE 2
#define FLOPPY_MSR_MASK_DRIVE2_POS_MODE 4
#define FLOPPY_MSR_MASK_DRIVE3_POS_MODE 8
#define FLOPPY_MSR_MASK_BUSY            16
#define FLOPPY_MSR_MASK_DMA             32
#define FLOPPY_MSR_MASK_DATAIO          64
#define FLOPPY_MSR_MASK_DATAREG         128


// =========================================================
// ===== COMMANDS
// =========================================================

#define FLOPPY_COMMAND_READ_TRACK       0x2
#define FLOPPY_COMMAND_SPECIFY          0x3
#define FLOPPY_COMMAND_CHECK_STATUS     0x4
#define FLOPPY_COMMAND_WRITE_SECTOR     0x5
#define FLOPPY_COMMAND_READ_SECTOR      0x6
#define FLOPPY_COMMAND_CALIBRATE        0x7
#define FLOPPY_COMMAND_CHECK_INTERRUPT  0x8
#define FLOPPY_COMMAND_WRITE_DEL_SECTOR 0x9
#define FLOPPY_COMMAND_READ_IDENTIFY    0xA
#define FLOPPY_COMMAND_READ_DEL_SECTOR  0xC
#define FLOPPY_COMMAND_FORMAT_TRACK     0xD
#define FLOPPY_COMMAND_SEEK             0xF

// ----- EXTENDED COMMAND BITS -----
#define FLOPPY_COMMAND_EXTENDED_SKIP        0x20
#define FLOPPY_COMMAND_EXTENDED_DENSITY     0x40
#define FLOPPY_COMMAND_EXTENDED_MULTITRACK  0x80


// =========================================================
// ===== GAP 3
// =========================================================

#define FLOPPY_GAP3_LENGTH_STD          42
#define FLOPPY_GAP3_LENGTH_5_14         32
#define FLOPPY_GAP3_LENGTH_3_5          27


// =========================================================
// ===== DRIVER FUNCTIONS
// =========================================================

//void floppy_reset();

// Intializes the floppy disk controller
void floppy_init();

void floppy_disable();
void floppy_enable();
void floppy_reset();

// void floppy_motor(int enable);

void floppy_readSector(u8 head, u8 track, u8 sector);

u8 floppy_readData();

int floppy_busy();

#endif
