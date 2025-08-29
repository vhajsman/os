#ifndef __TIMER_H
#define __TIMER_H

#include "types.h"
#include "devices/pic.h"

// #include "devices/pic.h"
// #define PIT_DEFAULT_FREQUENCY 18.2065
// #define	PIT_MASK_BINCOUNT           1       //00000001
// #define	PIT_MASK_MODE               0xE     //00001110
// #define	PIT_MASK_RL                 0x30    //00110000
// #define	PIT_MASK_COUNTER            0xC0    //11000000
// 
// #define	PIT_BINCOUNT_BINARY         0       //0         // Use when setting I86_PIT_OCW_MASK_BINCOUNT
// #define	PIT_BINCOUNT_BCD            1       //1
// #define	PIT_MODE_TERMINALCOUNT      0       //0000      // Use when setting I86_PIT_OCW_MASK_MODE
// #define	PIT_MODE_ONESHOT            0x2     //0010
// #define	PIT_MODE_RATEGEN	        0x4     //0100
// #define	PIT_MODE_SQUAREWAVEGEN      0x6     //0110
// #define	PIT_MODE_SOFTWARETRIG       0x8     //1000
// #define	PIT_MODE_HARDWARETRIG       0xA     //1010
// #define	PIT_RL_LATCH                0       //000000    // Use when setting I86_PIT_OCW_MASK_RL
// #define	PIT_RL_LSBONLY              0x10    //010000
// #define	PIT_RL_MSBONLY              0x20    //100000
// #define	PIT_RL_DATA                 0x30    //110000
// #define	PIT_COUNTER_0               0       //00000000  // Use when setting I86_PIT_OCW_MASK_COUNTER
// #define	PIT_COUNTER_1               0x40    //01000000
// #define	PIT_COUNTER_2               0x80    //10000000
// 
// #define	PIT_REG_COUNTER0            0x40
// #define	PIT_REG_COUNTER1            0x41
// #define	PIT_REG_COUNTER2            0x42
// #define	PIT_REG_COMMAND             0x43
// 
// #define PIT_CALCULATE_DIVISOR(frequency) ((1193180 + (frequency) / 2) / (frequency))
// 
// #define PIT_BASE_FREQUENCY 1193182  // Base frequency of the PIT in Hz
// #define TARGET_FREQUENCY 1000
// 
// u8 pit_addressing(u8 counter);
// void pit_init();
// void pit_start(u32 frequency, u8 counter, u8 mode);
// 
// void pit_wait(u32 ticks);
// void pit_sleep(u32 ms);
// 
// u32 pit_directRead();
// void pit_directWrite(u32 val);
// 

#define PIT_FREQUENCY 1193180

// ? I/O port  Usage
// ? 0x40      Channel 0 data port (read/write)
// ? 0x41      Channel 1 data port (read/write)
// ? 0x42      Channel 2 data port (read/write)
// ? 0x43      Mode/Command register (write only, a read is ignored)

#define PIT_REG_DATA0   0x40
#define PIT_REG_DATA1   0x41
#define PIT_REG_DATA2   0x42
#define PIT_REG_MODE    0x43
#define PIT_REG_COMMAND 0x43

void pit_init(u32 freq);
void sleep(u32 ms);
void wait(u32 ticks);

#define pit_sleep sleep
#define pit_wait wait

u32 pit_get();

#endif
