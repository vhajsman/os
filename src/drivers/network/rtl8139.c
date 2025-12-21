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

u32 rtl8139_currentPacket = 0;

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

enum rtl8139_packetStatusBits {
    PacketROK  = 0x0,
    PacketFAE  = 0x1,
    PacketCRC  = 0x2,
    PacketLONG = 0x3,
    PacketRUNT = 0x4,
    PacketISE  = 0x5,
    PacketBAR  = 0xD,
    PacketPAM  = 0xE,
    PacketMAR  = 0xF
};

typedef struct {
    u16 status;
    u16 length;
} __attribute__((packed))  rtl8139_rxheader_t;

#define __rtl8192_virtToPhys(__virt) \
    ((u32) __virt)

const char* rtl8139_packetStatusToString(u8 packetStatus) {
    switch(packetStatus) {
        case PacketROK:  return "OK";
        case PacketFAE:  return "Frame Alignment Error";
        case PacketCRC:  return "CRC Error";
        case PacketLONG: return "Oversized Packet";
        case PacketRUNT: return "Runt Packet";
        case PacketISE:  return "Internal Symbol Error";
        case PacketBAR:  return "Broadcast Address Error";
        case PacketPAM:  return "Physical Address Multicast";
        case PacketMAR:  return "Multicast Address";
        default:         return "Unknown";
    }
}

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

    u8* buff = (u8*) kmalloc(64);
    size_t res = rtl8139_recv(buff, 64);

    debug_message("test: ", "rtl8139", KERNEL_MESSAGE);
    debug_number(res, 10);
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

rtl8139_rxheader_t rtl8139_readPacket(u32 offset) {
    rtl8139_rxheader_t header;
    header.status = *(u16*)(rtl8139_rxbuffer + offset + 0);
    header.length = *(u16*)(rtl8139_rxbuffer + offset + 2);

    return header;
}

size_t rtl8139_recv(u8* buff, size_t buff_len) {
    if(!buff || !buff_len || !rtl8139_rxbuffer) {
        debug_message("invalid params", "rtl8139", KERNEL_ERROR);
        return 0;
    }

    rtl8139_rxheader_t header = rtl8139_readPacket(rx_offset);

    if(!(header.status & 0x01)) {
        debug_message("packet not ready", "rtl8139", KERNEL_ERROR);
        return 0;
    }

    if(((header.status >> 3) & 0x0F) != PacketROK) {
        debug_message("packet error: ", "rtl8139", KERNEL_ERROR);
        debug_append(rtl8139_packetStatusToString(header.status));

        // update rx_offset depending on packet length and align to 4 bytes
        rx_offset += (header.length + 4 + 3) & ~3;
        rx_offset %= RTL8139_RXBUFFER_SIZE;

        // update CAPR
        outportw(rtl8139_io_base + RTL8139_REG_CAPR, 
            (rx_offset >= 16) ? rx_offset - 16 : RTL8139_RXBUFFER_SIZE + rx_offset - 16
        );

        return 0;
    }

    // strip CRC
    if(header.length < 4)
        return 0;
    header.length -= 4;

    if(header.length > buff_len)
        header.length = buff_len;

    u32 dataOffset = rx_offset + 4;
    if(dataOffset + header.length <= RTL8139_RXBUFFER_SIZE) {
        memcpy(buff, (const void*)rtl8139_rxbuffer + dataOffset, header.length);
    } else {
        u32 first = RTL8139_RXBUFFER_SIZE - dataOffset;
        memcpy(buff, (const void*)rtl8139_rxbuffer + dataOffset, first);
        memcpy(buff + first, (const void*)rtl8139_rxbuffer, header.length - first);
    }

    // update rx_offset and align to 4 bytes
    rx_offset += (header.length + 4 + 3) & ~3;
    rx_offset %= RTL8139_RXBUFFER_SIZE;

    // update CAPR
    outportw(rtl8139_io_base + RTL8139_REG_CAPR,
        (rx_offset >= 16) ? rx_offset - 16 : RTL8139_RXBUFFER_SIZE + rx_offset - 16
    );

    return header.length;
}

u8 rtl8139_send(const u8* data, size_t len) {
    if(!len || !data) {
        debug_message("invalid params", "rtl8139", KERNEL_ERROR);
        return 1;
    }

    if(len > RTL8139_TXBUFFER_SIZE)
        len = RTL8139_TXBUFFER_SIZE;

    static int tx_curr = 0;
    size_t tx_len = (len < 60) ? 60 : len;

    memcpy(rtl8139_txbuffer[tx_curr], data, len);
    if(tx_len > len)
        memcpy(rtl8139_txbuffer[tx_curr] + len, 0, tx_len - len);

    // sync with DMA
    asm volatile("" ::: "memory");

    u32 tsd = (tx_len & 0x1FFF); // packet length
    outportl(rtl8139_io_base + RTL8139_REG_TSAD0 + tx_curr * 4, (u32)rtl8139_txbuffer[tx_curr]);
    outportl(rtl8139_io_base + RTL8139_REG_TSD0  + tx_curr * 4, tsd);

    for(int i = 0; i < 1000000; i++) {
        u32 status = inportl(rtl8139_io_base + RTL8139_REG_TSD0 + tx_curr * 4);
        if(status & (1 << 15))
            break;

        if(status & (1 << 14)){
            debug_message("error sending packet", "rtl8139", KERNEL_ERROR);
            return 2;
        }
    }

    tx_curr = (tx_curr + 1) % RTL8139_TXBUFFER_COUNT;
    return 0;
}

#undef _require_pci_dev_zero
