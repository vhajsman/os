#ifdef _BUILD_INSIDERS

#include "console.h"
#include "shell.h"
#include "kernel.h"
#include "string.h"
#include "memory/memory.h"
#include "string.h"

extern void shell_printError(char* kind, char* message, int start, int end);

int __mmdu(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {

    if(!strcmp("help\0", tokens[1])) {
        callback_stdout("MMDU - Memory Management Debug Utility\n\n");

        callback_stdout("USAGE: mmdu <sub-command> [options]\n");
        callback_stdout("sub-command    options\n");
        callback_stdout("----------------------------------------\n");
        callback_stdout("malloc         <size> [count]\n");
        callback_stdout("bestfit        <size> [count]\n");
        callback_stdout("free           <address>\n");
        callback_stdout("list           [<address-start> <length>]\n");
        callback_stdout("dump           <address-start> <length> [ASCII|hex]\n\n");

        callback_stdout("ALL NUMBERS IN HEXADECIMAL!!!");

        return 0;
    }

    if(!strcmp("malloc\0", tokens[1])) {
        unsigned int count = tokc == 4 ? atoi(tokens[3]) : 1;
        for(unsigned int i = 0; i < count; i ++) {
            void* addr = kmalloc(atoi(tokens[2]));
    
            char buf_res[32];
            itoa(buf_res, 16, (u32) &addr);
            callback_stdout(addr == NULL ? "null" : buf_res);
            callback_stdout("\n");
        }

        return 0;
    }

    if(!strcmp("bestfit\0", tokens[1])) {
        unsigned int count = tokc == 4 ? atoi(tokens[3]) : 1;
        for(unsigned int i = 0; i < count; i ++) {
            void* addr = (void*) memory_bestfit(atoi(tokens[2]));
    
            char buf_res[32];
            itoa(buf_res, 16, (u32) &addr);
            callback_stdout(addr == NULL ? "null" : buf_res);
            callback_stdout("\n");
        }

        return 0;
    }

    if(!strcmp("free\0", tokens[1])) {
        void* addr = (void*) atoi(tokens[2]);
        kfree(addr);

        return 0;
    }

    shell_printError("Error", "Invalid sub-command", 6, strlen(tokens[1]) - 1);
    return 2;
}
#endif