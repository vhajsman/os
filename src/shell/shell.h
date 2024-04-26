#ifndef __SHELL_H
#define __SHELL_H

#include "types.h"

#define MAX_TOKS    30

struct shell_parseout {
    int tok_count;
    char* tok_str;
    char* tok_arr[MAX_TOKS];
};

int shell_handleUserInput(char* userInput);

void shell();
void shell_setPrefix(char* prefix);

#define SHELL_CALLPTR(X) \
    int (*X)(const char* callParams, u8* uref)


#endif
