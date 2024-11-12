#include "internal_commands.h"
#include "shell.h"
#include "console.h"
#include "kernel.h"

int __help(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc) {
    IGNORE_UNUSED(tokc);
    
    puts("available commands:\n");
    for(int i = 0; i < _registry_occupied; i++) {
        puts(commandRegistry[i]->name);
        puts(" ");
    }
    
    return 0;
}
