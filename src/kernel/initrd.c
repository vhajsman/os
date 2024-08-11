#include "initrd.h"
#include "fs/tar.h"
#include "multiboot.h"
#include "types.h"
#include "debug.h"

u32 initrd_start;
u32 initrd_end;
u32 initrd_size;

const char* initrd_data;

void initrd_load(MULTIBOOT_INFO* mb_info) {
    if (mb_info->modules_count > 0) {
        multiboot_module_t* mod = (multiboot_module_t*) mb_info->modules_addr;

        initrd_start = mod->mod_start;
        initrd_end = mod->mod_end;
        initrd_size = initrd_end - initrd_start;

        initrd_data = (const char*) initrd_start;

        debug_message("initrd loaded.", "initrd", KERNEL_OK);

        debug_message("initrd address range: ", "initrd", KERNEL_MESSAGE);
        debug_number(initrd_start, 16);
        debug_append(" - ");
        debug_number(initrd_end, 16);
    } else {
        debug_message("initrd not found", "initrd", KERNEL_FATAL);

        initrd_start    = 0;
        initrd_end      = 0;
        initrd_size     = 0;
        initrd_data     = NULL;

        return;
    }

    tar_list(initrd_data);

    // INITRD TEST
    static char buffer[64];
    size_t s = tar_readf(initrd_data, "test.txt", buffer, 64);

    if(!s) {
        debug_message("File not fount", "initrd", KERNEL_ERROR);
        return;
    }

    debug_message("test.txt: ", "initrd", KERNEL_MESSAGE);
    debug_append(buffer);
}
