#include "timer.h"
#include "ioport.h"
#include "kernel.h"
#include "console.h"
#include "irq/isr.h"
#include "irq/irqdef.h"

u32 _ticks = 0;

void timer_phase(int freq) {
    int divisor = 1193180 / freq;

    outportb(0x43, 0x36);
    outportb(0x40, divisor & 0xFF);
    outportb(0x40, divisor >> 8);
}

u32 timer_read() {
    u32 count = 0;

    asm("cli");

    outportb(0x43, 0x00);
    count = inportb(0x40);
    count |= inportb(0x40) << 8;

    asm("sti");
    return count;
}

void timer_write(u32 time) {
    asm("cli");

    outportb(0x40, time & 0xFF);
    outportb(0x40, (time & 0xFF00) >> 8);

    asm("sti");
    return;
}

void timer_handler(REGISTERS* r) {
    _ticks++;

    //if(_ticks % 18 == 0)
        //printf("1 second has passed.\n");
}

void timer_install() {
    timer_phase(18);
    isr_registerInterruptHandler(IRQ_BASE + 0, timer_handler);
}

void timer_wait(u32 ticks) {
    u32 eticks = _ticks + ticks;
    while(_ticks < eticks);
}

void timer_sleepSecs(u32 s) {
    u32 _t = _ticks;
    for(u32 i = 0; i < s; i ++)
        while(_t % 18 != 0 || _t > 0);
}
