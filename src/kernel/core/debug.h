#ifndef __DEBUG_H
#define __DEBUG_H

#include "types.h"
#include "libc/stdint.h"

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

/*  Inserts a pseudo-breakpoint, where another debug
    functions are not usable.
    These can be used to perform a binary space
    isolation (binary chop) through the code.*/
void debug_breakpoint();

void debug_printf(const char* interface, enum kernel_statusLevels level, const char* format, ...);
void debug_number(int number, int base);

#define __debug_messagen(message, interface, level, number, base)   \
    char ___b_itoa_##number##_[64];                                 \
    itoa(___b_itoa_##number##_, base, number);                      \
                                                                    \
    debug_message(message, interface, level);                       \
                                                                    \
    if(base == 16)  debug_append("0x");                             \
    if(base == 12)  debug_append("0c");                             \
    if(base == 2)   debug_append("0b");                             \
                                                                    \
    debug_append(___b_itoa_##number##_);
#define __debug_messagea(message, interface, level, number, base)   \
    char ___b_itoa_##number##_[64];                                 \
    itoa(___b_itoa_##number##_, (int) &base, number);               \
                                                                    \
    debug_message(message, interface, level);                       \
                                                                    \
    if(base == 16)  debug_append("0x");                             \
    if(base == 12)  debug_append("0c");                             \
    if(base == 2)   debug_append("0b");                             \
                                                                    \
    debug_append(___b_itoa_##number##_);

void debug_messagen(const char* message, const char* interface, enum kernel_statusLevels level, int number, int base);
void debug_messagea(const char* message, const char* interface, enum kernel_statusLevels level, int number, int base);

#define debug_var(__identifier, __interface, __level, __base) { \
    debug_message(#__identifier, __interface, __level);         \
    debug_append(" = ");                                        \
    debug_number(__identifier, __base);                         \
}

void debug_putc(char c);

// =========================================================
// =================== STACK TRACE (debug/stacktrace.c)
// =========================================================

#define __wrap_return_address(x)    \
    ((x) >= 0 && (x) < 16 ? __builtin_return_address(x) : NULL)

/**
 * @brief Dumps stack trace
 * 
 * @param depth how many addresses to dump
 * @param _fn_print pointer to function that is used as print
 */
void debug_dumpStackTrace(u8 depth, void (*_fn_print)(unsigned char*));

// =========================================================
// =================== SYMBOL LOOKUP (debug/symbol.c, symbol.h)
// =========================================================

/**
 * @brief Get name of symbol of kernel binary
 * 
 * @param addr symbol address
 * @return const char* 
 */
const char* debug_lookup(uintptr_t addr);

#endif