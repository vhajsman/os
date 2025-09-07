#ifndef __MULTITASK_H
#define __MULTITASK_H

#include "kernel.h"

#define MT_STATE_READY      1
#define MT_STATE_RUNNING    2
#define MT_STATE_DONE       3

typedef unsigned int pid_t;

typedef struct kernel_threadCtrlBlk kernel_tcb_t;
typedef struct kernel_threadCtrlBlk {
    // u32 eax, ebx, ecx, edx;
    // u32 esi, edi, ebp;
    // u32 esp;
    // u32 eip;
    // u32 eflags;
    // u32 cs, ss;

    REGISTERS regs;

    kernel_tcb_t* parent;   // parent process pointer
    kernel_tcb_t* next;     // next process in linked list pointer
                            //
    u8 state;               // current thread state
    pid_t pid;              // process ID
                            //
    void(*task_func)();     // task function

    // TODO: implement task name, cputime, priority
} kernel_tcb_t;

void mt_init();
void mt_switch(REGISTERS* regs);

kernel_tcb_t* mt_newtask(void(*task_func)());

pid_t mt_getPidCounter();
kernel_tcb_t* mt_getCurrent();

#endif

