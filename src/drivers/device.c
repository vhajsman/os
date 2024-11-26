#include "device.h"
#include "string.h"

device_t* kernel_deviceList[MAX_DEVICES] = {NULL};

int device_findfreeslot() {
    for(int i = 0; i < MAX_DEVICES; i++) {
        if(kernel_deviceList[i] == NULL)
            return i;
    }

    return -1;
}

int device_findByFilename(const char* filename) {
    if(filename == NULL)
        return -1;

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

    return kernel_deviceList[index];
}

int device_append(device_t* dev) {
    int slot = device_findfreeslot();
    if(slot < 0)
        return 1;
    
    kernel_deviceList[slot] = dev;
    return 0;
}

void device_remove(int index) {
    kernel_deviceList[index] = NULL;
}

void device_uniquify(char* filename, size_t buffer_size) {
    if(filename == NULL || buffer_size <= strlen(filename) + 1)
        return NULL;

    char suffix = 'a';
    size_t olen = strlen(filename);

    for(int i = 0; i < MAX_DEVICES; i++) {
        if(kernel_deviceList[i] != NULL && (strcmp(filename, kernel_deviceList[i]->filename) != 0)) {
            if(olen + 2 >= buffer_size)
                return;
            
            filename[olen] = suffix++;
            filename[olen + 1] = '\0';
            
            i = -1;
        }
    }
}
