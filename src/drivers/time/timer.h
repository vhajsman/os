#ifndef __TIMER_H
#define __TIMER_H

#include "types.h"

#include "devices/pic.h"

#define	PIT_MASK_BINCOUNT           1       //00000001
#define	PIT_MASK_MODE               0xE     //00001110
#define	PIT_MASK_RL                 0x30    //00110000
#define	PIT_MASK_COUNTER            0xC0    //11000000

#define	PIT_BINCOUNT_BINARY         0       //0         // Use when setting I86_PIT_OCW_MASK_BINCOUNT
#define	PIT_BINCOUNT_BCD            1       //1
#define	PIT_MODE_TERMINALCOUNT      0       //0000      // Use when setting I86_PIT_OCW_MASK_MODE
#define	PIT_MODE_ONESHOT            0x2     //0010
#define	PIT_MODE_RATEGEN	        0x4     //0100
#define	PIT_MODE_SQUAREWAVEGEN      0x6     //0110
#define	PIT_MODE_SOFTWARETRIG       0x8     //1000
#define	PIT_MODE_HARDWARETRIG       0xA     //1010
#define	PIT_RL_LATCH                0       //000000    // Use when setting I86_PIT_OCW_MASK_RL
#define	PIT_RL_LSBONLY              0x10    //010000
#define	PIT_RL_MSBONLY              0x20    //100000
#define	PIT_RL_DATA                 0x30    //110000
#define	PIT_COUNTER_0               0       //00000000  // Use when setting I86_PIT_OCW_MASK_COUNTER
#define	PIT_COUNTER_1               0x40    //01000000
#define	PIT_COUNTER_2               0x80    //10000000

#define	PIT_REG_COUNTER0            0x40
#define	PIT_REG_COUNTER1            0x41
#define	PIT_REG_COUNTER2            0x42
#define	PIT_REG_COMMAND             0x43

#define PIT_CALCULATE_DIVISOR(FREQUENCY)    \
    (1193180 / FREQUENCY)

u8 pit_addressing(u8 counter);
void pit_init();
void pit_start(u32 frequency, u8 counter, u8 mode);

#endif
