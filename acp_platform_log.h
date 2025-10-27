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

#ifndef ACP_PLATFORM_LOG_H
#define ACP_PLATFORM_LOG_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdarg.h>

/**
 * @brief Log levels for ACP platform logging.
 */
#define ACP_LOG_ERROR(...) acp_platform_log("ERROR", __VA_ARGS__)
#define ACP_LOG_WARN(...) acp_platform_log("WARN", __VA_ARGS__)
#define ACP_LOG_INFO(...) acp_platform_log("INFO", __VA_ARGS__)
#define ACP_LOG_DEBUG(...) acp_platform_log("DEBUG", __VA_ARGS__)

    /**
     * @brief Default logging implementation.
     * Users may override this in platform-specific builds.
     */
    static inline void acp_platform_log(const char *level, const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        fprintf(stderr, "[ACP][%s] ", level);
        vfprintf(stderr, fmt, args);
        fprintf(stderr, "\n");
        va_end(args);
    }

#ifdef __cplusplus
}
#endif

#endif // ACP_PLATFORM_LOG_H
