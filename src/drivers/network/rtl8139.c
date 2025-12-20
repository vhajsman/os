#define __require_pci_dev_zero

#include "rtl8139.h"        //
#include "pci.h"            //
#include "debug.h"          //
#include "memory/memory.h"  // kmalloc, kfree
#include "string.h"         // memcpy
#include "ioport.h"         //
#include "console.h"        // puts
#include "time/timer.h"     // pit_wait

pci_dev_t rtl8139_device;
u16 rtl8139_io_base;
u32 rtl8139_io_bar0;

volatile u8* rtl8139_rxbuffer = NULL;
u8* rtl8139_txbuffer[RTL8139_TXBUFFER_COUNT] = {NULL};

u8 rtl8139_mac[6];

static u32 rx_offset = 0;

void mac2str(const mac_t mac, char buffer[18]) {
    const char* hex = "0123456789ABCDEF";

    int pos = 0;
    for(int i = 0; i < 6; i++) {
        buffer[pos++] = hex[(mac[i] >> 4) & 0xF];
        buffer[pos++] = hex[mac[i] & 0xF];

        if(i < 5)
            buffer[pos++] = ':';
    }

    buffer[pos] = '\0';
}

void rtl8139_command(u8 command) {
    outportb(rtl8139_io_base + RTL8139_REG_CMD, command);

    if(command & RTL8139_CMD_RESET) // reset required
        while(inportb(rtl8139_io_base + RTL8139_REG_CMD) & command);
}

void rtl8139_reset() {
    debug_message("performing RTL8139 reset...", "rtl8139", KERNEL_MESSAGE);

    rtl8139_command(RTL8139_CMD_RESET);

    // wait for reset complete
    while(inportb(rtl8139_io_base + RTL8139_REG_CMD) & 0x10) {
        for(volatile int i = 0; i < 1000; i++);
    }

    pit_wait(100);
}

u8** rtl8139_setupTx() {
    for(int i = 0; i < RTL8139_TXBUFFER_COUNT; i++) {
        rtl8139_txbuffer[i] = (u8*) kmalloc(RTL8139_TXBUFFER_SIZE);

        if(!rtl8139_txbuffer[i]) {
            debug_message("not enough memory for TX buffer", "rtl8139", KERNEL_ERROR);

            for(int i0 = 0; i0 < i; i0++)
                kfree(rtl8139_txbuffer[i0]);

            return NULL;
        }
    }

    for(int i = 0; i < RTL8139_TXBUFFER_COUNT; i++) {
        outportl(rtl8139_io_base + RTL8139_REG_TSAD0 + i * 4, (u32)rtl8139_txbuffer[i]);

        u8 cmd = inportb(rtl8139_io_base + RTL8139_REG_CMD);
        cmd |= RTL8139_TX_EN;
        outportb(rtl8139_io_base + RTL8139_REG_CMD, cmd);
    };

    return rtl8139_txbuffer;
}

volatile u8* rtl8139_setupRx(bool acceptBroadcast, bool acceptMulticast, bool acceptRunt, u32 rblen_raw) {
    u32 rxconfig = 0;
    rxconfig |= acceptBroadcast ? (1 << 0) : 0;
    rxconfig |= acceptMulticast ? (1 << 1) : 0;
    rxconfig |= acceptRunt      ? (1 << 2) : 0;
    rxconfig |= (1 << 4);   // accept phys. match
    rxconfig |= (1 << 7);   // wrap
    rxconfig |= (rblen_raw << 11);

    rtl8139_rxbuffer = (u8*) kmalloc((size_t) RTL8139_RXBUFFER_SIZE);
    if(rtl8139_rxbuffer == NULL) {
        debug_message("not enough memory for RX buffer", "rtl8139", KERNEL_ERROR);
        return NULL;
    }

    outportl(rtl8139_io_base + RTL8139_REG_RBSTART, (u32) rtl8139_rxbuffer);
    outportl(rtl8139_io_base + RTL8139_REG_RCR, rxconfig);
    outportl(rtl8139_io_base + RTL8139_REG_CMD, RTL8139_RX_EN);

    debug_message("RX buffer size: ", "rtl8139", KERNEL_MESSAGE);
    debug_number(RTL8139_RXBUFFER_SIZE, 10);

    return rtl8139_rxbuffer;
}

void rtl8139_init() {
    rtl8139_device = pci_getDevice(
        RTL8139_VENDOR_ID, 
        RTL8139_DEVICE_ID, 
        PCI_HEADER_TYPE_DEVICE
    );

    if(/*rtl8139_device == dev_zero*/ rtl8139_device.bits == 0) {
        // RTL8139 not detected
        
        debug_message("RTL8139 not found", "rtl8139", KERNEL_ERROR);
        return;
    }

    rtl8139_io_bar0 = pci_config_read(&rtl8139_device, PCI_BAR0);   // read BAR0
    rtl8139_io_base = rtl8139_io_bar0 & ~0x03;                      // ignore last 2 bits
    
    // --- Enable IO and bus mastering --- 
    u16 cmd = pci_config_read(&rtl8139_device, PCI_COMMAND) & 0xFFFF;
    cmd |= (1 << 0); // IO space
    cmd |= (1 << 2); // Bus mastering

    pci_config_write16(&rtl8139_device, PCI_COMMAND, cmd);

    
    // reset 
    rtl8139_reset();

    if(!rtl8139_setupTx())                                      goto e_exit;
    if(!rtl8139_setupRx(true, true, true, 2 /*rblen=32kib*/))   goto e_exit;

    // --- Get MAC address --- 
    for(int i = 0; i < 6; i++)
        rtl8139_mac[i] = inportb(rtl8139_io_base + i);

    char mac_str[18];
    mac2str(rtl8139_mac, mac_str);

    puts("RTL8139 physical address: ");
    puts(mac_str);
    puts("\n");

    debug_message("MAC address: ", "rtl8139", KERNEL_MESSAGE);
    debug_append(mac_str);

    // TEST

    int i = 0;
    while(1) {
        if(i > 350) {
            goto e_timeout;
            return;
        }

        u8 t_packet[1536];
        size_t t_packet_len = rtl8139_recv(t_packet, sizeof(t_packet));
        if(t_packet_len > 0) {
            debug_message("rtl8139_init(): packet accepted", "rtl8139", KERNEL_MESSAGE);
            break;
        }

        pit_wait(10);
        i++;
    }

    debug_message("RTL8139 init subroutine done", "rtl8139", KERNEL_MESSAGE);
    return;

e_timeout:
    debug_message("RTL8139 init error: timed out", "rtl8139", KERNEL_ERROR);
    puts("RTL8139: rtl8139_init(): timeout\n");
    goto e_exit;

e_exit:
    free((void*) rtl8139_rxbuffer);

    for(int i01 = 0; i01 < RTL8139_TXBUFFER_COUNT; i01++)
        free(rtl8139_txbuffer[i01]);

    return;
}

u8 rtl8139_send(const u8* data, size_t len) {
    static int tx_curr = 0;

    if(len > RTL8139_TXBUFFER_SIZE)
        return 1;

    memcpy(rtl8139_txbuffer[tx_curr], data, len);
    outportl(rtl8139_io_base + 0x10 + tx_curr * 4, len & 0xFFFF);

    tx_curr = (tx_curr + 1) % RTL8139_TXBUFFER_COUNT;
    return 0;
}

size_t rtl8139_recv(u8* buff, size_t buff_len) {
    u16 status = *(volatile u16*)(rtl8139_rxbuffer + rx_offset);

    // packet not ready
    if(!(status & 0x01))
        return 0;

    u16 packet_len = *(volatile u16*)(rtl8139_rxbuffer + rx_offset + 2);

    // invalid packet
    if(status & 0x1E) {
        rx_offset += (packet_len + 4 + 3) & ~3;
        rx_offset %= RTL8139_RXBUFFER_SIZE;

        goto update_capr;
    }

    packet_len -= 4; // strip CRC
    if(packet_len > buff_len)
        return 0;

    u32 data_offset = rx_offset + 4;

    // handle wrap
    if(data_offset + packet_len <= RTL8139_RXBUFFER_SIZE) {
        memcpy(buff, (const void*)(rtl8139_rxbuffer + data_offset), packet_len);
    } else {
        u32 first = RTL8139_RXBUFFER_SIZE - data_offset;

        memcpy(buff, (const void*)(rtl8139_rxbuffer + data_offset), first);
        memcpy(buff + first, (const void*)(rtl8139_rxbuffer), packet_len - first);
    }

    rx_offset += (packet_len + 4 + 3) & ~3;
    rx_offset %= RTL8139_RXBUFFER_SIZE;

update_capr:
    outportw(rtl8139_io_base + RTL8139_REG_CAPR,
        (rx_offset >= 16) ? (rx_offset - 16) : (RTL8139_RXBUFFER_SIZE + rx_offset - 16)
    );

    return packet_len;
}
#undef _require_pci_dev_zero
