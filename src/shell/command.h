#ifndef __SHELL_COMMAND_H
#define __SHELL_COMMAND_H

#include "shell.h"

int shell_command_handle(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokenCount, void (*callback_stdout) (char*), char* (*callback_stdin) (void));


// === COMMAND REGISTRY ===

#define MAX_INTERNAL_COMMANDS   256

#define CREATE_INTERNAL_COMMAND(__ARGC_MIN, __ARGC_MAX, __NAME, __ENTRY)    \
    (struct shell_command) {                                                \
        .argc_min = (__ARGC_MIN),                                           \
        .argc_max = (__ARGC_MAX),                                           \
        .name = (__NAME),                                                   \
        .entry = (__ENTRY)                                                  \
    }

struct shell_command {
    u8 argc_min;    // If 0, no minimum is set
    u8 argc_max;    // If 0, no maximum is set

    char* name;

    int (*entry)(
        char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], 
        int tokc,
        void (*callback_stdout) (char*),
        char* (callback_stdin) (void)
    );
};

extern struct shell_command* commandRegistry[MAX_INTERNAL_COMMANDS];
extern int _registry_occupied;

void registerCommand(struct shell_command* command);
struct shell_command* findCommand(char* commandName);

#endif
