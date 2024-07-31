#include "internal_commands.h"
#include "shell.h"
#include "console.h"

int __echo(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc) {
    puts(tokens[1]);
    putc('\n');
    
    return 0;
}
