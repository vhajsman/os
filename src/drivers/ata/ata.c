#include "ata.h"
#include "ide.h"
#include "debug.h"
#include "types.h"
#include "ioport.h"

bool checkDriverLbaSupport(u8 drive) {
    return (ide_devices[drive].capabilities & 0x0200) != 0;
}

u8 ide_ata_access(u8 direction, u8 drive, u32 lba, u8 numsects, u16 selector, u32 edi) {
    u8 lba_mode;
    u8 lba_io[6];
    u32 channel     = ide_devices[drive].channel;
    u32 slavebit    = ide_devices[drive].drive;
    u32 bus         = ide_channels[channel].base;
    u32 words       = 256;
    u16 cyl;
    u8 head, sect, err;
    u8 dma, cmd;

    ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].nIEN = (ide_irq_invoked = 0x00) + 0x02);

    if(lba > 0x10000000) {      // * LBA48
        lba_mode    = 2;
        lba_io[0]   = (lba & 0x000000FF) >> 0;
        lba_io[1]   = (lba & 0x0000FF00) >> 8;
        lba_io[2]   = (lba & 0x00FF0000) >> 16;
        lba_io[3]   = (lba & 0xFF000000) >> 24;
        lba_io[4]   = 0x00000000;
        lba_io[5]   = 0x00000000;
        head        = 0;
    } else if(checkDriverLbaSupport(drive)) {       // * LBA24
        lba_mode    = 1;
        lba_io[0]   = (lba & 0x00000FF) >> 0;
        lba_io[1]   = (lba & 0x000FF00) >> 8;
        lba_io[2]   = (lba & 0x0FF0000) >>16;
        lba_io[3]   = 0x00000000;
        lba_io[4]   = 0x00000000;
        lba_io[5]   = 0x00000000;
        head        = (lba & 0xF000000) >> 24;
    } else {        // * CHS
        lba_mode    = 0;
        sect        = (lba % 63) + 1;
        cyl         = (lba + 1 - sect) / (16 * 63);
        lba_io[0]   = sect;
        lba_io[1]   = (cyl >> 0) & 0xFF;
        lba_io[2]   = (cyl >> 8) & 0xFF;
        lba_io[3]   = 0;
        lba_io[4]   = 0;
        lba_io[5]   = 0;
        head        = (lba + 1 - sect) % (16 * 63) / 63;
    }

    dma = 0;

    // If busy, wait
    while(ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY);

    // Drive select
    ide_write(channel, ATA_REG_HDDEVSEL, (lba_mode == 0 ? 0xA0 : 0xE0) | (slavebit << 4) | head);

    if(lba_mode == 2) {
        ide_write(channel, ATA_REG_SECCOUNT1, 0);
        ide_write(channel, ATA_REG_LBA3, lba_io[3]);
        ide_write(channel, ATA_REG_LBA4, lba_io[4]);
        ide_write(channel, ATA_REG_LBA5, lba_io[5]);
    }

    ide_write(channel, ATA_REG_SECCOUNT0, numsects);
    ide_write(channel, ATA_REG_LBA0, lba_io[0]); 
    ide_write(channel, ATA_REG_LBA1, lba_io[1]);
    ide_write(channel, ATA_REG_LBA2, lba_io[2]); 

    if (lba_mode == 0 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;
    if (lba_mode == 1 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO;
    if (lba_mode == 2 && dma == 0 && direction == 0) cmd = ATA_CMD_READ_PIO_EXT;
    if (lba_mode == 0 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
    if (lba_mode == 1 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA;
    if (lba_mode == 2 && dma == 1 && direction == 0) cmd = ATA_CMD_READ_DMA_EXT;
    if (lba_mode == 0 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 1 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO;
    if (lba_mode == 2 && dma == 0 && direction == 1) cmd = ATA_CMD_WRITE_PIO_EXT;
    if (lba_mode == 0 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
    if (lba_mode == 1 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA;
    if (lba_mode == 2 && dma == 1 && direction == 1) cmd = ATA_CMD_WRITE_DMA_EXT;
    ide_write(channel, ATA_REG_COMMAND, cmd);

    if(dma) {
        // TODO
    } else {
        if(direction == 0) {
            for(int i = 0; i < numsects; i++) {
                err = ide_polling(channel, 1);
                if(err)
                    return 1;

                asm("pushw %es");
                asm("mov %%ax, %%es"::"a"(selector));
                asm("rep insw"::
                    "c"(words), 
                    "d"(bus), 
                    "D"(edi)
                );
                asm("popw %es");

                edi += (words*2); 
            }
        } else {
            for(int i = 0; i < numsects; i++) {
                ide_polling(channel, 0);
                
                asm("pushw %ds");
                asm("mov %%ax, %%ds"::"a"(selector));
                asm("rep outsw"::
                    "c"(words), 
                    "d"(bus), 
                    "S"(edi)
                );
                asm("popw %ds");

                edi += (words*2); 
            }
        }

        ide_write(channel, ATA_REG_COMMAND, (u8[]) {
            ATA_CMD_CACHE_FLUSH,
            ATA_CMD_CACHE_FLUSH,
            ATA_CMD_CACHE_FLUSH_EXT
        } [lba_mode]);

        ide_polling(channel, 0);
    }

    return 0;
}

int ata_readSector(u8 drive, u32 lba, char* buffer) {
    if(buffer == NULL)
        return 1;
    
    u8 status = ide_ata_access(ATA_READ, drive, lba, 1, 0x10, (u32) buffer);
    return status ? 1 : 0;
}

int ata_writeSector(u8 drive, u32 lba, char* buffer) {
    if(buffer == NULL)
        return 1;
    
    u8 status = ide_ata_access(ATA_WRITE, drive, lba, 1, 0x10, (u32) buffer);
    return status ? 1 : 0;
}

void ata_init() {
    
}