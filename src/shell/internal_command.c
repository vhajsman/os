#include "internal_commands.h"
#include "shell.h"

#define INCLUDE_INTERNAL_COMMAND(__COMMAND_NAME)                            \
    extern int __##__COMMAND_NAME(                                          \
        char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH],              \
        int tokc,                                                           \
        void (*callback_stdout) (char*),                                    \
        char* (*callback_stdin) (void)                                      \
    );                                                                      \
                                                                            \
    struct shell_command command_##__COMMAND_NAME =                         \
    CREATE_INTERNAL_COMMAND(0, 0, #__COMMAND_NAME, __##__COMMAND_NAME);

INCLUDE_INTERNAL_COMMAND(test);
INCLUDE_INTERNAL_COMMAND(hwinfo);
INCLUDE_INTERNAL_COMMAND(echo);
INCLUDE_INTERNAL_COMMAND(krnldbg);
INCLUDE_INTERNAL_COMMAND(help);
INCLUDE_INTERNAL_COMMAND(exp);
INCLUDE_INTERNAL_COMMAND(cd);
INCLUDE_INTERNAL_COMMAND(df);
INCLUDE_INTERNAL_COMMAND(mount);

void shell_registerAll() {
    registerCommand(&command_test);
    registerCommand(&command_hwinfo);
    registerCommand(&command_echo);
    registerCommand(&command_krnldbg);
    registerCommand(&command_cd);
    registerCommand(&command_help);
    registerCommand(&command_exp);
    registerCommand(&command_df);
    registerCommand(&command_mount);
}