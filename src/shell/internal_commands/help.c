#include "internal_commands.h"
#include "shell.h"
#include "console.h"
#include "kernel.h"

int __help(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {
    IGNORE_UNUSED(tokc);
    
    callback_stdout("available commands:\n");
    for(int i = 0; i < _registry_occupied; i++) {
        callback_stdout(commandRegistry[i]->name);
        callback_stdout(" ");
    }
    
    return 0;
}
