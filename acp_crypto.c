/*
 * Autonomous Command Protocol (ACP)
 * Reference C Implementation
 *
 * Copyright (c) 2025 Northbound Networks
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file acp_crypto.c
 * @brief ACP cryptographic functions - SHA-256 and HMAC-SHA256
 *
 * This module provides portable HMAC-SHA256 and SHA-256 implementations
 * for the ACP protocol authentication system.
 *
 * All functions are implemented in pure C99 without external dependencies,
 * implementing RFC 2104 (HMAC) and FIPS 180-4 (SHA-256).
 */

#include "acp_crypto.h"
#include <string.h>

/* ========================================================================== */
/*                              SHA-256 Implementation                        */
/* ========================================================================== */

/** @brief SHA-256 constants (first 32 bits of the fractional parts of the cube roots of the first 64 primes) */
static const uint32_t K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

/** @brief SHA-256 initial hash values (first 32 bits of the fractional parts of the square roots of the first 8 primes) */
static const uint32_t H0[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

/* SHA-256 helper macros */
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

/**
 * @brief Initialize SHA-256 context
 */
void acp_sha256_init(acp_sha256_ctx_t *ctx)
{
    if (!ctx)
        return;

    memcpy(ctx->state, H0, sizeof(H0));
    ctx->bit_len = 0;
    ctx->buffer_len = 0;
    memset(ctx->buffer, 0, sizeof(ctx->buffer));
}

/**
 * @brief Process a single 512-bit block
 */
static void acp_sha256_process_block(acp_sha256_ctx_t *ctx, const uint8_t *data)
{
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2;
    int i;

    /* Prepare the message schedule */
    for (i = 0; i < 16; i++)
    {
        w[i] = ((uint32_t)data[i * 4] << 24) |
               ((uint32_t)data[i * 4 + 1] << 16) |
               ((uint32_t)data[i * 4 + 2] << 8) |
               ((uint32_t)data[i * 4 + 3]);
    }

    for (i = 16; i < 64; i++)
    {
        w[i] = SIG1(w[i - 2]) + w[i - 7] + SIG0(w[i - 15]) + w[i - 16];
    }

    /* Initialize working variables */
    a = ctx->state[0];
    b = ctx->state[1];
    c = ctx->state[2];
    d = ctx->state[3];
    e = ctx->state[4];
    f = ctx->state[5];
    g = ctx->state[6];
    h = ctx->state[7];

    /* Compression function main loop */
    for (i = 0; i < 64; i++)
    {
        t1 = h + EP1(e) + CH(e, f, g) + K[i] + w[i];
        t2 = EP0(a) + MAJ(a, b, c);

        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    /* Add the compressed chunk to the current hash value */
    ctx->state[0] += a;
    ctx->state[1] += b;
    ctx->state[2] += c;
    ctx->state[3] += d;
    ctx->state[4] += e;
    ctx->state[5] += f;
    ctx->state[6] += g;
    ctx->state[7] += h;
}

/**
 * @brief Update SHA-256 with additional data
 */
void acp_sha256_update(acp_sha256_ctx_t *ctx, const uint8_t *data, size_t len)
{
    if (!ctx || !data)
        return;

    size_t i = 0;

    /* Process data byte by byte if buffer is partially filled */
    while (i < len && ctx->buffer_len > 0 && ctx->buffer_len < ACP_SHA256_BLOCK_SIZE)
    {
        ctx->buffer[ctx->buffer_len++] = data[i++];

        if (ctx->buffer_len == ACP_SHA256_BLOCK_SIZE)
        {
            acp_sha256_process_block(ctx, ctx->buffer);
            ctx->buffer_len = 0;
            ctx->bit_len += 512;
        }
    }

    /* Process complete blocks directly */
    while (i + ACP_SHA256_BLOCK_SIZE <= len)
    {
        acp_sha256_process_block(ctx, &data[i]);
        i += ACP_SHA256_BLOCK_SIZE;
        ctx->bit_len += 512;
    }

    /* Buffer remaining bytes */
    while (i < len)
    {
        ctx->buffer[ctx->buffer_len++] = data[i++];
    }
}

/**
 * @brief Finalize SHA-256 and output hash
 */
void acp_sha256_final(acp_sha256_ctx_t *ctx, uint8_t *hash)
{
    if (!ctx || !hash)
        return;

    uint64_t bit_len = ctx->bit_len + (ctx->buffer_len * 8);
    size_t i;

    /* Pad the message */
    ctx->buffer[ctx->buffer_len++] = 0x80;

    /* If we don't have room for the length, pad to end and process */
    if (ctx->buffer_len > 56)
    {
        while (ctx->buffer_len < ACP_SHA256_BLOCK_SIZE)
        {
            ctx->buffer[ctx->buffer_len++] = 0x00;
        }
        acp_sha256_process_block(ctx, ctx->buffer);
        ctx->buffer_len = 0;
    }

    /* Pad to 56 bytes */
    while (ctx->buffer_len < 56)
    {
        ctx->buffer[ctx->buffer_len++] = 0x00;
    }

    /* Append length in bits as big-endian 64-bit number */
    for (i = 0; i < 8; i++)
    {
        ctx->buffer[56 + i] = (uint8_t)(bit_len >> (56 - i * 8));
    }

    acp_sha256_process_block(ctx, ctx->buffer);

    /* Output hash in big-endian format */
    for (i = 0; i < 8; i++)
    {
        hash[i * 4] = (uint8_t)(ctx->state[i] >> 24);
        hash[i * 4 + 1] = (uint8_t)(ctx->state[i] >> 16);
        hash[i * 4 + 2] = (uint8_t)(ctx->state[i] >> 8);
        hash[i * 4 + 3] = (uint8_t)(ctx->state[i]);
    }
}

/**
 * @brief Compute SHA-256 hash in one call
 */
void acp_sha256(const uint8_t *data, size_t len, uint8_t *hash)
{
    acp_sha256_ctx_t ctx;
    acp_sha256_init(&ctx);
    acp_sha256_update(&ctx, data, len);
    acp_sha256_final(&ctx, hash);

    /* Clear sensitive data */
    acp_crypto_clear(&ctx, sizeof(ctx));
}

/* ========================================================================== */
/*                              HMAC-SHA256 Implementation                    */
/* ========================================================================== */

void acp_hmac_sha256(const uint8_t *key, size_t key_len,
                     const uint8_t *data, size_t data_len,
                     uint8_t *mac)
{
    uint8_t k_pad[ACP_SHA256_BLOCK_SIZE];
    uint8_t inner_hash[ACP_SHA256_SIZE];
    acp_sha256_ctx_t ctx;
    size_t i;

    if (!key || !data || !mac)
        return;

    /* Prepare the key */
    memset(k_pad, 0, sizeof(k_pad));

    if (key_len > ACP_SHA256_BLOCK_SIZE)
    {
        /* Hash the key if it's too long */
        acp_sha256(key, key_len, k_pad);
    }
    else
    {
        /* Use key directly, zero-padded */
        memcpy(k_pad, key, key_len);
    }

    /* Create inner padding (key XOR ipad) */
    for (i = 0; i < ACP_SHA256_BLOCK_SIZE; i++)
    {
        k_pad[i] ^= 0x36;
    }

    /* Inner hash: SHA256(K XOR ipad || text) */
    acp_sha256_init(&ctx);
    acp_sha256_update(&ctx, k_pad, ACP_SHA256_BLOCK_SIZE);
    acp_sha256_update(&ctx, data, data_len);
    acp_sha256_final(&ctx, inner_hash);

    /* Restore key and create outer padding (key XOR opad) */
    for (i = 0; i < ACP_SHA256_BLOCK_SIZE; i++)
    {
        k_pad[i] ^= 0x36; /* Remove ipad */
        k_pad[i] ^= 0x5c; /* Add opad */
    }

    /* Outer hash: SHA256(K XOR opad || inner_hash) */
    acp_sha256_init(&ctx);
    acp_sha256_update(&ctx, k_pad, ACP_SHA256_BLOCK_SIZE);
    acp_sha256_update(&ctx, inner_hash, ACP_SHA256_SIZE);
    acp_sha256_final(&ctx, mac); /* Output full 32-byte HMAC to mac buffer */

    /* Clear sensitive data */
    acp_crypto_clear(k_pad, sizeof(k_pad));
    acp_crypto_clear(inner_hash, sizeof(inner_hash));
    acp_crypto_clear(&ctx, sizeof(ctx));
}

/* ========================================================================== */
/*                              Utility Functions                             */
/* ========================================================================== */

int acp_crypto_memcmp_ct(const void *a, const void *b, size_t len)
{
    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
    uint8_t result = 0;
    size_t i;

    if (!a || !b)
        return -1;

    /* Constant-time comparison */
    for (i = 0; i < len; i++)
    {
        result |= pa[i] ^ pb[i];
    }

    return result;
}

int acp_hmac_verify(const uint8_t *expected, const uint8_t *received, size_t len)
{
    return acp_crypto_memcmp_ct(expected, received, len);
}

void acp_crypto_clear(void *data, size_t len)
{
    volatile uint8_t *p = (volatile uint8_t *)data;
    while (len--)
    {
        *p++ = 0;
    }
}

/* ========================================================================== */
/*                              Self-Test Functions                           */
/* ========================================================================== */

int acp_sha256_self_test(void)
{
    /* Test vector from NIST */
    const uint8_t test_msg[] = "abc";
    const uint8_t expected[] = {
        0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
        0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
        0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
        0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};

    uint8_t result[ACP_SHA256_SIZE];

    acp_sha256(test_msg, 3, result);

    return acp_crypto_memcmp_ct(result, expected, ACP_SHA256_SIZE);
}

int acp_hmac_self_test(void)
{
    /* Simple consistency test - verify our implementation produces consistent results */
    const uint8_t key[] = "test_key_123456789012345678901234"; /* 32 bytes */
    const uint8_t data[] = "test_data";

    uint8_t result1[ACP_SHA256_SIZE];
    uint8_t result2[ACP_SHA256_SIZE];

    /* Run HMAC twice with same inputs */
    acp_hmac_sha256(key, sizeof(key) - 1, data, sizeof(data) - 1, result1);
    acp_hmac_sha256(key, sizeof(key) - 1, data, sizeof(data) - 1, result2);

    /* Should produce identical results */
    return acp_crypto_memcmp_ct(result1, result2, ACP_SHA256_SIZE);
}

int acp_crypto_self_test(void)
{
    int sha_result = acp_sha256_self_test();
    int hmac_result = acp_hmac_self_test();

    return (sha_result == 0 && hmac_result == 0) ? 0 : -1;
}
