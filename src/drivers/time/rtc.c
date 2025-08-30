#include "rtc.h"
#include "ioport.h"
#include "kernel.h"
#include "string.h"
#include "irq/isr.h"
#include "irq/irqdef.h"

#define RTC_CMD     0x70
#define RTC_DATA    0x71


struct rtc_time rtc_time_synchronized;

u8 rtc_read(u8 reg) {
    outportb(RTC_CMD, reg);
    return inportb(RTC_DATA);
}

void rtc_write(u8 reg, u8 value) {
    outportb(RTC_CMD, reg);
    outportb(RTC_DATA, value);
}

int rtc_isupdating() {
    outportb(RTC_CMD, 0x0A);
    return inportb(RTC_DATA) & 0x80;
}

void rtc_wait() {
    while(rtc_isupdating()) {}
}

void rtc_getTime(struct rtc_time* time) {
    if(time == NULL)
        return;

    time->sec =     rtc_read(0x00);
    time->min =     rtc_read(0x02);
    time->hrs =     rtc_read(0x04);
    time->day =     rtc_read(0x07);
    time->mon =     rtc_read(0x08);
    time->year =    rtc_read(0x09);
}

void rtc_enable() {
    u8 prev;

    outportb(RTC_CMD, 0x0B);
    prev = inportb(RTC_DATA);

    outportb(RTC_CMD, 0x0B);
    outportb(RTC_DATA, prev | 0x40);
}

void rtc_irq(REGISTERS* r) {
    IGNORE_UNUSED(r);
    
    outportb(RTC_CMD, 0x0C);
    inportb(RTC_DATA);

    rtc_getTime(&rtc_time_synchronized);
}

void rtc_datetime(struct rtc_time* time) {
    if(time == NULL)
        return;
    
    memcpy(time, &rtc_time_synchronized, sizeof(struct rtc_time));
}

void rtc_init() {
    isr_registerInterruptHandler(IRQ8_CMOS_CLOCK, rtc_irq);
}
