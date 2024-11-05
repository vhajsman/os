#ifndef __STDARG_H
#define __STDARG_H

#include <string.h>
#include "types.h"
#include "memory/memory.h"

typedef struct __va_list {
    void** args;
    size_t count;
} va_list;

#define va_start(ap, last)                                              \
    do {                                                                \
        ap.count = 0;                                                   \
        ap.args = malloc(10 * sizeof(void*));                           \
    } while(0)

#define va_arg(ap, type)                                                \
    (type) (ap.args[ap.count++])

#define va_end(ap)                                                      \
    free(ap.args)

#define va_push(ap, val)                                                \
    do {                                                                \
        ap.args[ap.count] = malloc(sizeof(val));                        \
        memcpy(ap.args[ap.count], &val, sizeof(val));                   \
        ap.count++;                                                     \
    } while(0)

#endif
