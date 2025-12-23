#include "isr.h"
#include "idt.h"
#include "time/timer.h"
#include "console.h"
#include "debug.h"
#include "irqdef.h"
#include "libc/string.h" // memset

void isr_doStats(isrpb_t* isr, u32 start);
void isr_dispatch(isrpb_t* isr, REGISTERS* reg, int is_irq);

// ISR g_interrupt_handlers[NO_INTERRUPT_HANDLERS];
isrpb_t g_interrupt_handlers[NO_INTERRUPT_HANDLERS];

char *exception_messages[32] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "BOUND Range Exceeded",
    "Invalid Opcode",
    "Device Not Available (No Math Coprocessor)",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection",
    "Page Fault",
    "Unknown Interrupt (intel reserved)",
    "x87 FPU Floating-Point Error (Math Fault)",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

u8 _irq_busy = 0;

void default_irq_handler(REGISTERS *reg) {
    pic8259_eoi(reg->int_no - IRQ_BASE);
}

void isr_registerInterruptHandler(int num, ISR handler) {
    // printf("IRQ %d registered\n", num);

    if(num >= NO_INTERRUPT_HANDLERS) {
        debug_message("invalid irq number: ", "IRQ", KERNEL_ERROR);
        debug_number(num, 16);
        return;
    }

    bool isPit = num == IRQ_BASE + IRQ0_TIMER;

    isrpb_t* dest = &g_interrupt_handlers[num];
    dest->handler = handler;
    dest->avgTimeElapsed = 0;
    dest->avgTrigPerSecond = 0;
    dest->maxTimeElapsed = 0;
    dest->minTimeElapsed = 0;
    dest->trigCount = 0;
    dest->completeCount = 0;
    dest->trigTimestamp = 0;
    dest->flags = IsrEnabled | IsrReentrant | (isPit ? IsrPriorityCrit : IsrDoStats);
    dest->additionalContextResolv = NULL;
}

void isr_registerInterruptHandlerWithParams(int num, isrpb_t* params) {
    if(num >= NO_INTERRUPT_HANDLERS) {
        debug_message("invalid irq number: ", "IRQ", KERNEL_ERROR);
        debug_number(num, 16);
        return;
    }

    isrpb_t* dest = &g_interrupt_handlers[num];
    *dest = *params;
}

isrpb_t* isr_getParamBlock(int num) {
    if(num >= NO_INTERRUPT_HANDLERS)
        return NULL;

    return &g_interrupt_handlers[num];
}

void isr_endInterrupt(int num) {
    pic8259_eoi(num);
}

void isr_irqHandler(REGISTERS *reg) {
    if(reg->int_no >= NO_INTERRUPT_HANDLERS) {
        // kernel_panic(reg, -1); // unexcepted
        pic8259_eoi(reg->int_no - IRQ_BASE);
        return;
    }

    isrpb_t* isr = &g_interrupt_handlers[reg->int_no];
    isr_dispatch(isr, reg, 1);
}

void print_registers(REGISTERS *reg) {
    printf("REGISTERS:\n");
    printf("err_code=%d\n", reg->err_code);
    printf("eax=0x%x, ebx=0x%x, ecx=0x%x,   edx=0x%x\n", reg->eax,          reg->ebx, reg->ecx, reg->edx);
    printf("edi=0x%x, esi=0x%x, ebp=0x%x,   esp=0x%x\n", reg->edi,          reg->esi, reg->ebp, reg->esp);
    printf("eip=0x%x, cs=0x%x,  ss=0x%x,    eflags=0x%x, useresp=0x%x\n",   reg->eip, reg->ss,  reg->eflags, reg->useresp);
}

void isr_doStats(isrpb_t* isr, u32 start) {
    u32 now = pit_get();
    u32 elapsed = now - start;
    if(elapsed == 0)
        elapsed = 1;

    u32 tdiff = now - isr->trigTimestamp;
    if(tdiff >= PIT_FREQUENCY) {
        isr->avgTrigPerSecond = (isr->trigCountTmp * PIT_FREQUENCY) / tdiff;
        isr->trigCountTmp = 0;
    }

    if(isr->completeCount > 0) {
        isr->avgTimeElapsed =   ((isr->avgTimeElapsed * isr->completeCount) + elapsed)
                                / (isr->completeCount + 1);

        if(elapsed > isr->maxTimeElapsed) isr->maxTimeElapsed = elapsed;
        if(elapsed < isr->minTimeElapsed) isr->minTimeElapsed = elapsed;

        return;
    }

    isr->avgTimeElapsed = elapsed;
    isr->minTimeElapsed = elapsed;
    isr->maxTimeElapsed = elapsed;
}

void isr_dispatch(isrpb_t* isr, REGISTERS* reg, int is_irq) {
    IGNORE_UNUSED(is_irq);

    if(!isr->handler)
        return;
    if(!(isr->flags & IsrEnabled))
        return;

    isr->trigCount++;
    isr->trigCountTmp++;
    isr->trigTimestamp = pit_get();

    if((isr->flags & IsrActive) && !(isr->flags & IsrReentrant))
        return;

    isr->flags |= IsrActive;

    u32 start = 0;

    // do stats only if IsrDoStats and not IsrPriorityCrit
    int doStats = (isr->flags & IsrDoStats) && !(isr->flags & IsrPriorityCrit);
    if(doStats)
        start = isr->trigTimestamp;

    ISR handler = isr->handler;
    handler(reg);

    if(doStats)
        isr_doStats(isr, start);

    isr->flags &= ~IsrActive;
    isr->completeCount++;

    if(is_irq && reg->int_no >= IRQ_BASE)
        pic8259_eoi(reg->int_no - IRQ_BASE);
}

/**
 * invoke exception routine,
 * being called in exception.asm
 */
void isr_exception_handler(REGISTERS *reg) {
    if(reg->int_no < 32) {
        kernel_panic(reg, reg->int_no);
        return;
    }

    if(reg->int_no >= NO_INTERRUPT_HANDLERS)
        return;

    isrpb_t* isr = &g_interrupt_handlers[reg->int_no];
    isr_dispatch(isr, reg, 0);

    /*
    if(reg->int_no < NO_INTERRUPT_HANDLERS) {
        isrpb_t *isr = &g_interrupt_handlers[reg->int_no];
        if(isr->handler) {
            if(!(isr->flags & IsrEnabled))
                return;

            isr->trigCount++;

            if((isr->flags & IsrActive) && !(isr->flags & IsrReentrant))
                return;

            isr->flags |= IsrActive;

            if(!(isr->flags & IsrPriorityCrit))
                isr->trigTimestamp = pit_get();

            ISR handler = isr->handler;
            handler(reg);

            isr->completeCount++;

            if((isr->flags & IsrDoStats) && !(isr->flags & IsrPriorityCrit)) {
                u32 elapsed = pit_get() - isr->trigTimestamp;

                isr->avgTimeElapsed = ((isr->avgTimeElapsed * (isr->completeCount -1)) + elapsed) / isr->completeCount;
                isr->maxTimeElapsed = elapsed > isr->maxTimeElapsed ? elapsed : isr->maxTimeElapsed;

                if(!isr->completeCount || elapsed < isr->minTimeElapsed) 
                    isr->minTimeElapsed = elapsed;
            }

            isr->flags &= ~IsrActive;
        }
    }*/
}

void isr_init() {
    memset(g_interrupt_handlers, 0x00, sizeof(g_interrupt_handlers));
    for(int i = 0; i < NO_INTERRUPT_HANDLERS; i++) {
        g_interrupt_handlers[i].handler = NULL;
        g_interrupt_handlers[i].flags = 0x00;
        g_interrupt_handlers[i].additionalContextResolv = NULL;
    }
}

void irq_done() {}
