#ifndef __TIMER_H
#define __TIMER_H

#include "types.h"

#include "devices/pic.h"

void timer_phase(int freq);
u32 timer_read();
void timer_write(u32 time);
void timer_install();
void timer_wait(u32 ticks);
void timer_sleepSecs(u32 s);

#endif
