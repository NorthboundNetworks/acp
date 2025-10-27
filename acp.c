/**
 * @file acp.c
 * @brief ACP core API implementation
 *
 * This module implements the core ACP protocol functions for frame encoding,
 * decoding, and session management. Currently contains stubs that will be
 * filled in during subsequent implementation phases.
 *
 * @version 0.3.0
 * @date 2025-10-27
 */

#include "acp_protocol.h"
#include "acp_version.h"
#include "acp_errors.h"
#include "acp_crc16.h"
#include <string.h>
#include <stdio.h>

/* ========================================================================== */
/*                            Library State                                   */
/* ========================================================================== */

/** @brief Global library initialization flag */
static int acp_initialized = 0;

/* ========================================================================== */
/*                           Core API Functions                              */
/* ========================================================================== */

/**
 * @brief Initialize the ACP library
 */
acp_result_t acp_init(void)
{
    if (acp_initialized)
    {
        return ACP_OK; /* Already initialized */
    }

    /* Initialize CRC16 lookup table */
    acp_crc16_init_table();

    /* TODO: Initialize other subsystems as they're implemented */
    /* - Platform shims */
    /* - Crypto subsystem */
    /* - Keystore backend */

    acp_initialized = 1;
    return ACP_OK;
}

/**
 * @brief Cleanup the ACP library
 */
void acp_cleanup(void)
{
    if (!acp_initialized)
    {
        return; /* Not initialized */
    }

    /* TODO: Cleanup subsystems */
    /* - Close keystore */
    /* - Cleanup platform shims */
    /* - Clear sensitive data */

    acp_initialized = 0;
}

/**
 * @brief Encode an ACP frame
 */
acp_result_t acp_encode_frame(
    uint8_t type,
    uint8_t flags,
    const uint8_t *payload,
    size_t payload_len,
    acp_session_t *session,
    uint8_t *output,
    size_t *output_len)
{
    /* Parameter validation */
    if (payload == NULL && payload_len > 0)
    {
        return ACP_ERR_INVALID_PARAM;
    }
    if (output == NULL || output_len == NULL)
    {
        return ACP_ERR_INVALID_PARAM;
    }
    if (payload_len > ACP_MAX_PAYLOAD_SIZE)
    {
        return ACP_ERR_PAYLOAD_TOO_LARGE;
    }
    if (!acp_is_valid_frame_type(type))
    {
        return ACP_ERR_INVALID_TYPE;
    }

    /* Check authentication requirements */
    if (acp_frame_requires_auth(type) && !(flags & ACP_FLAG_AUTHENTICATED))
    {
        return ACP_ERR_AUTH_REQUIRED;
    }
    if ((flags & ACP_FLAG_AUTHENTICATED) && session == NULL)
    {
        return ACP_ERR_SESSION_NOT_INIT;
    }

    /* TODO: Implement frame encoding */
    /* This is a stub - actual implementation will be in T021 */

    /* For now, return minimum buffer size estimate */
    size_t min_size = sizeof(acp_wire_header_t) + payload_len + ACP_CRC16_SIZE;
    if (flags & ACP_FLAG_AUTHENTICATED)
    {
        min_size += ACP_HMAC_TAG_LEN;
    }

    /* Add COBS overhead estimate (worst case: ~1% increase + 1 byte) */
    min_size = min_size + (min_size / 100) + 2;

    if (*output_len < min_size)
    {
        *output_len = min_size;
        return ACP_ERR_BUFFER_TOO_SMALL;
    }

    /* Stub: Just return error for now */
    return ACP_ERR_NOT_IMPLEMENTED;
}

/**
 * @brief Decode an ACP frame from stream
 */
acp_result_t acp_decode_frame(
    const uint8_t *input,
    size_t input_len,
    acp_frame_t *frame,
    size_t *consumed,
    acp_session_t *session)
{
    /* Parameter validation */
    if (input == NULL || frame == NULL || consumed == NULL)
    {
        return ACP_ERR_INVALID_PARAM;
    }
    if (input_len == 0)
    {
        return ACP_ERR_NEED_MORE_DATA;
    }

    /* TODO: Implement frame decoding */
    /* This is a stub - actual implementation will be in T022 */

    *consumed = 0;
    memset(frame, 0, sizeof(*frame));

    /* Stub: Just return error for now */
    return ACP_ERR_NOT_IMPLEMENTED;
}

/* ========================================================================== */
/*                          Session Management                                */
/* ========================================================================== */

/**
 * @brief Initialize an ACP session
 */
acp_result_t acp_session_init(
    acp_session_t *session,
    uint32_t key_id,
    const uint8_t *key,
    size_t key_len,
    uint64_t nonce)
{
    if (session == NULL || key == NULL)
    {
        return ACP_ERR_INVALID_PARAM;
    }
    if (key_len == 0 || key_len > sizeof(session->key))
    {
        return ACP_ERR_KEY_TOO_SHORT;
    }

    /* TODO: Full session initialization will be implemented in T029 */

    /* Basic initialization for now */
    memset(session, 0, sizeof(*session));
    session->key_id = key_id;
    memcpy(session->key, key, key_len);
    session->nonce = nonce;
    session->next_sequence = 1; /* Start at 1, not 0 */
    session->last_accepted_seq = 0;
    session->policy_flags = 0;
    session->initialized = true;

    return ACP_OK;
}

/**
 * @brief Rotate session key or nonce
 */
acp_result_t acp_session_rotate(
    acp_session_t *session,
    const uint8_t *new_key,
    size_t new_key_len,
    uint64_t new_nonce)
{
    if (session == NULL || !session->initialized)
    {
        return ACP_ERR_SESSION_NOT_INIT;
    }

    /* TODO: Full implementation in T029 */

    /* Basic rotation for now */
    if (new_key != NULL && new_key_len > 0 && new_key_len <= sizeof(session->key))
    {
        memcpy(session->key, new_key, new_key_len);
        if (new_key_len < sizeof(session->key))
        {
            memset(&session->key[new_key_len], 0, sizeof(session->key) - new_key_len);
        }
    }

    session->nonce = new_nonce;
    session->next_sequence = 1; /* Reset sequence on rotation */
    session->last_accepted_seq = 0;

    return ACP_OK;
}

/**
 * @brief Reset session sequence counters
 */
acp_result_t acp_session_reset_sequence(acp_session_t *session)
{
    if (session == NULL || !session->initialized)
    {
        return ACP_ERR_SESSION_NOT_INIT;
    }

    session->next_sequence = 1;
    session->last_accepted_seq = 0;

    return ACP_OK;
}

/* ========================================================================== */
/*                            Utility Functions                              */
/* ========================================================================== */

/**
 * @brief Validate frame type
 */
bool acp_is_valid_frame_type(uint8_t type)
{
    switch (type)
    {
    case ACP_FRAME_TYPE_TELEMETRY:
    case ACP_FRAME_TYPE_COMMAND:
    case ACP_FRAME_TYPE_SYSTEM:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Check if frame type requires authentication
 */
bool acp_frame_requires_auth(uint8_t type)
{
    switch (type)
    {
    case ACP_FRAME_TYPE_COMMAND:
        return true; /* Commands MUST be authenticated */
    case ACP_FRAME_TYPE_TELEMETRY:
    case ACP_FRAME_TYPE_SYSTEM:
        return false; /* MAY be authenticated, but not required */
    default:
        return false;
    }
}

/* ========================================================================== */
/*                         Byte Order Conversion                             */
/* ========================================================================== */

/**
 * @brief Convert host uint16 to network byte order
 */
uint16_t acp_htons(uint16_t host_val)
{
    /* Check if we're on a little-endian system */
    static const uint16_t test = 0x0001;
    const uint8_t *test_bytes = (const uint8_t *)&test;

    if (test_bytes[0] == 0x01)
    {
        /* Little-endian: swap bytes */
        return ((host_val & 0xFF) << 8) | ((host_val >> 8) & 0xFF);
    }
    else
    {
        /* Big-endian: no conversion needed */
        return host_val;
    }
}

/**
 * @brief Convert network uint16 to host byte order
 */
uint16_t acp_ntohs(uint16_t net_val)
{
    /* Network to host is same as host to network for 16-bit */
    return acp_htons(net_val);
}

/**
 * @brief Convert host uint32 to network byte order
 */
uint32_t acp_htonl(uint32_t host_val)
{
    /* Check if we're on a little-endian system */
    static const uint16_t test = 0x0001;
    const uint8_t *test_bytes = (const uint8_t *)&test;

    if (test_bytes[0] == 0x01)
    {
        /* Little-endian: swap bytes */
        return ((host_val & 0x000000FFU) << 24) |
               ((host_val & 0x0000FF00U) << 8) |
               ((host_val & 0x00FF0000U) >> 8) |
               ((host_val & 0xFF000000U) >> 24);
    }
    else
    {
        /* Big-endian: no conversion needed */
        return host_val;
    }
}

/**
 * @brief Convert network uint32 to host byte order
 */
uint32_t acp_ntohl(uint32_t net_val)
{
    /* Network to host is same as host to network for 32-bit */
    return acp_htonl(net_val);
}