#include "multitask.h"
#include "memory/memory.h"      // kmalloc
#include "console.h"            // puts
#include "string.h"             // memset, memcpy
#include "kernel.h"             // kernel_panic, types, register getters
#include "debug.h"

pid_t mt_pid_counter = 0;
kernel_tcb_t* mt_tcb_current = NULL;

void _mt_panic(const char* s) {
    puts("kernel panic: mt: ");
    puts(s);
    puts("\n");

    debug_message("kernel panic: ", "mt", KERNEL_ERROR);
    debug_append(s);

    kernel_panic(NULL, -1);
}

void _mt_stub() {
    if(mt_tcb_current && mt_tcb_current->task_func)
        mt_tcb_current->task_func();

    mt_tcb_current->state = MT_STATE_DONE;

    for(;;)
        __asm__ __volatile__("nop");
}

// inject running context to thread
void mt_getctx(kernel_tcb_t* thread) {
    if(!thread)
        return;

    thread->regs.eip     = 0x00;
    thread->regs.cs      = 0x08;
    thread->regs.ss      = 0x10;
    thread->regs.esp     = get_esp();
    thread->regs.ebp     = get_ebp();
    thread->regs.eflags  = get_eflags();
}

void mt_makectx(kernel_tcb_t* thread, u32 stack_top /*, void(*stub)()*/) {
    if(!thread || !stack_top) {
        debug_message("mt_makectx(): invalid parameters", "mt", KERNEL_ERROR);
        return;
    }

    thread->regs.esp     = stack_top;
    thread->regs.ebp     = stack_top;
    thread->regs.eip     = (u32) _mt_stub;
    thread->regs.eflags  = 0x202;
    thread->regs.cs      = 0x08;
    thread->regs.ss      = 0x10;
}

void mt_init() {
    __asm__ __volatile__("cli");    // disable interrupts
    
    if(mt_pid_counter > 0) {
        _mt_panic("PID not zero");
        return;
    }
    
    kernel_tcb_t* thread = (kernel_tcb_t*) kmalloc(sizeof(kernel_tcb_t));
    if(!thread) {
        _mt_panic("kmalloc() fail");
        return;
    }

    memset(thread, 0, sizeof(kernel_tcb_t));
    thread->pid     = 0;                // process id is zero
    thread->state   = MT_STATE_RUNNING; // set thread state as running
    thread->next    = thread;           // refers to itself as next thread
    thread->parent  = NULL;             // no parrent thread
    
    // inject current running context to thread control block
    mt_getctx(thread);

    // set this thread as a current one
    mt_tcb_current = thread;
    mt_tcb_current->next = mt_tcb_current;

    __asm__ __volatile__("sti");    // enable interrupts again
}

static u32 mt_initstack(void(*stub)(), u8* stack_base, size_t stack_size) {
    uintptr_t top = (uintptr_t)(stack_base + stack_size);

    // align to 16
    top &= ~(uintptr_t) 0x0F;
    u32* stack_top = (u32*) top;

    *--stack_top = (u32) stub;  // EIP (instruction pointer)
    *--stack_top = (u32) 0x08;  // CS
    *--stack_top = (u32) 0x202; // EFLAGS (IF = 1)

    return (u32) stack_top;
}

kernel_tcb_t* mt_newtask(void(*task_func)()) {
    if(!task_func) {
        debug_message("mt_newtask() task_func is NULL", "mt", KERNEL_ERROR);
        return NULL;
    }

    if(mt_pid_counter == 0)
        debug_message("PID 0 should only be use for kernel main", "mt", KERNEL_WARNING);

    kernel_tcb_t* thread = (kernel_tcb_t*) kmalloc(sizeof(kernel_tcb_t));
    if(!thread) {
        _mt_panic("kmalloc() fail for thread");
        return NULL;
    }

    memset(thread, 0, sizeof(*thread));

    __asm__ __volatile__("cli"); // disable interrupts

    thread->pid         = ++mt_pid_counter;
    thread->state       = MT_STATE_READY;
    thread->task_func   = task_func;
    thread->parent      = NULL;     // TODO: handle parent thread

    const size_t _STACK_SIZE = 4096;
    u8* stack_base = (u8*) kmalloc(_STACK_SIZE);
    if(!stack_base) {
        kfree(thread);
        _mt_panic("kmalloc() fail for stack_base");
        return NULL;
    }

    thread->regs.esp = mt_initstack(_mt_stub, stack_base, _STACK_SIZE);
    thread->regs.ebp = thread->regs.esp;

    if(mt_tcb_current) {
        thread->next = mt_tcb_current->next;
        mt_tcb_current->next = thread;
    } else {
        thread->next = thread;
        mt_tcb_current = thread;
    }

    debug_message("new thread PID: ", "mt", KERNEL_MESSAGE);
    debug_number(thread->pid, 10);

    __asm__ __volatile__("sti"); // enable interrupts

    return thread;
}

void mt_switch(REGISTERS* regs) {
    // __asm__ __volatile__("cli");

    if(!mt_tcb_current || mt_tcb_current->next == mt_tcb_current)
        return;

    kernel_tcb_t* old = mt_tcb_current;

    memcpy(&old->regs, regs, sizeof(REGISTERS));

    mt_tcb_current = mt_tcb_current->next;
    kernel_tcb_t* new = mt_tcb_current;

    memcpy(regs, &new->regs, sizeof(REGISTERS));

    debug_message("switch tasks: ", "mt", KERNEL_MESSAGE);
    debug_number(old->pid, 10);
    debug_append(", ");
    debug_number(new->pid, 10);

    // __asm__ __volatile__("sti");
}

pid_t mt_getPidCounter() {
    return mt_pid_counter;
}

kernel_tcb_t* mt_getCurrent() {
    return mt_tcb_current;
}
