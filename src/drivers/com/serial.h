#ifndef __SERIAL_H
#define __SERIAL_H


#define COM1 0x3F8
#define COM2 0x2F8

#include "types.h"

#define SERIAL_OFFSET_DATA          0
#define SERIAL_OFFSET_IRQ_ENABLE    1
#define SERIAL_OFFSET_DIV_LEAST     0
#define SERIAL_OFFSET_DIV_MOST      1
#define SERIAL_OFFSET_IRQ_ID        2
#define SERIAL_OFFSET_FIFO_CONTROL  2
#define SERIAL_OFFSET_LINE_CONTROL  3
#define SERIAL_OFFSET_MODEM_CONTROL 4
#define SERIAL_OFFSET_LINE_STATUS   5
#define SERIAL_OFFSET_MODEM_STATUS  6
#define SERIAL_OFFSET_SCRATCH       7

void serial_init();

int serial_received(u16 port);
char serial_readByte(u16 port);
int serial_available(u16 port);
void serial_writeByte(u16 port, char data);
void serial_write(u16 port, const char* data);

#endif