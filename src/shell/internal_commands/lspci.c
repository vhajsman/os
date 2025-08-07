#include "console.h"
#include "shell.h"
#include "kernel.h"
#include "string.h"
#include "pci.h"

extern void shell_printError(char* kind, char* message, int start, int end);

int __lspci(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {
    IGNORE_UNUSED(tokc);
    IGNORE_UNUSED(tokens);
    IGNORE_UNUSED(callback_stdin);

    char buff_a[16];

    for(u8 bus = 0; bus < 256; bus++) {
        for(u8 device = 0; device < 32; device++) {
            for(u8 function = 0; function < 8; function++) {
                pci_dev_t dev = {0};

                dev.bus_num =       bus;
                dev.device_num =    device;
                dev.function_num =  function;

                u32 id = pci_config_read(&dev, PCI_VENDOR_ID);
                if((id & 0xFFFF) != 0xFFFF && id != 0xFFFFFFFF) {
                    u16 vendorId = id & 0xFFFF;
                    u16 deviceId = id >> 16;

//                    if(!vendorId)
//                        continue;

                    itoa(buff_a, 16, bus);          callback_stdout(buff_a);    callback_stdout(":");
                    itoa(buff_a, 16, device);       callback_stdout(buff_a);    callback_stdout(".");
                    itoa(buff_a, 16, function);     callback_stdout(buff_a);    callback_stdout(" vendorId=");
                    itoa(buff_a, 16, vendorId);     callback_stdout(buff_a);    callback_stdout(" deviceId=");
                    itoa(buff_a, 16, deviceId);     callback_stdout(buff_a);    callback_stdout("\n");
                }
            }
        }
    }

    return 0;
}
