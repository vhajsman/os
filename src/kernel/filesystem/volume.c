#include "fs.h"
#include "debug.h"
#include "string.h"

filesystemptr_t fs_list[VOLUME_MAX];

file_t volume_openf(const char* filename) {
    file_t file;

    if(filename) {
        unsigned char device = 'a';
        char* fname = (char*) filename;

        if(fname[1] == ':') {
            device = filename[0];
            fname += 2;
        } else {
            debug_message("Volume not specified. Defaulting to volume A.", "Volume", KERNEL_INFORMATION);
        }

        if(fs_list[device - 'a']) {
            file = fs_list[device - 'a']->open(fname);
            file.device = device;

            return file;
        }

        debug_message("Open file ", "Volume", KERNEL_MESSAGE);
        debug_append(filename);
        debug_append(" on volume");
        debug_append(device);
    }

    file.flags = FS_INVALID;
    return file;
}

void volume_registerFs(filesystemptr_t fs, unsigned int device_id) {
    if(device_id < VOLUME_MAX && fs) {
        debug_message("Register new volume ", "Volume", KERNEL_OK);
        debug_append((const char) device_id + 'a');

        if(fs_list[device_id])
            debug_message("Registred volume replaced", "Volume", KERNEL_WARNING);
        
        fs_list[device_id] = fs;
    }
}