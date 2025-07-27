#ifndef __PCI_H
#define __PCI_H

#include "kernel.h"

typedef union pci_dev {
    u32 bits;

    struct {
        u32 always_zero  : 2;
        u32 field_num    : 6;
        u32 function_num : 3;
        u32 device_num   : 5;
        u32 bus_num      : 8;
        u32 reserved     : 7;
        u32 enable       : 1;
    };
} pci_dev_t;

// Ports
#define PCI_CONFIG_ADDRESS      0xCF8
#define PCI_CONFIG_DATA         0xCFC

// Offset
#define PCI_VENDOR_ID            0x00
#define PCI_DEVICE_ID            0x02
#define PCI_COMMAND              0x04
#define PCI_STATUS               0x06
#define PCI_REVISION_ID          0x08
#define PCI_PROG_IF              0x09
#define PCI_SUBCLASS             0x0a
#define PCI_CLASS                0x0b
#define PCI_CACHE_LINE_SIZE      0x0c
#define PCI_LATENCY_TIMER        0x0d
#define PCI_HEADER_TYPE          0x0e
#define PCI_BIST                 0x0f
#define PCI_BAR0                 0x10
#define PCI_BAR1                 0x14
#define PCI_BAR2                 0x18
#define PCI_BAR3                 0x1C
#define PCI_BAR4                 0x20
#define PCI_BAR5                 0x24
#define PCI_INTERRUPT_LINE       0x3C
#define PCI_SECONDARY_BUS        0x09

// Device type
#define PCI_HEADER_TYPE_DEVICE  0
#define PCI_HEADER_TYPE_BRIDGE  1
#define PCI_HEADER_TYPE_CARDBUS 2
#define PCI_TYPE_BRIDGE         0x0604
#define PCI_TYPE_SATA           0x0106
#define PCI_NONE                0xFFFF

#define DEVICE_PER_BUS           32
#define FUNCTION_PER_DEVICE      32

#define PCI_VENDOR_ID_ANY 0xFFFF



u32 pci_read(pci_dev_t device, u32 field);
void pci_write(pci_dev_t device, u32 field, u32 value);

u32 pci_getDeviceType(pci_dev_t device);
u32 pci_bridge(pci_dev_t device);
u32 pci_reachEnd(pci_dev_t device);

pci_dev_t pci_scanFunction(u16 vendorId, u16 deviceId, u32 bus, u32 device, u32 function, int device_type);
pci_dev_t pci_scanDevice(u16 vendorId, u16 deviceId, u32 bus, u32 device, int device_type);
pci_dev_t pci_scanBus(u16 vendorId, u16 deviceId, u32 bus, int device_type);

pci_dev_t pci_getDevice(u16 vendorId, u16 deviceId, int device_type);

void pci_init();

#ifdef __require_pci_size_map
    extern u32 pci_size_map[100];
#endif

#ifdef __require_pci_dev_zero
    extern pci_dev_t dev_zero;
#endif

u32 pci_config_read(pci_dev_t* dev, u8 offset);
void pci_config_write16(pci_dev_t* dev, u8 offset, u16 data);
u32 pci_read_bar0(pci_dev_t* dev);

#endif
