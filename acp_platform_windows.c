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
 * @file acp_platform_win32.c
 * @brief Windows (Win32) platform implementation for ACP
 *
 * Provides Windows-specific implementations for:
 * - Time functions (acp_platform_time.h)
 * - Mutex operations (acp_platform_mutex.h) 
 * - Logging functions (acp_platform_log.h)
 * - Keystore access (acp_platform_keystore.h)
 *
 * @version 0.3.0
 * @date 2025-10-27
 */

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>

#include "acp_platform_time.h"
#include "acp_platform_mutex.h"
#include "acp_platform_log.h"
#include "acp_platform_keystore.h"

/* ========================================================================== */
/*                               Time Functions                               */
/* ========================================================================== */

/**
 * Get current time in milliseconds since epoch
 */
uint64_t acp_platform_get_time_ms(void) {
    FILETIME ft;
    ULARGE_INTEGER ull;
    
    /* Get system time as FILETIME (100-nanosecond intervals since Jan 1, 1601) */
    GetSystemTimeAsFileTime(&ft);
    
    /* Convert to 64-bit value */
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    
    /* Convert to milliseconds since Unix epoch (Jan 1, 1970) */
    /* FILETIME epoch is 11644473600 seconds before Unix epoch */
    const uint64_t EPOCH_DIFFERENCE_MS = 11644473600000ULL;
    return (ull.QuadPart / 10000ULL) - EPOCH_DIFFERENCE_MS;
}

/**
 * Get high-resolution monotonic time in microseconds
 */
uint64_t acp_platform_get_monotonic_us(void) {
    static LARGE_INTEGER frequency = {0};
    LARGE_INTEGER counter;
    
    /* Initialize frequency on first call */
    if (frequency.QuadPart == 0) {
        if (!QueryPerformanceFrequency(&frequency)) {
            /* Fallback to GetTickCount64 if high-res timer not available */
            return GetTickCount64() * 1000ULL;
        }
    }
    
    /* Get current counter value */
    if (!QueryPerformanceCounter(&counter)) {
        return GetTickCount64() * 1000ULL;
    }
    
    /* Convert to microseconds */
    return (counter.QuadPart * 1000000ULL) / frequency.QuadPart;
}

/**
 * Sleep for specified milliseconds
 */
void acp_platform_sleep_ms(uint32_t ms) {
    Sleep((DWORD)ms);
}

/* ========================================================================== */
/*                              Mutex Functions                               */
/* ========================================================================== */

/**
 * Initialize a mutex
 */
int acp_platform_mutex_init(acp_mutex_t *mutex) {
    if (!mutex) {
        return -1;
    }
    
    CRITICAL_SECTION *cs = malloc(sizeof(CRITICAL_SECTION));
    if (!cs) {
        return -1;
    }
    
    InitializeCriticalSection(cs);
    mutex->handle = cs;
    return 0;
}

/**
 * Destroy a mutex
 */
void acp_platform_mutex_destroy(acp_mutex_t *mutex) {
    if (mutex && mutex->handle) {
        CRITICAL_SECTION *cs = (CRITICAL_SECTION *)mutex->handle;
        DeleteCriticalSection(cs);
        free(cs);
        mutex->handle = NULL;
    }
}

/**
 * Lock a mutex
 */
int acp_platform_mutex_lock(acp_mutex_t *mutex) {
    if (!mutex || !mutex->handle) {
        return -1;
    }
    
    CRITICAL_SECTION *cs = (CRITICAL_SECTION *)mutex->handle;
    EnterCriticalSection(cs);
    return 0;
}

/**
 * Unlock a mutex
 */
int acp_platform_mutex_unlock(acp_mutex_t *mutex) {
    if (!mutex || !mutex->handle) {
        return -1;
    }
    
    CRITICAL_SECTION *cs = (CRITICAL_SECTION *)mutex->handle;
    LeaveCriticalSection(cs);
    return 0;
}

/**
 * Try to lock a mutex (non-blocking)
 */
int acp_platform_mutex_trylock(acp_mutex_t *mutex) {
    if (!mutex || !mutex->handle) {
        return -1;
    }
    
    CRITICAL_SECTION *cs = (CRITICAL_SECTION *)mutex->handle;
    BOOL result = TryEnterCriticalSection(cs);
    return result ? 0 : -1;
}

/* ========================================================================== */
/*                             Logging Functions                              */
/* ========================================================================== */

/**
 * Log a message with specified level
 */
void acp_platform_log(acp_log_level_t level, const char *file, int line, 
                     const char *func, const char *fmt, ...) {
    /* Get current timestamp */
    SYSTEMTIME st;
    GetLocalTime(&st);
    
    /* Log level strings */
    const char *level_str;
    switch (level) {
        case ACP_LOG_ERROR:   level_str = "ERROR"; break;
        case ACP_LOG_WARN:    level_str = "WARN ";  break;
        case ACP_LOG_INFO:    level_str = "INFO ";  break;
        case ACP_LOG_DEBUG:   level_str = "DEBUG"; break;
        default:              level_str = "?????"; break;
    }
    
    /* Print timestamp and level */
    fprintf(stderr, "[%04d-%02d-%02d %02d:%02d:%02d.%03d] [ACP][%s] ",
            st.wYear, st.wMonth, st.wDay,
            st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
            level_str);
    
    /* Print location info for errors and debug */
    if (level == ACP_LOG_ERROR || level == ACP_LOG_DEBUG) {
        const char *filename = strrchr(file, '\\');
        if (!filename) filename = strrchr(file, '/');
        if (!filename) filename = file;
        else filename++;
        
        fprintf(stderr, "%s:%d:%s() ", filename, line, func);
    }
    
    /* Print formatted message */
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    
    fprintf(stderr, "\n");
    fflush(stderr);
}

/* ========================================================================== */
/*                            Keystore Functions                              */
/* ========================================================================== */

/* Default keystore file location in Windows */
#define ACP_WIN32_KEYSTORE_DIR  "C:\\ProgramData\\ACP"
#define ACP_WIN32_KEYSTORE_FILE "C:\\ProgramData\\ACP\\acp_keystore.bin"

/**
 * Initialize keystore (create directory if needed)
 */
int acp_platform_keystore_init(void) {
    /* Create keystore directory if it doesn't exist */
    DWORD attrs = GetFileAttributesA(ACP_WIN32_KEYSTORE_DIR);
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        if (!CreateDirectoryA(ACP_WIN32_KEYSTORE_DIR, NULL)) {
            DWORD error = GetLastError();
            if (error != ERROR_ALREADY_EXISTS) {
                return -1;
            }
        }
    } else if (!(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
        /* Path exists but is not a directory */
        return -1;
    }
    
    return 0;
}

/**
 * Load a key from keystore
 */
int acp_platform_keystore_load_key(uint32_t key_id, uint8_t *key_buffer, size_t buffer_size) {
    if (!key_buffer || buffer_size < 32) {
        return -1;
    }
    
    /* Open keystore file */
    HANDLE hFile = CreateFileA(ACP_WIN32_KEYSTORE_FILE,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return -1;
    }
    
    /* Read keystore header to find key */
    struct keystore_header {
        uint32_t magic;      /* 'ACPK' */
        uint32_t version;    /* Keystore format version */
        uint32_t key_count;  /* Number of keys */
        uint32_t reserved;   /* Reserved for future use */
    } header;
    
    DWORD bytes_read;
    if (!ReadFile(hFile, &header, sizeof(header), &bytes_read, NULL) ||
        bytes_read != sizeof(header) ||
        header.magic != 0x4B504341) {  /* 'ACPK' in little-endian */
        CloseHandle(hFile);
        return -1;
    }
    
    /* Search for key */
    for (uint32_t i = 0; i < header.key_count; i++) {
        struct key_entry {
            uint32_t key_id;
            uint32_t key_len;
            uint8_t key_data[32];
        } entry;
        
        if (!ReadFile(hFile, &entry, sizeof(entry), &bytes_read, NULL) ||
            bytes_read != sizeof(entry)) {
            CloseHandle(hFile);
            return -1;
        }
        
        if (entry.key_id == key_id && entry.key_len <= buffer_size) {
            memcpy(key_buffer, entry.key_data, entry.key_len);
            CloseHandle(hFile);
            return (int)entry.key_len;
        }
    }
    
    CloseHandle(hFile);
    return -1;  /* Key not found */
}

/**
 * Store a key in keystore
 */
int acp_platform_keystore_store_key(uint32_t key_id, const uint8_t *key_data, size_t key_len) {
    if (!key_data || key_len == 0 || key_len > 32) {
        return -1;
    }
    
    /* For this implementation, we'll create a simple append-only keystore */
    /* In production, you'd want to handle key updates and compaction */
    
    HANDLE hFile = CreateFileA(ACP_WIN32_KEYSTORE_FILE,
                               GENERIC_WRITE,
                               0,
                               NULL,
                               OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return -1;
    }
    
    /* Check if file is new */
    DWORD file_size = GetFileSize(hFile, NULL);
    if (file_size == 0) {
        /* Write header for new keystore */
        struct keystore_header header = {
            .magic = 0x4B504341,  /* 'ACPK' */
            .version = 1,
            .key_count = 1,
            .reserved = 0
        };
        
        DWORD bytes_written;
        if (!WriteFile(hFile, &header, sizeof(header), &bytes_written, NULL)) {
            CloseHandle(hFile);
            return -1;
        }
    } else {
        /* Update key count in existing file */
        SetFilePointer(hFile, 8, NULL, FILE_BEGIN);  /* Offset of key_count */
        uint32_t key_count;
        DWORD bytes_read;
        if (ReadFile(hFile, &key_count, sizeof(key_count), &bytes_read, NULL)) {
            key_count++;
            SetFilePointer(hFile, 8, NULL, FILE_BEGIN);
            WriteFile(hFile, &key_count, sizeof(key_count), &bytes_read, NULL);
        }
        
        /* Seek to end for appending */
        SetFilePointer(hFile, 0, NULL, FILE_END);
    }
    
    /* Write key entry */
    struct key_entry {
        uint32_t key_id;
        uint32_t key_len;
        uint8_t key_data[32];
    } entry = {0};
    
    entry.key_id = key_id;
    entry.key_len = (uint32_t)key_len;
    memcpy(entry.key_data, key_data, key_len);
    
    DWORD bytes_written;
    BOOL result = WriteFile(hFile, &entry, sizeof(entry), &bytes_written, NULL);
    
    CloseHandle(hFile);
    return result ? 0 : -1;
}

/**
 * Delete a key from keystore
 */
int acp_platform_keystore_delete_key(uint32_t key_id) {
    /* For this simple implementation, we don't support key deletion */
    /* In production, you'd mark the key as deleted or rebuild the keystore */
    (void)key_id;  /* Suppress unused parameter warning */
    return -1;     /* Not implemented */
}

/**
 * List available keys in keystore
 */
int acp_platform_keystore_list_keys(uint32_t *key_ids, size_t max_keys) {
    if (!key_ids || max_keys == 0) {
        return -1;
    }
    
    HANDLE hFile = CreateFileA(ACP_WIN32_KEYSTORE_FILE,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    
    if (hFile == INVALID_HANDLE_VALUE) {
        return 0;  /* No keystore file = no keys */
    }
    
    /* Read header */
    struct keystore_header {
        uint32_t magic;
        uint32_t version;
        uint32_t key_count;
        uint32_t reserved;
    } header;
    
    DWORD bytes_read;
    if (!ReadFile(hFile, &header, sizeof(header), &bytes_read, NULL) ||
        bytes_read != sizeof(header) ||
        header.magic != 0x4B504341) {
        CloseHandle(hFile);
        return -1;
    }
    
    /* Read key IDs */
    size_t keys_found = 0;
    for (uint32_t i = 0; i < header.key_count && keys_found < max_keys; i++) {
        uint32_t key_id;
        if (!ReadFile(hFile, &key_id, sizeof(key_id), &bytes_read, NULL) ||
            bytes_read != sizeof(key_id)) {
            break;
        }
        
        key_ids[keys_found++] = key_id;
        
        /* Skip rest of key entry */
        SetFilePointer(hFile, sizeof(uint32_t) + 32, NULL, FILE_CURRENT);
    }
    
    CloseHandle(hFile);
    return (int)keys_found;
}

#endif /* _WIN32 */