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
 * @file acp_platform_mutex.h
 * @brief ACP Platform Mutex Interface
 *
 * Autonomous Command Protocol (ACP) v0.3.
 *
 * @version 0.3
 * @date 2025-10-26
 * @author Paul Zanna
 */

#ifndef ACP_PLATFORM_MUTEX_H
#define ACP_PLATFORM_MUTEX_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    /**
     * @brief Opaque handle for platform mutex
     */
    typedef void *acp_mutex_t;

    /**
     * @brief Create a new mutex
     * @return acp_mutex_t handle, or NULL on failure
     */
    acp_mutex_t acp_mutex_create(void);

    /**
     * @brief Lock the mutex (blocking)
     * @param mutex Handle to the mutex
     * @return int 0 on success, non-zero on error
     */
    int acp_mutex_lock(acp_mutex_t mutex);

    /**
     * @brief Unlock the mutex
     * @param mutex Handle to the mutex
     * @return int 0 on success, non-zero on error
     */
    int acp_mutex_unlock(acp_mutex_t mutex);

    /**
     * @brief Destroy a mutex and release resources
     * @param mutex Handle to the mutex
     * @return int 0 on success, non-zero on error
     */
    int acp_mutex_destroy(acp_mutex_t mutex);

#ifdef __cplusplus
}
#endif

#endif // ACP_PLATFORM_MUTEX_H