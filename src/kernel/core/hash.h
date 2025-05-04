#ifndef __HASH_H
#define __HASH_H

#include "types.h"

typedef struct {
    u32 state[8];
    u64 bitlen;

    u8 data[64];
    u64 datalen;
} sha256_ctx;

void sha256_init(sha256_ctx* ctx);
void sha256_update(sha256_ctx* ctx, const u8* data, size_t len);
void sha256_final(sha256_ctx* ctx, u8 hash[32]);

void sha256_wrapper(const u8* data, size_t len, u8 out[32]);

#ifndef HASH 
#define HASH(DATA, LEN, OUT) \
    sha256_wrapper(DATA, LEN, OUT)
#endif

#endif
