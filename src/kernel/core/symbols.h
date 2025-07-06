#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "libc/stdint.h"

typedef struct {
    uintptr_t addr;
    const char* name;
} symbol_t;

extern symbol_t kernel_symbols[];
extern size_t kernel_symbol_count;

#endif