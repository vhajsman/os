#ifndef __DEVICE_H
#define __DEVICE_H

#include "types.h"

#define MAX_STORAGE_DEVICES 16

extern struct device_storage* storage_devices[MAX_STORAGE_DEVICES];
extern size_t storage_device_count;

struct device_storage {
    char name[8];
    u64  capacity;
    u32  sector_size;

    void* ctx;

    int(*callback_readSector)(void* ctx, u32 sector, void* buffer);
    int(*callback_writeSector)(void* ctx, u32 sector, void* buffer);
};

int addStorageDevice(struct device_storage* dev);
struct device_storage* getStorageDeviceByName(const char* dev_name);
void __debugStorageDevices();

// int mountStorageDevice(const char* dev_name, const char* fs_type);

#endif