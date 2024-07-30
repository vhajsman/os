#ifndef __ISR_H
#define __ISR_H

#include "types.h"
#include "kernel.h"

#define NO_INTERRUPT_HANDLERS    256

typedef void (*ISR)(REGISTERS *);

void isr_registerInterruptHandler(int num, ISR handler);
void isr_endInterrupt(int num);
void isr_exceptionHandler(REGISTERS reg);
void isr_irqHandler(REGISTERS *reg);

extern char* exception_messages[32];

extern void exception_0();
extern void exception_1();
extern void exception_2();
extern void exception_3();
extern void exception_4();
extern void exception_5();
extern void exception_6();
extern void exception_7();
extern void exception_8();
extern void exception_9();
extern void exception_10();
extern void exception_11();
extern void exception_12();
extern void exception_13();
extern void exception_14();
extern void exception_15();
extern void exception_16();
extern void exception_17();
extern void exception_18();
extern void exception_19();
extern void exception_20();
extern void exception_21();
extern void exception_22();
extern void exception_23();
extern void exception_24();
extern void exception_25();
extern void exception_26();
extern void exception_27();
extern void exception_28();
extern void exception_29();
extern void exception_30();
extern void exception_31();
extern void exception_128();

extern void irq_0();
extern void irq_1();
extern void irq_2();
extern void irq_3();
extern void irq_4();
extern void irq_5();
extern void irq_6();
extern void irq_7();
extern void irq_8();
extern void irq_9();
extern void irq_10();
extern void irq_11();
extern void irq_12();
extern void irq_13();
extern void irq_14();
extern void irq_15();

#define ISR(VECTOR)                                                 \
    void __irqhandler(REGISTER* r);                                 \
    isr_registerInterruptHandler(IRQ_BASE + VECTOR, __irqhandler);  \
                                                                    \
    void __irqhandler(REGISTER* r)

void print_registers(REGISTERS *reg);

#endif