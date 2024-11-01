#ifndef __HOOK_H
#define __HOOK_H

#define HOOKS_MAX   256

typedef int(*hook_callback_t)(void* context);

enum kernel_hook_type {
    kernel_hook_irq,
    kernel_hook_syscall,
    kernel_hook_function
};

struct kernel_hook_entry {
    // enum kernel_hook_type type;

    char* name;

    hook_callback_t callback;
    void* arg;
};

struct kernel_hook_list {
    char* name;
    
    struct kernel_hook_entry hooks[HOOKS_MAX];
    int count;
};

// Registers a new hook. Returns its index in list or -1 if not enough space.
int hook_register(struct kernel_hook_list* list, hook_callback_t callback, char* name, void* arg);

int hook_call(struct kernel_hook_list* list);

#endif
