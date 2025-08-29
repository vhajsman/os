#ifdef _BUILD_INSIDERS

#include "console.h"
#include "shell.h"
#include "kernel.h"
#include "string.h"
#include "random.h"

int __random(char tokens[SHELL_MAX_TOKENS][SHELL_MAX_TOKEN_LENGTH], int tokc, void (*callback_stdout) (char*), char* (*callback_stdin) (void)) {
    IGNORE_UNUSED(tokc);
    IGNORE_UNUSED(tokens);
    IGNORE_UNUSED(callback_stdin);

    u8 rand[_RNG_SEED_BYTES + 1];
    rng(&kernel_seed_ctx, rand, _RNG_SEED_BYTES);
    rand[_RNG_SEED_BYTES] = '\0';

    callback_stdout((char*) rand);

    return 0;
}
#endif
