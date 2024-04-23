#ifndef __DEBUG_H
#define __DEBUG_H

#include "types.h"

enum kernel_statusLevels {
    KERNEL_MESSAGE,
    KERNEL_INFORMATION,
    KERNEL_IMPORTANT,
    KERNEL_WARNING,
    KERNEL_ERROR,
    KERNEL_FATAL,
    KERNEL_VERBOSE,
    KERNEL_OK,
    KERNEL_FAIL,
    KERNEL_PENDING
};

void debug_setVerbose(int verbose);
//UNDONE: void debug_setTimestamp(int timestamp);

void debug_setPort(u16 port);

void debug_append(      const  char* data);
void debug_message(     const  char* message, 
                        const  char* interface,
                        enum kernel_statusLevels level);
void debug_separator(   const  char* title);

#endif