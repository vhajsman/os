#include "device.h"
#include "string.h"
#include "debug.h"

device_t* kernel_deviceList[MAX_DEVICES] = {NULL};

int device_findfreeslot() {
    for(int i = 0; i < MAX_DEVICES; i++) {
        if(kernel_deviceList[i] == NULL)
            return i;
    }

    return -1;
}

int device_findByFilename(const char* filename) {
    for(int i = 0; i < MAX_DEVICES; i++) {
        if(kernel_deviceList[i] == NULL)
            continue;

        if(strcmp(filename, kernel_deviceList[i]->filename) != 0)
            return i;
    }

    return -1;
}

device_t* device_get(int index) {
    if(index < 0 || index >= MAX_DEVICES)
        return NULL;
    
    device_t* dev = kernel_deviceList[index];

    if(dev == NULL)
        return NULL;

    debug_message("found device: ", "device", KERNEL_MESSAGE);
    debug_append(dev->filename);

    return dev;
}

int device_append(device_t* dev) {
    int slot = device_findfreeslot();
    if(slot < 0)
        return 1;
    
    kernel_deviceList[slot] = dev;
    debug_message("registered device: ", "device", KERNEL_OK);
    debug_append(dev->filename);
    
    return 0;
}

void device_remove(int index) {
    kernel_deviceList[index] = NULL;
}

void device_uniquify(char* filename, size_t buffer_size) {
    if(filename == NULL || buffer_size < 2)
        return;
    
    size_t base_len = strlen(filename);
    if(base_len + 2 >= buffer_size) {
        debug_message("Filename buffer too small in device_uniquify.", "device", KERNEL_ERROR);
        return;
    }

    for(char suffix = 'a'; suffix <= 'z'; suffix++) {
        filename[base_len - 1] = suffix;
        filename[base_len] = '\0';

        int unique = 1;
        for(int i = 0; i < MAX_DEVICES; i++) {
            if(kernel_deviceList[i] != NULL && strcmp(filename, kernel_deviceList[i]->filename) == 0) {
                unique = 0;
                break;
            }
        }

        if(unique)
            return;
    }
    
    debug_message("Failed to uniquify filename. Too many conflicts.", "device", KERNEL_ERROR);
}
