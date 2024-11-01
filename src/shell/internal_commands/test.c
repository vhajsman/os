#include "console.h"
#include "shell.h"
#include "kernel.h"

int __test(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc) {
    IGNORE_UNUSED(tokc);
    IGNORE_UNUSED(tokens);

    puts("Hello world.");
    return 0;
}
