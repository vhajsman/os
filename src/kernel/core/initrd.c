#include "initrd.h"
#include "fs/tar.h"
#include "multiboot.h"
#include "types.h"
#include "debug.h"
#include "kernel.h"
#include "console.h"
#include "memory/memory.h"

u32 initrd_start;
u32 initrd_end;
u32 initrd_size;

const char* initrd_data;

void initrd_load(MULTIBOOT_INFO* mb_info) {
    if (mb_info->modules_count > 0) {
        multiboot_module_t* mod = (multiboot_module_t*) mb_info->modules_addr;

        if (mod->mod_start >= mod->mod_end || mod->mod_start == 0) {
            debug_message("Invalid initrd module range", "initrd", KERNEL_FATAL);
            
            goto loadfail;
            return;
        }

        initrd_start = mod->mod_start;
        initrd_end = mod->mod_end;
        initrd_size = initrd_end - initrd_start;

        initrd_data = (const char*) initrd_start;

        debug_message("initrd loaded.", "initrd", KERNEL_OK);

        debug_message("initrd address range: ", "initrd", KERNEL_MESSAGE);
        debug_number(initrd_start, 16);
        debug_append(" - ");
        debug_number(initrd_end, 16);

        if (initrd_size < 512) {
            debug_message("initrd size is smaller than a tar header", "initrd", KERNEL_FATAL);

            goto loadfail;
            return;
        }

        //tar_list(initrd_data);
        return;
    } else {
        debug_message("initrd not found", "initrd", KERNEL_FATAL);

        initrd_start    = 0;
        initrd_end      = 0;
        initrd_size     = 0;
        initrd_data     = NULL;

        goto loadfail;
        return;
    }

loadfail:
    kout(KERNEL_ERROR, "initrd", "could not initialize initrd,", NULL);
    return;
}

int initrd_read(const char* filename, char* buffer, size_t buffer_size) {
    return tar_readf(initrd_data, filename, buffer, buffer_size);
}
