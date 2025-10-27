/**
 * @file acp_crypto_stub.c
 * @brief Stub implementation of crypto functions for session testing
 */

#include "acp_crypto.h"
#include <string.h>

void acp_hmac_sha256(const uint8_t *key, size_t key_len,
                     const uint8_t *data, size_t data_len,
                     uint8_t *mac)
{
    (void)key;
    (void)key_len;
    (void)data;
    (void)data_len;
    
    /* Simple stub - fill with test pattern */
    if (mac) {
        for (int i = 0; i < ACP_HMAC_SIZE; i++) {
            mac[i] = (uint8_t)(0xA0 + i);
        }
    }
}

int acp_hmac_verify(const uint8_t *expected, const uint8_t *received, size_t len)
{
    return memcmp(expected, received, len);
}

void acp_crypto_clear(void *data, size_t len)
{
    volatile uint8_t *p = (volatile uint8_t *)data;
    while (len--) {
        *p++ = 0;
    }
}