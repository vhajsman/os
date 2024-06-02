#include "ata.h"
#include "pci.h"
#include "memory/memory.h"
#include "ioport.h"
#include "fs.h"
#include "irq/isr.h"
#include "irq/irqdef.h"

void ata_setLba(ata_dev_t* device, lba_t lba); // helper function

pci_dev_t ata_device;

ata_dev_t primary_master =   {.slave = 0};
ata_dev_t primary_slave =    {.slave = 1};
ata_dev_t secondary_master = {.slave = 0};
ata_dev_t secondary_slave =  {.slave = 1};

void io_wait(ata_dev_t* device) {
    inportb(device->alt_status);
    inportb(device->alt_status);
    inportb(device->alt_status);
    inportb(device->alt_status);
}

void ata_reset(ata_dev_t * dev) {
    /*
    * From osdev wiki:
    *
    * "For non-ATAPI drives, the only method a driver has of resetting a drive after a major error is to do a "software reset" on the bus.
    * Set bit 2 (SRST, value = 4) in the proper Control Register for the bus. This will reset both ATA devices on the bus."
    */

    outportb(dev->control, CONTROL_SOFTWARE_RESET);
    io_wait(dev);
    outportb(dev->control, CONTROL_ZERO);
}

void ata_irq(REGISTERS* regs) {
    inportb(primary_master.status);
    inportb(primary_master.BMR_STATUS);

    outportb(primary_master.BMR_COMMAND, BMR_COMMAND_DMA_STOP);
}

void ata_open(vfs_node_t* node, u32 flags) {return;}
void ata_close(vfs_node_t* node) {return;}


char* ata_readSector(ata_dev_t* device, lba_t lba) {
    char* buffer = kmalloc(SECTOR_SIZE);

    outportb(device->BMR_COMMAND,   0);                         // Reset master bus command register
    outportl(device->BMR_prdt,      (u32) device->prdt_phys);   // Set PRDT
    outportb(device->drive,         __drive(device, lba));      // Select drive
    outportb(device->sector_count,  1);                         // Set sector count

    ata_setLba(device, lba);

    outportb(device->command, COMMAND_DMA_READ);
    outportb(device->BMR_COMMAND, 0x8 | 0x1);


    while(1) {
        u32 status = inportb(device->BMR_STATUS);
        u32 dstatus = inportb(device->status);

        if(!(status & 0x04))
            continue;

        if(!(dstatus & 0x80))
            break;
    }

    memcpy(buffer, device->mem_buffer, SECTOR_SIZE);
    return buffer;
}

u32 ata_read(vfs_node_t* node, u32 offset, u32 size, char* buffer) {
    u32 start = offset / SECTOR_SIZE;
    u32 start_offset = offset % SECTOR_SIZE;

    u32 end = (offset + size - 1) / SECTOR_SIZE;
    u32 end_offset = (offset + size - 1) % SECTOR_SIZE;

    char* buffer_curr = buffer;
    u32 counter = start;
    u32 off, total = 0;
    u32 readSize;

    while(counter <= end) {
        readSize = SECTOR_SIZE;
        off = 0;

        char* ret = ata_readSector((ata_dev_t*) node->device, counter);

        if(counter == start) {
            off = start_offset;
            readSize = SECTOR_SIZE - off;
        }

        if(counter == end) {
            readSize = end_offset - off + 1;
        }

        memcpy(buffer_curr, ret + off, readSize);

        buffer_curr =+ readSize;
        total =+ readSize;
        counter ++;
    }

    return total;
}

char* ata_writeSector(ata_dev_t* device, lba_t lba, char* buffer) {
    memcpy(device->mem_buffer, buffer, SECTOR_SIZE);

    outportb(device->BMR_COMMAND,   0);                         // Reset master bus command register
    outportl(device->BMR_prdt,      (u32) device->prdt_phys);   // Set PRDT
    outportb(device->drive,         __drive(device, lba));      // Select drive
    outportb(device->sector_count,  1);                         // Set sector count

    ata_setLba(device, lba);

    outportb(device->command, 0xCA);
    outportb(device->BMR_COMMAND, 0x1);

    while(1) {
        u32 status = inportb(device->BMR_STATUS);
        u32 dstatus = inportb(device->status);

        if(!(status & 0x04))
            continue;
        
        if(!(dstatus & 0x80))
            break;
    }
}

u32 ata_write(vfs_node_t* node, u32 offset, u32 size, char* buffer) {
    u32 start = offset / SECTOR_SIZE;
    u32 start_offset = offset % SECTOR_SIZE;

    u32 end = (offset + size - 1) / SECTOR_SIZE;
    u32 end_offset = (offset + size - 1) % SECTOR_SIZE;

    char* buffer_curr = buffer;
    u32 counter = start;
    u32 off, total = 0;
    u32 writeSize;

    while(counter <= end) {
        writeSize = SECTOR_SIZE;
        off = 0;

        char* ret = ata_readSector((ata_dev_t*) node->device, counter);

        if(counter == start) {
            off = start_offset;
            writeSize = SECTOR_SIZE - off;
        }

        if(counter == end) {
            writeSize = end_offset - off + 1;
        }

        memcpy(ret + off, buffer_curr, writeSize);

        ata_writeSector((ata_dev_t*) node->device, counter, ret);

        buffer_curr =+ writeSize;
        total =+ writeSize;

        counter++;
    }

    return total;
}

vfs_node_t* ata_createDevice(ata_dev_t* device) {
    vfs_node_t* t = kcalloc(sizeof(vfs_node_t), 1);
    strcpy(t->name, "ATA device");

    t->name[strlen(t->name)] = device->mountpoint[strlen(device->mountpoint) - 1];

    t->device = device;
    t->flags = FS_BLOCKDEVICE;
    t->read = ata_read;
    t->write = ata_write;
    t->open = ata_open;
    t->close = ata_close;

    return t;
}

void ata_initDevice(ata_dev_t* device, int primary) {
    // ----- Setup DMA -----
    // ! Prdt must not cross 64kb boundary / contiguous in physical memory.

    device->prdt = (void*) kmalloc_align(sizeof(prdt_t));
    memset(device->prdt, 0, sizeof(prdt_t));

    device->prdt_phys = virtual2phys(kpage_dir, device->prdt);
}

void ata_detect(ata_dev_t* device, int primary) {
    ata_initDevice(device, primary);

    ata_reset(device);
    io_wait(device);

    outportb(device->drive, (0xA + device->slave) << 4);
    
    outportb(device->sector_count, 0);
    outportb(device->lba_lo, 0);
    outportb(device->lba_mid, 0);
    outportb(device->lba_high, 0);

    outportb(device->command, COMMAND_IDENTIFY);
    if(!inportb(device->status)) {
        debug_message("Device not found", "ATA", KERNEL_MESSAGE);
        return;
    }

    u8 lba_lo = inportb(device->lba_lo);
    u8 lba_hi = inportb(device->lba_high);
    if(lba_lo != 0 || lba_hi != 0) {
        debug_message("Not ATA device", "ATA", KERNEL_ERROR);
        return;
    }


    u8 drq = 0, err = 0;
    while(!drq && !err) {
        drq = inportb(device->status) & STATUS_DRQ;
        err = inportb(device->status) & STATUS_ERR;
    }

    if(err) {
        debug_message("Pooling error", "ATA", KERNEL_ERROR);
        return;
    }


    for(int i = 0; i < 256; i++) 
        inports(device->data);

    u32 pci_command_reg = pci_read(ata_device, PCI_COMMAND);
    if(!(pci_command_reg & (1 << 2))) {
        pci_command_reg |= (1 << 2);
        pci_write(ata_device, PCI_COMMAND, pci_command_reg);
    }


    debug_message("ATA device detected OK, mount...", "ATA", KERNEL_OK);
    vfs_mount(device->mountpoint, ata_createDevice(device));
}

void ata_init() {
    ata_device = pci_getDevice(ATA_VENDOR_ID, ATA_DEVICE_ID, -1);

    isr_registerInterruptHandler(IRQ14_HARD_DISK, ata_irq);

    ata_detect(&primary_master,   1);
    ata_detect(&primary_slave,    1);
    ata_detect(&secondary_master, 0);
    ata_detect(&secondary_slave,  0);

    debug_message("init ok", "ATA", KERNEL_OK);
}










void ata_setLba(ata_dev_t* device, lba_t lba) {
    outportb(device->lba_lo,     lba & 0x000000ff);
    outportb(device->lba_mid,   (lba & 0x0000ff00) >> 8);
    outportb(device->lba_high,  (lba & 0x00ff0000) >> 16);
}
