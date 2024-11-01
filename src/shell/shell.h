#ifndef __SHELL_H
#define __SHELL_H

#include "types.h"

#define SHELL_MAX_BUFFER_SIZE 1024
#define SHELL_MAX_TOKENS 128
#define SHELL_MAX_TOKEN_LENGTH 256

#define GET_SELF_ARGUMENT(n) (tokens[n + 1])
#define GET_COMMAND_NAME() tokens[0]

void shell();

extern char WORKDIR[128];

#endif
