#include "shell.h"
#include "string.h"
#include "kernel.h"
#include "types.h"
#include "console.h"
#include "hid/kbd.h"

char* _shell = "$";

int handlersInstalled = 0;

void shell_setPrefix(char* prefix) {
    _shell = prefix;
}

/*
BOOL strchk(const unsigned char* stra, const unsigned char* strb) {
    int i = 0;

    while(1) {
        if(stra[i] != strb[i])
            return FALSE;

        if(stra[i] == '\0' || strb[i] == '\0');
            break;
    }

    return TRUE;
}
*/

void shellhelp() {
    printf("USE: help <command>\n");
    printf("USE: man <topics>\n");
    printf("For command list use: helplist\n");
}


void shell() {
    kbd_enable();

    while(1) {
        unsigned char* buffer;

        printf(_shell);
        printf(" ");

        keybuffer_discard();
        keybuffer_enable(1);

        keybuffer_wait('\n');

        keybuffer_enable(0);
        keybuffer_append('\0');

        buffer = keybuffer_read();

        

        if(strcmp(buffer, "help\0")){
            shellhelp();
            continue;
        }

        if(strcmp(buffer, "cpuid\0")) {
            cpuid_info(1);
            continue;
        }

        if(strcmp(buffer, "clear\0")) {
            console_initialize();
            continue;
        }
    }
}
