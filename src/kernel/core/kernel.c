#include "kernel.h"
#include "console.h"
#include "string.h"
#include "gdt/gdt.h"
#include "irq/idt.h"
#include "irq/isr.h"
#include "hid/kbd.h"
#include "ioport.h"
#include "shell/shell.h"
#include "com/serial.h"
#include "time/timer.h"
#include "memory/memory.h"
#include "multiboot.h"
#include "debug.h"
#include "pci.h"
#include "ata/ide.h"
#include "initrd.h"

bool _kernel_exitOk = 0;

#include "stack.h"
u32 __stack_chk_guard = STACK_CHK_GUARD;

__attribute__((noreturn))
void __stack_chk_fail(void) {
    while(1);
}

__attribute__((noreturn))
void __stack_chk_fail_local(void) {
    while(1);
}

extern char* _levelsStrings[10];

u8 _levelColors[10] = {
    7, 7, 5, 14, 4, 4, 8, 10, 4, 1
};

void kout(enum kernel_statusLevels lvl, char* interface, char* message, char* query) {
    // --- APPEND MESSAGE TO DEBUG LOG ---
    debug_message(message, interface, lvl);

    if(query != NULL) {
        debug_append(" ");
        debug_append(query);
    }

    // --- PRINT MESSAGE ON SCREEN ---
    
    if(message == NULL) // ? Nothing to output to screen
        return;

    u8 color = _levelColors[lvl];
    colorPrint(_levelsStrings[lvl], color);
    putc(' ');

    if(interface != NULL) {
        putc('@');
        colorPrint(interface, color);
        putc(':');
        putc(' ');
    }

    colorPrint(message, color);
    
    if(query != NULL) {
        putc(' ');
        colorPrint(query, 15);
    }

    putc('\n');
}

void kernel_exit() {
    asm("cli");

    if(_kernel_exitOk)
        return;

    kout(KERNEL_FATAL, "kernel", "Kernel exited unexceptedly (crashed), call panic.", NULL);
    kernel_panic((REGISTERS*) 0, 6);
}

#define MULTIBOOT_HEADER_MAGIC      0x1BADB002
#define MULTIBOOT_HEADER_FLAGS      0x00000003  // Flags pro framebuffer
#define MULTIBOOT_CHECKSUM          -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

#define FRAMEBUFFER_WIDTH           1280
#define FRAMEBUFFER_HEIGHT          720
#define FRAMEBUFFER_BPP             32

__attribute__((section(".multiboot")))
const struct multiboot_header {
    u32 magic;
    u32 flags;
    u32 checksum;
    u32 header_addr;
    u32 load_addr;
    u32 load_end_addr;
    u32 bss_end_addr;
    u32 entry_addr;
    u32 mode_type;
    u32 width;
    u32 height;
    u32 depth;
} multiboot __attribute__((used)) = {
    MULTIBOOT_HEADER_MAGIC,
    MULTIBOOT_HEADER_FLAGS,
    MULTIBOOT_CHECKSUM,
    0,                  
    0,
    0,
    0,
    0,
    1,                  // 1 znamená framebuffer mode.
    FRAMEBUFFER_WIDTH,
    FRAMEBUFFER_HEIGHT,
    FRAMEBUFFER_BPP
};  

#if defined(_OPT_CPU_RELAX_INST_PAUSE)
    #define __opt_cpu_relax_inst "pause"
#elif defined(_OPT_CPU_RELAX_INST_NOP)
    #define __opt_cpu_relax_inst "nop"
#else
    #define __opt_cpu_relax_inst "nop"
#endif

inline void cpu_relax() {
    __asm__ __volatile__(__opt_cpu_relax_inst ::: "memory");
}

