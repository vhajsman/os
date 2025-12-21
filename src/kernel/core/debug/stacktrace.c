#include "debug.h"
#include "string.h"
#include "libc/stdint.h"
#include "stack.h"

// TODO: implement per-task stack as soon as multitasking is implemented.
#define _BOOTSTRAP_STACK_SIZE   KERNEL_STACK_SIZE       // TODO: implement per-task stack as
                                                        // soon as multitasking is implemented.

uintptr_t stack_base =  (uintptr_t) bootstrap_stack;
uintptr_t stack_top  =  (uintptr_t) bootstrap_stack + KERNEL_STACK_SIZE;

static inline u8 frameInStackRange(const kernel_stack_frame_t* frame) {
    uintptr_t pf = (uintptr_t) frame;
    return (pf >= stack_base) && (pf < stack_top);
}

static inline u8 validateRetAddr(const void* addr) {
    return ((uintptr_t) addr > 0x10000 /*kernel text section start*/);
}

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

//        if(!frameInStackRange(frame) || !validateRetAddr(frame->ret_addr))
//            break;

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
        _fn_print((unsigned char*) ": ");
        _fn_print((unsigned char*) debug_lookup((uintptr_t) addr));
        _fn_print((unsigned char*) "\n");
    }
}
