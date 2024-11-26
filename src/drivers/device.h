#ifndef __DEVICE_H
#define __DEVICE_H

#include "types.h"

#define MAX_DEVICES          32
#define DEVICE_NAME_MAX_SIZE 64

typedef enum kernel_device_type {
    DEVICE_NULL,
    DEVICE_RAW,
    DEVICE_PORT,
    DEVICE_STORAGE
} device_type_t;

typedef struct kernel_device {
    char filename[DEVICE_NAME_MAX_SIZE];
    char* model;
    device_type_t type;

    u32 capacity;
    union {
        u32 sectorSize;
        size_t blockSize;
    };
    
    void* context;
    size_t context_size;

    int (*mReadSector) (device_t* dev, void* context, u32 sector, void* buffer);
    int (*mWriteSector) (device_t* dev, void* context, u32 sector, void* buffer);
} device_t;

int device_findByFilename(const char* filename);
device_t* device_get(int index);
int device_append(device_t* dev);
void device_remove(int index);
void device_uniquify(char* filename, size_t buffer_size);

#endif