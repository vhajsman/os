#ifndef __IDE_H
#define __IDE_H

#include "types.h"

struct ide_channelRegisters {
    unsigned short base;  // I/O Base.
    unsigned short ctrl;  // Control Base
    unsigned short bmide; // Bus Master IDE
    unsigned char  nIEN;  // nIEN (No Interrupt);
};

struct ide_device {
    unsigned char  reserved;    // 0 (Empty) or 1 (This Drive really exists).
    unsigned char  channel;     // 0 (Primary Channel) or 1 (Secondary Channel).
    unsigned char  drive;       // 0 (Master Drive) or 1 (Slave Drive).
    unsigned short type;        // 0: ATA, 1:ATAPI.
    unsigned short signature;   // Drive Signature
    unsigned short capabilities;// Features.
    unsigned int   commandSets; // Command Sets Supported.
    unsigned int   size;        // Size in Sectors.
    unsigned char  model[41];   // Model in string.
};

extern struct ide_channelRegisters ide_channels[2];
extern struct ide_device ide_devices[4];

u8 ide_read(u8 channel, u8 reg);
void ide_write(u8 channel, u8 reg, u8 data);
void ide_readbuffer(u8 channel, u8 reg, u32* buffer, unsigned int quads);
u8 ide_polling(u8 channel, unsigned int advanced_check);
void ide_init(unsigned int BAR0, unsigned int BAR1, unsigned int BAR2, unsigned int BAR3, unsigned int BAR4);


extern volatile u8 ide_irq_invoked; // ide.c
extern void ide_irqwait();

#endif
