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

        initrd_start    = NULL;
        initrd_end      = NULL;
        initrd_size     = NULL;
        initrd_data     = NULL;
    }
}
