#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
#include "console.h"

#define KEYBOARD_DATA_PORT          0x60
#define KEYBOARD_STATUS_PORT        0x64
#define KEYBOARD_COMMAND_PORT       0x64

#define KEYBOARD_RESPONSE_SELF_PASS 0xAA
#define KEYBOARD_RESPONSE_ECHO      0xEE
#define KEYBOARD_RESPONSE_ACK       0xFA
#define KEYBOARD_RESPONSE_SELF_FAIL 0xFC
#define KEYBOARD_RESPONSE_RESEND    0xFE
#define KEYBOARD_RESPONSE_KEY_ERROR 0xFF
#define KEYBOARD_RESPONSE_BUFFER_OV 0xFF

#define KEYBOARD_COMMAND_SET_LEDS   0xED
#define KEYBOARD_COMMAND_ECHO       0xEE
#define KEYBOARD_COMMAND_SCANCODE   0xF0
#define KEYBOARD_COMMAND_IDENTIFY   0xF2
#define KEYBOARD_COMMAND_SET_RATE   0xF3
#define KEYBOARD_COMMAND_SCAN       0xF4
#define KEYBOARD_COMMAND_NO_SCAN    0xF5
#define KEYBOARD_COMMAND_DEFAULTS   0xF6
#define KEYBOARD_COMMAND_RESEND     0xFE
#define KEYBOARD_COMMAND_SELF       0xFF
#define KEYBOARD_COMMAND_RESET      0xFF

#define KEYBOARD_CTRL_STATUS_MASK_OUT_BUF   1		//00000001
#define KEYBOARD_CTRL_STATUS_MASK_IN_BUF    2		//00000010
#define KEYBOARD_CTRL_STATUS_MASK_SYSTEM    4		//00000100
#define KEYBOARD_CTRL_STATUS_MASK_CMD_DATA  8		//00001000
#define KEYBOARD_CTRL_STATUS_MASK_LOCKED    0x10	//00010000
#define KEYBOARD_CTRL_STATUS_MASK_AUX_BUF   0x20	//00100000
#define KEYBOARD_CTRL_STATUS_MASK_TIMEOUT   0x40	//01000000
#define KEYBOARD_CTRL_STATUS_MASK_PARITY    0x8     //10000000


// Initializes the keyboard driver
void kbd_init();

void kbd_enable();
void kbd_disable();

void kbd_discard();

void kbd_setLeds(int n, int c, int s);

// =========================================================
// ===== READING INPUT
// =========================================================

u8 kbd_getLastKey();
char kbd_getLastChar();

char kbd_toChar(u8 scancode, u8 uppercase, u8 altgr);

#endif
