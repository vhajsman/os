#include "interface.h"
#include "debug.h"
#include "string.h"
#include "console.h"
#include "video/vga.h"

void kernel_interface_msg_pending(int init_deinit, struct kernel_interface* interface);
void kernel_interface_msg_done(enum kernel_interface_msg status);

struct kernel_interface* kernel_interface_list[INTERFACE_SLOT_COUNT] = { NULL };

int kernel_interface_findByName(char* name_short) {
    for(int i = 0; i < INTERFACE_SLOT_COUNT; i++) {
        if(!strcmp(kernel_interface_list[i]->name_short, name_short))
            return i;
    }

    debug_message("Could not find interface: ", "module", KERNEL_ERROR);
    debug_append(name_short);

    return -1;
}

struct kernel_interface* kernel_interface_get(int slot) {
    return (slot < 0 || slot > INTERFACE_SLOT_COUNT - 1) ? NULL : kernel_interface_list[slot];
}

void kernel_interface_init(int slot, struct kernel_interface_callflags* callflags) {
    // TODO: add proper error handling.

    if(slot < 0 || slot > INTERFACE_SLOT_COUNT - 1 || !callflags) {
        debug_message("invalid interface call request", "module", KERNEL_ERROR);
        return;
    }

    struct kernel_interface* interface = kernel_interface_get(slot);
    if(interface == NULL) {
        debug_message("interface slot empty: ", "module", KERNEL_ERROR);
        debug_number(slot, 10);

        callflags->exit_code = -101;
        return;
    }

    kernel_interface_msg_pending(1, interface);

    // Do not handle NULL initializer pointer error, since it could be handled from
    // kernel_interface_load already.

    interface->init(callflags);

    debug_message("handled interface on slot ", "module", KERNEL_MESSAGE);
    debug_number(slot, 10);
    debug_append(" with exit code: ");
    debug_number(callflags->exit_code, 10);

    interface->status = 1000;

    kernel_interface_msg_done(callflags->exit_code + 1);
}

void kernel_interface_deinit(int slot, struct kernel_interface_callflags* callflags) {
    // TODO: add proper error handling.

    if(slot < 0 || slot > INTERFACE_SLOT_COUNT - 1 || !callflags) {
        debug_message("invalid interface call request", "module", KERNEL_ERROR);
        return;
    }

    struct kernel_interface* interface = kernel_interface_get(slot);
    if(interface == NULL) {
        debug_message("interface slot empty: ", "module", KERNEL_ERROR);
        debug_number(slot, 10);

        callflags->exit_code = -101;
        return;
    }

    kernel_interface_msg_pending(0, interface);

    // Do not handle NULL initializer pointer error, since it could be handled from
    // kernel_interface_load already.

    interface->deinit(callflags);

    debug_message("handled interface on slot ", "module", KERNEL_MESSAGE);
    debug_number(slot, 10);
    debug_append(" with exit code: ");
    debug_number(callflags->exit_code, 0);

    interface->status = 0001;

    kernel_interface_msg_done(callflags->exit_code + 1);
}

int kernel_interface_integrity(struct kernel_interface* interface) {
    if(interface->name_short == NULL || interface->init == NULL || interface->deinit == NULL)
        return 1;

    return 0;
}

int kernel_interface_findFreeSlot() {
    for(int i = 0; i < INTERFACE_SLOT_COUNT; i++) {
        if(kernel_interface_list[i] == NULL) {
            return i;
        }
    }

    return -1;
}

int kernel_interface_load(int slot, struct kernel_interface* interface) {
    if(kernel_interface_integrity(interface)) {
        debug_message("interface integrity check failed", "module", KERNEL_ERROR);
        return 1;
    }

    if(slot < 0) {
        slot = kernel_interface_findFreeSlot();

        if(slot < 0) {
            debug_message("not enough free slots for interface load: ", "module", KERNEL_ERROR);
            debug_append(interface->name_short);

            return 2;
        }
    } else {
        if(kernel_interface_list[slot] != NULL) {
            debug_message("interface slot already occupied: ", "module", KERNEL_ERROR);
            debug_number(slot, 10);

            return 2;
        }
    }

    if(slot > INTERFACE_SLOT_COUNT - 1) {
        debug_message("slot not in available range for: ", "module", KERNEL_ERROR);
        debug_append(interface->name_short);

        return 3;
    }

    kernel_interface_list[slot] = interface;
    interface->status = 0001;

    debug_message("load interface: ", "module", KERNEL_OK);
    debug_append(interface->name_short);

    return 0;
}

int kernel_interface_unload(int slot) {
    if(kernel_interface_list[slot] == NULL) {
        debug_message("interface not even loaded: ", "module", KERNEL_WARNING);
        debug_number(slot, 10);

        return 1;
    }

    struct kernel_interface* interface = kernel_interface_get(slot);

    kernel_interface_list[slot] = NULL;
    interface->status = 0000;

    return 0;
}

char* kernel_interface_msg_tags[6] = {
    " WAIT ",
    " DONE ",
    " WARN ",
    " PART ",
    "FAILED",
    " DEPS "
};

enum vga_color kernel_interface_msg_color[6] = {
    VGA_COLOR_LIGHT_GREY,
    VGA_COLOR_LIGHT_GREEN,
    VGA_COLOR_LIGHT_BROWN,
    VGA_COLOR_LIGHT_BROWN,
    VGA_COLOR_LIGHT_RED,
    VGA_COLOR_LIGHT_GREY
};

u8 kernel_interface_msg_y = 0;

void kernel_interface_msg_pending(int init_deinit, struct kernel_interface* interface) {
    putc('\n');
    putc('[');
    colorPrint(kernel_interface_msg_tags[KERNEL_INTERFACE_PENDING], kernel_interface_msg_color[KERNEL_INTERFACE_PENDING]);
    putc(']');
    putc(' ');

    puts(init_deinit ? "Intialize " : "Deinitialize ");

    colorPrint(interface->name_friendly == NULL ? interface->name_short : interface->name_friendly, VGA_COLOR_WHITE);

    kernel_interface_msg_y = console_position.y;
}

void kernel_interface_msg_done(enum kernel_interface_msg status) {
    console_position.x = 1;
    console_position.y = kernel_interface_msg_y;

    colorPrint(kernel_interface_msg_tags[status], kernel_interface_msg_color[status]);

    console_posx = 0;
}

void kernel_interface_inithandler() {
    kernel_interface_msg_y = console_position.y;
}
