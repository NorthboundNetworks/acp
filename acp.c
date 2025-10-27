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
#include "acp_cobs.h"
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

    /* Construct frame structure */
    acp_frame_t frame = {0};
    frame.version = ACP_PROTOCOL_VERSION;
    frame.type = type;
    frame.flags = flags;
    frame.length = (uint16_t)payload_len;

    /* Set sequence number for authenticated frames */
    if (flags & ACP_FLAG_AUTHENTICATED)
    {
        frame.sequence = session->next_sequence;
    }

    /* Copy payload */
    if (payload_len > 0)
    {
        memcpy(frame.payload, payload, payload_len);
    }

    /* Encode the frame using the framer */
    size_t frame_size;
    int result = acp_frame_encode(&frame, output, *output_len, &frame_size);
    if (result != ACP_OK)
    {
        return result;
    }

    /* For authenticated frames, append HMAC tag */
    if (flags & ACP_FLAG_AUTHENTICATED)
    {
        /* Check if we have space for HMAC tag */
        if (*output_len < frame_size + ACP_HMAC_TAG_LEN)
        {
            *output_len = frame_size + ACP_HMAC_TAG_LEN;
            return ACP_ERR_BUFFER_TOO_SMALL;
        }

        /* Calculate HMAC over the encoded frame (excluding delimiters) */
        uint8_t hmac_tag[32]; /* Full SHA-256 output */
        acp_hmac_sha256(session->key, ACP_KEY_SIZE,
                        output + 1, frame_size - 2, /* Skip delimiters */
                        hmac_tag);

        /* Append truncated HMAC tag after the complete frame */
        memcpy(output + frame_size, hmac_tag, ACP_HMAC_TAG_LEN);
        frame_size += ACP_HMAC_TAG_LEN;

        /* Update session sequence number */
        session->next_sequence++;
    }

    *output_len = frame_size;
    return ACP_OK;
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

    *consumed = 0;
    memset(frame, 0, sizeof(*frame));

    /* For authenticated frames, we need to handle HMAC verification */
    /* First, try to find frame boundaries to determine if frame is authenticated */

    /* Find frame start delimiter */
    if (input[0] != ACP_COBS_DELIMITER)
    {
        return ACP_ERR_MALFORMED_FRAME;
    }

    /* Find frame end delimiter */
    size_t frame_end = 0;
    for (size_t i = 1; i < input_len; i++)
    {
        if (input[i] == ACP_COBS_DELIMITER)
        {
            frame_end = i;
            break;
        }
    }

    if (frame_end == 0)
    {
        return ACP_ERR_NEED_MORE_DATA;
    }

    /* Try to decode the frame without HMAC first to check if it's authenticated */
    acp_frame_t temp_frame;
    size_t frame_consumed;
    int result = acp_frame_decode(input, frame_end + 1, &temp_frame, &frame_consumed);
    if (result != ACP_OK)
    {
        return result;
    }

    /* If frame is authenticated, verify HMAC */
    if (temp_frame.flags & ACP_FLAG_AUTHENTICATED)
    {
        if (session == NULL || !session->initialized)
        {
            return ACP_ERR_SESSION_NOT_INIT;
        }

        /* Check if we have HMAC tag after the frame */
        size_t total_size = frame_consumed + ACP_HMAC_TAG_LEN;
        if (input_len < total_size)
        {
            return ACP_ERR_NEED_MORE_DATA;
        }

        /* Verify HMAC over the encoded frame (excluding delimiters) */
        uint8_t expected_hmac[32];
        acp_hmac_sha256(session->key, ACP_KEY_SIZE,
                        input + 1, frame_consumed - 2, /* Skip delimiters */
                        expected_hmac);

        /* Compare with received HMAC tag (constant-time) */
        const uint8_t *received_hmac = input + frame_consumed;
        if (acp_crypto_memcmp_ct(expected_hmac, received_hmac, ACP_HMAC_TAG_LEN) != 0)
        {
            return ACP_ERR_AUTH_FAILED;
        }

        /* Verify sequence number for replay protection */
        if (temp_frame.sequence <= session->last_accepted_seq)
        {
            return ACP_ERR_REPLAY;
        }

        /* Update session state */
        session->last_accepted_seq = temp_frame.sequence;
        *consumed = total_size;
    }
    else
    {
        /* Unauthenticated frame */
        *consumed = frame_consumed;

        /* Enforce authentication policy for command frames */
        if (acp_frame_requires_auth(temp_frame.type))
        {
            return ACP_ERR_AUTH_REQUIRED;
        }
    }

    /* Copy the decoded frame */
    *frame = temp_frame;
    return ACP_OK;
}

/* ========================================================================== */
/*                          Session Management                                */
/* ========================================================================== */

/* Session management functions are implemented in acp_session.c */

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
