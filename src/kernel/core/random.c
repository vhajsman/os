#include "random.h"
#include "string.h"
#include "time/rtc.h"
#include "hash.h"
#include "debug.h"

static u32 xor128_step(u32 state[4]) {
    u32 t = state[0] ^ (state[0] << 11);

    state[0] = state[1]; 
    state[1] = state[2]; 
    state[2] = state[3];
    state[3] = state[3] ^ (state[3] >> 19) ^ (t ^ (t >> 8));

    return state[3];
}

void rng_simple(struct rng_ctx* ctx, u8* out, size_t len) {
    for(size_t i = 0; i < len; i++) {
        u32 r = xor128_step(ctx->state);
        r ^= (u32)(ctx->counter++);

        out[i] = (u8)(r >> ((i % 4) * 8));
    }
}

void rng_init(struct rng_ctx* ctx, const rng_seed_t seed) {
    ctx->counter = 0;
    ctx->generate = rng_simple;
    memcpy(ctx->seed, seed, _RNG_SEED_BYTES);

    for(int i = 0; i < 4; i++)
        memcpy(&ctx->state[i], &seed[i * 4], sizeof(u32));
}

void rng(struct rng_ctx* ctx, u8* out, size_t len) {
    if (ctx->generate) {
        ctx->generate(ctx, out, len);
    } else {
        debug_message("RNG generate function is NULL!", "RNG", KERNEL_ERROR);
    }
}
