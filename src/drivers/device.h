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
    device_type_t type;

    int (*mReadSector) (void* context, u32 sector, void* buffer);
    int (*mWriteSector) (void* context, u32 sector, void* buffer);
} device_t;

#endif