#include "console.h"
#include "shell.h"
#include "kernel.h"

int __test(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {
    IGNORE_UNUSED(tokc);
    IGNORE_UNUSED(tokens);
    IGNORE_UNUSED(callback_stdin);

    callback_stdout("Hello world.");
    return 0;
}
