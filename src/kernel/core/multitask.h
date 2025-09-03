#ifndef __MULTITASK_H
#define __MULTITASK_H

#include "kernel.h"

#define MT_STATE_READY      1
#define MT_STATE_RUNNING    2
#define MT_STATE_TERM       3

typedef unsigned int pid_t;

typedef struct kernel_threadCtrlBlk kernel_tcb_t;
typedef struct kernel_threadCtrlBlk {
    REGISTERS regs;
    kernel_tcb_t* next;
    u8 state;

    pid_t pid;

    // TODO: implement task name, cputime, priority
} kernel_tcb_t;

void mt_init();

#endif

