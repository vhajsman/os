#define __require_pci_dev_zero

#include "rtl8139.h"        //
#include "pci.h"            //
#include "debug.h"          //
#include "memory/memory.h"  // kmalloc, kfree
#include "string.h"         // memcpy
#include "ioport.h"         //
#include "console.h"        // puts
#include "time/timer.h"     // pit_wait
#include "irq/isr.h"        // interrupts
#include "irq/irqdef.h"
#include "cpuid.h"

pci_dev_t rtl8139_device;
u16 rtl8139_io_base; u16 rtl8139_mem_base;
u32 rtl8139_io_bar0;
u32 rtl8139_irqno;

__attribute__((aligned(256))) static u8 rtl8139_rxbuffer_static[RTL8139_RXBUFFER_SIZE];
volatile u8* rtl8139_rxbuffer = NULL;
u8* rtl8139_txbuffer[RTL8139_TXBUFFER_COUNT] = {NULL};

u8 rtl8139_mac[6];
static u32 rx_offset = 0;

enum rtl8139_irqStatusBit {
    RxOK       = 0x0001,
    RxErr      = 0x0002,
    TxOK       = 0x0004,
    TxErr      = 0x0008,
    RxOverflow = 0x0010,
    RxUnderrun = 0x0020,
    RxFIFOOver = 0x0040,
    PCSTimeout = 0x4000,
    PCIErr     = 0x8000,
};

enum rtl8139_rxconfBits {
    AcceptAllPhys   = 0x01,
    AcceptMyPhys    = 0x02,
    AcceptMulticast = 0x04,
    AcceptRunt      = 0x10,
    AcceptErr       = 0x20,
    AcceptBroadcast = 0x08,
};

#define __rtl8192_virtToPhys(__virt) \
    ((u32) __virt)

void rtl8139_reset() {
    u32 eax, ebx, ecx, edx;
    cpuid(0x01, &eax, &ebx, &ecx, &edx);

    outportb(rtl8139_io_base + RTL8139_REG_CMD, RTL8139_CMD_RESET);

    if((ecx >> 31) & 1) {
        // If running in VM(QEMU), do not wait for RST bit, but use PIT.
        // Prevents a bug found in QEMU, where the RST bit never is set
        // to low(0), causing infinite loop.

        debug_message("vm detected, using pit wait", "rtl8139", KERNEL_MESSAGE);

        pit_wait(220);
        return;
    }

    while((inportb(rtl8139_io_base + RTL8139_REG_CMD) & 0x10) != 0) {}
}

void rtl8139_irq(REGISTERS*) {
    if(!rtl8139_io_base) {
        debug_message("invalid IO base port", "rtl8139", KERNEL_ERROR);
        return;
    }

    // TODO: encode the status and execute payload
}

void rtl8139_initrx() {
    rtl8139_rxbuffer = (volatile u8*) &rtl8139_rxbuffer_static;
    memset((void*) rtl8139_rxbuffer, 0x00, RTL8139_RXBUFFER_SIZE);

    debug_message("RTL8139_RXBUFFER_SIZE=", "rtl8139", KERNEL_MESSAGE); 
    debug_number(RTL8139_RXBUFFER_SIZE, 10);

    // Set RX buffer address
    outportl(rtl8139_io_base + RTL8139_REG_RBSTART, __rtl8192_virtToPhys(rtl8139_rxbuffer));

    // Set TOK and ROK high
    outportl(rtl8139_io_base + RTL8139_REG_IMR, RxOK | TxOK);

    // Write RX configuration
    outportl(rtl8139_io_base + RTL8139_REG_RCR, 
        (1 << AcceptAllPhys)     |
        (1 << AcceptMyPhys)      |
        (1 << AcceptMulticast)   |
        (1 << AcceptBroadcast)   |
        (1 << 7)                 |   // WARP
        (2 << 11)                    // 32kb buffer
    );

    // Set RE and TE bits high
    outportb(rtl8139_io_base + RTL8139_REG_CMD, 0x0C);
}

void rtl8139_init() {
    rtl8139_device = pci_getDevice(
        RTL8139_VENDOR_ID, 
        RTL8139_DEVICE_ID, 
        PCI_HEADER_TYPE_DEVICE
    );

    if(rtl8139_device.bits == 0) { // RTL8139 not detected
        debug_message("RTL8139 not found", "rtl8139", KERNEL_ERROR);
        return;
    }

    rtl8139_io_bar0  = pci_config_read(&rtl8139_device, PCI_BAR0);   // read BAR0
    rtl8139_io_base  = rtl8139_io_bar0 & ~0x03;                      // ignore last 2 bits
    rtl8139_mem_base = rtl8139_io_bar0 & ~0x0F;
    
    // --- Enable IO and bus mastering --- 
    u16 cmd = pci_config_read(&rtl8139_device, PCI_COMMAND) & 0xFFFF;
    cmd |= (1 << 0); // IO space
    cmd |= (1 << 2); // Bus mastering

    pci_config_write16(&rtl8139_device, PCI_COMMAND, cmd);

    // Power on the device
    outportb(rtl8139_io_base + RTL8139_REG_CONFIG1, 0x00);
    rtl8139_reset();

    rtl8139_initrx();

    rtl8139_irqno = pci_config_read(&rtl8139_device, PCI_INTERRUPT_LINE) & 0xFF;
    isr_registerInterruptHandler(IRQ_BASE + rtl8139_irqno, &rtl8139_irq);

    debug_message("IRQ = IRQ_BASE + ", "rtl8139", KERNEL_MESSAGE);
    debug_number(rtl8139_irqno, 10);
    
    u32 mac_l = inportl(rtl8139_io_base + 0x00);
    u16 mac_u = inports(rtl8139_io_base + 0x04);
    rtl8139_mac[0] = mac_l >> 0;
    rtl8139_mac[1] = mac_l >> 8;
    rtl8139_mac[2] = mac_l >> 16;
    rtl8139_mac[3] = mac_l >> 24;
    rtl8139_mac[4] = mac_u >> 0;
    rtl8139_mac[5] = mac_u >> 8;

    char mac_str[18]; macToString(rtl8139_mac, mac_str);
    debug_message("MAC address: ", "rtl8139", KERNEL_MESSAGE);
    debug_append(mac_str);
}

#undef _require_pci_dev_zero
