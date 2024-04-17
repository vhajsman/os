#ifndef __SHELL_H
#define __SHELL_H

#include "types.h"

void shell();
void shell_setPrefix(char* prefix);

#define SHELL_CALLPTR(X) \
    int (*X)(const char* callParams, u8* uref)


#endif
