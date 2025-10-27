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
 * @file acp_crypto.h
 * @brief ACP cryptographic functions - SHA-256 and HMAC-SHA256
 *
 * Provides portable implementations of SHA-256 hash and HMAC-SHA256
 * message authentication code functions for the ACP protocol.
 *
 * Features:
 * - Pure C99 implementation, no external dependencies
 * - Constant-time operations for security-critical functions
 * - Self-test vectors for validation
 * - Optimized for embedded and cross-platform use
 *
 * @version 0.3.0
 * @date 2025-10-27
 */

#ifndef ACP_CRYPTO_H
#define ACP_CRYPTO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>

/* ========================================================================== */
/*                              Constants                                     */
/* ========================================================================== */

/** @brief SHA-256 hash output size in bytes */
#define ACP_SHA256_SIZE 32

/** @brief SHA-256 block size in bytes */
#define ACP_SHA256_BLOCK_SIZE 64

/** @brief HMAC-SHA256 output size in bytes */
#define ACP_HMAC_SIZE 16 /**< Truncated to 16 bytes per ACP spec */

/** @brief HMAC-SHA256 full output size (before truncation) */
#define ACP_HMAC_FULL_SIZE 32

/** @brief Maximum key length for HMAC */
#define ACP_HMAC_MAX_KEY_SIZE 64

    /* ========================================================================== */
    /*                              Data Structures                              */
    /* ========================================================================== */

    /**
     * @brief SHA-256 context structure for incremental hashing
     */
    typedef struct
    {
        uint32_t state[8];  /**< Hash state (8x 32-bit words) */
        uint8_t buffer[64]; /**< Input buffer for partial blocks */
        uint64_t bit_len;   /**< Total bit length processed */
        size_t buffer_len;  /**< Current buffer fill level */
    } acp_sha256_ctx_t;

    /**
     * @brief HMAC-SHA256 context structure for incremental authentication
     */
    typedef struct
    {
        acp_sha256_ctx_t inner; /**< Inner hash context */
        acp_sha256_ctx_t outer; /**< Outer hash context */
        uint8_t key_pad[64];    /**< Padded key storage */
    } acp_hmac_ctx_t;

    /* ========================================================================== */
    /*                            SHA-256 Functions                              */
    /* ========================================================================== */

    /**
     * @brief Initialize SHA-256 context
     * @param ctx Context to initialize
     */
    void acp_sha256_init(acp_sha256_ctx_t *ctx);

    /**
     * @brief Update SHA-256 hash with new data
     * @param ctx SHA-256 context
     * @param data Input data to hash
     * @param len Length of input data in bytes
     */
    void acp_sha256_update(acp_sha256_ctx_t *ctx, const uint8_t *data, size_t len);

    /**
     * @brief Finalize SHA-256 hash and produce output
     * @param ctx SHA-256 context
     * @param hash Output buffer (must be at least 32 bytes)
     */
    void acp_sha256_final(acp_sha256_ctx_t *ctx, uint8_t *hash);

    /**
     * @brief Compute SHA-256 hash in one operation
     * @param data Input data to hash
     * @param len Length of input data
     * @param hash Output hash (32 bytes)
     */
    void acp_sha256(const uint8_t *data, size_t len, uint8_t *hash);

    /* ========================================================================== */
    /*                           HMAC-SHA256 Functions                           */
    /* ========================================================================== */

    /**
     * @brief Initialize HMAC-SHA256 context
     * @param ctx HMAC context to initialize
     * @param key HMAC key
     * @param key_len Length of key in bytes
     */
    void acp_hmac_init(acp_hmac_ctx_t *ctx, const uint8_t *key, size_t key_len);

    /**
     * @brief Update HMAC with new data
     * @param ctx HMAC context
     * @param data Input data to authenticate
     * @param len Length of input data in bytes
     */
    void acp_hmac_update(acp_hmac_ctx_t *ctx, const uint8_t *data, size_t len);

    /**
     * @brief Finalize HMAC and produce authentication tag
     * @param ctx HMAC context
     * @param mac Output MAC (16 bytes truncated, or 32 bytes full)
     * @param truncated If true, output 16-byte truncated MAC
     */
    void acp_hmac_final(acp_hmac_ctx_t *ctx, uint8_t *mac, int truncated);

    /**
     * @brief Compute HMAC-SHA256 in one operation
     * @param key HMAC key
     * @param key_len Length of key in bytes
     * @param data Input data to authenticate
     * @param data_len Length of input data
     * @param mac Output MAC (16 bytes)
     */
    void acp_hmac_sha256(const uint8_t *key, size_t key_len,
                         const uint8_t *data, size_t data_len,
                         uint8_t *mac);

    /* ========================================================================== */
    /*                            Utility Functions                              */
    /* ========================================================================== */

    /**
     * @brief Constant-time memory comparison
     *
     * Compares two memory regions in constant time to prevent timing attacks.
     * This is critical for MAC verification.
     *
     * @param a First memory region
     * @param b Second memory region
     * @param len Number of bytes to compare
     * @return 0 if equal, non-zero if different
     */
    int acp_crypto_memcmp_ct(const void *a, const void *b, size_t len);

    /**
     * @brief Verify HMAC-SHA256 authentication tag
     *
     * Performs constant-time verification of HMAC tag to prevent timing attacks.
     *
     * @param expected Expected MAC value
     * @param received Received MAC value to verify
     * @param len Length of MAC (typically 16 bytes for ACP)
     * @return 0 if verification succeeds, non-zero if fails
     */
    int acp_hmac_verify(const uint8_t *expected, const uint8_t *received, size_t len);

    /**
     * @brief Clear sensitive data from memory
     *
     * Securely clears memory containing sensitive data like keys or contexts.
     * Prevents compiler optimization from eliminating the clear operation.
     *
     * @param data Memory to clear
     * @param len Number of bytes to clear
     */
    void acp_crypto_clear(void *data, size_t len);

    /* ========================================================================== */
    /*                             Test Functions                                */
    /* ========================================================================== */

    /**
     * @brief Run SHA-256 self-tests with known test vectors
     * @return 0 if all tests pass, non-zero if any test fails
     */
    int acp_sha256_self_test(void);

    /**
     * @brief Run HMAC-SHA256 self-tests with known test vectors
     * @return 0 if all tests pass, non-zero if any test fails
     */
    int acp_hmac_self_test(void);

    /**
     * @brief Run all cryptographic self-tests
     * @return 0 if all tests pass, non-zero if any test fails
     */
    int acp_crypto_self_test(void);

#ifdef __cplusplus
}
#endif

#endif /* ACP_CRYPTO_H */
