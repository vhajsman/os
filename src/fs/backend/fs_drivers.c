#include "fs/fs.h"
#include "debug.h"
#include "string.h"

fs_driver_entry_t fs_drivers[MAX_FS_DRIVERS] = {0};

int fs_driver_register(const char* name, fs_mount_callback_t mount_callback) {
    if(name == NULL || mount_callback == NULL) {
        debug_message("fs_driver_register(): invalid parameters", "fs", KERNEL_ERROR);
        return 1;
    }

    for(int i = 0; i < MAX_FS_DRIVERS; i++) {
        if(fs_drivers[i].name != NULL)
            continue;

        fs_drivers[i].name = name;
        fs_drivers[i].mount_callback = mount_callback;

        return 0;
    }

    return 1;
}

fs_mount_callback_t fs_driver_getCallback(const char* name) {
    if(name == NULL) {
        debug_message("fs_driver_getCallback(): invalid parameters", "fs", KERNEL_ERROR);
        return NULL;
    }

    for(int i = 0; i < MAX_FS_DRIVERS; i++) {
        if(fs_drivers[i].name && strcmp(fs_drivers[i].name, name) == 0)
            return fs_drivers[i].mount_callback;
    }

    return NULL;
}
