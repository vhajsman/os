#include "isr.h"
#include "idt.h"
#include "time/timer.h"
#include "console.h"

ISR g_interrupt_handlers[NO_INTERRUPT_HANDLERS];



void isr_registerInterruptHandler(int num, ISR handler) {
    printf("IRQ %d registered\n", num);

    if (num < NO_INTERRUPT_HANDLERS)
        g_interrupt_handlers[num] = handler;
}

void isr_endInterrupt(int num) {
    pic8259_eoi(num);
}

void isr_irqHandler(REGISTERS *reg) {
    if (g_interrupt_handlers[reg->int_no] != NULL) {
        ISR handler = g_interrupt_handlers[reg->int_no];
        handler(reg);
    }
    
    pic8259_eoi(reg->int_no);
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
    if (reg.int_no < 32) {
        printf("EXCEPTION: %s\n", exception_messages[reg.int_no]);
        print_registers(&reg);
        
        kernel_panic(&reg, (signed int) reg.int_no);
    }
    if (g_interrupt_handlers[reg.int_no] != NULL) {
        ISR handler = g_interrupt_handlers[reg.int_no];
        handler(&reg);
    }
}
