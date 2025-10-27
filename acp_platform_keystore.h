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
 * @file acp_platform_keystore.h
 * @brief ACP Platform Keystore Interface
 *
 * Platform abstraction for persistent key storage used by the
 * Autonomous Command Protocol (ACP). This header defines a generic interface
 * for saving and retrieving binary blobs associated with a named keystore.
 * Platform-specific implementations should provide the backing logic for
 * desktop, embedded, or secure environments.
 *
 * @version 0.3
 * @date 2025-10-26
 * @author Paul Zanna
 */

#ifndef ACP_PLATFORM_KEYSTORE_H
#define ACP_PLATFORM_KEYSTORE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>

    /**
     * @brief Store a binary blob in the platform keystore.
     *
     * @param keyspace   Logical namespace (e.g., "acp_keys" or "session0")
     * @param key_name   Null-terminated key name
     * @param data       Pointer to binary data
     * @param data_len   Length of binary data
     * @return int       0 on success, non-zero on error
     */
    int acp_platform_keystore_store(const char *keyspace, const char *key_name, const void *data, size_t data_len);

    /**
     * @brief Load a binary blob from the platform keystore.
     *
     * @param keyspace   Logical namespace
     * @param key_name   Null-terminated key name
     * @param out_buf    Output buffer to receive the data
     * @param buf_len    Size of output buffer
     * @param out_len    [out] Actual length of data loaded
     * @return int       0 on success, non-zero on error
     */
    int acp_platform_keystore_load(const char *keyspace, const char *key_name, void *out_buf, size_t buf_len, size_t *out_len);

    /**
     * @brief Erase a key from the platform keystore.
     *
     * @param keyspace   Logical namespace
     * @param key_name   Null-terminated key name
     * @return int       0 on success, non-zero on error
     */
    int acp_platform_keystore_erase(const char *keyspace, const char *key_name);

    /**
     * @brief Get key by ID from keystore (simplified interface)
     *
     * @param key_id     Key identifier
     * @param key_out    Output buffer for key data
     * @param key_len    Input: buffer size, Output: actual key length
     * @return int       0 on success, non-zero on error
     */
    int acp_platform_keystore_get_key(uint32_t key_id, uint8_t *key_out, size_t *key_len);

    /**
     * @brief Store key by ID in keystore (simplified interface)
     *
     * @param key_id     Key identifier
     * @param key        Key data to store
     * @param key_len    Length of key data
     * @return int       0 on success, non-zero on error
     */
    int acp_platform_keystore_store_key(uint32_t key_id, const uint8_t *key, size_t key_len);

#ifdef __cplusplus
}
#endif

#endif // ACP_PLATFORM_KEYSTORE_H
