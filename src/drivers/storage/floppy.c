#include "floppy.h"
#include "irq/irqdef.h"
#include "irq/isr.h"
#include "ioport.h"
#include "kernel.h"
#include "time/timer.h"

const int floppy_spt = 18;
static u8 _currentDrive = 0;

u8 _irq_fired = 0;

void floppy_sendCommand(u8 command);

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

void floppy_checkInterrupt(u32* st0, u32* cyl) {
    floppy_sendCommand(FLOPPY_COMMAND_CHECK_INTERRUPT);

    *st0 = floppy_readData();
    *cyl = floppy_readData();
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
    // u32 st0;

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


// =========================================================
// ===== FLOPPY DISK MOTOR CONTROL
// =========================================================

void floppy_motor(int enable) {
    if(_currentDrive > 3)
        return;
    
    u32 motor = 0;

    switch(_currentDrive) {
        case 0: motor = FLOPPY_DOR_MASK_MOTOR0; break;
        case 1: motor = FLOPPY_DOR_MASK_MOTOR1; break;
        case 2: motor = FLOPPY_DOR_MASK_MOTOR2; break;
        case 3: motor = FLOPPY_DOR_MASK_MOTOR3; break;

        default:
            // Prevent from mistaking addresses. 
            // Actually should not be even reached if used as excepted.

            return; 
    }

    _SET_DOR(
        enable 
        ? (_currentDrive | motor | FLOPPY_DOR_MASK_RESET | FLOPPY_DOR_MASK_DMA)
        : FLOPPY_DOR_MASK_RESET
    );

    pit_wait(20);
}


// =========================================================
// ===== DRIVE CONFIGURATION
// =========================================================

int floppy_config(u32 stepr, u32 loadt, u32 unloadt, int dma) {
    u32 data[2];

    data[0] = ((stepr & 0x0F) << 4 | (unloadt & 0x0F));
    data[1] = ((loadt       ) << 1 | dma ? 1 : 0);

    floppy_sendCommand(data[0]);
    floppy_sendCommand(data[1]);

    return 0;
}


// =========================================================
// ===== CALIBRATE, SEEK
// =========================================================

int floppy_calibrate(u32 drive) {
    u32 st0, cyl;

    if(drive >= 4)
        return 1;
    
    floppy_motor(1);

    for(int i = 0; i < 10; i ++) {
        floppy_sendCommand(FLOPPY_COMMAND_CALIBRATE);
        floppy_sendCommand(drive);

        floppy_irqwait();
        floppy_checkInterrupt(&st0, &cyl);

        if(!cyl) {
            floppy_motor(0);
            return 0;
        }
    }

    floppy_motor(0);
    return 2;
}

int floppy_seek(u32 cyl, u32 head) {
    u32 st0, _cyl;

    if(_currentDrive >= 4)
        return 1;
    
    for(int i = 0; i < 10; i ++) {
        floppy_sendCommand(FLOPPY_COMMAND_SEEK);
        floppy_sendCommand((head) << 2 | _currentDrive);
        floppy_sendCommand(cyl);

        floppy_irqwait();
        floppy_checkInterrupt(&st0, &_cyl);

        if(_cyl == cyl)
            return 0;
    }

    return 2;
}


// =========================================================
// ===== ENABLE, DISABLE, RESET
// =========================================================

void floppy_disable() {
    _SET_DOR(0x00);
}

void floppy_enable() {
    _SET_DOR(FLOPPY_DOR_MASK_RESET | FLOPPY_DOR_MASK_DMA);
}

void floppy_reset() {
    u32 st0, cyl;

    floppy_disable();
    floppy_enable();

    floppy_irqwait();

    for(int i = 0; i < 4; i ++)
        floppy_checkInterrupt(&st0, &cyl);
    
    _SET_CCR(0);

    floppy_config(0x03, 0x0F, 240, 1);
    floppy_calibrate(_currentDrive);
}


// =========================================================
// ===== DRIVER INIT
// =========================================================

void floppy_init() {
    isr_registerInterruptHandler(IRQ_BASE + IRQ6_DISKETTE_DRIVE, floppy_irq);

    floppy_reset();
}