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

void mt_switch() {
    if(!mt_tcb_current || !mt_tcb_current->next)
        return;

    __asm__ __volatile__("cli");

    kernel_tcb_t* old = mt_tcb_current;
    kernel_tcb_t* new = mt_tcb_current->next;

    // save regs to old task
    __asm__ __volatile__ (
        "movl %%esp, %0\n\t"
        "movl %%ebp, %1\n\t"
        "pushf\n\t"
        "pop %2\n\t"
        :   "=m"(old->regs.esp), 
            "=m"(old->regs.ebp), 
            "=m"(old->regs.eflags)
        :
        : "memory"
    );

    mt_tcb_current = new;

    // load regs from new task
    __asm__ __volatile__ (
        "movl %0, %%esp\n\t"
        "movl %1, %%ebp\n\t"
        "push %2\n\t"
        "popf\n\t"
        :
        :   "m"(new->regs.esp), 
            "m"(new->regs.ebp), 
            "m"(new->regs.eflags)
        : "memory"
    );

    __asm__ __volatile__("sti");
}
