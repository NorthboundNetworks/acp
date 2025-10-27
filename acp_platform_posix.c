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
 * @file acp_platform_posix.c
 * @brief POSIX platform implementation for ACP
 *
 * Provides default implementations of platform abstraction functions
 * for POSIX-compliant systems (Linux, macOS, etc.).
 *
 * @version 0.3.0
 * @date 2025-10-27
 */

#include "acp_errors.h"
#include "acp_platform_log.h"
#include "acp_platform_time.h"
#include "acp_platform_mutex.h"
#include "acp_platform_keystore.h"
#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* ========================================================================== */
/*                               Time Functions                              */
/* ========================================================================== */

/**
 * @brief Get monotonic timestamp in milliseconds
 */
uint64_t acp_platform_get_time_ms(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0)
    {
        return (uint64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    }
    return 0;
}

/**
 * @brief Sleep for specified milliseconds
 */
void acp_platform_sleep_ms(uint32_t ms)
{
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}

/* ========================================================================== */
/*                              Mutex Functions                              */
/* ========================================================================== */

/**
 * @brief Create a mutex
 */
acp_mutex_t *acp_platform_mutex_create(void)
{
    pthread_mutex_t *mutex = malloc(sizeof(pthread_mutex_t));
    if (mutex != NULL)
    {
        if (pthread_mutex_init(mutex, NULL) != 0)
        {
            free(mutex);
            return NULL;
        }
    }
    return (acp_mutex_t *)mutex;
}

/**
 * @brief Destroy a mutex
 */
void acp_platform_mutex_destroy(acp_mutex_t *mutex)
{
    if (mutex != NULL)
    {
        pthread_mutex_destroy((pthread_mutex_t *)mutex);
        free(mutex);
    }
}

/**
 * @brief Lock a mutex
 */
int acp_platform_mutex_lock(acp_mutex_t *mutex)
{
    if (mutex == NULL)
        return -1;
    return pthread_mutex_lock((pthread_mutex_t *)mutex);
}

/**
 * @brief Unlock a mutex
 */
int acp_platform_mutex_unlock(acp_mutex_t *mutex)
{
    if (mutex == NULL)
        return -1;
    return pthread_mutex_unlock((pthread_mutex_t *)mutex);
}

/* ========================================================================== */
/*                            Keystore Functions                             */
/* ========================================================================== */

/**
 * @brief Get key from default file-based keystore
 *
 * This is a simple implementation that reads keys from files.
 * In production, this should use secure key storage.
 */
int acp_platform_keystore_get_key(uint32_t key_id, uint8_t *key_out, size_t *key_len)
{
    if (key_out == NULL || key_len == NULL)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/acp_keys/key_%08x.bin", key_id);

    FILE *file = fopen(filename, "rb");
    if (file == NULL)
    {
        return ACP_ERR_KEY_NOT_FOUND;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0 || (size_t)file_size > *key_len)
    {
        fclose(file);
        return ACP_ERR_BUFFER_TOO_SMALL;
    }

    size_t read_size = fread(key_out, 1, file_size, file);
    fclose(file);

    if (read_size != (size_t)file_size)
    {
        return ACP_ERR_KEYSTORE_READ;
    }

    *key_len = read_size;
    return ACP_OK;
}

/**
 * @brief Store key in default file-based keystore
 */
int acp_platform_keystore_store_key(uint32_t key_id, const uint8_t *key, size_t key_len)
{
    if (key == NULL || key_len == 0)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    /* Ensure keystore directory exists */
    mkdir("/tmp/acp_keys", 0700);

    char filename[256];
    snprintf(filename, sizeof(filename), "/tmp/acp_keys/key_%08x.bin", key_id);

    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        return ACP_ERR_KEYSTORE_WRITE;
    }

    size_t written = fwrite(key, 1, key_len, file);
    fclose(file);

    if (written != key_len)
    {
        return ACP_ERR_KEYSTORE_WRITE;
    }

    return ACP_OK;
}
