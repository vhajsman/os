#include "isr.h"
#include "idt.h"
#include "time/timer.h"
#include "console.h"
#include "debug.h"
#include "irqdef.h"

ISR g_interrupt_handlers[NO_INTERRUPT_HANDLERS];

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

void isr_registerInterruptHandler(int num, ISR handler) {
    // printf("IRQ %d registered\n", num);

    if (num < NO_INTERRUPT_HANDLERS)
        g_interrupt_handlers[num] = handler;
}

void isr_endInterrupt(int num) {
    pic8259_eoi(num);
}

void isr_irqHandler(REGISTERS *reg) {
    if(g_interrupt_handlers[reg->int_no] != NULL) {
        _irq_busy = 1;
        // __asm__ __volatile__("cli");

        ISR handler = g_interrupt_handlers[reg->int_no];
        handler(reg);

        _irq_busy = 0;
        // __asm__ __volatile__("sti");

        // debug_message("IRQ subroutine done: ", "IRQ", KERNEL_MESSAGE);
        // debug_number(reg->int_no, 16);

    }
    
    pic8259_eoi(reg->int_no + IRQ_BASE);
}

void print_registers(REGISTERS *reg) {
    printf("REGISTERS:\n");
    printf("err_code=%d\n", reg->err_code);
    printf("eax=0x%x, ebx=0x%x, ecx=0x%x,   edx=0x%x\n", reg->eax,          reg->ebx, reg->ecx, reg->edx);
    printf("edi=0x%x, esi=0x%x, ebp=0x%x,   esp=0x%x\n", reg->edi,          reg->esi, reg->ebp, reg->esp);
    printf("eip=0x%x, cs=0x%x,  ss=0x%x,    eflags=0x%x, useresp=0x%x\n",   reg->eip, reg->ss,  reg->eflags, reg->useresp);
}

/**
 * invoke exception routine,
 * being called in exception.asm
 */
void isr_exception_handler(REGISTERS reg) {
    if (reg.int_no < 32)
        kernel_panic(&reg, (signed int) reg.int_no);
        
    if (g_interrupt_handlers[reg.int_no] != NULL) {
        ISR handler = g_interrupt_handlers[reg.int_no];
        handler(&reg);
    }
}

void irq_done() {}
