#include "internal_commands.h"
#include "shell.h"
#include "console.h"
#include "string.h"
#include "device.h"
#include "debug.h"
#include "kernel.h"
#include "fs.h"

int unmountByMountPoint(char* path, void (*callback_stdout) (char*)) {
    int r = fs_unmount(path);
    if(r != 0) {
        callback_stdout("failed to unmount device from: ");
        callback_stdout(path);

        return 100 + r;
    }

    return 0;
}

int unmountByDevicePath(char* path, void (*callback_stdout) (char*)) {
    device_t* dev = device_get(device_findByFilename(path));
    if(dev == NULL) {
        callback_stdout("device not found: ");
        callback_stdout(path);

        return 2;
    }

    if(dev->type != DEVICE_STORAGE) {
        callback_stdout("invalid device type: ");
        callback_stdout(path);

        return 3;
    }

    int mount_count = dev->mount_count;
    if(mount_count == 0) {
        callback_stdout("device not mounted: ");
        callback_stdout(path);

        return 4;
    }

    for(int i = 0; i < mount_count; i++) {
        int r = fs_unmount(fs_findMntByDevice(dev));

        if(r != 0) {
            callback_stdout("failed to unmount device: ");
            callback_stdout(path);

            return 100 + r;
        }
    }

    return;
}

int __unmount(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {
    IGNORE_UNUSED(callback_stdin);

    if(tokc != 2) {
        callback_stdout("invalid parameters");
        return 1;
    }

    char* path = tokens[1];

    if(strncmp("/devices", path, 9) == 0)
        return unmountByDevicePath(path, callback_stdout);
    
    int ur = unmountByMountPoint(path, callback_stdout);
    // if(ur == 1000) {
    //     callback_stdout("WARNING: device successfully unmounted from the destination mount point, but is still mounted on more mount points.");
    //     return 0;
    // }

    return ur;
}