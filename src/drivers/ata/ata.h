#ifndef __ATA_H
#define __ATA_H

#include "types.h"

#define ATA_SR_BSY                  0x80    // Busy
#define ATA_SR_DRDY                 0x40    // Drive ready
#define ATA_SR_DF                   0x20    // Drive write fault
#define ATA_SR_DSC                  0x10    // Drive seek complete
#define ATA_SR_DRQ                  0x08    // Data request ready
#define ATA_SR_CORR                 0x04    // Corrected data
#define ATA_SR_IDX                  0x02    // Index
#define ATA_SR_ERR                  0x01    // Error

#define ATA_ERR_BBK                 0x80    // Bad block
#define ATA_ERR_UNC                 0x40    // Uncorrectable data
#define ATA_ERR_MC                  0x20    // Media changed
#define ATA_ERR_IDNF                0x10    // ID mark not found
#define ATA_ERR_MCR                 0x08    // Media change request
#define ATA_ERR_ABRT                0x04    // Command aborted
#define ATA_ERR_TK0NF               0x02    // Track 0 not found
#define ATA_ERR_AMNF                0x01    // No address mark

#define ATA_CMD_READ_PIO            0x20
#define ATA_CMD_READ_PIO_EXT        0x24
#define ATA_CMD_READ_DMA            0xC8
#define ATA_CMD_READ_DMA_EXT        0x25
#define ATA_CMD_WRITE_PIO           0x30
#define ATA_CMD_WRITE_PIO_EXT       0x34
#define ATA_CMD_WRITE_DMA           0xCA
#define ATA_CMD_WRITE_DMA_EXT       0x35
#define ATA_CMD_CACHE_FLUSH         0xE7
#define ATA_CMD_CACHE_FLUSH_EXT     0xEA
#define ATA_CMD_PACKET              0xA0
#define ATA_CMD_IDENTIFY_PACKET     0xA1
#define ATA_CMD_IDENTIFY            0xEC

#define ATA_IDENT_DEVICETYPE        0
#define ATA_IDENT_CYLINDERS         2
#define ATA_IDENT_HEADS             6
#define ATA_IDENT_SECTORS           12
#define ATA_IDENT_SERIAL            20
#define ATA_IDENT_MODEL             54
#define ATA_IDENT_CAPABILITIES      98
#define ATA_IDENT_FIELDVALID        106
#define ATA_IDENT_MAX_LBA           120
#define ATA_IDENT_COMMANDSETS       164
#define ATA_IDENT_MAX_LBA_EXT       200

#define ATA_REG_DATA                0x00
#define ATA_REG_ERROR               0x01
#define ATA_REG_FEATURES            0x01
#define ATA_REG_SECCOUNT0           0x02
#define ATA_REG_LBA0                0x03
#define ATA_REG_LBA1                0x04
#define ATA_REG_LBA2                0x05
#define ATA_REG_HDDEVSEL            0x06
#define ATA_REG_COMMAND             0x07
#define ATA_REG_STATUS              0x07
#define ATA_REG_SECCOUNT1           0x08
#define ATA_REG_LBA3                0x09
#define ATA_REG_LBA4                0x0A
#define ATA_REG_LBA5                0x0B
#define ATA_REG_CONTROL             0x0C
#define ATA_REG_ALTSTATUS           0x0C
#define ATA_REG_DEVADDRESS          0x0D

#define IDE_ATA                     0x00
#define IDE_ATAPI                   0x01
#define IDE_UNKNOWN                 0xAF

#define ATA_MASTER                  0x00
#define ATA_SLAVE                   0x01

#define ATA_PRIMARY                 0x00
#define ATA_SECONDARY               0x01
#define ATA_READ                    0x00
#define ATA_WRITE                   0x01

int ata_readSector(u8 drive, u32 lba, char* buffer);

#define ATAPI_CMD_READ              0xA8;

int atapi_readSector(u8 drive, u32 lba, char* buffer);

#endif
