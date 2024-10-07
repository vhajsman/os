#include "mouse.h"
#include "kernel.h"
#include "debug.h"
#include "irq/isr.h"
#include "irq/irqdef.h"

u8 mouse_cycle=0;
s8 mouse_byte[3];
s8 mouse_x=0;
s8 mouse_y=0;

void mouse_irq(REGISTERS* r) {
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

            mouse_x = mouse_byte[1];
            mouse_y = mouse_byte[2];

            mouse_cycle=0;
            break;
        
        default:
            debug_message("invalid mouse cycle: ", "mouse", KERNEL_ERROR);
            debug_number(mouse_cycle, 10);

            return;
    }
}

void mouse_wait(u8 type) {
    int _timeout=100000;

    if(type == 0) {
        while(_timeout--) {
            if((inportb(0x64) & 1) == 1)
                return;
        }

        return;
    }

    while(_timeout--) {
        if((inportb(0x64) & 2) == 0)
            return;
    }

    return;
}

void mouse_write(u8 command) {
    mouse_wait(1);
    outportb(0x64, 0xD4);

    mouse_wait(1);
    outportb(0x60, command);
}

u8 mouse_read() {
    mouse_wait(0); 
    return inportb(0x60);
}

void mouse_init() {
    u8 _status;
    
    mouse_wait(1);
    outportb(0x64, 0xA8);
    
    mouse_wait(1);
    outportb(0x64, 0x20);

    mouse_wait(0);
    _status = (inportb(0x60) | 2);

    mouse_wait(1);
    outportb(0x64, 0x60);

    mouse_wait(1);
    outportb(0x60, _status);
    
    // Use defaults
    mouse_write(0xF6);
    mouse_read();
    
    //Enable
    mouse_write(0xF4);
    mouse_read();

    isr_registerInterruptHandler(IRQ12_AUXILIARY, mouse_irq);
}
