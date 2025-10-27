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
 * @file acp_framer.c
 * @brief ACP frame encoding and decoding implementation
 *
 * Integrates COBS framing with CRC16 integrity checking to provide
 * complete ACP frame processing as specified in the protocol.
 *
 * @version 0.3.0
 * @date 2025-10-27
 */

#include "acp_protocol.h"
#include "acp_errors.h"
#include "acp_crc16.h"
#include "acp_cobs.h"
#include "acp_platform_log.h"

#include <string.h>

/* ========================================================================== */
/*                           Frame Processing                                 */
/* ========================================================================== */

int acp_frame_encode(const acp_frame_t *frame, uint8_t *output, size_t output_size, size_t *bytes_written)
{
    if (!frame || !output || !bytes_written)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    *bytes_written = 0;

    /* Calculate variable header size based on flags */
    size_t header_size = acp_wire_header_size(frame->flags);
    size_t wire_frame_size = header_size + frame->length + 2; /* +2 for CRC */

    /* Check if we have space for worst-case COBS encoding */
    size_t max_encoded_size = acp_cobs_max_encoded_size(wire_frame_size) + 2; /* +2 for delimiters */
    if (output_size < max_encoded_size)
    {
        ACP_LOG_ERROR("Output buffer too small: need %zu, have %zu", max_encoded_size, output_size);
        return ACP_ERR_BUFFER_TOO_SMALL;
    }

    /* Build wire frame in temporary buffer */
    uint8_t wire_frame[ACP_MAX_FRAME_SIZE];
    if (wire_frame_size > sizeof(wire_frame))
    {
        ACP_LOG_ERROR("Frame too large: %zu bytes", wire_frame_size);
        return ACP_ERR_PAYLOAD_TOO_LARGE;
    }

    /* Create base wire header */
    acp_wire_header_base_t *base_header = (acp_wire_header_base_t *)wire_frame;
    base_header->version = frame->version;
    base_header->type = frame->type;
    base_header->flags = frame->flags;
    base_header->reserved = 0;
    base_header->length = frame->length; /* Host byte order for now */

    /* Convert length to network byte order */
    base_header->length = ((base_header->length & 0xFF) << 8) | ((base_header->length >> 8) & 0xFF);

    /* Add conditional sequence field if authenticated */
    uint8_t *payload_start = wire_frame + sizeof(acp_wire_header_base_t);
    if (frame->flags & ACP_FLAG_AUTHENTICATED)
    {
        /* Add sequence number in network byte order */
        uint32_t seq_be = ((frame->sequence & 0xFF) << 24) |
                          (((frame->sequence >> 8) & 0xFF) << 16) |
                          (((frame->sequence >> 16) & 0xFF) << 8) |
                          ((frame->sequence >> 24) & 0xFF);
        memcpy(payload_start, &seq_be, sizeof(uint32_t));
        payload_start += sizeof(uint32_t);
    }

    /* Copy payload */
    if (frame->length > 0)
    {
        memcpy(payload_start, frame->payload, frame->length);
    }

    /* Calculate and append CRC16 */
    uint16_t crc = acp_crc16_calculate(wire_frame, wire_frame_size - 2);
    wire_frame[wire_frame_size - 2] = (uint8_t)(crc & 0xFF);
    wire_frame[wire_frame_size - 1] = (uint8_t)((crc >> 8) & 0xFF);

    /* COBS encode the frame */
    size_t encoded_len;
    int result = acp_cobs_encode(wire_frame, wire_frame_size, output + 1, output_size - 2, &encoded_len);
    if (result != ACP_OK)
    {
        ACP_LOG_ERROR("COBS encoding failed: %d", result);
        return result;
    }

    /* Add frame delimiters */
    output[0] = ACP_COBS_DELIMITER;
    output[encoded_len + 1] = ACP_COBS_DELIMITER;

    *bytes_written = encoded_len + 2;

    ACP_LOG_DEBUG("Encoded frame: type=0x%02X, payload=%zu bytes, total=%zu bytes",
                  frame->type, (size_t)frame->length, *bytes_written);

    return ACP_OK;
}

int acp_frame_decode(const uint8_t *input, size_t input_size, acp_frame_t *frame, size_t *bytes_consumed)
{
    if (!input || !frame || !bytes_consumed)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    *bytes_consumed = 0;

    /* Need at least minimum frame size (base header + CRC + delimiters) */
    if (input_size < sizeof(acp_wire_header_base_t) + 2 + 2)
    { /* base header + CRC + delimiters */
        return ACP_ERR_NEED_MORE_DATA;
    }

    /* Find frame boundaries */
    if (input[0] != ACP_COBS_DELIMITER)
    {
        ACP_LOG_WARN("Missing frame start delimiter");
        return ACP_ERR_MALFORMED_FRAME;
    }

    /* Find end delimiter */
    size_t frame_end = 0;
    for (size_t i = 1; i < input_size; i++)
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

    /* COBS decode the frame content */
    uint8_t decoded_frame[ACP_MAX_FRAME_SIZE];
    size_t decoded_len;
    int result = acp_cobs_decode(input + 1, frame_end - 1, decoded_frame, sizeof(decoded_frame), &decoded_len);
    if (result != ACP_OK)
    {
        ACP_LOG_ERROR("COBS decoding failed: %d", result);
        return result;
    }

    /* Need at least base header + CRC */
    if (decoded_len < sizeof(acp_wire_header_base_t) + 2)
    {
        ACP_LOG_WARN("Decoded frame too short: %zu bytes", decoded_len);
        return ACP_ERR_MALFORMED_FRAME;
    }

    /* Parse base wire header */
    const acp_wire_header_base_t *base_header = (const acp_wire_header_base_t *)decoded_frame;

    /* Calculate expected header size based on flags */
    size_t expected_header_size = acp_wire_header_size(base_header->flags);

    /* Verify we have enough data for the full header */
    if (decoded_len < expected_header_size + 2)
    {
        ACP_LOG_WARN("Decoded frame too short for header: need %zu+2, have %zu",
                     expected_header_size, decoded_len);
        return ACP_ERR_MALFORMED_FRAME;
    }

    /* Verify CRC */
    uint16_t calculated_crc = acp_crc16_calculate(decoded_frame, decoded_len - 2);
    uint16_t received_crc = ((uint16_t)decoded_frame[decoded_len - 1] << 8) | decoded_frame[decoded_len - 2];

    if (calculated_crc != received_crc)
    {
        ACP_LOG_ERROR("CRC mismatch: calculated=0x%04X, received=0x%04X", calculated_crc, received_crc);
        return ACP_ERR_CRC_MISMATCH;
    }

    /* Convert length from network byte order */
    uint16_t payload_len = ((base_header->length & 0xFF) << 8) | ((base_header->length >> 8) & 0xFF);

    /* Verify frame size consistency */
    if (expected_header_size + payload_len + 2 != decoded_len)
    {
        ACP_LOG_ERROR("Frame size mismatch: header says %zu+%u+2, got %zu",
                      expected_header_size, payload_len, decoded_len);
        return ACP_ERR_MALFORMED_FRAME;
    }

    /* Fill in frame structure */
    frame->version = base_header->version;
    frame->type = base_header->type;
    frame->flags = base_header->flags;
    frame->length = payload_len;

    /* Parse conditional sequence field */
    uint32_t sequence = 0;
    const uint8_t *payload_start = decoded_frame + sizeof(acp_wire_header_base_t);
    if (base_header->flags & ACP_FLAG_AUTHENTICATED)
    {
        /* Extract sequence number from network byte order */
        uint32_t seq_be;
        memcpy(&seq_be, payload_start, sizeof(uint32_t));
        sequence = ((seq_be & 0xFF) << 24) |
                   (((seq_be >> 8) & 0xFF) << 16) |
                   (((seq_be >> 16) & 0xFF) << 8) |
                   ((seq_be >> 24) & 0xFF);
        payload_start += sizeof(uint32_t);
    }
    frame->sequence = sequence;

    /* Copy payload if present */
    if (payload_len > 0)
    {
        if (payload_len > ACP_MAX_PAYLOAD_SIZE)
        {
            ACP_LOG_ERROR("Payload too large: %u bytes", payload_len);
            return ACP_ERR_PAYLOAD_TOO_LARGE;
        }
        memcpy(frame->payload, payload_start, payload_len);
    }

    *bytes_consumed = frame_end + 1;

    ACP_LOG_DEBUG("Decoded frame: type=0x%02X, payload=%u bytes, consumed=%zu bytes",
                  frame->type, payload_len, *bytes_consumed);

    return ACP_OK;
}

size_t acp_frame_encoded_size(const acp_frame_t *frame)
{
    if (!frame)
    {
        return 0;
    }

    size_t header_size = acp_wire_header_size(frame->flags);
    size_t wire_size = header_size + frame->length + 2; /* +2 for CRC */
    return acp_cobs_max_encoded_size(wire_size) + 2;    /* +2 for delimiters */
}

/* ========================================================================== */
/*                         Frame Builder Helpers                             */
/* ========================================================================== */

int acp_frame_create_telemetry(acp_frame_t *frame, uint8_t msg_type, uint32_t sequence,
                               const void *payload, size_t payload_len)
{
    if (!frame)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    if (payload_len > ACP_MAX_PAYLOAD_SIZE)
    {
        return ACP_ERR_PAYLOAD_TOO_LARGE;
    }

    frame->version = ACP_PROTOCOL_VERSION;
    frame->type = msg_type;
    frame->flags = 0; /* No special flags for basic telemetry */
    frame->sequence = sequence;
    frame->length = (uint16_t)payload_len;

    if (payload_len > 0 && payload)
    {
        memcpy(frame->payload, payload, payload_len);
    }

    return ACP_OK;
}

int acp_frame_create_command(acp_frame_t *frame, uint8_t msg_type, uint32_t sequence,
                             uint8_t priority, const void *payload, size_t payload_len)
{
    if (!frame)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    if (payload_len > ACP_MAX_PAYLOAD_SIZE)
    {
        return ACP_ERR_PAYLOAD_TOO_LARGE;
    }

    frame->version = ACP_PROTOCOL_VERSION;
    frame->type = msg_type;
    frame->flags = (priority & 0x03) << 2; /* Pack priority into flags */
    frame->sequence = sequence;
    frame->length = (uint16_t)payload_len;

    if (payload_len > 0 && payload)
    {
        memcpy(frame->payload, payload, payload_len);
    }

    return ACP_OK;
}
