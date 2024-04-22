#include "debug.h"
#include "com/serial.h"
#include "console.h"
#include "type/string.h"

int _verbose = 0;
int _timestamp = 0; // UNDONE

u16 _port;

static char* _levelsStrings[7] = {
    "[   ***   ]\0",
    "[ NOTE    ]\0",
    "[ NOTE !! ]\0",
    "[ WARNING ]\0",
    "[ ERROR   ]\0",
    "[ FATAL   ]\0",
    "[ VERBOSE ]\0"
};

void debug_setVerbose(int verbose) {
    _verbose = verbose ? 1 : 0;
    debug_message("Verbose flag enabled", 0, KERNEL_VERBOSE);
}

void debug_setPort(u16 port) {
    if(port != COM1 && port != COM2) {
        printf("Error: Debug port not supported");
        return;
    }

    _port = port;

    puts("Port ");
    puts(port == COM1 ? "COM1" : "COM2");
    puts(" used as a debug port. ");
    puts("You should be able to use debug features after connecting the debugger via this port.\n");

    // ! SERIAL COMMUNICATION SHOULD BE ALREADY INIT!!!
    // serial_init();
}

void debug_append(const  char* data) {
    serial_write(_port, data);
}

#define __INTERFACE_MAX_CHARS   12

void debug_message(const  char* message, const  char* interface, enum kernel_statusLevels level) {
    if(level == KERNEL_VERBOSE && !_verbose)
        // * Message level verbose, but verbosity not allowed.
        return;
    
    debug_append("DEBUG: ");
    
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

    debug_append("\n\r");
}

#define __SEPARATOR_LENGTH  80

void debug_separator(const  char* title) {
    if(title != NULL) {
        debug_append("DEBUG: << [ ");
        debug_append(title);
        debug_append(" ] >>\n\r");
    }

    for(int i = 0; i < __SEPARATOR_LENGTH; i ++)
        debug_append("-");

    debug_append("\n\r");
}