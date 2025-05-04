#ifndef __RANDOM_H
#define __RANDOM_H

#include "types.h"

#ifndef _RNG_SEED_BYTES
#define _RNG_SEED_BYTES 32
#endif

typedef u8 rng_seed_t[_RNG_SEED_BYTES];

struct rng_ctx {
    rng_seed_t seed;
    u32 state[4];
    u64 counter;

    void (*generate)(struct rng_ctx* ctx, u8* out, size_t len);
};

void rng_init(struct rng_ctx* ctx, const rng_seed_t seed);
void rng(struct rng_ctx* ctx, u8* out, size_t len);

#endif
