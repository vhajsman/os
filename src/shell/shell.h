#ifndef __SHELL_H
#define __SHELL_H

#include "types.h"

#define MAX_TOKS    30

struct shell_parseout {
    int tok_count;
    char* tok_str;
    char* tok_arr[MAX_TOKS];
};

struct shell_execcall {
    char* prompt;

    union {
        char* executable;
        char* internalCommand;
        char* identifier;
        char* call;
    };

    size_t argc;    // Argument count
    char** argv;    // Arguments
};

int shell_handleUserInput(char* userInput);

void shell();
void shell_setPrefix(char* prefix);

char* shell_getPrompt();

#define SHELL_CALLPTR(X) \
    int (*X)(const char* callParams, u8* uref)


#endif
