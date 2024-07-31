#include "shell.h"
#include "string.h"
#include "kernel.h"
#include "types.h"
#include "console.h"
#include "hid/kbd.h"

#define SHELL_MAX_BUFFER_SIZE 1024

char* shell_prompt = ">";

void shell_displayPrompt() {
    puts(shell_prompt);
    putc(' ');
}

void shell() {
    static char uinput[SHELL_MAX_BUFFER_SIZE];

    while(1) {
        putc('\n');

        shell_displayPrompt();
        gets(uinput, SHELL_MAX_BUFFER_SIZE);

        puts("User input: ");
        puts(uinput);
    }
}
