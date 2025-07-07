#ifndef __STACK_H
#define __STACK_H

#define KERNEL_STACK_TOP    0x9FBFF
#define STACK_CHK_GUARD     0xe2dee396

typedef struct kernel_stack_frame {
    struct kernel_stack_frame* ebp;
    void* ret_addr;
} kernel_stack_frame_t;

#endif
