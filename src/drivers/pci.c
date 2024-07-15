#include "pci.h"
#include "ioport.h"
#include "debug.h"
#include "console.h"
#include "string.h" // For debug macros

u32 pci_size_map[100];
pci_dev_t dev_zero = {0};

u32 pci_read(pci_dev_t device, u32 field) {
    u32 size = pci_size_map[field];

    device.field_num = (field & 0xFC) >> 2;
    device.enable = 1;

    outportl(PCI_CONFIG_ADDRESS, device.bits);

    switch(size) {
        case 1:
            return inportb(PCI_CONFIG_DATA + (field & 3));
            break;
        case 2:
            return inports(PCI_CONFIG_DATA + (field & 2));
            break;
        case 4:
            return inportl(PCI_CONFIG_DATA);
            break;

        default:
            return 0xFFFF;
    }
}

void pci_write(pci_dev_t device, u32 field, u32 value) {
    device.field_num = (field & 0xFC) >> 2;
    device.enable = 1;

    outportl(PCI_CONFIG_ADDRESS, device.bits);
    outportl(PCI_CONFIG_DATA, value);
}

u32 pci_getDeviceType(pci_dev_t device) {
    return (pci_read(device, PCI_CLASS) << 8) | pci_read(device, PCI_SUBCLASS);
}

u32 pci_bridge(pci_dev_t device) {
    return pci_read(device, PCI_SECONDARY_BUS);
}

u32 pci_reachEnd(pci_dev_t device) {
    return !pci_read(device, PCI_HEADER_TYPE);
}

pci_dev_t pci_scanFunction(u16 vendorId, u16 deviceId, u32 bus, u32 device, u32 function, int device_type) {
    pci_dev_t dev = {0};

    dev.bus_num = bus;
    dev.device_num = device;
    dev.function_num = function;

    if(pci_getDeviceType(dev) == PCI_TYPE_BRIDGE)
        pci_scanBus(vendorId, deviceId, pci_bridge(dev), device_type);

    if(device_type == -1 || device_type == pci_getDeviceType(dev)) {    // ? Device found
        u32 _deviceId = pci_read(dev, PCI_DEVICE_ID);
        u32 _vendorId = pci_read(dev, PCI_VENDOR_ID);

        if(_deviceId == deviceId && _vendorId == vendorId)
            return dev;
    }

    return dev_zero;
}

pci_dev_t pci_scanDevice(u16 vendorId, u16 deviceId, u32 bus, u32 device, int device_type) {
    pci_dev_t dev = {0};

    dev.bus_num = bus;
    dev.device_num = device;

    if(pci_read(dev, PCI_VENDOR_ID) == PCI_NONE)
        return dev_zero;

    pci_dev_t t = pci_scanFunction(vendorId, deviceId, bus, device, 0, device_type);

    if(t.bits)
        return t;

    if(pci_reachEnd(dev))
        return dev_zero;

    for(int f = 1; f < FUNCTION_PER_DEVICE; f++) {
        if(pci_read(dev, PCI_VENDOR_ID) != PCI_NONE) {
            t = pci_scanFunction(vendorId, deviceId, bus, device, f, device_type);

            if(t.bits)
                return t;
        }
    }

    return dev_zero;
}

pci_dev_t pci_scanBus(u16 vendorId, u16 deviceId, u32 bus, int device_type) {
    for(int d = 0; d < DEVICE_PER_BUS; d++) {
        pci_dev_t t = pci_scanDevice(vendorId, deviceId, bus, d, device_type);

        if(t.bits)
            return t;
    }

    return dev_zero;
}

pci_dev_t pci_getDevice(u16 vendorId, u16 deviceId, int device_type) {
    pci_dev_t t = pci_scanBus(vendorId, deviceId, 0, device_type);

    if(t.bits)
        return t;

    if(pci_reachEnd(dev_zero)) {
        debug_message("Failed to get PCI device", "PCI", KERNEL_ERROR);
        debug_messagen(" -> vendorId: ", "PCI", KERNEL_ERROR, vendorId, 16);
        debug_messagen(" -> vendorId: ", "PCI", KERNEL_ERROR, deviceId, 16);
        debug_messagen(" -> device type: ", "PCI", KERNEL_ERROR, DEVICE_PER_BUS, 10);

        puts("Failed to get PCI device.\n");

        return dev_zero;
    }

    for(int f = 1; f < FUNCTION_PER_DEVICE; f++) {
        pci_dev_t dev = {0};
        dev.function_num = f;

        if(pci_read(dev, PCI_VENDOR_ID) == PCI_NONE)
            break;

        t = pci_scanBus(vendorId, deviceId, f, device_type);

        if(t.bits)
            return t;
    }

    return dev_zero;
}

void pci_init() {
	pci_size_map[PCI_VENDOR_ID]         = 2;
	pci_size_map[PCI_DEVICE_ID]         = 2;
	pci_size_map[PCI_COMMAND]	        = 2;
	pci_size_map[PCI_STATUS]	        = 2;
	pci_size_map[PCI_SUBCLASS]	        = 1;
	pci_size_map[PCI_CLASS]		        = 1;
	pci_size_map[PCI_CACHE_LINE_SIZE]	= 1;
	pci_size_map[PCI_LATENCY_TIMER]		= 1;
	pci_size_map[PCI_HEADER_TYPE]       = 1;
	pci_size_map[PCI_BIST]              = 1;
	pci_size_map[PCI_BAR0]              = 4;
	pci_size_map[PCI_BAR1]              = 4;
	pci_size_map[PCI_BAR2]              = 4;
	pci_size_map[PCI_BAR3]              = 4;
	pci_size_map[PCI_BAR4]              = 4;
	pci_size_map[PCI_BAR5]              = 4;
	pci_size_map[PCI_INTERRUPT_LINE]	= 1;
	pci_size_map[PCI_SECONDARY_BUS]		= 1;

    debug_message("init ok", "PCI", KERNEL_OK);
}
