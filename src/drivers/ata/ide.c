    #include "ide.h"
    #include "types.h"
    #include "ioport.h"
    #include "ata.h"
    #include "debug.h"
    #include "time/timer.h"
    #include "irq/isr.h"
    #include "irq/irqdef.h"
    #include "kernel.h"

    struct ide_channelRegisters ide_channels[2];
    struct ide_device ide_devices[4];

    u8 ide_buf[2048] = {0};
    volatile u8 ide_irq_invoked = 0;
    static u8 atapi_packet[12] = {0xA8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    void ide_irq(REGISTERS* r);
    void ide_irqwait();

    u8 ide_read(u8 channel, u8 reg) {
        u8 result;
        if (reg > 0x07 && reg < 0x0C)
            ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channels[channel].nIEN);

        if (reg < 0x08)         result = inportb(ide_channels[channel].base  + reg - 0x00);
        else if (reg < 0x0C)    result = inportb(ide_channels[channel].base  + reg - 0x06);
        else if (reg < 0x0E)    result = inportb(ide_channels[channel].ctrl  + reg - 0x0A);
        else if (reg < 0x16)    result = inportb(ide_channels[channel].bmide + reg - 0x0E);

        if (reg > 0x07 && reg < 0x0C)
            ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].nIEN);

        return result;
    }

    void ide_write(u8 channel, u8 reg, u8 data) {
        if (reg > 0x07 && reg < 0x0C)
            ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channels[channel].nIEN);

        if (reg < 0x08)         outportb(ide_channels[channel].base  + reg - 0x00, data);
        else if (reg < 0x0C)    outportb(ide_channels[channel].base  + reg - 0x06, data);
        else if (reg < 0x0E)    outportb(ide_channels[channel].ctrl  + reg - 0x0A, data);
        else if (reg < 0x16)    outportb(ide_channels[channel].bmide + reg - 0x0E, data);

        if (reg > 0x07 && reg < 0x0C)
            ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].nIEN);
    }

    void ide_readbuffer(u8 channel, u8 reg, u32* buffer, unsigned int quads) {
        if (reg > 0x07 && reg < 0x0C)
            ide_write(channel, ATA_REG_CONTROL, 0x80 | ide_channels[channel].nIEN);

        asm("pushw %es; movw %ds, %ax; movw %ax, %es");

        if (reg < 0x08)         insl(ide_channels[channel].base  + reg - 0x00, buffer, quads);
        else if (reg < 0x0C)    insl(ide_channels[channel].base  + reg - 0x06, buffer, quads);
        else if (reg < 0x0E)    insl(ide_channels[channel].ctrl  + reg - 0x0A, buffer, quads);
        else if (reg < 0x16)    insl(ide_channels[channel].bmide + reg - 0x0E, buffer, quads);
        
        asm("popw %es;");

        if (reg > 0x07 && reg < 0x0C)
            ide_write(channel, ATA_REG_CONTROL, ide_channels[channel].nIEN);
    }

    u8 ide_polling(u8 channel, unsigned int advanced_check) {
        // Reading the Alternate Status port wastes 100ns; loop four times.
        for(int i = 0; i < 4; i++)
            ide_read(channel, ATA_REG_ALTSTATUS);

        int i = 0;
        while (ide_read(channel, ATA_REG_STATUS) & ATA_SR_BSY) {
            if(i == 1500000) {
                debug_message("IDE polling timeout: device not responding", "ide", KERNEL_ERROR);
                return 4;
            }

            i++;
        }

        if (advanced_check) {
            u8 state = ide_read(channel, ATA_REG_STATUS);

            if (state & ATA_SR_ERR)
                return 2;

            if (state & ATA_SR_DF)
                return 1; // Device Fault.

            if ((state & ATA_SR_DRQ) == 0)
                return 3; // DRQ should be set

        }

        return 0;
    }

    #define _ide_debugerr(errc, msg)    {       \
        debug_message(msg, "ide", KERNEL_ERROR);\
        err = errc;                             \
    }

    u8 ide_debugerr(unsigned int drive, u8 err) {
        if (err == 0)
            return err;

        if (err == 1) {
            debug_message("Device fault", "ide", KERNEL_ERROR);
            err = 19;
        }

        else if (err == 2) {
            u8 st = ide_read(ide_devices[drive].channel, ATA_REG_ERROR);

            if (st & ATA_ERR_AMNF)   _ide_debugerr(7,  "no address mark found");
            if (st & ATA_ERR_TK0NF)  _ide_debugerr(3,  "no media or media error");
            if (st & ATA_ERR_ABRT)   _ide_debugerr(20, "abort");
            if (st & ATA_ERR_MCR)    _ide_debugerr(3,  "no media or media error");
            if (st & ATA_ERR_IDNF)   _ide_debugerr(21, "no ID mark found");
            if (st & ATA_ERR_MC)     _ide_debugerr(3,  "no media or media error");
            if (st & ATA_ERR_UNC)    _ide_debugerr(22, "uncorrectable data error");
            if (st & ATA_ERR_BBK)    _ide_debugerr(13, "bad sectors");

        } else  if(err == 3) {       _ide_debugerr(23, "nothing to read");}
        else  if (err == 4)          _ide_debugerr(8,  "read-only");
        
        debug_append(" @ ");
        debug_append((const char *[]) {"Primary ", "Secondary "} [ide_devices[drive].channel]);
        debug_append((const char *[]) {"Master ",  "Slave "}     [ide_devices[drive].drive]);
        debug_append((const char*) ide_devices[drive].model);

        return err;
    }

    void ide_init_detect() {
        int count = 0;
        int k = 0;

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                u8 err = 0, type = IDE_ATA, status;
                ide_devices[count].reserved = 0;

                ide_write(i, ATA_REG_HDDEVSEL, 0xA0 | (j << 4));
                wait(10);

                ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
                wait(10);

                if(ide_read(i, ATA_REG_STATUS) == 0) 
                    continue;
                
                err = ide_polling(i, 1);

                while(1) {
                    status = ide_read(i, ATA_REG_STATUS);

                    if((status & ATA_SR_ERR)) {
                        err = 1;
                        break;
                    }

                    if(!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ)) 
                        break;
                }

                if (err != 0) {
                    u8 cl = ide_read(i, ATA_REG_LBA1);
                    u8 ch = ide_read(i, ATA_REG_LBA2);

                    if (cl == 0x14 && ch == 0xEB) {
                        type = IDE_ATA;
                    } else if (cl == 0x69 && ch == 0x96) {
                        type = IDE_ATAPI;
                    } else {
                        type = IDE_UNKNOWN;

                        debug_message("Unknown IDE device type ", "ide", KERNEL_WARNING);
                        debug_number(type, 16); 

                        continue;
                    }

                    ide_write(i, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
                    wait(10);
                }

                ide_readbuffer(i, ATA_REG_DATA, (u32*) ide_buf, 128);

                ide_devices[count].reserved     = 1;
                ide_devices[count].type         = type;
                ide_devices[count].channel      = i;
                ide_devices[count].drive        = j;
                ide_devices[count].signature    = *((unsigned short *)(ide_buf + ATA_IDENT_DEVICETYPE));
                ide_devices[count].capabilities = *((unsigned short *)(ide_buf + ATA_IDENT_CAPABILITIES));
                ide_devices[count].commandSets  = *((unsigned int *)  (ide_buf + ATA_IDENT_COMMANDSETS));

                if (ide_devices[count].commandSets & (1 << 26)) {
                    ide_devices[count].size   = *((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA_EXT));
                } else {
                    // Device uses CHS or 28-bit Addressing:
                    ide_devices[count].size   = *((unsigned int *)(ide_buf + ATA_IDENT_MAX_LBA));
                }

                for(k = 0; k < 40; k += 2) {
                    ide_devices[count].model[k] = ide_buf[ATA_IDENT_MODEL + k + 1];
                    ide_devices[count].model[k + 1] = ide_buf[ATA_IDENT_MODEL + k];
                }

                ide_devices[count].model[40] = '\0';
                count++;
            }
        }
    }

    void ide_init_debugsummary() {
        for(int i = 0; i < 4; i++)
            if (ide_devices[i].reserved == 1) {
                unsigned int _size_gb = ide_devices[i].size / 1024 / 1024 / 2;

                debug_message("Found drive: ", "ide", KERNEL_OK);

                // Device type
                debug_append((const char *[]){"ATA  ", "ATAPI"}[ide_devices[i].type]);
                debug_append(" ");

                // Device capacity (gb)
                debug_number(_size_gb, 10);
                debug_append(" GB (");
                debug_number(ide_devices[i].size, 10);
                debug_append(" bytes) : ");

                // Device model
                debug_append((const char*) ide_devices[i].model);
            }
    }

    void ide_init(unsigned int BAR0, unsigned int BAR1, unsigned int BAR2, unsigned int BAR3, unsigned int BAR4) {
        // int j, k = 0;

        ide_channels[ATA_PRIMARY  ].base  = (BAR0 & 0xFFFFFFFC) + 0x1F0 * (!BAR0);
        ide_channels[ATA_PRIMARY  ].ctrl  = (BAR1 & 0xFFFFFFFC) + 0x3F6 * (!BAR1);
        ide_channels[ATA_SECONDARY].base  = (BAR2 & 0xFFFFFFFC) + 0x170 * (!BAR2);
        ide_channels[ATA_SECONDARY].ctrl  = (BAR3 & 0xFFFFFFFC) + 0x376 * (!BAR3);
        ide_channels[ATA_PRIMARY  ].bmide = (BAR4 & 0xFFFFFFFC) + 0; // Bus Master IDE
        ide_channels[ATA_SECONDARY].bmide = (BAR4 & 0xFFFFFFFC) + 8; // Bus Master IDE

        ide_write(ATA_PRIMARY  , ATA_REG_CONTROL, 2);
        ide_write(ATA_SECONDARY, ATA_REG_CONTROL, 2);

        ide_init_detect();
        ide_init_debugsummary();

        isr_registerInterruptHandler(IRQ14_HARD_DISK, ide_irq);
    }

    void ide_irqwait() {
        while(!ide_irq_invoked);
        ide_irq_invoked = 0;
    }

    void ide_irq(REGISTERS* r) {
        IGNORE_UNUSED(r);
        ide_irq_invoked = 1;
    }
