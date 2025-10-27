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
 * @file acp_platform_time.h
 * @brief ACP Platform Time Interface
 *
 * Autonomous Command Protocol (ACP) v0.3.
 *
 * @version 0.3
 * @date 2025-10-26
 * @author Paul Zanna
 */

#ifndef ACP_PLATFORM_TIME_H
#define ACP_PLATFORM_TIME_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

    /**
     * @brief Get current monotonic time in milliseconds.
     *
     * Used for timestamping, timeouts, and session tracking.
     *
     * @return uint64_t Current time in milliseconds.
     */
    uint64_t acp_platform_get_time_ms(void);

    /**
     * @brief Sleep/block the current thread for the specified number of milliseconds.
     *
     * @param delay_ms Duration to sleep in milliseconds
     */
    void acp_platform_delay_ms(uint32_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif // ACP_PLATFORM_TIME_H