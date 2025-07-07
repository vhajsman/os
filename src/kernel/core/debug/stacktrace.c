#include "debug.h"
#include "string.h"
#include "libc/stdint.h"
#include "stack.h"

void debug_captureStackTrace(void** buffer, unsigned int maxFrames) {
    if(maxFrames == 0 || buffer == NULL) {
        debug_message("debug_captureStackTrace(): invalid parameters", "debug", KERNEL_ERROR);
        return;
    }

    kernel_stack_frame_t* frame;

    // get EBP register value
    asm volatile("movl %%ebp, %0" : "=r" (frame));

    for(unsigned int i = 0; i < maxFrames; i++) {
        if(frame == NULL || frame->ebp == NULL)
            break;

        buffer[i] = frame->ret_addr;
        frame = frame->ebp;
    }
}

void debug_dumpStackTrace(u8 depth, void (*_fn_print)(unsigned char*)) {
    if(depth <= 0 || _fn_print == NULL) {
        debug_message("debug_dumpStackTrace(): invalid parameters", "debug", KERNEL_ERROR);
        return;
    }

    void* trace[depth];
    memset(trace, 0x00, sizeof(trace));

    debug_captureStackTrace(trace, depth);

    for(unsigned int i = 0; i < depth; i++) {
        void* addr = trace[i];
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
        _fn_print(": ");
        _fn_print((unsigned char*) debug_lookup((uintptr_t) addr));
        _fn_print((unsigned char*) "\n");
    }
}
