#ifndef __PIC_H
#define __PIC_H

#define PIC1            0x20  /* IO base address for master PIC */
#define PIC2            0xA0  /* IO base address for slave PIC */
#define PIC1_COMMAND    PIC1
#define PIC1_DATA       (PIC1+1)
#define PIC2_COMMAND    PIC2
#define PIC2_DATA       (PIC2+1)

#define PIC_EOI         0x20

#define ICW1            0x11
#define ICW4_8086       0x01    /* 8086/88 (MCS-80/85) mode */

void pic8259_init();
void pic8259_eoi(u8 irq);

#endif
