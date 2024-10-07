#include "hook.h"
#include "debug.h"
#include "console.h"

int hook_register(struct kernel_hook_list* list, hook_callback_t callback, unsigned char* name, void* arg) {
    unsigned char* error_message;

    if(callback == NULL) {
        error_message = "invalid callback function";
        goto _error;
    }

    if(list == NULL) {
        error_message = "invalid hook list";
        goto _error;
    }

    if(name == NULL)
        name = "(null)";

    if(list->count < MAX_HOOKS) {
        list->hooks[list->count].handler = handler;
        list->hooks[list->count].arg = arg;
        list->hooks[list->count].name = name;

        debug_message("Registered a new hook: \"", "hook", KERNEL_MESSAGE);
        debug_append(name);
        debug_append("\" @ ");
        debug_append(list->name);
        debug_append(":");
        debug_number(list->count, 10);

        list->count++;
        return list->count - 1;
    }

    error_message = "not enough space for a new hook";
    goto _error;

_error:
    debug_message("Could not register a new hook: \"", "hook", KERNEL_ERROR);
    debug_append(name);
    debug_append("\" on: ");
    debug_append(list->name);
    debug_append(" - ");

    debug_append(error_message);

    return -1;
}

int hook_call(struct kernel_hook_list* list) {
    if(list == NULL) {
        debug_message("Could not execute hook list: invalid list", "hook", KERNEL_ERROR);
        return -1;
    }

    debug_message("Executing \"", "hook", KERNEL_MESSAGE);
    debug_append(list->name);
    debug_append("\" hooks...");

    puts("\nExecuting \"");
    puts(list->name);
    puts("\" hooks...");

    int error_count = 0;
    int status;

    for(int i = 0; i < list->count; i++) {
        if(list->hooks[i].callback == NULL) {
            debug_message("--> Found empty hook entry on ", "hook", KERNEL_MESSAGE);
            debug_number(i, 10);

            error_count++;
            continue;
        }

        puts("\n--> executing hook: ");
        puts(list->hooks[i]->name);
        puts("... ");

        status = list->hooks[i].callback(list->hooks[i].arg);

        debug_message("--> executed hook: ", "hook", KERNEL_MESSAGE);
        debug_append(list->hooks[i].name);
        debug_append("; with code: ");
        debug_number(status, 16);

        if(status == 0) {
            colorPrint("OK", VGA_COLOR_LIGHT_GREEN);
        } else {
            colorPrint("FAILED", VGA_COLOR_LIGHT_RED);
            error_count++;
        }
    }

    puts("\nHook list execution completed.");

    debug_message("Completed running all hooks with ", "hook", KERNEL_MESSAGE);
    debug_number(error_count, 10);
    debug_append(" errors in total ");
    debug_number(list->count, 10);
    debug_append(" of executed callbacks.");
}
