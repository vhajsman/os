// #include "timer.h"
// #include "ioport.h"
// #include "kernel.h"
// #include "console.h"
// #include "irq/isr.h"
// #include "irq/irqdef.h"
// 
// #include "debug.h"
// 
// u32 _ticks = 0;
// 
// void pit_irq(REGISTERS* r) {
//     IGNORE_UNUSED(r);
// 
//     _ticks++;
// }
// 
// u8 pit_addressing(u8 counter) {
//     switch (counter) {
//         case PIT_COUNTER_0: 
//             return PIT_REG_COUNTER0;
//         case PIT_COUNTER_1: 
//             return PIT_REG_COUNTER1;
//         case PIT_COUNTER_2: 
//             return PIT_REG_COUNTER2;
//         default: 
//             return PIT_REG_COUNTER0;
//     }
// }
// 
// void pit_sendCommand(u8 command) {
//     outportb(PIT_REG_COMMAND, command);
// }
// 
// void pit_sendData (u16 data, u8 counter) {
//     u8 port = pit_addressing(counter);
//     
//     outportb(port, data & 0xFF);         // Low byte
//     outportb(port, (data >> 8) & 0xFF);  // High byte
// }
// 
// u8 pit_readData (u8 counter) {
//     return inportb(pit_addressing(counter));
// }
// 
// void pit_init() {
//     isr_registerInterruptHandler(IRQ_BASE + IRQ0_TIMER, pit_irq);
//     debug_message("Init OK", "PIT", KERNEL_OK);
// 
//     pit_start(1000, PIT_COUNTER_2, PIT_MODE_SQUAREWAVEGEN);
// }
// 
// void pit_start(u32 frequency, u8 counter, u8 mode) {
//     if (!frequency) return;
// 
//     u16 divisor = PIT_CALCULATE_DIVISOR(frequency);
//     u8 ocw = 0;
// 
//     ocw = (ocw & ~PIT_MASK_MODE)    | mode;
//     ocw = (ocw & ~PIT_MASK_RL)      | PIT_RL_DATA;
//     ocw = (ocw & ~PIT_MASK_COUNTER) | (counter << 6);
// 
//     pit_sendCommand(ocw);
//     pit_sendData(divisor, counter);  // Stačí zavolat jednou
// 
//     _ticks = 0;
// }
// 
// void pit_wait(u32 ticks) {
//     u32 finish = _ticks + ticks;
// 
//     while(_ticks < finish) {}
// }
// 
// u32 pit_directRead() {
//     u32 count = 0;
// 
// 	outportb(0x43, 0);
//  
// 	count = inportb(0x40);
// 	count |= inportb(0x40) << 8;
//  
// 	return count;
// }
// 
// void pit_directWrite(u32 val) {
// 	outportb(0x40,  val & 0xFF);
// 	outportb(0x40, (val >> 8) & 0xFF);
//     
//     _ticks = val;
// }
// 
// void pit_sleep(u32 ms) {
//     u32 start_ticks = _ticks;
//     u32 ticks_to_wait = ms;
// 
//     debug_message("Sleeping...", "PIT", KERNEL_OK);
//     while ((_ticks - start_ticks) < ticks_to_wait) {
//         // Busy-wait loop
//     }
//     debug_message("Woke up!", "PIT", KERNEL_OK);
// }
// 

#include "timer.h"
#include "ioport.h"
#include "kernel.h"
#include "irq/isr.h"
#include "irq/irqdef.h"
#include "multitask.h"
#include "console.h"

void pit_handler(REGISTERS* r);

// PIT tick counter
volatile u32 tick = 0;

// extern void pit_stub(REGISTERS* regs);

void pit_init(u32 freq) {
    u32 divisor = PIT_FREQUENCY / freq;

    // Command byte: channel 0, access mode lobyte/hibyte, operating mode 3 (square wave generator)
    outportb(PIT_REG_MODE, 0x36);

    // Send the frequency divisor to channel 0 data port
    outportb(PIT_REG_DATA0, (u8)  (divisor & 0xFF));        // Low byte
    outportb(PIT_REG_DATA0, (u8) ((divisor >> 8) & 0xFF));  // High byte

    isr_registerInterruptHandler(IRQ_BASE + IRQ0_TIMER, pit_handler);
}

void pit_handler(REGISTERS* r) {
    mt_switch(r);
    tick++;
}

void sleep(u32 ms) {
    u32 target = tick + ms * (PIT_FREQUENCY / 1000);
    while (tick < target) {
        
    }
}

void wait(u32 ticks) {
    u32 target = tick + ticks;
    while (tick < target) {
        
    }
}

u32 pit_get() {
    return tick;
}
