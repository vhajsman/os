#include "idt.h"
#include "isr.h"
#include "time/timer.h"

IDT g_idt[NO_IDT_DESCRIPTORS];
IDT_PTR g_idt_ptr;

void idt_setEntry(int index, u32 base, u16 seg_sel, u8 flags) {
    IDT *this = &g_idt[index];

    this->base_low = base & 0xFFFF;
    this->segment_selector = seg_sel;
    this->zero = 0;
    this->type = flags | 0x60;
    this->base_high = (base >> 16) & 0xFFFF;
}

void idt_init() {
    g_idt_ptr.base_address = (u32)g_idt;
    g_idt_ptr.limit = sizeof(g_idt) - 1;

    pic8259_init();



    idt_setEntry(0,   (u32) exception_0,     0x08, 0x8E);
    idt_setEntry(1,   (u32) exception_1,     0x08, 0x8E);
    idt_setEntry(2,   (u32) exception_2,     0x08, 0x8E);
    idt_setEntry(3,   (u32) exception_3,     0x08, 0x8E);
    idt_setEntry(4,   (u32) exception_4,     0x08, 0x8E);
    idt_setEntry(5,   (u32) exception_5,     0x08, 0x8E);
    idt_setEntry(6,   (u32) exception_6,     0x08, 0x8E);
    idt_setEntry(7,   (u32) exception_7,     0x08, 0x8E);
    idt_setEntry(8,   (u32) exception_8,     0x08, 0x8E);
    idt_setEntry(9,   (u32) exception_9,     0x08, 0x8E);
    idt_setEntry(10,  (u32) exception_10,    0x08, 0x8E);
    idt_setEntry(11,  (u32) exception_11,    0x08, 0x8E);
    idt_setEntry(12,  (u32) exception_12,    0x08, 0x8E);
    idt_setEntry(13,  (u32) exception_13,    0x08, 0x8E);
    idt_setEntry(14,  (u32) exception_14,    0x08, 0x8E);
    idt_setEntry(15,  (u32) exception_15,    0x08, 0x8E);
    idt_setEntry(16,  (u32) exception_16,    0x08, 0x8E);
    idt_setEntry(17,  (u32) exception_17,    0x08, 0x8E);
    idt_setEntry(18,  (u32) exception_18,    0x08, 0x8E);
    idt_setEntry(19,  (u32) exception_19,    0x08, 0x8E);
    idt_setEntry(20,  (u32) exception_20,    0x08, 0x8E);
    idt_setEntry(21,  (u32) exception_21,    0x08, 0x8E);
    idt_setEntry(22,  (u32) exception_22,    0x08, 0x8E);
    idt_setEntry(23,  (u32) exception_23,    0x08, 0x8E);
    idt_setEntry(24,  (u32) exception_24,    0x08, 0x8E);
    idt_setEntry(25,  (u32) exception_25,    0x08, 0x8E);
    idt_setEntry(26,  (u32) exception_26,    0x08, 0x8E);
    idt_setEntry(27,  (u32) exception_27,    0x08, 0x8E);
    idt_setEntry(28,  (u32) exception_28,    0x08, 0x8E);
    idt_setEntry(29,  (u32) exception_29,    0x08, 0x8E);
    idt_setEntry(30,  (u32) exception_30,    0x08, 0x8E);
    idt_setEntry(31,  (u32) exception_31,    0x08, 0x8E);
    idt_setEntry(32,  (u32) irq_0,           0x08, 0x8E);
    idt_setEntry(33,  (u32) irq_1,           0x08, 0x8E);
    idt_setEntry(34,  (u32) irq_2,           0x08, 0x8E);
    idt_setEntry(35,  (u32) irq_3,           0x08, 0x8E);
    idt_setEntry(36,  (u32) irq_4,           0x08, 0x8E);
    idt_setEntry(37,  (u32) irq_5,           0x08, 0x8E);
    idt_setEntry(38,  (u32) irq_6,           0x08, 0x8E);
    idt_setEntry(39,  (u32) irq_7,           0x08, 0x8E);
    idt_setEntry(40,  (u32) irq_8,           0x08, 0x8E);
    idt_setEntry(41,  (u32) irq_9,           0x08, 0x8E);
    idt_setEntry(42,  (u32) irq_10,          0x08, 0x8E);
    idt_setEntry(43,  (u32) irq_11,          0x08, 0x8E);
    idt_setEntry(44,  (u32) irq_12,          0x08, 0x8E);
    idt_setEntry(45,  (u32) irq_13,          0x08, 0x8E);
    idt_setEntry(46,  (u32) irq_14,          0x08, 0x8E);
    idt_setEntry(47,  (u32) irq_15,          0x08, 0x8E);
    idt_setEntry(128, (u32) exception_128,   0x08, 0x8E);

    idt_load((u32) &g_idt_ptr);
    asm volatile("sti");
}