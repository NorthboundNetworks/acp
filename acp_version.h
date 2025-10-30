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
 * @file acp_version.h
 * @brief ACP version information and macros
 *
 * @version 0.3.0
 * @date 2025-10-27
 * @copyright Copyright (c) 2025 Northbound Networks Pty. Ltd. MIT License.
 */

#ifndef ACP_VERSION_H
#define ACP_VERSION_H

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================================================================== */
/*                            Version Information                             */
/* ========================================================================== */

/** @brief ACP library major version */
#ifndef ACP_VERSION_MAJOR
#define ACP_VERSION_MAJOR 0
#endif

/** @brief ACP library minor version */
#ifndef ACP_VERSION_MINOR
#define ACP_VERSION_MINOR 3
#endif

/** @brief ACP library patch version */
#ifndef ACP_VERSION_PATCH
#define ACP_VERSION_PATCH 0
#endif

/** @brief ACP library version string */
#define ACP_VERSION_STRING "0.3.0"

/** @brief ACP library build date (set by build system) */
#ifndef ACP_BUILD_DATE
#define ACP_BUILD_DATE __DATE__
#endif

/** @brief ACP library build time (set by build system) */
#ifndef ACP_BUILD_TIME
#define ACP_BUILD_TIME __TIME__
#endif

/** @brief Git commit hash (set by build system if available) */
#ifndef ACP_GIT_COMMIT
#define ACP_GIT_COMMIT "unknown"
#endif

/* ========================================================================== */
/*                             Version Macros                                */
/* ========================================================================== */

/**
 * @brief Create version number from major, minor, patch
 *
 * @param major Major version number
 * @param minor Minor version number
 * @param patch Patch version number
 * @return Combined version number
 */
#define ACP_MAKE_VERSION(major, minor, patch) \
    (((major) << 16) | ((minor) << 8) | (patch))

/** @brief Current ACP library version as packed integer */
#define ACP_VERSION \
    ACP_MAKE_VERSION(ACP_VERSION_MAJOR, ACP_VERSION_MINOR, ACP_VERSION_PATCH)

/**
 * @brief Check if current version is at least the specified version
 *
 * @param major Required major version
 * @param minor Required minor version
 * @param patch Required patch version
 * @return 1 if current version >= required, 0 otherwise
 */
#define ACP_VERSION_AT_LEAST(major, minor, patch) \
    (ACP_VERSION >= ACP_MAKE_VERSION(major, minor, patch))

/**
 * @brief Extract major version from packed version
 *
 * @param version Packed version number
 * @return Major version
 */
#define ACP_VERSION_MAJOR_FROM(version) (((version) >> 16) & 0xFF)

/**
 * @brief Extract minor version from packed version
 *
 * @param version Packed version number
 * @return Minor version
 */
#define ACP_VERSION_MINOR_FROM(version) (((version) >> 8) & 0xFF)

/**
 * @brief Extract patch version from packed version
 *
 * @param version Packed version number
 * @return Patch version
 */
#define ACP_VERSION_PATCH_FROM(version) ((version) & 0xFF)

/* ========================================================================== */
/*                           Compatibility Checks                            */
/* ========================================================================== */

/**
 * @brief Protocol compatibility check
 *
 * Check if wire protocol version is compatible with this library version.
 *
 * @param wire_major Major version from wire protocol
 * @param wire_minor Minor version from wire protocol
 * @return 1 if compatible, 0 otherwise
 */
#define ACP_PROTOCOL_COMPATIBLE(wire_major, wire_minor) \
    ((wire_major) == ACP_VERSION_MAJOR && (wire_minor) <= ACP_VERSION_MINOR)

    /* ========================================================================== */
    /*                           Runtime Information                              */
    /* ========================================================================== */

    /**
     * @brief Version information structure
     */
    typedef struct
    {
        unsigned int major;     /**< Major version */
        unsigned int minor;     /**< Minor version */
        unsigned int patch;     /**< Patch version */
        const char *string;     /**< Version string */
        const char *build_date; /**< Build date */
        const char *build_time; /**< Build time */
        const char *git_commit; /**< Git commit hash */
    } acp_version_info_t;

    /**
     * @brief Get runtime version information
     *
     * @return Pointer to static version information structure
     */
    const acp_version_info_t *acp_get_version_info(void);

    /**
     * @brief Get version string
     *
     * @return Static version string (e.g., "0.3.0")
     */
    const char *acp_get_version_string(void);

    /**
     * @brief Get packed version number
     *
     * @return Version as packed integer
     */
    unsigned int acp_get_version_number(void);

    /**
     * @brief Check library/protocol compatibility
     *
     * @param required_major Required major version
     * @param required_minor Required minor version
     * @return 1 if compatible, 0 otherwise
     */
    int acp_check_compatibility(unsigned int required_major, unsigned int required_minor);

#ifdef __cplusplus
}
#endif

#endif /* ACP_VERSION_H */
