#include "irq/isr.h"
#include "irq/idt.h"
#include "ioport.h"
#include "devices/pic.h"

void pic8259_init() {
    u8 a1, a2;

    a1 = inportb(PIC1_DATA);
    a2 = inportb(PIC2_DATA);

    outportb(PIC1_COMMAND, ICW1);
    outportb(PIC2_COMMAND, ICW1);

    outportb(PIC1_DATA, 0x20);      // map vector offset of all default IRQ's from 0x20 to 0x27 in master(ICW2)
    outportb(PIC2_DATA, 0x28);      // map vector offset of all default IRQ's from 0x28 to 0x2F in slave(ICW2)

    outportb(PIC1_DATA, 4);         // ICW3: tell master PIC that there is a slave PIC at IRQ2 (0000 0100)
    outportb(PIC2_DATA, 2);         // ICW3: tell slave PIC its cascade identity (0000 0010)

    outportb(PIC1_DATA, ICW4_8086);
    outportb(PIC2_DATA, ICW4_8086);

    // restore the mask registers
    outportb(PIC1_DATA, a1);
    outportb(PIC2_DATA, a2);
}

void pic8259_eoi(u8 irq) {
    if(irq >= 0x28)
        outportb(PIC2, PIC_EOI);
    outportb(PIC1, PIC_EOI);
}