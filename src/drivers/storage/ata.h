#ifndef __ATA_H
#define __ATA_H

#include "kernel/kernel.h"
#include "memory/paging.h"
#include "filesystem/vfs.h"

extern page_directory_t* kpage_dir;

typedef struct prdt {
	u32 buffer_phys;
	u16 transfer_size;
	u16 mark_end;
}__attribute__((packed)) prdt_t;

typedef struct ata_dev {
	// A list of register address
	u16 data;
	u16 error;
	u16 sector_count;

	union {
		u16 sector_num;
		u16 lba_lo ;
	};
	union {
		u16 cylinder_low;
		u16 lba_mid ;
	};
	union {
		u16 cylinder_high;
		u16 lba_high;
	};
	union {
		u16 drive;
		u16 head;
	};
	union {
		u16 command;
		u16 status;
	};
	union {
		u16 control;
		u16 alt_status;
	};

	int slave;
	u32 bar4;
	u32 BMR_COMMAND;
	u32 BMR_prdt;
	u32 BMR_STATUS;


	prdt_t * prdt;
	u8 * prdt_phys;

	u8 * mem_buffer;
	u8 * mem_buffer_phys;

	char mountpoint[32];
}__attribute__((packed)) ata_dev_t;


// ATA PCI info
#define ATA_VENDOR_ID 0x8086
#define ATA_DEVICE_ID 0x7010

// Control reg
#define CONTROL_STOP_INTERRUPT 0x2
#define CONTROL_SOFTWARE_RESET 0x4
#define CONTROL_HIGH_ORDER_BYTE 0x80
#define CONTROL_ZERO 0x00


// Command reg
#define COMMAND_IDENTIFY 0xEC
#define COMMAND_DMA_READ 0xC8
#define ATA_CMD_READ_PIO 0x20

// Status reg
#define STATUS_ERR 0x0
#define STATUS_DRQ 0x8
#define STATUS_SRV 0x10
#define STATUS_DF  0x20
#define STATUS_RDY 0x40
#define STATUS_BSY 0x80

// Bus Master Reg Command
#define BMR_COMMAND_DMA_START 0x1
#define BMR_COMMAND_DMA_STOP 0x0
#define BMR_COMMAND_READ 0x8
#define BMR_STATUS_INT 0x4
#define BMR_STATUS_ERR 0x2


// Prdt
#define SECTOR_SIZE 512
#define MARK_END 0x8000

typedef u32 lba_t;

#define __drive(device, lba) \
    (0xE8 | device->slave << 4 | (lba & 0x0f000000) >> 24)

//void io_wait(ata_dev_t* device);
void ata_reset(ata_dev_t * dev);

void ata_open(vfs_node_t* node, u32 flags);
void ata_close(vfs_node_t* node);

u32 ata_read(vfs_node_t* node, u32 offset, u32 size, char* buffer);
u32 ata_write(vfs_node_t* node, u32 offset, u32 size, char* buffer);
char* ata_readSector(ata_dev_t* device, lba_t lba);
char* ata_writeSector(ata_dev_t* device, lba_t lba, char* buffer);

vfs_node_t* ata_createDevice(ata_dev_t* device);
void ata_initDevice(ata_dev_t* device, int primary);
void ata_detect(ata_dev_t* device, int primary);

void ata_init();

#endif
