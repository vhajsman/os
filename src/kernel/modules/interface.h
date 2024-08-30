#ifndef __INTERFACE_H
#define __INTERFACE_H

#include "types.h"

#define INTERFACE_SLOT_COUNT    512

struct kernel_interface_callflags {
    u8 flags;

    /*
        KERNEL INTERFACE EXIT CODES CHEATSHEET
        
        Exit code   Meaning
        ----------------------------------------
        0           OK
        1           Partially OK / OK but requires attention
        2           OK but functionallity is limited
        3           Initialization failed
        4           Waiting for dependencies to init
    */
    int exit_code;
};

typedef struct kernel_interface {
    char* name_short;       // Interface name for kernel, i.e. KBD for keyboard
    char* name_friendly;    // Interface name for when it has to be user-friendly

    void (*init)  (struct kernel_interface_callflags* callflags);
    void (*deinit) (struct kernel_interface_callflags* callflags);

    int status;
} kernel_interface_t;

int kernel_interface_findByName(char* name_short);
struct kernel_interface* kernel_interface_get(int slot);

void kernel_interface_init(int slot, struct kernel_interface_callflags* callflags);
void kernel_interface_deinit(int slot, struct kernel_interface_callflags* callflags);

int kernel_interface_load(int slot, struct kernel_interface* interface);
int kernel_interface_unload(int slot);

void interface_test();

enum kernel_interface_msg {
    KERNEL_INTERFACE_PENDING                = 0,
    KERNEL_INTERFACE_OKAY                   = 1,
    KERNEL_INTERFACE_REQUIRES_ATTENTION     = 2,
    KERNEL_INTERFACE_LIMITED_FUNCTIONALITY  = 3,
    KERNEL_INTERFACE_FAILED                 = 4,
    KERNEL_INTERFACE_WAITING_FOR_DEPS       = 5
};

#endif
