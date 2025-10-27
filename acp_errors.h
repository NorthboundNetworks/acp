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
 * @file acp_errors.h
 * @brief ACP error codes and error handling utilities
 *
 * @version 0.3.0
 * @date 2025-10-27
 * @copyright Copyright (c) 2025 Northbound Networks Pty. Ltd. MIT License.
 */

#ifndef ACP_ERRORS_H
#define ACP_ERRORS_H

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================================================================== */
/*                              Error Categories                              */
/* ========================================================================== */

/** @brief Success code */
#define ACP_SUCCESS_BASE 0

/** @brief Generic error base */
#define ACP_ERR_GENERIC_BASE -1

/** @brief Frame format error base */
#define ACP_ERR_FRAME_BASE -10

/** @brief COBS framing error base */
#define ACP_ERR_COBS_BASE -20

/** @brief Integrity error base */
#define ACP_ERR_INTEGRITY_BASE -30

/** @brief Authentication error base */
#define ACP_ERR_AUTH_BASE -40

/** @brief Platform error base */
#define ACP_ERR_PLATFORM_BASE -50

/** @brief Keystore error base */
#define ACP_ERR_KEYSTORE_BASE -60

/** @brief Configuration error base */
#define ACP_ERR_CONFIG_BASE -70

/** @brief Resource error base */
#define ACP_ERR_RESOURCE_BASE -80

/** @brief System error base */
#define ACP_ERR_SYSTEM_BASE -90

/* ========================================================================== */
/*                             Specific Errors                               */
/* ========================================================================== */

/* Success */
#define ACP_OK 0 /**< Operation successful */

/* Generic errors (-1 to -9) */
#define ACP_ERR_INVALID_PARAM -1    /**< Invalid parameter passed */
#define ACP_ERR_BUFFER_TOO_SMALL -2 /**< Output buffer too small */
#define ACP_ERR_NEED_MORE_DATA -3   /**< Need more input data */
#define ACP_ERR_INVALID_STATE -4    /**< Invalid operation state */
#define ACP_ERR_TIMEOUT -5          /**< Operation timed out */
#define ACP_ERR_CANCELLED -6        /**< Operation was cancelled */
#define ACP_ERR_NOT_FOUND -7        /**< Requested item not found */
#define ACP_ERR_ALREADY_EXISTS -8   /**< Item already exists */
#define ACP_ERR_NOT_SUPPORTED -9    /**< Operation not supported */

/* Frame format errors (-10 to -19) */
#define ACP_ERR_INVALID_VERSION -10   /**< Unsupported protocol version */
#define ACP_ERR_INVALID_TYPE -11      /**< Invalid frame type */
#define ACP_ERR_PAYLOAD_TOO_LARGE -12 /**< Payload exceeds maximum size */
#define ACP_ERR_MALFORMED_FRAME -13   /**< Frame structure is malformed */
#define ACP_ERR_INVALID_FLAGS -14     /**< Invalid flag combination */
#define ACP_ERR_INVALID_LENGTH -15    /**< Invalid length field */
#define ACP_ERR_RESERVED_FIELD -16    /**< Reserved field not zero */
#define ACP_ERR_SEQUENCE_ERROR -17    /**< Sequence number error */
#define ACP_ERR_FRAME_TOO_SHORT -18   /**< Frame too short for type */
#define ACP_ERR_FRAME_TOO_LONG -19    /**< Frame exceeds maximum size */

/* COBS framing errors (-20 to -29) */
#define ACP_ERR_COBS_DECODE -20       /**< COBS decode error */
#define ACP_ERR_COBS_ENCODE -21       /**< COBS encode error */
#define ACP_ERR_COBS_INVALID_RUN -22  /**< Invalid COBS run length */
#define ACP_ERR_COBS_ZERO_BYTE -23    /**< Unexpected zero byte in COBS data */
#define ACP_ERR_COBS_OVERRUN -24      /**< COBS decode buffer overrun */
#define ACP_ERR_COBS_UNDERRUN -25     /**< COBS decode underrun */
#define ACP_ERR_COBS_NO_DELIMITER -26 /**< Missing COBS frame delimiter */
#define ACP_ERR_COBS_BAD_FRAME -27    /**< Malformed COBS frame */

/* Integrity errors (-30 to -39) */
#define ACP_ERR_CRC_MISMATCH -30     /**< CRC16 verification failed */
#define ACP_ERR_CRC_CALCULATE -31    /**< CRC calculation error */
#define ACP_ERR_CHECKSUM_INVALID -32 /**< Generic checksum failure */
#define ACP_ERR_DATA_CORRUPTION -33  /**< Data corruption detected */

/* Authentication errors (-40 to -49) */
#define ACP_ERR_AUTH_REQUIRED -40    /**< Authentication required but missing */
#define ACP_ERR_AUTH_FAILED -41      /**< HMAC authentication failed */
#define ACP_ERR_REPLAY -42           /**< Replay attack detected */
#define ACP_ERR_KEY_NOT_FOUND -43    /**< Key not found in keystore */
#define ACP_ERR_SESSION_NOT_INIT -44 /**< Session not initialized */
#define ACP_ERR_SESSION_EXPIRED -45  /**< Session has expired */
#define ACP_ERR_NONCE_REUSE -46      /**< Nonce reuse detected */
#define ACP_ERR_HMAC_CALCULATE -47   /**< HMAC calculation failed */
#define ACP_ERR_KEY_TOO_SHORT -48    /**< Key material too short */
#define ACP_ERR_AUTH_NOT_ALLOWED -49 /**< Authentication not allowed for frame type */
#define ACP_ERR_REPLAY_ATTACK -42    /**< Alias for replay detection */

/* Platform errors (-50 to -59) */
#define ACP_ERR_PLATFORM_LOG -50    /**< Platform logging error */
#define ACP_ERR_PLATFORM_TIME -51   /**< Platform time error */
#define ACP_ERR_PLATFORM_MUTEX -52  /**< Platform mutex error */
#define ACP_ERR_PLATFORM_RANDOM -53 /**< Platform random number error */
#define ACP_ERR_PLATFORM_IO -54     /**< Platform I/O error */

/* Keystore errors (-60 to -69) */
#define ACP_ERR_KEYSTORE_READ -60    /**< Keystore read error */
#define ACP_ERR_KEYSTORE_WRITE -61   /**< Keystore write error */
#define ACP_ERR_KEYSTORE_CORRUPT -62 /**< Keystore corruption */
#define ACP_ERR_KEYSTORE_LOCKED -63  /**< Keystore is locked */
#define ACP_ERR_KEYSTORE_FORMAT -64  /**< Invalid keystore format */
#define ACP_ERR_KEY_EXPIRED -65      /**< Key has expired */
#define ACP_ERR_KEY_REVOKED -66      /**< Key has been revoked */

/* Configuration errors (-70 to -79) */
#define ACP_ERR_CONFIG_INVALID -70 /**< Invalid configuration */
#define ACP_ERR_CONFIG_MISSING -71 /**< Required configuration missing */
#define ACP_ERR_CONFIG_PARSE -72   /**< Configuration parse error */

/* Resource errors (-80 to -89) */
#define ACP_ERR_OUT_OF_MEMORY -80  /**< Out of memory */
#define ACP_ERR_RESOURCE_BUSY -81  /**< Resource is busy */
#define ACP_ERR_RESOURCE_LIMIT -82 /**< Resource limit exceeded */

/* System errors (-90 to -99) */
#define ACP_ERR_NOT_IMPLEMENTED -90 /**< Feature not implemented */
#define ACP_ERR_SYSTEM_ERROR -91    /**< Generic system error */
#define ACP_ERR_PERMISSION -92      /**< Permission denied */
#define ACP_ERR_NETWORK -93         /**< Network error */
#define ACP_ERR_FILE_IO -94         /**< File I/O error */
#define ACP_ERR_INTERNAL -99        /**< Internal library error */

/* ========================================================================== */
/*                           Error Classification                             */
/* ========================================================================== */

/**
 * @brief Check if error code indicates success
 *
 * @param err Error code to check
 * @return 1 if success, 0 if error
 */
#define ACP_IS_SUCCESS(err) ((err) == ACP_OK)

/**
 * @brief Check if error code indicates failure
 *
 * @param err Error code to check
 * @return 1 if error, 0 if success
 */
#define ACP_IS_ERROR(err) ((err) < ACP_OK)

/**
 * @brief Check if error is recoverable (temporary)
 *
 * @param err Error code to check
 * @return 1 if recoverable, 0 if permanent
 */
#define ACP_IS_RECOVERABLE(err)           \
    ((err) == ACP_ERR_NEED_MORE_DATA ||   \
     (err) == ACP_ERR_BUFFER_TOO_SMALL || \
     (err) == ACP_ERR_TIMEOUT ||          \
     (err) == ACP_ERR_RESOURCE_BUSY)

/**
 * @brief Check if error is security-related
 *
 * @param err Error code to check
 * @return 1 if security error, 0 otherwise
 */
#define ACP_IS_SECURITY_ERROR(err) \
    ((err) >= ACP_ERR_AUTH_BASE && (err) < ACP_ERR_PLATFORM_BASE)

    /* ========================================================================== */
    /*                            Error Information                               */
    /* ========================================================================== */

    /**
     * @brief Error information structure
     */
    typedef struct
    {
        int code;             /**< Error code */
        const char *name;     /**< Error name (e.g., "ACP_ERR_CRC_MISMATCH") */
        const char *message;  /**< Human-readable error message */
        const char *category; /**< Error category */
        int recoverable;      /**< 1 if recoverable, 0 if permanent */
    } acp_error_info_t;

    /**
     * @brief Get detailed error information
     *
     * @param err Error code
     * @return Pointer to static error information structure
     */
    const acp_error_info_t *acp_get_error_info(int err);

    /**
     * @brief Get human-readable error message
     *
     * @param err Error code
     * @return Static error message string
     */
    const char *acp_error_string(int err);

    /**
     * @brief Get error category name
     *
     * @param err Error code
     * @return Static category string
     */
    const char *acp_error_category(int err);

    /**
     * @brief Check if error code is valid/recognized
     *
     * @param err Error code to validate
     * @return 1 if valid, 0 if unknown error code
     */
    int acp_is_valid_error_code(int err);

#ifdef __cplusplus
}
#endif

#endif /* ACP_ERRORS_H */
