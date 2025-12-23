#ifdef _BUILD_INSIDERS

#include "console.h"
#include "shell.h"
#include "kernel.h"
#include "string.h"
#include "time/timer.h"

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

const char* flagName(unsigned int bit) {
    switch (bit) {
        case IsrActive:         return "IsrActive";
        case IsrEnabled:        return "IsrEnabled";
        case IsrPriorityHigh:   return "IsrPriorityHigh";
        case IsrPriorityCrit:   return "IsrPriorityCrit";
        case IsrReentrant:      return "IsrReentrant";
        case IsrDoStats:        return "IsrDoStats";
        case IsrShared:         return "IsrShared";
        case IsrWakeup:         return "IsrWakeup";
        default:
            return "Unknown";
    }
}

int __irqlookup(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {
    callback_stdout("IRQ_BASE = ");
    outnum(IRQ_BASE, 16);
    callback_stdout("\n");

    if(tokc > 1) {
        if(!strcmp("--help\0", tokens[1])) {
            callback_stdout("irqlookup                    lists lookup for every registered IRQ handler\n");
            callback_stdout("irqlookup --help             shows this\n");
            callback_stdout("irqlookup --show-irq [irq]   shows detailed lookup for IRQ handler specified\n");
            callback_stdout("irqlookup --decode-flags [f] decodes IRQ parameter block table flags\n");
            callback_stdout("\n");
            callback_stdout("when passing [irq] argument, the utility assumes IRQ_BASE is already trimmed.\n");
            callback_stdout("when passing [f] or [irq] argument, use hexadecimal format without prefix/suffix.\n");

            return 0;
        }

        if(!strcmp("--show-irq\0", tokens[1])) {
            unsigned int i = atoi(tokens[2]);
            isrpb_t* p = isr_getParamBlock(i);
            if(p->handler == NULL) {
                callback_stdout("handler not registered.\n");
                return 0;
            }

            callback_stdout("               IRQ no. : "); outnum(i, 10); callback_stdout("\n");
            callback_stdout("               Handler : "); outnum((u32) p->handler, 16);callback_stdout("\n");
            callback_stdout("                         -> "); callback_stdout(debug_lookup((uintptr_t) p->handler)); callback_stdout("\n");
            callback_stdout("   additionalCtxResolv : ");

            if(p->additionalContextResolv) {
                outnum((u32) p->additionalContextResolv, 16); callback_stdout("\n");
                callback_stdout("                         -> "); callback_stdout(debug_lookup((uintptr_t) p->additionalContextResolv));
            }

            callback_stdout("\n");

            u64 time_tot = ((p->flags & IsrDoStats) && !(p->flags & IsrPriorityCrit) && p->avgTimeElapsed != 0) 
                ? (p->completeCount * p->avgTimeElapsed)
                : 0;

            callback_stdout(" IRQ acknowledge count : "); outnum(p->trigCount, 10); callback_stdout(" times\n");
            callback_stdout("    ISR complete count : "); outnum(p->completeCount, 10); callback_stdout(" times\n");
            callback_stdout("        last timestamp : "); outnum(p->trigTimestamp, 10); callback_stdout("\n");

            callback_stdout("  average time elapsed : "); outnum(p->avgTimeElapsed, 10); callback_stdout(" ticks\n"); 
            callback_stdout("  minimal time elapsed : "); outnum(p->minTimeElapsed, 10); callback_stdout(" ticks\n");
            callback_stdout("  maximal time elapsed : "); outnum(p->maxTimeElapsed, 10); callback_stdout(" ticks\n");

            callback_stdout(" average IRQ frequency : "); outnum(p->avgTrigPerSecond, 10); callback_stdout(" per second\n");

            if(time_tot) {
                callback_stdout("    total time elapsed : "); outnum(time_tot, 10); callback_stdout(" ticks\n");
            }

            callback_stdout(" parameter block flags : ");
            if(p->flags == 0) {
                callback_stdout("(no flags)\n");
                return 0;
            }

            int first = 1;
            for(unsigned int bit = 1; bit != 0; bit <<= 1) {
                if(p->flags & bit) {
                    if(!first)
                        callback_stdout("\n                         ");

                    callback_stdout((char*) flagName(bit));
                    first = 0;
                }
            }

            callback_stdout("\n");
            return 0;
        }

        if(!strcmp("--decode-flags\0", tokens[1])) {
            unsigned int flags = atoi(tokens[2]);
            if(flags == 0) {
                callback_stdout("This IRQ has not flags.\n");
                return 0;
            }

            int first = 1;
            for(unsigned int bit = 1; bit != 0; bit <<= 1) {
                if(flags & bit) {
                    if(!first)
                        callback_stdout(" | ");

                    callback_stdout((char*) flagName(bit));
                    first = 0;
                }
            }

            return 0;
        }

        callback_stdout("invalid arguments.\n");
        return 1;
    }
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
        callback_stdout(" cc="); outnum(p->completeCount, 10);
        callback_stdout(" tc="); outnum(p->trigCount, 10);
        
        callback_stdout(" flags=");
        outnum(p->flags, 16);

        callback_stdout("\n");
    }

    return 0;
}
#endif
