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
    return
    (counter == PIT_COUNTER_0) 
    ? PIT_REG_COUNTER0
    : ((counter==PIT_COUNTER_1) 
        ? PIT_REG_COUNTER1
        : PIT_REG_COUNTER2);
}

void pit_sendCommand(u8 command) {
    outportb(PIT_REG_COMMAND, command);
}

void pit_sendData (u16 data, u8 counter) {
	outportb(pit_addressing(counter), data);
}

u8 pit_readData (u8 counter) {
    return inportb(pit_addressing(counter));
}

void pit_init() {
    isr_registerInterruptHandler(IRQ_BASE + IRQ0_TIMER, pit_irq);
    debug_message("OK", "PIT", KERNEL_OK);
}

void pit_start(u32 frequency, u8 counter, u8 mode) {
    if(! frequency) // TODO: Make the timer stop.
        return;
    
    u16 divisor = PIT_CALCULATE_DIVISOR(frequency);

    u8 ocw = 0;
    ocw = (ocw & ~PIT_MASK_MODE)    | mode;
	ocw = (ocw & ~PIT_MASK_RL)      | PIT_RL_DATA;
	ocw = (ocw & ~PIT_MASK_COUNTER) | counter;

    pit_sendCommand(ocw);

    // ? Set the frequency
    pit_sendData( divisor       & 0xFF, 0);
    pit_sendData((divisor >> 8) & 0xFF, 0);

    _ticks = 0;
}
