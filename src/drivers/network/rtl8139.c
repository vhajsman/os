#define __require_pci_dev_zero

#include "rtl8139.h"
#include "pci.h"
#include "debug.h"
#include "memory/memory.h"
#include "ioport.h"
#include "console.h"

pci_dev_t rtl8139_device;
u16 rtl8139_io_base;
u32 rtl8139_io_bar0;

u8* rtl8139_rxbuffer = NULL;

u8 rtl8139_mac[6];

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
    rtl8139_command(RTL8139_CMD_RESET);
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


    // --- Set up RX buffer --- 
    rtl8139_rxbuffer = (u8*) kmalloc((size_t) RTL8139_RXBUFFER_SIZE);
    if(rtl8139_rxbuffer == NULL) {
        debug_message("not enough memory for RX buffer", "rtl8139", KERNEL_ERROR);
        return;
    }

    outportl(rtl8139_io_base + RTL8139_REG_RBSTART, (u32) rtl8139_rxbuffer);

    debug_message("RX buffer size: ", "rtl8139", KERNEL_MESSAGE);
    debug_number(RTL8139_RXBUFFER_SIZE, 10);


    // --- Enable RX, TX --- 
    rtl8139_command(RTL8139_RX_EN | RTL8139_TX_EN);


    // --- Do RX configuration --- 
    u32 rxconfig =  (1 << 0)    |   // accept broadcast
                    (1 << 1)    |   // accept multicast
                    (1 << 2)    |   // accept runt
                    (1 << 4)    ;   // accept physical match

    outportl(rtl8139_io_base + RTL8139_REG_RCR, rxconfig);

    
    // --- Get MAC address --- 
    for(int i = 0; i < 6; i++)
        rtl8139_mac[i] = inportb(rtl8139_io_base + i);

    char mac_str[18];
    mac2str(rtl8139_mac, mac_str);

    puts("RTL8139 physical address: ");
    puts(mac_str);
    puts("\n");
}

#undef _require_pci_dev_zero
