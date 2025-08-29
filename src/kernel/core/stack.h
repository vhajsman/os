#ifndef __STACK_H
#define __STACK_H

#define KERNEL_STACK_TOP    0x9FBFF
#define STACK_CHK_GUARD     0xE2DEE396

#define KERNEL_STACK_SIZE   0x1000 // 4 KiB

typedef struct kernel_stack_frame {
    struct kernel_stack_frame* ebp;
    void* ret_addr;
} kernel_stack_frame_t;

extern char bootstrap_stack[];

#endif
