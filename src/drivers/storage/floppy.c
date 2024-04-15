#include "floppy.h"
#include "irq/irqdef.h"
#include "irq/isr.h"
#include "ioport.h"
#include "kernel.h"

const int floppy_spt = 18;
static u8 _currentDrive = 0;

u8 _irq_fired = 0;


void floppy_init() {
    isr_registerInterruptHandler(IRQ_BASE + IRQ6_DISKETTE_DRIVE, floppy_irq);
}

// =========================================================
// ===== INTERRUPT HANDLING ROUTINES
// =========================================================

void floppy_irq(REGISTERS* r) {
    IGNORE_UNUSED(r);

    _irq_fired = 1;
}

void floppy_irqwait() {
    while(! _irq_fired);
    _irq_fired = 0;
}


// =========================================================
// ===== BASIC FDC IO
// =========================================================

#define _GET_DOR()      inportb(FLOPPY_DOR)
#define _GET_MSR()      inportb(FLOPPY_MSR)
#define _GET_FIFO()     inportb(FLOPPY_FIFO)
#define _GET_CCR()      inportb(FLOPPY_CCR)

#define _SET_DOR(X)     outportb(FLOPPY_DOR, X)
#define _SET_MSR(X)     outportb(FLOPPY_MSR, X)
#define _SET_FIFO(X)    outportb(FLOPPY_FIFO, X)
#define _SET_CCR(X)     outportb(FLOPPY_CCR, X)

int floppy_busy() {
    if(inportb(FLOPPY_MSR) & FLOPPY_MSR_MASK_BUSY)
        return 1;
    
    return 0;
}

void floppy_sendCommand(u8 command) {
    for(int i = 0; i < 500; i ++) {
        if(inportb(FLOPPY_MSR) & FLOPPY_MSR_MASK_DATAREG) {
            outportb(FLOPPY_FIFO, command);
            return;
        }
    }
}

u8 floppy_readData() {
    for(int i = 0; i < 500; i ++) {
        if(inportb(FLOPPY_MSR) & FLOPPY_MSR_MASK_DATAREG)
            return inportb(FLOPPY_FIFO);
    }

    return 0;
}

// =========================================================
// ===== ISA DMA
// =========================================================

const int floppy_dma_buffer = 0x1000;

void floppy_dma_init() {
    outportb (0x0a, 0x06);  // mask dma channel 2
    outportb (0xd8, 0xff);  // reset master flip-flop
    outportb (0x04, 0x00);  // address=0x1000 
    outportb (0x04, 0x10);
    outportb (0xd8, 0xff);  // reset master flip-flop
    outportb (0x05, 0xff);  // count to 0x23ff (number of bytes in a 3.5" floppy disk track)
    outportb (0x05, 0x23);
    outportb (0x80, 0x00);  // external page register = 0
    outportb (0x0a, 0x02);  // unmask dma channel 2
}

void floppy_dma_read() {
    outportb (0x0a, 0x06); // mask dma channel 2
    outportb (0x0b, 0x56); // single transfer, address increment, autoinit, read, channel 2
    outportb (0x0a, 0x02); // unmask dma channel 2
}

void floppy_dma_write() {
    outportb (0x0a, 0x06); // mask dma channel 2
    outportb (0x0b, 0x5a); // single transfer, address increment, autoinit, write, channel 2
    outportb (0x0a, 0x02); // unmask dma channel 2
}


// =========================================================
// ===== SECTOR READ/WRITE
// =========================================================

void floppy_readSector(u8 head, u8 track, u8 sector) {
    u32 st0, cylinder;

    floppy_dma_read();

    floppy_sendCommand( FLOPPY_COMMAND_READ_SECTOR | 
                        FLOPPY_COMMAND_EXTENDED_MULTITRACK |
                        FLOPPY_COMMAND_EXTENDED_SKIP |
                        FLOPPY_COMMAND_EXTENDED_DENSITY);
    floppy_sendCommand(head << 2 | _currentDrive);
    floppy_sendCommand(track);
    floppy_sendCommand(head);
    floppy_sendCommand(sector);
    floppy_sendCommand(FLOPPY_SECTOR_DTL_512);
    floppy_sendCommand(((sector + 1) >= floppy_spt) ? floppy_spt : sector + 1);
    floppy_sendCommand(FLOPPY_GAP3_LENGTH_3_5);
    floppy_sendCommand(0xFF);
}

