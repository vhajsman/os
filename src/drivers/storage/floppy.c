#include "floppy.h"
#include "irq/irqdef.h"
#include "irq/isr.h"
#include "ioport.h"
#include "kernel.h"

u8 _irq_fired = 0;

void floppy_irq(REGISTERS* r) {
    IGNORE_UNUSED(r);

    _irq_fired = 1;
}

void floppy_irqwait() {
    while(! _irq_fired);
    _irq_fired = 0;
}

void floppy_init() {
    isr_registerInterruptHandler(IRQ_BASE + IRQ6_DISKETTE_DRIVE, floppy_irq);
}
