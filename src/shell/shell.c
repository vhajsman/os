#include "shell.h"
#include "string.h"
#include "kernel.h"
#include "types.h"
#include "console.h"
#include "hid/kbd.h"
#include "debug.h"
#include "fs/tar.h"
#include "command.h"
#include "initrd.h"
#include "script.h"
#include "memory/memory.h"

char WORKDIR[128];

extern void shell_command_inithandler();

char* shell_prompt = " >";

void shell_displayPrompt() {
    puts(WORKDIR);
    puts(shell_prompt);
    putc(' ');
}

void shell_parse(const char* input, char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int* tokenCount) {
    // const char* start = input;
    
    int inQuotes = 0;
    int tokenIndex = 0;
    int charIndex = 0;

    *tokenCount = 0;

    while(*input) {
        if(*input == '\"') {
            inQuotes = !inQuotes;
        } else if(*input == ' ' && !inQuotes) {
            // End of token
            if(charIndex > 0) {
                tokens[tokenIndex][charIndex] = '\0';

                tokenIndex++;
                charIndex = 0;
            }
        } else {
            if(charIndex < SHELL_MAX_TOKEN_LENGTH - 1)
                tokens[tokenIndex][charIndex++] = *input;
        }

        input++;
    }

    // Handle the last token
    if(charIndex > 0) {
        tokens[tokenIndex][charIndex] = '\0';
        tokenIndex++;
    }

    *tokenCount = tokenIndex;

    // --- Debug ---
    debug_message("Parse user input command:", "shell", KERNEL_MESSAGE);

    for(int i = 0; i < tokenIndex; i++) {
        debug_message("Parse: Token ", "shell", KERNEL_MESSAGE);
        debug_number(i, 10);

        debug_append(": ");
        debug_append(tokens[i]);
    }
}

void _puts(const char* c) {
    puts(c);
}

void shell() {
    WORKDIR[0] = '/';
    WORKDIR[1] = '\0';

    shell_command_inithandler();

    char* autorun = malloc(10240);
    if(autorun == NULL)
        return;

    memset(autorun, 0, 10240);

    if(tar_readf(initrd_data, "etc/shrc", autorun, 10240) > 0) {
        setColor(VGA_COLOR_LIGHT_BLUE);
        script_run(autorun);
        setColor(7);
    }
    
    static char uinput[SHELL_MAX_BUFFER_SIZE];
    char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH];
    int tokenCount = 0;

    char* script = malloc(10240);
    if(script == NULL)
        goto sh_exit;

    while(1) {
        if(!strcmp("exit\0", tokens[0])) {
            goto sh_exit;
        }
        
        putc('\n');

        shell_displayPrompt();
        gets(uinput, SHELL_MAX_BUFFER_SIZE);

        //puts("User input: ");
        //puts(uinput);

        memset(script, 0, 10240);
        if(tar_readf(initrd_data, uinput, script, 10240) > 0) {
            int s = script_run(autorun);
            if(s == -1)
                puts("invalid signature.\n");
                
            continue;
        }

        shell_parse(uinput, tokens, &tokenCount);
        shell_command_handle(tokens, tokenCount, (void (*)(char*)) puts, NULL);
    }

    goto sh_exit;

sh_exit:
    free(autorun);
    free(script);

    return;
}
