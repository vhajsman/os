#include "internal_commands.h"
#include "shell.h"
#include "console.h"
#include "kernel.h"

int __echo(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {
    IGNORE_UNUSED(tokc);
    callback_stdout(tokens[1]);
    callback_stdout("\n");
    
    return 0;
}
