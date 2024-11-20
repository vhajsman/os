#include "console.h"
#include "shell.h"
#include "debug.h"
#include "string.h"
#include "com/serial.h"
#include "kernel.h"

extern void shell_printError(char* kind, char* message, int start, int end);

int __krnldbg(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {
    IGNORE_UNUSED(tokc);
    
    if(!strcmp("--insert-breakpoint\0", tokens[1])) {
        debug_breakpoint();
        return 0;
    }

    if(!strcmp("--start\0", tokens[1])) {
        if(!strcmp("COM1\0", tokens[2])) {
            debug_setPort(COM1);
            return 0;
        }

        if(!strcmp("COM2\0", tokens[2])) {
            debug_setPort(COM2);
            return 0;
        }

        shell_printError("IOError", "Not a valid serial port", 17, 5);
        return 2;
    }

    if(!strcmp("--stop\0", tokens[1])) {
        debug_setPort(0);
        return 0;
    }

    if(!strcmp("--verbose\0", tokens[1])) {
        if(!strcmp("true\0", tokens[2])) {
            debug_setVerbose(1);
            return 0;
        }

        if(!strcmp("false\0", tokens[2])) {
            debug_setVerbose(0);
            return 0;
        }

        shell_printError("TypeError", "Excepted a boolean value", 18, 4);
        return 1;
    }

    shell_printError("", "Invalid sub-command", 9, 9 + strlen(tokens[1]));
    return 1;
}