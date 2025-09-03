#ifndef __RTL8139_H
#define __RTL8139_H

#define RTL8139_VENDOR_ID 0x10EC
#define RTL8139_DEVICE_ID 0x8139

#define RTL8139_REG_CMD     0x37
#define RTL8139_REG_RBSTART 0x30
#define RTL8139_REG_RCR     0x44

#define RTL8139_CMD_RESET   0x10
#define RTL8139_RX_EN       0x08
#define RTL8139_TX_EN       0x04

#define RTL8139_RXBUFFER_SIZE ((1024 * 8) + 16 + 1500)

void rtl8139_init();

typedef u8 mac_t[6];

#endif

