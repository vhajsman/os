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

int strchk(const unsigned char* stra, const unsigned char* strb) {
    int i = 0;

    while(1) {
        if(stra[i] != strb[i])
            return 0;

        if(stra[i] == '\0' && strb[i] == '\0')
            break;
    }

    return 1;
}

void shellhelp() {
    printf("USE: help <command>\n");
    printf("USE: man <topics>\n");
    printf("For command list use: helplist\n");
}


void shell() {
    colorPrint("Welcome to CubeBox OS shell! Use 'help' command to get help.\n", vga_entryColor(VGA_COLOR_LIGHT_MAGENTA, 0));
    kbd_enable();

    while(1) {
        char* buffer;

        printf(_shell);
        printf(" ");

        keybuffer_discard();
        keybuffer_enable(1);

        keybuffer_wait('\n');

        keybuffer_append('\0');
        keybuffer_enable(0);

        buffer = keybuffer_read();

        putc('\n');

        /*
        if(!strcmp(buffer, "help")){
            shellhelp();
            continue;
        }

        if(!strcmp(buffer, "cpuid")) {
            cpuid_info(1);
            continue;
        }

        if(!strcmp(buffer, "clear")) {
            console_initialize();
            continue;
        }
        */

       shell_handleUserInput(buffer);
    }
}

char* shell_getPrompt() {
    if(_shell[0] == '\0')
        return NULL;

    return _shell;
}
