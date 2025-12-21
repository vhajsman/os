#ifndef __RTL8139_H
#define __RTL8139_H

#include "kernel.h"
#include "net.h"

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139

#define RTL8139_REG_CMD     0x37
#define RTL8139_REG_RBSTART 0x30
#define RTL8139_REG_RCR     0x44
#define RTL8139_REG_TSAD0   0x20
#define RTL8139_REG_TSAD1   0x24
#define RTL8139_REG_TSAD2   0x28
#define RTL8139_REG_TSAD3   0x2C
#define RTL8139_REG_ISR     0x3E
#define RTL8139_REG_IMR     0x3C
#define RTL8139_REG_CAPR    0x38
#define RTL8139_REG_TCR     0x40
#define RTL8139_REG_CONFIG1 0x52

#define RTL8139_CMD_RESET   0x10
#define RTL8139_RX_EN       0x08
#define RTL8139_TX_EN       0x04

#define RTL8139_RXBUFFER_SIZE 32768

#define RTL8139_TXBUFFER_COUNT 4
#define RTL8139_TXBUFFER_SIZE  2048

void rtl8139_init();

u8 rtl8139_send(const u8* data, size_t len);
size_t rtl8139_recv(u8* buff, size_t buff_len);

#endif

