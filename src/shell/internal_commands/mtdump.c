#include "internal_commands.h"
#include "shell.h"
#include "console.h"
#include "kernel.h"
#include "multitask.h"
#include "string.h"

int __mtdump(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {
    IGNORE_UNUSED(tokc);
    IGNORE_UNUSED(callback_stdin);

    callback_stdout("Thread count: ");
    char b[16]; itoa(b, 10, (int) 1 + mt_getPidCounter()); callback_stdout(b);

    callback_stdout("\n\n");

    kernel_tcb_t* first = mt_getCurrent();
    kernel_tcb_t* t = first;

    char* state_string[] = {
        "NULL\0",
        "READY\0",
        "RUNNING\0",
        "DONE\0"
    };

    do {
        callback_stdout("Thread: PID ");
        char pid_str[16];   itoa(pid_str, 10, (int) t->pid);        callback_stdout(pid_str);
        callback_stdout(": next ");
        char nxt_str[16];   itoa(nxt_str, 10, (int) t->next->pid);  callback_stdout(nxt_str);

        if(t->parent) {
            callback_stdout(" parent ");
            char par_str[16]; itoa(par_str, 10, (int) t->parent->pid);
            callback_stdout(par_str);
        }

        callback_stdout(" state: ");
        callback_stdout(state_string[t->state]);
        callback_stdout("\n");

        t = t->next;
    } while(t != first);

    return 0;
}
