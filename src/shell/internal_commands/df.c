#include "internal_commands.h"
#include "shell.h"
#include "console.h"
#include "string.h"
#include "device.h"
#include "debug.h"
#include "kernel.h"

int __df(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {
    IGNORE_UNUSED(callback_stdin);
    IGNORE_UNUSED(tokc);
    IGNORE_UNUSED(tokens);

    for(int i = 0; i < MAX_DEVICES; i++) {
        device_t* dev = device_get(i);
        if(dev == NULL || dev->type != DEVICE_STORAGE)
            continue;
        
        callback_stdout("storage device: ");
        callback_stdout(dev->filename);
        callback_stdout(": ");
        
        char c[12];
        itoa(c, 10, dev->capacity);
        callback_stdout(c);
        callback_stdout(" bytes, ");
        
        //callback_stdout("not mounted\n");

        if(fs_ismounted(dev)) {
            char* mount = fs_findMntByDevice(dev);

            if(mount == NULL) {
                callback_stdout("mount point not valid\n");
                continue;
            }

            callback_stdout("mount point: ");
            callback_stdout(mount);
        } else {
            callback_stdout("not mounted");
        }

        callback_stdout("\n");
    }


    return 0;
}