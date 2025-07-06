#include "debug.h"
#include "string.h"
#include "libc/stdint.h"

void* get_return_address(int i) {
    switch(i) {
        case 0: return __builtin_return_address(0);
        case 1: return __builtin_return_address(1);
        case 2: return __builtin_return_address(2);
        case 3: return __builtin_return_address(3);
        case 4: return __builtin_return_address(4);
        case 5: return __builtin_return_address(5);
        case 6: return __builtin_return_address(6);
        case 7: return __builtin_return_address(7);
        case 8: return __builtin_return_address(8);
        case 9: return __builtin_return_address(9);
        case 10: return __builtin_return_address(10);
        case 11: return __builtin_return_address(11);
        case 12: return __builtin_return_address(12);
        case 13: return __builtin_return_address(13);
        case 14: return __builtin_return_address(14);
        case 15: return __builtin_return_address(15);
        default: return NULL;
    }
}

void debug_dumpStackTrace(u8 depth, void (*_fn_print)(unsigned char*)) {
    if(depth <= 0 || _fn_print == NULL) {
        debug_message("debug_dumpStackTrace(): invalid parameters", "debug", KERNEL_ERROR);
        return;
    }

    for(int i = 0; i < depth; i++) {
        void* addr = get_return_address(i);
        if(!addr || (uintptr_t) addr < 0x10000)
            break;

        char addr_string[32 + 1];
        itoa(addr_string, 16, (uintptr_t) addr);

        char i_string[8];
        itoa(i_string, 16, i);

        _fn_print((unsigned char*) "  [trace frame 0x");
        _fn_print((unsigned char*) i_string);
        _fn_print((unsigned char*) "] 0x");
        _fn_print((unsigned char*) addr_string);
        _fn_print((unsigned char*) "\n");
    }
}
