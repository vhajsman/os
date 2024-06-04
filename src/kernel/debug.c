#include "debug.h"
#include "com/serial.h"
#include "console.h"
#include "type/string.h"

int _verbose = 0;
int _timestamp = 0; // UNDONE
int _debug_enable = 0;

u16 _port;

#define __check_debug   \
    if(!_debug_enable)  \
        return

char* _levelsStrings[10] = {
    "[   ***   ]\0",
    "[ NOTE    ]\0",
    "[ NOTE !! ]\0",
    "[ WARNING ]\0",
    "[ ERROR   ]\0",
    "[ FATAL   ]\0",
    "[ VERBOSE ]\0",
    "[ OKAY    ]\0",
    "[ FAIL    ]\0",
    "[ PENDING ]\0"
};

void debug_setVerbose(int verbose) {
    __check_debug;

    _verbose = verbose ? 1 : 0;
    debug_message("Verbose flag enabled", 0, KERNEL_VERBOSE);
}

void debug_setPort(u16 port) {
    if(port == 0) {
        _debug_enable = 0;
        return;
    }
    
    if(port != COM1 && port != COM2) {
        _debug_enable = 0;
        printf("Error: Debug port not supported");
        return;
    }

    _port = port;
    _debug_enable = 1;

    colorPrint("Port ", vga_entryColor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    colorPrint(port == COM1 ? "COM1" : "COM2", vga_entryColor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    colorPrint(" used as a debug port. ", vga_entryColor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    colorPrint("You should be able to use debug features after connecting the debugger via this port.\n", vga_entryColor(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));

    // ! SERIAL COMMUNICATION SHOULD BE ALREADY INIT!!!
    // serial_init();
}

void debug_append(const  char* data) {
    __check_debug;
    serial_write(_port, data);
}

#define __INTERFACE_MAX_CHARS   12

void debug_message(const  char* message, const  char* interface, enum kernel_statusLevels level) {
    __check_debug;

    if(level == KERNEL_VERBOSE && !_verbose)
        // * Message level verbose, but verbosity not allowed.
        return;
    
    //debug_append("\n");
    debug_append("\n\rDEBUG: ");
    
    if(message == NULL) {
        debug_append("\n\r");
        return;
    }

    debug_append(_levelsStrings[level]);

    if(interface != NULL) {
        debug_append("[");
        debug_append(interface);
        debug_append("] ");
    }

    /*for(int i = 0; i < strlen(message); i ++) {
        if(message[i] == '\n') {
            debug_append("\n\r");

            // ? "DEBUG: "          07
            // ? INTERFACE          14
            // ? LEVEL              12
            // ? ---------------------
            // ? TOTAL              33

            for(int j = 0; j < 33; j ++) 
                debug_append(" ");
        } else {
            debug_append(message[i]);
        }
    }*/

    debug_append(message);
}

#define __SEPARATOR_LENGTH  80

void debug_separator(const  char* title) {
    __check_debug;

    if(title != NULL) {
        debug_append("DEBUG: << [ ");
        debug_append(title);
        debug_append(" ] >>\n\r");
    }

    for(int i = 0; i < __SEPARATOR_LENGTH; i ++)
        debug_append("-");

    debug_append("\n\r");
}

void debug_breakpoint() {
    // __check_debug;

    asm volatile("1: jmp 1b");
}

void debug_number(int number, int base) {
    char* res;
    itoa(res, base, number);

    switch(base) {
        case 0:
        case 10:
            break;

        case 1:
            debug_append("0b");
            break;
        
        case 12:
            debug_append("0c");
            break;
        
        case 16:
            debug_append("0x");
            break;

        default:
            goto badbase;
            break;
    }

    debug_append(res);
    return;

badbase:
    debug_message("Invalid number base", "debug_number()", KERNEL_ERROR);
    return;
}

void debug_printf(const char* interface, enum kernel_statusLevels level, const char* format, ...) {
    __check_debug;

    if(level == KERNEL_VERBOSE && !_verbose)
        // * Message level verbose, but verbosity not allowed.
        return;
    
    //debug_append("\n");
    debug_append("\n\rDEBUG: ");
    
    if(format == NULL) {
        debug_append("\n\r");
        return;
    }

    debug_append(_levelsStrings[level]);

    if(interface != NULL) {
        debug_append("[");
        debug_append(interface);
        debug_append("] ");
    }
    
    char **arg = (char **)&format;
    char buf[32];
    int c;

    arg++;

    memset(buf, 0, sizeof(buf));

    while ((c = *format++) != 0) {
        if (c != '%') {
            debug_append(c);
        } else {
            char *p, *p2;
            int pad0 = 0, pad = 0;

            c = *format++;

            if (c == '0') {
                pad0 = 1;
                c = *format++;
            }

            if (c >= '0' && c <= '9') {
                pad = c - '0';
                c = *format++;
            }

            switch (c) {
                case 'd':
                case 'u':
                case 'x':
                    itoa(buf, c, *((int *)arg++));
                    p = buf;

                    goto string;

                    break;

                case 's':
                    p = *arg++;
                    if (!p)
                        p = "(null)";

                string:
                    for (p2 = p; *p2; p2++);
                    for (; p2 < p + pad; p2++)
                        debug_append(pad0 ? '0' : ' ');

                    while (*p)
                        debug_append(*p++);

                    break;

                default:
                    debug_append(*((int *)arg++));
                    break;
            }
        }
    }
}
