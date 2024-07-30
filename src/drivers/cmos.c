#include "cmos.h"
#include "ioport.h"

void cmos_write(u16 reg, u8 value) {
    outportb(0x70, reg);
    outportb(0x71, value);
}

u8 cmos_read(u16 reg) {
    outportb(0x70, reg);
    return inportb(0x71);
}