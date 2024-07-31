#include "console.h"
#include "shell.h"
#include "kernel.h"
#include "string.h"

extern void shell_printError(char* kind, char* message, int start, int end);

int __hwinfo(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc) {
    if(!strcmp("cpuid\0", tokens[1])) {
        cpuid_info(1);
        return 0;
    }

    shell_printError("Error", "Invalid sub-command", 6, strlen(tokens[1]) - 1);
    return 2;
}
