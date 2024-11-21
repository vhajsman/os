#include "ata.h"
#include "ide.h"

u8 ide_atapi_read(u8 drive, u32 lba, u8 numsects, u16 selector, u32 edi) {
    u32 channel     = ide_devices[drive].channel;
    u32 slavebit    = ide_devices[drive].drive;
    u32 bus         = ide_channels[channel].base; 
    u32 words       = 2048 / 2;
    
    u8 err;

    ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].nIEN = ide_irq_invoked = 0x0);

    u8 atapi_packet[12];
    atapi_packet[ 0] = ATAPI_CMD_READ;
    atapi_packet[ 1] = 0x0;
    atapi_packet[ 2] = (lba>>24) & 0xFF;
    atapi_packet[ 3] = (lba>>16) & 0xFF;
    atapi_packet[ 4] = (lba>> 8) & 0xFF;
    atapi_packet[ 5] = (lba>> 0) & 0xFF;
    atapi_packet[ 6] = 0x0;
    atapi_packet[ 7] = 0x0;
    atapi_packet[ 8] = 0x0;
    atapi_packet[ 9] = numsects;
    atapi_packet[10] = 0x0;
    atapi_packet[11] = 0x0;

    ide_write(channel, ATA_REG_HDDEVSEL, slavebit << 4);
    for(int i = 0; i < 4; i++)
        ide_read(channel, ATA_REG_ALTSTATUS);
    
    // Set up pio mode
    ide_write(channel, ATA_REG_FEATURES, 0);

    ide_write(channel, ATA_REG_LBA1, (words * 2) & 0xFF);
    ide_write(channel, ATA_REG_LBA2, (words * 2) >> 8);

    ide_write(channel, ATA_REG_COMMAND, ATA_CMD_PACKET);

    asm("rep outsw" ::
        "c" (6),
        "d" (bus),
        "S" (atapi_packet)
    );

    for(int i = 0; i < numsects; i++) {
        ide_irqwait();

        err = ide_polling(channel, 1);
        if(err)
            return err;
        
        asm("pushw %es");
        asm("mov %%ax, %%es"::"a"(selector));
        asm("rep insw"::"c"(words), "d"(bus), "D"(edi));
        asm("popw %es");

        edi += (words*2); 
    }

    ide_irqwait();

    while(ide_read(channel, ATA_REG_STATUS) & (ATA_SR_BSY | ATA_SR_DRQ));
    return 0;
}

int atapi_readSector(u8 drive, u32 lba, char* buffer) {
    u8 status = ide_atapi_read(drive, lba, 1, 0x10, (u32) buffer);
    return status ? 1 : 0;
}

