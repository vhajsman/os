#ifndef __SHELL_COMMAND_H
#define __SHELL_COMMAND_H

#include "shell.h"

int shell_command_handle(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokenCount);

#endif
