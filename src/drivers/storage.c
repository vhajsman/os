#include "device.h"
#include "string.h"
#include "memory/memory.h"
#include "console.h"

struct device_storage* storage_devices[MAX_STORAGE_DEVICES] = {NULL};
size_t storage_device_count = 0;

int addStorageDevice(struct device_storage* dev) {
    if(dev == NULL)
        return 1;

    if(storage_device_count >= MAX_STORAGE_DEVICES)
        return 2;

    storage_devices[storage_device_count++] = dev;
    return 0;
}

struct device_storage* getStorageDeviceByName(const char* dev_name) {
    for(size_t i = 0; i < storage_device_count; i++) {
        if(strcmp(storage_devices[i]->name, dev_name) == 0)
            return storage_devices[i];
    }

    return NULL;
}


void __debugStorageDevices() {
    for(int i = 0; i < MAX_STORAGE_DEVICES; i++) {
        debug_message("storage device ", "device", KERNEL_MESSAGE);
        debug_number(i, 10);
        debug_append(": ");

        if(storage_devices[i] == NULL)
            continue;

        debug_append(storage_devices[i]->name);
    }
}
