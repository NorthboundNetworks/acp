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
 * @brief ACP Cryptographic Functions Implementation
 *
 * Provides HMAC-SHA256 authentication, key management, and cryptographic utilities
 * for the Autonomous Control Protocol (ACP) v0.3 using mbedTLS.
 *
 * @version 0.3
 * @date 2025-10-26
 * @author Paul Zanna
 */

#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include "acp_platform_log.h"
#include "acp_platform_keystore.h"
#include "acp_platform_entropy.h"
#include "mbedtls/md.h"
#include "mbedtls/pkcs5.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "acp_crypto.h"
#include "acp_protocol.h"

// TAG macro removed for platform abstraction

// ===========================================================================
// GLOBAL STATE AND CONSTANTS
// ===========================================================================

#define ACP_NVS_NAMESPACE "acp_keys"
#define ACP_NVS_KEY_PREFIX "key_"
#define ACP_KEYSTORE_MAX_KEYS 16
#define ACP_CRYPTO_MAGIC 0x53435031 // "ACP1" magic number

/**
 * @brief Crypto subsystem state
 */
static struct
{
    bool initialized;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    nvs_handle_t nvs_handle;
    acp_crypto_stats_t stats;
} crypto_state = {.initialized = false};

// ===========================================================================
// UTILITY FUNCTIONS
// ===========================================================================

/**
 * @brief Secure memory clear to prevent key recovery
 *
 * Uses volatile pointer to prevent compiler optimization.
 */
void acp_crypto_secure_memclear(void *ptr, uint16_t size)
{
    if (!ptr || size == 0)
        return;

    volatile uint8_t *vptr = (volatile uint8_t *)ptr;
    for (uint16_t i = 0; i < size; i++)
    {
        vptr[i] = 0;
    }
}

/**
 * @brief Constant-time memory compare to prevent timing attacks
 */
bool acp_crypto_secure_memcmp(const void *a, const void *b, uint16_t size)
{
    if (!a || !b)
        return false;

    const uint8_t *pa = (const uint8_t *)a;
    const uint8_t *pb = (const uint8_t *)b;
    uint8_t result = 0;

    for (uint16_t i = 0; i < size; i++)
    {
        result |= (pa[i] ^ pb[i]);
    }

    return (result == 0);
}

// ===========================================================================
// ENTROPY AND RANDOM NUMBER GENERATION
// ===========================================================================

/**
 * @brief Initialize entropy and random number generation
 */
static int init_entropy(void)
{
    int ret;

    // Initialize entropy context
    mbedtls_entropy_init(&crypto_state.entropy);
    mbedtls_ctr_drbg_init(&crypto_state.ctr_drbg);

    // Seed the random number generator
    const char *pers = "acp_crypto_rng";
    ret = mbedtls_ctr_drbg_seed(&crypto_state.ctr_drbg,
                                mbedtls_entropy_func,
                                &crypto_state.entropy,
                                (const uint8_t *)pers,
                                strlen(pers));
    if (ret != 0)
    {
        ACP_LOG_ERROR("mbedtls_ctr_drbg_seed failed: -0x%04X", -ret);
        return ret;
    }

    ACP_LOG_INFO("Entropy and RNG initialized");
    return 0;
}

int acp_crypto_get_random(uint8_t *buffer, uint16_t size)
{
    if (!buffer || size == 0)
    {
        return ACP_CRYPTO_ERR_INVALID_PARAM;
    }

    if (!crypto_state.initialized)
    {
        return ACP_CRYPTO_ERR_HARDWARE;
    }

    int ret = mbedtls_ctr_drbg_random(&crypto_state.ctr_drbg, buffer, size);
    if (ret != 0)
    {
        ACP_LOG_ERROR("Random generation failed: -0x%04X", -ret);
        return ACP_CRYPTO_ERR_HARDWARE;
    }

    crypto_state.stats.random_bytes_generated += size;
    return ACP_CRYPTO_OK;
}

int acp_crypto_seed_rng(const uint8_t *seed, uint16_t seed_size)
{
    if (!crypto_state.initialized)
    {
        return ACP_CRYPTO_ERR_HARDWARE;
    }

    if (seed && seed_size > 0)
    {
        int ret = mbedtls_ctr_drbg_reseed(&crypto_state.ctr_drbg, seed, seed_size);
        if (ret != 0)
        {
            ACP_LOG_ERROR("RNG reseeding failed: -0x%04X", -ret);
            return ACP_CRYPTO_ERR_HARDWARE;
        }
        ACP_LOG_INFO("RNG reseeded with %u bytes", seed_size);
    }

    return ACP_CRYPTO_OK;
}

// ===========================================================================
// HMAC-SHA256 IMPLEMENTATION
// ===========================================================================

int acp_crypto_hmac_generate(const uint8_t *key,
                             const uint8_t *message,
                             uint16_t message_len,
                             uint8_t *tag_out)
{

    if (!key || !message || !tag_out || message_len == 0)
    {
        return ACP_CRYPTO_ERR_INVALID_PARAM;
    }

    if (!crypto_state.initialized)
    {
        return ACP_CRYPTO_ERR_HARDWARE;
    }

    uint8_t hmac_full[ACP_HMAC_FULL_SIZE];
    int ret;

    // Calculate HMAC-SHA256
    ret = mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
                          key, ACP_HMAC_KEY_SIZE,
                          message, message_len,
                          hmac_full);

    if (ret != 0)
    {
        ACP_LOG_ERROR("HMAC generation failed: -0x%04X", -ret);
        return ACP_CRYPTO_ERR_HARDWARE;
    }

    // Truncate to 128 bits (16 bytes) for protocol efficiency
    memcpy(tag_out, hmac_full, ACP_HMAC_TAG_SIZE);

    // Clear full HMAC from memory
    acp_crypto_secure_memclear(hmac_full, sizeof(hmac_full));

    crypto_state.stats.hmac_operations++;

    ACP_LOG_DEBUG("HMAC generated for %u bytes", message_len);
    return ACP_CRYPTO_OK;
}

int acp_crypto_hmac_verify(const uint8_t *key,
                           const uint8_t *message,
                           uint16_t message_len,
                           const uint8_t *tag_expected)
{

    if (!key || !message || !tag_expected || message_len == 0)
    {
        return ACP_CRYPTO_ERR_INVALID_PARAM;
    }

    uint8_t tag_calculated[ACP_HMAC_TAG_SIZE];

    // Generate HMAC for comparison
    int ret = acp_crypto_hmac_generate(key, message, message_len, tag_calculated);
    if (ret != ACP_CRYPTO_OK)
    {
        return ret;
    }

    // Constant-time comparison to prevent timing attacks
    bool match = acp_crypto_secure_memcmp(tag_calculated, tag_expected, ACP_HMAC_TAG_SIZE);

    // Clear calculated tag from memory
    acp_crypto_secure_memclear(tag_calculated, sizeof(tag_calculated));

    if (match)
    {
        crypto_state.stats.auth_successes++;
        ACP_LOG_DEBUG("HMAC verification successful");
        return ACP_CRYPTO_OK;
    }
    else
    {
        crypto_state.stats.auth_failures++;
        ACP_LOG_WARN("HMAC verification failed");
        return ACP_CRYPTO_ERR_AUTH_FAIL;
    }
}

// ===========================================================================
// KEY GENERATION AND DERIVATION
// ===========================================================================

int acp_crypto_generate_key(uint8_t *key_out)
{
    if (!key_out)
    {
        return ACP_CRYPTO_ERR_INVALID_PARAM;
    }

    int ret = acp_crypto_get_random(key_out, ACP_HMAC_KEY_SIZE);
    if (ret == ACP_CRYPTO_OK)
    {
        crypto_state.stats.key_generations++;
        ACP_LOG_INFO("Generated new cryptographic key");
    }

    return ret;
}

int acp_crypto_derive_key(const char *password,
                          uint16_t password_len,
                          const uint8_t *salt,
                          uint32_t iterations,
                          uint8_t *key_out)
{

    if (!password || !salt || !key_out || password_len == 0)
    {
        return ACP_CRYPTO_ERR_INVALID_PARAM;
    }

    if (iterations < 1000)
    {
        ACP_LOG_WARN("Low iteration count: %u", iterations);
    }

    mbedtls_md_context_t md_ctx;
    mbedtls_md_init(&md_ctx);

    int ret = mbedtls_md_setup(&md_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    if (ret != 0)
    {
        ACP_LOG_ERROR("MD setup failed: -0x%04X", -ret);
        mbedtls_md_free(&md_ctx);
        return ACP_CRYPTO_ERR_HARDWARE;
    }

    ret = mbedtls_pkcs5_pbkdf2_hmac(&md_ctx,
                                    (const uint8_t *)password, password_len,
                                    salt, ACP_SALT_SIZE,
                                    iterations,
                                    ACP_HMAC_KEY_SIZE,
                                    key_out);

    mbedtls_md_free(&md_ctx);

    if (ret != 0)
    {
        ACP_LOG_ERROR("PBKDF2 failed: -0x%04X", -ret);
        return ACP_CRYPTO_ERR_HARDWARE;
    }

    crypto_state.stats.key_generations++;
    ACP_LOG_INFO("Derived key from password (%u iterations)", iterations);

    return ACP_CRYPTO_OK;
}

// ===========================================================================
// KEY STORE IMPLEMENTATION
// ===========================================================================

int acp_crypto_keystore_init(void)
{
    // TODO: Replace with platform_keystore_init
    // Stubbed for platform abstraction
    ACP_LOG_INFO("Key store initialized (platform abstraction)");
    return ACP_CRYPTO_OK;
}

uint32_t acp_crypto_key_checksum(const acp_crypto_key_t *key)
{
    if (!key)
        return 0;

    // Simple checksum for integrity verification
    uint32_t checksum = ACP_CRYPTO_MAGIC;
    const uint8_t *data = (const uint8_t *)key;

    for (size_t i = 0; i < sizeof(acp_crypto_key_t); i++)
    {
        checksum = checksum * 31 + data[i];
    }

    return checksum;
}

int acp_crypto_keystore_store(uint32_t key_id, const acp_crypto_key_t *key)
{
    if (!key)
    {
        return ACP_CRYPTO_ERR_INVALID_PARAM;
    }

    if (!crypto_state.initialized)
    {
        return ACP_CRYPTO_ERR_HARDWARE;
    }

    // TODO: Replace with platform_keystore_store
    ACP_LOG_INFO("Stored key %u for role %u (platform abstraction)", key_id, key->operator_role);
    crypto_state.stats.keystore_operations++;
    return ACP_CRYPTO_OK;
}

int acp_crypto_keystore_load(uint32_t key_id, acp_crypto_key_t *key_out)
{
    if (!key_out)
    {
        return ACP_CRYPTO_ERR_INVALID_PARAM;
    }

    if (!crypto_state.initialized)
    {
        return ACP_CRYPTO_ERR_HARDWARE;
    }

    // TODO: Replace with platform_keystore_load
    ACP_LOG_DEBUG("Loaded key %u (platform abstraction)", key_id);
    crypto_state.stats.keystore_operations++;
    return ACP_CRYPTO_OK;
}

int acp_crypto_keystore_delete(uint32_t key_id)
{
    if (!crypto_state.initialized)
    {
        return ACP_CRYPTO_ERR_HARDWARE;
    }

    // TODO: Replace with platform_keystore_delete
    ACP_LOG_INFO("Deleted key %u (platform abstraction)", key_id);
    return ACP_CRYPTO_OK;
}

int acp_crypto_keystore_list(acp_crypto_key_t *key_list, uint8_t max_keys, uint8_t *count_out)
{
    if (!key_list || !count_out || max_keys == 0)
    {
        return ACP_CRYPTO_ERR_INVALID_PARAM;
    }

    if (!crypto_state.initialized)
    {
        return ACP_CRYPTO_ERR_HARDWARE;
    }
    // TODO: Replace with platform_keystore_list
    *count_out = 0;
    ACP_LOG_INFO("Listed %u keys from store (platform abstraction)", *count_out);
    return ACP_CRYPTO_OK;
}

int acp_crypto_keystore_clear(void)
{
    if (!crypto_state.initialized)
    {
        return ACP_CRYPTO_ERR_HARDWARE;
    }
    // TODO: Replace with platform_keystore_clear
    ACP_LOG_INFO("Cleared all keys from store (platform abstraction)");
    return ACP_CRYPTO_OK;
}

// ===========================================================================
// KEY UTILITIES
// ===========================================================================

bool acp_crypto_key_is_valid(const acp_crypto_key_t *key, uint32_t current_time)
{
    if (!key || !key->is_valid)
    {
        return false;
    }

    // Check expiration (0 means never expires)
    if (key->expires_time != 0 && current_time >= key->expires_time)
    {
        return false;
    }

    // Validate role
    switch (key->operator_role)
    {
    case ACP_ROLE_COMMANDER:
    case ACP_ROLE_PILOT:
    case ACP_ROLE_OBSERVER:
        break;
    default:
        return false;
    }

    return true;
}

int acp_crypto_key_create(acp_crypto_key_t *key_out,
                          const uint8_t *key_data,
                          uint32_t key_id,
                          uint8_t operator_role,
                          uint32_t validity_period,
                          uint32_t current_time)
{

    if (!key_out || !key_data)
    {
        return ACP_CRYPTO_ERR_INVALID_PARAM;
    }

    // Validate role
    switch (operator_role)
    {
    case ACP_ROLE_COMMANDER:
    case ACP_ROLE_PILOT:
    case ACP_ROLE_OBSERVER:
        break;
    default:
        return ACP_CRYPTO_ERR_INVALID_PARAM;
    }

    // Initialize key structure
    memset(key_out, 0, sizeof(acp_crypto_key_t));

    memcpy(key_out->key_data, key_data, ACP_HMAC_KEY_SIZE);
    key_out->key_id = key_id;
    key_out->created_time = current_time;
    key_out->expires_time = (validity_period > 0) ? (current_time + validity_period) : 0;
    key_out->key_version = 1;
    key_out->operator_role = operator_role;
    key_out->is_valid = true;

    ACP_LOG_INFO("Created key %u for role %u", key_id, operator_role);
    return ACP_CRYPTO_OK;
}

// ===========================================================================
// SYSTEM INTEGRATION
// ===========================================================================

int acp_crypto_init(void)
{
    if (crypto_state.initialized)
    {
        ACP_LOG_WARN("Crypto already initialized");
        return ACP_CRYPTO_OK;
    }

    ACP_LOG_INFO("Initializing ACP cryptographic subsystem");

    // Initialize statistics
    memset(&crypto_state.stats, 0, sizeof(crypto_state.stats));

    // Initialize entropy and RNG
    int ret = init_entropy();
    if (ret != 0)
    {
        return ACP_CRYPTO_ERR_HARDWARE;
    }

    // Initialize key store
    ret = acp_crypto_keystore_init();
    if (ret != ACP_CRYPTO_OK)
    {
        return ret;
    }

    // Run self-tests
    ret = acp_crypto_self_test();
    if (ret != ACP_CRYPTO_OK)
    {
        ACP_LOG_ERROR("Crypto self-test failed");
        return ret;
    }

    crypto_state.initialized = true;
    ACP_LOG_INFO("ACP crypto initialization complete");

    return ACP_CRYPTO_OK;
}

int acp_crypto_self_test(void)
{
    ACP_LOG_INFO("Running cryptographic self-tests");

    // Test HMAC with known vector (RFC 4231 Test Case 1)
    const uint8_t test_key[32] = {
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
        0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b};
    const char *test_data = "Hi There";
    const uint8_t expected_hmac[16] = {
        0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53,
        0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b};

    uint8_t calculated_hmac[ACP_HMAC_TAG_SIZE];
    int ret = acp_crypto_hmac_generate(test_key,
                                       (const uint8_t *)test_data,
                                       strlen(test_data),
                                       calculated_hmac);
    if (ret != ACP_CRYPTO_OK)
    {
        ACP_LOG_ERROR("HMAC generation test failed: %d", ret);
        return ret;
    }

    // Compare only first 16 bytes (truncated)
    if (!acp_crypto_secure_memcmp(calculated_hmac, expected_hmac, ACP_HMAC_TAG_SIZE))
    {
        ACP_LOG_ERROR("HMAC test vector mismatch");
        return ACP_CRYPTO_ERR_AUTH_FAIL;
    }

    // Test HMAC verification
    ret = acp_crypto_hmac_verify(test_key,
                                 (const uint8_t *)test_data,
                                 strlen(test_data),
                                 expected_hmac);
    if (ret != ACP_CRYPTO_OK)
    {
        ACP_LOG_ERROR("HMAC verification test failed: %d", ret);
        return ret;
    }

    // Test random number generation
    uint8_t random_test[32];
    ret = acp_crypto_get_random(random_test, sizeof(random_test));
    if (ret != ACP_CRYPTO_OK)
    {
        ACP_LOG_ERROR("Random generation test failed: %d", ret);
        return ret;
    }

    ACP_LOG_INFO("Cryptographic self-tests passed");
    return ACP_CRYPTO_OK;
}

void acp_crypto_get_stats(acp_crypto_stats_t *stats_out)
{
    if (stats_out)
    {
        memcpy(stats_out, &crypto_state.stats, sizeof(acp_crypto_stats_t));
    }
}

void acp_crypto_reset_stats(void)
{
    memset(&crypto_state.stats, 0, sizeof(acp_crypto_stats_t));
    ACP_LOG_INFO("Crypto statistics reset");
}