#include "shell.h"
#include "command.h"
#include "console.h"
#include "video/vga.h"
#include "string.h"
#include "debug.h"
#include "kernel.h"

#define GET_SELF_ARGUMENT(n) (tokens[n + 1])
#define GET_COMMAND_NAME() tokens[0]

u8 shell_errorColor;
u8 shell_hintColor;

void shell_printError(char* kind, char* message, int start, int end) {
    if(end != 0) {
        for(int i = 0; i < start + 2; i++)
            putc(' ');

        for(int i = 0; i < end + 2; i++)
            colorPrint("^", shell_errorColor);

        putc('\n');

        for(int i = 0; i < start + 2; i++)
            putc(' ');
    }
        
    colorPrint(kind, shell_errorColor);
    colorPrint(": ", shell_errorColor);
    colorPrint(message, shell_errorColor);
}

int shell_command_handle(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokenCount) {
    if(!tokenCount) {
        debug_message("Empty command not handled", "shell", KERNEL_MESSAGE);
        return 0;
    }

    debug_message("Executing: ", "shell", KERNEL_MESSAGE);
    debug_append(GET_COMMAND_NAME());
    debug_append(" with argc=");
    debug_number(tokenCount - 1, 10);

    if(!strcmp("null\0", GET_COMMAND_NAME()))
        return 0;

    if(!strcmp("echo\0", GET_COMMAND_NAME())) {
        if(tokenCount > 2) {
            shell_printError("CommandError", "Too much arguments in call", 0, 0);
            return 1;
        }

        if(tokenCount < 2) {
            shell_printError("CommandError", "Too few arguments in call", 5, 8);
            return 1;
        }

        puts(GET_SELF_ARGUMENT(0));
        return 0;
    }

    if(!strcmp("hwinfo\0", GET_COMMAND_NAME())) {
        if(tokenCount > 2) {
            shell_printError("CommandError", "Too much arguments in call", 0, 0);
            return 1;
        }

        if(tokenCount < 2) {
            shell_printError("CommandError", "Too few arguments in call", 5, 8);
            return 1;
        }

        if(!strcmp("cpuid\0", GET_SELF_ARGUMENT(0))) {
            cpuid_info(1);
            return 0;
        }

        shell_printError("Error", "Invalid sub-command", 6, strlen(GET_SELF_ARGUMENT(0)) - 1);
        return 2;
    }

    shell_printError("ParseError", "Internal command not found", 0, strlen(GET_COMMAND_NAME()) - 1);
    return 1;
}

void shell_command_inithandler() {
    shell_errorColor = vga_entryColor(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    shell_hintColor = vga_entryColor(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
}
