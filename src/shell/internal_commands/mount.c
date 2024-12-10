#include "internal_commands.h"
#include "shell.h"
#include "console.h"
#include "string.h"
#include "device.h"
#include "debug.h"
#include "kernel.h"
#include "fs.h"

int __mount(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {
    IGNORE_UNUSED(callback_stdin);

    if(tokc < 3) {
        callback_stdout("invalid parameters.\n");
        return 1;
    }

    char* dev_path = tokens[1];
    char* mnt_path = tokens[2];

    device_t* dev = device_get(device_findByFilename(dev_path));
    if(dev == NULL) {
        callback_stdout("device not found.\n");
        return 2;
    }

    int r = fs_mount(dev, mnt_path, "FAT32\n", 0b1111);
    if(r) {
        callback_stdout("failed to mount the device.\n");
        return 3;
    }

    callback_stdout("mount ");
    callback_stdout(dev->filename);
    callback_stdout(" -> ");
    callback_stdout(mnt_path);
    callback_stdout("\n");

    return 0;
}