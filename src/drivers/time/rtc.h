#ifndef __RTC_H
#define __RTC_H

#include "types.h"

struct rtc_time {
    u8 sec;
    u8 min;
    u8 hrs;
    u8 day;
    u8 mon;
    u16 year;
} __attribute__((packed)); // 7 bytes

int rtc_isupdating();
void rtc_wait();

void rtc_getTime(struct rtc_time* time);
void rtc_datetime(struct rtc_time* time);

void rtc_init();

#endif
