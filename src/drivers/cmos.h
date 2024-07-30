#ifndef __CMOS_H
#define __CMOS_H

#include "types.h"

// TODO: REGISTER 0x15, 0x16 - Base memory registers
// TODO: REGISTER 0x17, 0x18 - Extended memory registers
// TODO: REGISTER 0x2E, 0x2F - Checksum of CMOS addresses 10H through 20H
// TODO: REGISTER 0x30, 0x31 - Actual extended memory size
// TODO: REGISTER 0x33 - IBM 128K memory installed

#define CMOS_REG_CURRENT_SECOND         0x00
#define CMOS_REG_ALARM_SECOND           0x01
#define CMOS_REG_CURRENT_MINUTE         0x02
#define CMOS_REG_ALARM_MINUTE           0x03
#define CMOS_REG_CURRENT_HOUR           0x04
#define CMOS_REG_ALARM_HOUR             0x05
#define CMOS_REG_CURRENT_DAY_OF_WEEK    0x06    // ! (1 == Sunday)
#define CMOS_REG_CURRENT_DAY_OF_MONTH   0x07
#define CMOS_REG_CURRENT_MONTH          0x08
#define CMOS_REG_CURRENT_YEAR           0x09    // ! LAST 2 DIGITS
#define CMOS_REG_CURRENT_CENTURY        0x32

// ? STATUS REGISTER A
// ? ====================================
// ? Bit 7 – Update in Progress (Read)
// ? Bits 0 to 3 – Periodic Interupt Rate
// ?
// ? 3210 Hz    3210 Hz
// ? -----------------------
// ? 0001 256   0010 128
// ? 0011 8192  0100 4096
// ? 0101 2048  0110 1024
// ? 0111 512   1000 256
// ? 1001 128   1010 64
// ? 1011 32    1100 16
// ? 1101 8     1110 4
// ? 1111 2
#define CMOS_REG_STATUS_A   0x0A

// ? STATUS REGISTER B
// ? ====================================
// ? Bit 7 – Abort Update (allow access to clock data)
// ? Bit 6 – Enable Periodic Interupt
// ? Bit 5 – Enable Alarm Interupt
// ? Bit 4 – Enable Update Ended Interupt
// ? Bit 2 – Clock Data Type
// ?         0=BCD, 1=Binary
// ?         BIOS default=0
// ? Bit 1 – Hour Data Type
// ?         0=12 hour, 1=24 hour
// ?         BIOS default=1
// ? Bit 0 – Daylight Savings Enable
#define CMOS_REG_STATUS_B   0x0B

// ? STATUS REGISTER C - TYPE OF INTERRUPT
// ? ====================================
// ? Bit 7 – Any
// ? Bit 6 – Periodic
// ? Bit 5 – Alarm
// ? Bit 4 – Update Ended
#define CMOS_REG_STATUS_C   0x0C

// ? DIAGNOSTICS STATUS REGISTER
// ? ====================================
// ? Bit 7 – Clock Lost Power
// ? Bit 6 – CMOS Bad Checksum
// ? Bit 5 – Invalid Configuration @ POST
// ? Bit 4 – Memory Size Compare Error
// ? Bit 3 – Disk or Controller Error
// ? Bit 2 – Invalid Time or Data (32nd)
#define CMOS_REG_DIAGNOSTICS_STATUS 0x0E

// ? SHUTDOWN STATUS REGISTER
// ? ====================================
// ? 0x00 - power on reset
// ? 0x01 - memory size pass
// ? 0x02 - memory test pass
// ? 0x03 - memory test fail
// ? 0x04 - POST end, boot system
// ? 0x05 - JMP DWORD PTR 0:[0467h] with EOI
// ? 0x06 - protected tests pass
// ? 0x07 - protected tests fail
// ? 0x08 - memory size fail
// ? 0x09 - INT 15h block move
// ? 0x0A - JMP DWORD PTR 0:[0467h] without EOI
#define CMOS_REG_SHUTDOWN_STATUS 0x0F

// ? FLOPPY DRIVE TYPES REGISTER
// ? ====================================
// ? Bits 7 to 4 – Disk 0 (A:)
// ? Bits 3 to 0 – Disk 1 (B:)
// ?
// ? Values:
// ? 0000 - no drive
// ? 0001 - 360k
// ? 0010 - 1.2M
// ? 0011 - 720k
// ? 0100 - 1.44M
#define CMOS_REG_FLOPPY_DRIVE_TYPES 0x10

// ? HARD DRIVE TYPES REGISTER
// ? ====================================
// ? Bits 7 to 4 – Disk 0 (C:)
// ? Bits 3 to 0 – Disk 1 (D:)
// ?
// ? Values:
// ? 0000           – no drive
// ? 0001 to 1110   – drive types
// ? 1111           – specified at 0=19h, 1=1Ah
#define CMOS_REG_HARD_DRIVE_TYPES 0x12

// ? EQUIPEMENT REGISTER
// ? ====================================
// ? Bits 6, 7 – No. Of Floppy Drives
// ?             00=1, 01=2, 10=3, 11=4
// ? Bits 4, 5 – Display Type
// ?             00 – none, EGA, VGA, etc.
// ?             01 - 40x25 colour
// ?             10 - 80x25 colour
// ?             11 = 80x25 monochrome
// ? Bit 1     – Math Co-processor Available
// ? Bit 0     – Floppy Drive(s) Available
#define CMOS_REG_EQUIPEMENT 0x14

#define CMOS_REG_HARD_DRIVE0_TYPE   0x19 // Only when specified at12h
#define CMOS_REG_HARD_DRIVE1_TYPE   0x1A // Only when specified at12h

void cmos_write(u16 reg, u8 value);
u8 cmos_read(u16 reg);

#endif
