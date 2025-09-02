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

    for(u8 bus = 0; bus < 255; bus++) {
        for(u8 device = 0; device < 32; device++) {
            pci_dev_t dev = {0};

            dev.bus_num =       bus;
            dev.device_num =    device;
            dev.function_num =  0x00;

            u32 id = pci_config_read(&dev, PCI_VENDOR_ID);
            if((id & 0xFFFF) == 0xFFFF)
                continue; // no device

            u32 htype = pci_config_read(&dev, PCI_HEADER_TYPE);
            bool isMulti = (htype & 0x00800000) != 0;

            for(u8 function = 0; function < (isMulti ? 8 : 1); function++) {
                dev.function_num = function;

                id = pci_config_read(&dev, PCI_VENDOR_ID);
                if((id & 0xFFFF) == 0xFFFF)
                    continue;

                u16 vendorId = id & 0xFFFF;
                u16 deviceId = id >> 0xF;

                if(vendorId == 0xFFFF || vendorId == 0x0000)
                    continue;

                itoa(buff_a, 16, bus);          callback_stdout(buff_a);    callback_stdout(":");
                itoa(buff_a, 16, device);       callback_stdout(buff_a);    callback_stdout(".");
                itoa(buff_a, 16, function);     callback_stdout(buff_a);    callback_stdout(" vendorId=");
                itoa(buff_a, 16, vendorId);     callback_stdout(buff_a);    callback_stdout(" deviceId=");
                itoa(buff_a, 16, deviceId);     callback_stdout(buff_a);    callback_stdout("\n");

                debug_messagen("raw id=", "pci", KERNEL_MESSAGE, id, 16);
            }
        }
    }

    return 0;
}
