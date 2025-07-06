#include "debug.h"
#include "symbols.h"
#include "libc/stdint.h"

const char* debug_lookup(uintptr_t addr) {
    const char* best_match = "??";
    uintptr_t best_addr = 0x00000000;

    for(size_t i = 0; i < kernel_symbol_count; i++) {
        if(kernel_symbols[i].addr <= addr && kernel_symbols[i].addr >= best_addr) {
            best_addr = kernel_symbols[i].addr;
            best_match = kernel_symbols[i].name;
        }
    }

    return best_match;
}