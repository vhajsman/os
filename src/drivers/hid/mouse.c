#include "mouse.h"
#include "video/vga.h" // Positioning, cursor
#include "kernel.h"
#include "irq/irqdef.h"
#include "irq/isr.h"
#include "ioport.h"

u8 mouse_cycle;
s8 mouse_byte[3];

struct xy2d mouse_position;

void mouse_irq(REGISTERS* r) {
    IGNORE_UNUSED(r);
    
    switch(mouse_cycle) {
        case 0:
            mouse_byte[0] = inportb(0x60);
            mouse_cycle++;

            break;

        case 1:
            mouse_byte[1] = inportb(0x60);
            mouse_cycle++;

            break;

        case 2:
            mouse_byte[2] = inportb(0x60);

            mouse_position.x = mouse_byte[1];
            mouse_position.y = mouse_byte[2];

            mouse_cycle = 0;

            break;
    }
}

void mouse_wait(u8 a) {
    u32 timeout = 100000;

    if(a == 0) {
        while(timeout--) {
            if((inportb(0x64) & 1) == 1)
                return;
        }

        return;
    }

    while(timeout--) {
        if((inportb(0x64) & 2) == 1)
            return;
    }

    // return;
}

void mouse_writeCommand(u8 command) {
    mouse_wait(1);
    outportb(0x64, 0xD4);   // "Tell" the mouse about sending a command

    mouse_wait(1);
    outportb(0x60, command);
}

u8 mouse_readData() {
    mouse_wait(0);
    return inportb(0x60);
}

void mouse_init() {
    u8 status;

    mouse_wait(1);
    outportb(0x64, 0xA8);

    mouse_wait(1);
    outportb(0x64, 0x20);

    mouse_wait(0);
    status = (inportb(0x60) | 2);

    mouse_wait(1);
    outportb(0x64, 0x60);

    mouse_wait(1);
    outportb(0x60, status);

    mouse_writeCommand(MOUSE_COMMAND_DEFAULT);
    mouse_writeCommand(MOUSE_COMMAND_ENABLE);


    isr_registerInterruptHandler(IRQ_BASE + IRQ12_AUXILIARY, mouse_irq);
}