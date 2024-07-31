#include "timer.h"
#include "ioport.h"
#include "kernel.h"
#include "console.h"
#include "irq/isr.h"
#include "irq/irqdef.h"

#include "debug.h"

u32 _ticks = 0;

void pit_irq(REGISTERS* r) {
    IGNORE_UNUSED(r);

    _ticks++;
}

u8 pit_addressing(u8 counter) {
    switch (counter) {
        case PIT_COUNTER_0: 
            return PIT_REG_COUNTER0;
        case PIT_COUNTER_1: 
            return PIT_REG_COUNTER1;
        case PIT_COUNTER_2: 
            return PIT_REG_COUNTER2;
        default: 
            return PIT_REG_COUNTER0;
    }
}

void pit_sendCommand(u8 command) {
    outportb(PIT_REG_COMMAND, command);
}

void pit_sendData (u16 data, u8 counter) {
    u8 port = pit_addressing(counter);

    outportb(port, data & 0xFF);
    outportb(port, (data >> 8) & 0xFF);
}

u8 pit_readData (u8 counter) {
    return inportb(pit_addressing(counter));
}

void pit_init() {
    isr_registerInterruptHandler(IRQ_BASE + IRQ0_TIMER, pit_irq);
    debug_message("Init OK", "PIT", KERNEL_OK);
}

void pit_start(u32 frequency, u8 counter, u8 mode) {
    if(!frequency) 
        // TODO: Make the timer stop.
        return;
    
    u16 divisor = PIT_CALCULATE_DIVISOR(frequency);
    u8 ocw = 0;

    ocw = (ocw & ~PIT_MASK_MODE)    | mode;
	ocw = (ocw & ~PIT_MASK_RL)      | PIT_RL_DATA;
	ocw = (ocw & ~PIT_MASK_COUNTER) | (counter << 6);

    pit_sendCommand(ocw);

    // ? Set the frequency
    pit_sendData( divisor       & 0xFF, counter);
    pit_sendData((divisor >> 8) & 0xFF, counter);

    _ticks = 0;
}

void pit_wait(u32 ticks) {
    u32 finish = _ticks + ticks;

    while(_ticks < finish) {}
}

u32 pit_directRead() {
    u32 count = 0;

	outportb(0x43, 0);
 
	count = inportb(0x40);
	count |= inportb(0x40) << 8;
 
	return count;
}

void pit_directWrite(u32 val) {
	outportb(0x40,  val & 0xFF);
	outportb(0x40, (val >> 8) & 0xFF);
    
    _ticks = val;
}
