#ifdef _BUILD_INSIDERS

#include "console.h"
#include "shell.h"
#include "kernel.h"
#include "string.h"

#include "irq/isr.h"
#include "irq/irqdef.h"

extern void shell_printError(char* kind, char* message, int start, int end);

void __irqlookup_outnum(int num, int base, void(*callback_stdout)(char*)) {
    if(base == 16)
        callback_stdout("0x");

    char buff[16];
    itoa(buff, base, num);
    callback_stdout(buff);
}

#define outnum(num, base) \
    __irqlookup_outnum((int) num, base, callback_stdout);

int __irqlookup(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {
    callback_stdout("IRQ_BASE = ");
    outnum(IRQ_BASE, 16);
    callback_stdout("\n");

    for(int i = 0; i < NO_INTERRUPT_HANDLERS; i++) {
        isrpb_t* p = isr_getParamBlock(i);
        if(p->handler == NULL)
            continue;

        callback_stdout("IRQ_BASE+"); 
        outnum(i, 16);
        callback_stdout(" -> ("); 
        outnum((u32) p->handler, 16); 
        callback_stdout(") "); 
        callback_stdout(debug_lookup((uintptr_t) p->handler));
        callback_stdout(": ");
        
        if(p->additionalContextResolv) {
            callback_stdout("ACR=");
            outnum((u32) p->additionalContextResolv, 16);
            callback_stdout(" ");
        }

        callback_stdout("\n -> ");
        callback_stdout("avgTPS=");     outnum(p->avgTrigPerSecond, 10);
        callback_stdout(" avgTE=");     outnum(p->avgTimeElapsed, 10);
        callback_stdout(" minTE=");     outnum(p->minTimeElapsed, 10);
        callback_stdout(" maxTE=");     outnum(p->maxTimeElapsed, 10);
        callback_stdout(" lastTrig=");  outnum(p->trigTimestamp, 10);
        callback_stdout(", complete "); outnum(p->completeCount, 10);
        callback_stdout(" of "); outnum(p->trigCount, 10);
        
        callback_stdout(", flags=");
        outnum(p->flags, 16);

        callback_stdout("\n");
    }

    return 0;
}
#endif
