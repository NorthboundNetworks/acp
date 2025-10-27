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
 * @file acp_protocol.h
 * @brief ACP (Autonomous Command Protocol) - Public API Header
 *
 * This header defines the complete public interface for the ACP library,
 * including frame encoding/decoding, session management, and error handling.
 *
 * @version 0.3.0
 * @date 2025-10-27
 * @copyright Copyright (c) 2025 Northbound Networks Pty. Ltd.
 */

#ifndef ACP_PROTOCOL_H
#define ACP_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================================================================== */
/*                                 Constants                                  */
/* ========================================================================== */

/** @brief ACP protocol version (major.minor) */
#define ACP_PROTOCOL_VERSION_MAJOR 0
#define ACP_PROTOCOL_VERSION_MINOR 3

/** @brief Maximum payload size in bytes */
#define ACP_MAX_PAYLOAD_SIZE 1024

/** @brief Maximum frame size including all overhead */
#define ACP_MAX_FRAME_SIZE (ACP_MAX_PAYLOAD_SIZE + 64)

/** @brief HMAC tag length (truncated SHA-256) */
#define ACP_HMAC_TAG_LEN 16

/** @brief CRC16 size in bytes */
#define ACP_CRC16_SIZE 2

/** @brief COBS delimiter byte */
#define ACP_COBS_DELIMITER 0x00

/** @brief Frame sync byte 1 */
#define ACP_SYNC_BYTE_1 0xAA

/** @brief Frame sync byte 2 */
#define ACP_SYNC_BYTE_2 0x55

/** @brief ACP protocol version */
#define ACP_PROTOCOL_VERSION 0x11

    /* ========================================================================== */
    /*                               Frame Types                                  */
    /* ========================================================================== */

    /** @brief ACP frame type enumeration */
    typedef enum
    {
        ACP_FRAME_TYPE_TELEMETRY = 0x01, /**< Telemetry data (MAY be unauthenticated) */
        ACP_FRAME_TYPE_COMMAND = 0x02,   /**< Command data (MUST be authenticated) */
        ACP_FRAME_TYPE_SYSTEM = 0x03,    /**< System/status messages */
        ACP_FRAME_TYPE_RESERVED = 0xFF   /**< Reserved for future use */
    } acp_frame_type_t;

/* ========================================================================== */
/*                               Frame Flags                                 */
/* ========================================================================== */

/** @brief ACP frame flag bits */
#define ACP_FLAG_AUTHENTICATED 0x01 /**< Frame includes HMAC authentication */
#define ACP_FLAG_RESERVED_1 0x02    /**< Reserved for future use */
#define ACP_FLAG_RESERVED_2 0x04    /**< Reserved for future use */
#define ACP_FLAG_RESERVED_3 0x08    /**< Reserved for future use */
#define ACP_FLAG_RESERVED_4 0x10    /**< Reserved for future use */
#define ACP_FLAG_RESERVED_5 0x20    /**< Reserved for future use */
#define ACP_FLAG_RESERVED_6 0x40    /**< Reserved for future use */
#define ACP_FLAG_RESERVED_7 0x80    /**< Reserved for future use */

    /* ========================================================================== */
    /*                              Wire Format                                   */
    /* ========================================================================== */

    /**
     * @brief ACP wire header structure (network byte order)
     *
     * This structure represents the fixed header portion of all ACP frames
     * as they appear on the wire (after COBS encoding).
     */
    typedef struct __attribute__((packed))
    {
        uint8_t version;   /**< Protocol version (ACP_PROTOCOL_VERSION_MAJOR) */
        uint8_t type;      /**< Frame type (acp_frame_type_t) */
        uint8_t flags;     /**< Frame flags (ACP_FLAG_*) */
        uint8_t reserved;  /**< Reserved byte (must be 0 in v0.3) */
        uint16_t length;   /**< Payload length in bytes (0-1024) */
        uint32_t sequence; /**< Sequence number (only if ACP_FLAG_AUTHENTICATED set) */
    } acp_wire_header_t;

/* Static assertion to ensure wire header packing */
#define ACP_STATIC_ASSERT(cond, msg) typedef char acp_static_assert_##__LINE__[(cond) ? 1 : -1]
    ACP_STATIC_ASSERT(sizeof(acp_wire_header_t) == 10, "Wire header must be exactly 10 bytes"); /* ========================================================================== */
    /*                            Host Structures                                 */
    /* ========================================================================== */

    /**
     * @brief ACP frame structure (host representation)
     *
     * This structure represents a decoded ACP frame in host-native format
     * for application use.
     */
    typedef struct
    {
        uint8_t version;                       /**< Protocol version */
        uint8_t type;                          /**< Frame type */
        uint8_t flags;                         /**< Frame flags */
        uint16_t length;                       /**< Payload length */
        uint32_t sequence;                     /**< Sequence number (if authenticated) */
        uint8_t payload[ACP_MAX_PAYLOAD_SIZE]; /**< Payload data */
        uint16_t crc16;                        /**< CRC16-CCITT checksum */
        uint8_t hmac_tag[ACP_HMAC_TAG_LEN];    /**< HMAC tag (if authenticated) */
    } acp_frame_t;

    /**
     * @brief ACP session structure for authentication state
     */
    typedef struct
    {
        uint32_t key_id;            /**< Key identifier for keystore lookup */
        uint8_t key[32];            /**< HMAC key material (256 bits) */
        uint64_t nonce;             /**< Session nonce */
        uint32_t next_sequence;     /**< Next sequence number to send */
        uint32_t last_accepted_seq; /**< Last accepted sequence number */
        uint8_t policy_flags;       /**< Session policy (reserved) */
        bool initialized;           /**< Session initialization flag */
    } acp_session_t;

    /* ========================================================================== */
    /*                             Result Codes                                   */
    /* ========================================================================== */

    /**
     * @brief ACP result/error code enumeration
     */
    typedef enum
    {
        ACP_OK = 0, /**< Success */

        /* Generic errors */
        ACP_ERR_INVALID_PARAM = -1,    /**< Invalid parameter */
        ACP_ERR_BUFFER_TOO_SMALL = -2, /**< Output buffer too small */
        ACP_ERR_NEED_MORE_DATA = -3,   /**< Need more input data */

        /* Frame format errors */
        ACP_ERR_INVALID_VERSION = -10,   /**< Unsupported protocol version */
        ACP_ERR_INVALID_TYPE = -11,      /**< Invalid frame type */
        ACP_ERR_PAYLOAD_TOO_LARGE = -12, /**< Payload exceeds ACP_MAX_PAYLOAD_SIZE */
        ACP_ERR_MALFORMED_FRAME = -13,   /**< Frame structure is malformed */

        /* COBS framing errors */
        ACP_ERR_COBS_DECODE = -20, /**< COBS decode error */
        ACP_ERR_COBS_ENCODE = -21, /**< COBS encode error */

        /* Integrity errors */
        ACP_ERR_CRC_MISMATCH = -30, /**< CRC16 verification failed */

        /* Authentication errors */
        ACP_ERR_AUTH_REQUIRED = -40,    /**< Authentication required but not present */
        ACP_ERR_AUTH_FAILED = -41,      /**< HMAC authentication failed */
        ACP_ERR_REPLAY = -42,           /**< Replay attack detected */
        ACP_ERR_KEY_NOT_FOUND = -43,    /**< Key not found in keystore */
        ACP_ERR_SESSION_NOT_INIT = -44, /**< Session not initialized */
        ACP_ERR_SESSION_EXPIRED = -45,  /**< Session has expired */
        ACP_ERR_REPLAY_ATTACK = -42,    /**< Replay attack detected (alias) */

        /* System errors */
        ACP_ERR_NOT_IMPLEMENTED = -90, /**< Feature not implemented */
        ACP_ERR_INTERNAL = -99         /**< Internal error */
    } acp_result_t;

    /* ========================================================================== */
    /*                           Core API Functions                              */
    /* ========================================================================== */

    /**
     * @brief Initialize the ACP library
     *
     * Must be called before using any other ACP functions.
     *
     * @return ACP_OK on success, error code on failure
     */
    acp_result_t acp_init(void);

    /**
     * @brief Cleanup the ACP library
     *
     * Should be called when finished using the ACP library.
     */
    void acp_cleanup(void);

    /**
     * @brief Encode an ACP frame
     *
     * Encodes payload data into a complete ACP frame with COBS framing,
     * CRC16 integrity, and optional HMAC authentication.
     *
     * @param[in]  type           Frame type (acp_frame_type_t)
     * @param[in]  flags          Frame flags (ACP_FLAG_*)
     * @param[in]  payload        Payload data to encode
     * @param[in]  payload_len    Length of payload (0-ACP_MAX_PAYLOAD_SIZE)
     * @param[in]  session        Session for authentication (NULL for unauthenticated)
     * @param[out] output         Output buffer for encoded frame
     * @param[in,out] output_len  Input: buffer size, Output: encoded frame length
     *
     * @return ACP_OK on success, error code on failure
     */
    acp_result_t acp_encode_frame(
        uint8_t type,
        uint8_t flags,
        const uint8_t *payload,
        size_t payload_len,
        acp_session_t *session,
        uint8_t *output,
        size_t *output_len);

    /**
     * @brief Decode an ACP frame from stream
     *
     * Attempts to decode one complete ACP frame from input stream,
     * handling COBS framing, CRC16 verification, and optional HMAC validation.
     *
     * @param[in]  input         Input stream buffer
     * @param[in]  input_len     Length of input data
     * @param[out] frame         Decoded frame structure
     * @param[out] consumed      Number of input bytes consumed
     * @param[in]  session       Session for authentication (NULL for unauthenticated)
     *
     * @return ACP_OK on success, ACP_ERR_NEED_MORE_DATA if incomplete, other error codes on failure
     */
    acp_result_t acp_decode_frame(
        const uint8_t *input,
        size_t input_len,
        acp_frame_t *frame,
        size_t *consumed,
        acp_session_t *session);

    /* ========================================================================== */
    /*                          Session Management                                */
    /* ========================================================================== */

    /**
     * @brief Initialize an ACP session
     *
     * @param[out] session        Session structure to initialize
     * @param[in]  key_id         Key identifier for keystore lookup
     * @param[in]  key            HMAC key material (32 bytes recommended)
     * @param[in]  key_len        Length of key material
     * @param[in]  nonce          Session nonce (should be unique)
     *
     * @return ACP_OK on success, error code on failure
     */
    acp_result_t acp_session_init(
        acp_session_t *session,
        uint32_t key_id,
        const uint8_t *key,
        size_t key_len,
        uint64_t nonce);

    /**
     * @brief Rotate session key or nonce
     *
     * @param[in,out] session     Session to rotate
     * @param[in]     new_key     New key material (NULL to keep current)
     * @param[in]     new_key_len Length of new key
     * @param[in]     new_nonce   New nonce value
     *
     * @return ACP_OK on success, error code on failure
     */
    acp_result_t acp_session_rotate(
        acp_session_t *session,
        const uint8_t *new_key,
        size_t new_key_len,
        uint64_t new_nonce);

    /**
     * @brief Reset session sequence counters
     *
     * @param[in,out] session Session to reset
     *
     * @return ACP_OK on success, error code on failure
     */
    acp_result_t acp_session_reset_sequence(acp_session_t *session);

    /* ========================================================================== */
    /*                            Utility Functions                              */
    /* ========================================================================== */

    /**
     * @brief Get human-readable error string
     *
     * @param[in] result Error code
     *
     * @return Static string describing the error
     */
    const char *acp_error_string(acp_result_t result);

    /**
     * @brief Validate frame type
     *
     * @param[in] type Frame type to validate
     *
     * @return true if valid, false otherwise
     */
    bool acp_is_valid_frame_type(uint8_t type);

    /**
     * @brief Check if frame type requires authentication
     *
     * @param[in] type Frame type to check
     *
     * @return true if authentication required, false otherwise
     */
    bool acp_frame_requires_auth(uint8_t type);

    /**
     * @brief Convert host uint16 to network byte order
     *
     * @param[in] host_val Host byte order value
     *
     * @return Network byte order value
     */
    uint16_t acp_htons(uint16_t host_val);

    /**
     * @brief Convert network uint16 to host byte order
     *
     * @param[in] net_val Network byte order value
     *
     * @return Host byte order value
     */
    uint16_t acp_ntohs(uint16_t net_val);

    /**
     * @brief Convert host uint32 to network byte order
     *
     * @param[in] host_val Host byte order value
     *
     * @return Network byte order value
     */
    uint32_t acp_htonl(uint32_t host_val);

    /**
     * @brief Convert network uint32 to host byte order
     *
     * @param[in] net_val Network byte order value
     *
     * @return Host byte order value
     */
    uint32_t acp_ntohl(uint32_t net_val);

    /* ========================================================================== */
    /*                         Frame Processing Functions                         */
    /* ========================================================================== */

    /**
     * @brief Encode ACP frame to wire format
     */
    int acp_frame_encode(const acp_frame_t *frame, uint8_t *output, size_t output_size, size_t *bytes_written);

    /**
     * @brief Decode wire format to ACP frame
     */
    int acp_frame_decode(const uint8_t *input, size_t input_size, acp_frame_t *frame, size_t *bytes_consumed);

    /**
     * @brief Calculate encoded frame size
     */
    size_t acp_frame_encoded_size(const acp_frame_t *frame);

    /**
     * @brief Create telemetry frame
     */
    int acp_frame_create_telemetry(acp_frame_t *frame, uint8_t msg_type, uint32_t sequence,
                                   const void *payload, size_t payload_len);

    /**
     * @brief Create command frame
     */
    int acp_frame_create_command(acp_frame_t *frame, uint8_t msg_type, uint32_t sequence,
                                 uint8_t priority, const void *payload, size_t payload_len);

#ifdef __cplusplus
}
#endif

#endif /* ACP_PROTOCOL_H */
