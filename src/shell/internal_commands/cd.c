#include "internal_commands.h"
#include "shell.h"
#include "console.h"
#include "string.h"

extern void shell_printError(char* kind, char* message, int start, int end);

int __cd(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc) {
    if(tokc == 1) {
        puts(WORKDIR);
        return 0;
    }

    if(strlen(tokens[1]) >= 128) {
        shell_printError("Fs-Error", "Filename too long", 0, 0);
        return 1;
    }

    char* target = tokens[1];

    if((target[0] == '.' && target[1] == '/') || (target[0] != '.' && target[0] != '/')) {
        strcat(WORKDIR, target);
        strncpy(WORKDIR, target, 128);

        return 0;
    }

    strncpy(WORKDIR, target, 128);
    return 0;
}
