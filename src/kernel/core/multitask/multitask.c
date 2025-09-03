#include "multitask.h"
#include "memory/memory.h"      // kmalloc
#include "console.h"            // puts
#include "string.h"             // memset
#include "kernel.h"

pid_t mt_pid_counter = 0;
kernel_tcb_t* mt_tcb_current = NULL;

void mt_init() {
    __asm__ __volatile__("cli");

    if(mt_pid_counter != 0) {
        puts("PID 0 already occupied!\n");
        kernel_panic(NULL, -1);

        return;
    }

    kernel_tcb_t* task = (kernel_tcb_t*) kmalloc(sizeof(kernel_tcb_t));
    if(!task) {
        puts("unable to create thread control block for kernel main! kmalloc() error\n");
        kernel_panic(NULL, -1);

        return;
    }

    memset(task, 0, sizeof(kernel_tcb_t));

    task->pid   = 0;
    task->state = MT_STATE_RUNNING;
    task->next  = task;

    task->regs.esp      = get_esp();
    task->regs.ebp      = get_ebp();
    task->regs.eflags   = get_eflags();

    mt_tcb_current = task;

    __asm__ __volatile__("sti");
}
