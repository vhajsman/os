#include "multitask.h"          //
#include "memory/memory.h"      // kmalloc
#include "console.h"            // puts
#include "string.h"             // memset, memcpy
#include "kernel.h"             // kernel_panic, types, register getters
#include "debug.h"              //

extern void kmain(unsigned long magic, unsigned long addr);

pid_t mt_pid_counter         = 0;       // process ID counter
kernel_tcb_t* mt_tcb_current = NULL;    // current thread
kernel_tcb_t* mt_tcb_main    = NULL;    // main thread (PID 0)
kernel_tcb_t* mt_tcb_head    = NULL;    // first thread in list
kernel_tcb_t* mt_tcb_tail    = NULL;    // last thread in list

void _mt_stub();

// ===========================================================================================
// ===== HELPER FUNCTIONS
// ===========================================================================================

kernel_tcb_t* mt_tcb_getLast() {
    return mt_tcb_head ? mt_tcb_tail : NULL;
}

void mt_tcb_append(kernel_tcb_t* tcb) {
    if(!tcb) return;

    tcb->next = NULL;

    if(!mt_tcb_head) {
        mt_tcb_main = mt_tcb_head = mt_tcb_tail = mt_tcb_current = tcb;
        return;
    }

    mt_tcb_tail->next = tcb;
    mt_tcb_tail = tcb;
}

kernel_tcb_t* mt_tcb_getPrev(kernel_tcb_t* tcb) {
    if(!tcb || !mt_tcb_head) return NULL;

    kernel_tcb_t* curr = mt_tcb_head;
    while(curr) {
        if(curr->next == tcb)
            return curr;

        curr = curr->next;
    }

    return NULL;
}

kernel_tcb_t* mt_tcb_getNextReady(kernel_tcb_t* tcb) {
    if(!tcb) return NULL;

    kernel_tcb_t* next = tcb->next;
    while(next && next->state != MT_STATE_READY)
        next = next->next;

    if(!next) {
        next = mt_tcb_head;
        while(next && next->state != MT_STATE_READY && next != tcb)
            next = next->next;
    }

    return (next && next->state == MT_STATE_READY) ? next : NULL;
}

pid_t mt_getPidCounter() {
    return mt_pid_counter;
}

kernel_tcb_t* mt_getCurrent() {
    return mt_tcb_current;
}

kernel_tcb_t* mt_getMain() {
    return mt_tcb_main;
}

kernel_tcb_t* mt_getTcbByPid(pid_t pid) {
    if(!mt_tcb_head)
        return NULL;

    kernel_tcb_t* t = mt_tcb_head;
    while(t) {
        if(t->pid == pid)
            return t;

        t = t->next;
    }

    return NULL;
}

void mt_dump_list() {
    if(!mt_tcb_head)
        return;

    kernel_tcb_t* t = mt_tcb_head;
    do {
        debug_message("PID ", "mt", KERNEL_MESSAGE);
        debug_number(t->pid, 10);

        if(t->next) {
            debug_append(", NEXT PID");
            debug_number(t->next->pid, 10);
        }

        t = t->next;
    } while(t != mt_tcb_main);
}

void _mt_panic(const char* s) {
    puts("kernel panic: mt: ");
    puts(s);
    puts("\n");

    debug_message("kernel panic: ", "mt", KERNEL_ERROR);
    debug_append(s);

    kernel_panic(NULL, -1);
}

// ===========================================================================================
// ===== THREAD CPU CONTEXT INJECTION + STACK STUFF
// ===========================================================================================

// inject running context to thread
void mt_getctx(kernel_tcb_t* thread) {
    if(!thread)
        return;

    thread->regs.eip     = (u32) _mt_stub;
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


static u32 mt_initstack(void(*stub)(), u8* stack_base, size_t stack_size) {
    uintptr_t top = (uintptr_t)(stack_base + stack_size);

    // align to 16
    top &= ~(uintptr_t) 0x0F;
    u32* stack_top = (u32*) top;

    *--stack_top = 0;
    *--stack_top = (u32) stub;

    return (u32) stack_top;
}

// ===========================================================================================
// ===== STUB
// ===========================================================================================

__attribute__((noreturn))
void _mt_stub() {
    if(mt_tcb_current && mt_tcb_current->task_func)
        mt_tcb_current->task_func();

    mt_tcb_current->state = MT_STATE_DONE;

    // free stack and thread control block except main thread
    if(mt_tcb_current->pid != mt_tcb_main->pid) {
        kernel_tcb_t* prev = mt_tcb_getPrev(mt_tcb_current);

        if(prev)
            prev->next = mt_tcb_current->next;

        kfree(mt_tcb_current->stack_base);
        kfree(mt_tcb_current);
    }

    // find next ready thread
    kernel_tcb_t* next = mt_tcb_getNextReady(mt_tcb_current);
    if(next->state != MT_STATE_READY)
        next = mt_tcb_head;

    // switch to the next thread
    mt_tcb_current = next;

    __asm__ __volatile__(
        "movl %[esp], %%esp\n"
        "movl %[ebp], %%ebp\n"
        "jmp *%[eip]\n"
        :
        : [esp] "r" (mt_tcb_current->regs.esp),
          [ebp] "r" (mt_tcb_current->regs.ebp),
          [eip] "r" (mt_tcb_current->regs.eip)
        : "memory"
    );

    for(;;)
        __asm__ __volatile__("hlt");
}

// ===========================================================================================
// ===== THREAD SPAWNING
// ===========================================================================================

kernel_tcb_t* mt_adoptMain() {
    GLBLABEL_USING(_l_kmain_stage1);

    kernel_tcb_t* thread = (kernel_tcb_t*) kmalloc(sizeof(kernel_tcb_t));
    if(!thread) {
        _mt_panic("kmalloc() fail");
        return NULL;
    }

    pid_t pid = (mt_tcb_main || mt_tcb_head || mt_pid_counter > 0) ? ++mt_pid_counter : 0;

    memset(thread, 0, sizeof(kernel_tcb_t));
    thread->pid         = pid;                  // process id
    thread->state       = MT_STATE_READY;       // set thread state as ready
    thread->parent      = NULL;                 // no parrent thread
    thread->next        = NULL;                 // let append function figure out itself
    thread->task_func   = NULL;                 // no task_func for kmain

    const size_t _STACK_SIZE = 4096;
    u8* stack_base = (u8*) kmalloc(_STACK_SIZE);
    if(!stack_base) {
        kfree(thread);
        _mt_panic("kmalloc() fail for stack_base");
        return NULL;
    }

    thread->stack_base  = stack_base;
    thread->stack_size  = _STACK_SIZE;

    mt_getctx(thread);
    thread->regs.eip = (u32) _l_kmain_stage1;

    mt_tcb_main = thread;                       // set as a thread for kmain()
    mt_tcb_head = thread;                       // set as a first thread in list
    mt_tcb_tail = thread;                       // set as a last thread in list
    mt_tcb_current = thread;                    // set as a thread currently switched on

    return thread;

_after_mt_init:
    ;
}

void mt_init() {
    __asm__ __volatile__("cli");    // disable interrupts
    
    if(mt_pid_counter > 0) {
        _mt_panic("PID not zero");
        return;
    }

    kernel_tcb_t* m = mt_adoptMain();
    if(m->pid != (pid_t) 0) {
        puts("Warning: kmain() adopted with PID ");
        char buff[16]; itoa(buff, 10, (int) m->pid);
        puts(buff);
        puts(" instead of 0!\n");
    }

    __asm__ __volatile__("sti");            // enable interrupts again

    mt_dump_list();
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

    thread->pid         = ++mt_pid_counter; // assign process ID
    thread->state       = MT_STATE_READY;   // mark as ready
    thread->task_func   = task_func;        // thread executable code
    thread->parent      = NULL;             // TODO: handle parent thread
    thread->next        = NULL;

    const size_t _STACK_SIZE = 4096;
    u8* stack_base = (u8*) kmalloc(_STACK_SIZE);
    if(!stack_base) {
        kfree(thread);
        _mt_panic("kmalloc() fail for stack_base");
        return NULL;
    }

    thread->stack_base  = stack_base;
    thread->stack_size  = _STACK_SIZE;

    thread->regs.esp    = mt_initstack(_mt_stub, stack_base, _STACK_SIZE);  // ESP
    thread->regs.ebp    = thread->regs.esp;                                 // EBP
    thread->regs.cs     = 0x0008;                                           // CS
    thread->regs.ss     = 0x0010;                                           // SS
    thread->regs.eflags = 0x0202;                                           // FLAGS

    // append to linked list
    mt_tcb_append(thread);

    debug_message("new thread PID: ", "mt", KERNEL_MESSAGE);
    debug_number(thread->pid, 10);
    debug_append(", next PID: ");
    debug_number(thread->next->pid, 10);

    __asm__ __volatile__("sti"); // enable interrupts

    mt_dump_list();

    return thread;
}

// ===========================================================================================
// ===== CONTEXT SWITCH
// ===========================================================================================

void mt_switch(REGISTERS* regs) {
    IGNORE_UNUSED(regs);

    if(!mt_tcb_current)
        return;

    // next ready thread
    kernel_tcb_t* next = mt_tcb_getNextReady(mt_tcb_current);
    if(!next || next->state != MT_STATE_READY) {
        next = mt_tcb_main;
        if(next == mt_tcb_current)
            return;
    }

    if(next == mt_tcb_current)
        return;

    kernel_tcb_t* old = mt_tcb_current;
    mt_tcb_current = next;

    __asm__ __volatile__(
        "movl %%esp, %[old_esp]\n"
        "movl %%ebp, %[old_ebp]\n"
        "movl %[new_esp], %%esp\n"
        "movl %[new_ebp], %%ebp\n"
        "jmp *%[new_eip]\n"
        : [old_esp] "=m" (old->regs.esp),
          [old_ebp] "=m" (old->regs.ebp)
        : [new_esp] "r" (next->regs.esp),
          [new_ebp] "r" (next->regs.ebp),
          [new_eip] "r" (next->regs.eip)
        : "memory"
    );
}
