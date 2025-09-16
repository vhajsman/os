#include "internal_commands.h"
#include "shell.h"
#include "console.h"
#include "kernel.h"
#include "multitask.h"
#include "string.h"

int __mtdump(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {
    IGNORE_UNUSED(tokc);
    IGNORE_UNUSED(callback_stdin);
    kernel_tcb_t* first = mt_getCurrent();
    kernel_tcb_t* t = first;

    char* state_string[] = {
        "NULL\0",
        "READY\0",
        "RUNNING\0",
        "DONE\0"
    };

    if(tokc >= 3) {
        pid_t t_pid = (pid_t) atoi((const char*) tokens[2]);
        if(t_pid > mt_getPidCounter()) {
            callback_stdout("Thread with this PID never existed\n");
            return 1;
        }

        kernel_tcb_t* t_tcb = mt_getTcbByPid(t_pid);
        if(!t_tcb) {
            callback_stdout("Thread with this PID not found. Terminated? Done?\n");
            return 1;
        }

        if(memcmp((u8*) "--thread\0", (u8*) tokens[1], 8) != 0) {
            callback_stdout("Thread:           PID ");
            char pid_str[16]; itoa(pid_str, 10, (int) t_tcb->pid); callback_stdout(pid_str); callback_stdout("\n");

            callback_stdout("State:            "); 
            callback_stdout(state_string[t_tcb->state]);   callback_stdout("\n");
            
            callback_stdout("Routine addr:     0x");
            char fn_addr[16]; itoa(fn_addr, 16, (int) t_tcb->task_func); callback_stdout(fn_addr); callback_stdout("\n");

            callback_stdout("Stack base addr:  0x");
            char sta_str[16]; itoa(sta_str, 16, (int) t_tcb->stack_base); callback_stdout(sta_str); callback_stdout("\n");

            callback_stdout("Stack size:       ");
            char sz_str[16]; itoa(sz_str, 10, (int) t_tcb->stack_size); callback_stdout(sz_str); callback_stdout("\n");

            return 0;
        }

        return 1;
    }

    
    callback_stdout("PID counter ");
    char b[16]; itoa(b, 10, (int) 1 + mt_getPidCounter()); callback_stdout(b);

    callback_stdout(", kmain() PID 0.\nTree command:        mtdump --tree <thread PID>\n");
    callback_stdout("Thread info command: mtdump --thread <thread PID>\n");
    callback_stdout("------------------------------------\n");

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
