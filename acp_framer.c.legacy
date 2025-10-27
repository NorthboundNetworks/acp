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
 * @brief ACP Protocol Frame Processing - COBS Encoding/Decoding and Message Validation
 *
 * Implements Consistent Overhead Byte Stuffing (COBS) framing, CRC-16-CCITT validation,
 * and message parsing for the Autonomous Control Protocol (ACP) v0.3.
 *
 * @version 0.3
 * @date 2025-10-26
 * @author Paul Zanna
 */

#include <string.h>
#include <stddef.h>
#include <arpa/inet.h>
#include "acp_platform_log.h"
#include "acp_protocol.h"

// ===========================================================================
// CRC-16-CCITT IMPLEMENTATION
// ===========================================================================

/**
 * @brief CRC-16-CCITT Lookup Table
 *
 * Pre-computed CRC table for polynomial 0x1021 (x^16 + x^12 + x^5 + 1)
 */
static const uint16_t crc16_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0};

/**
 * @brief Calculate CRC-16-CCITT checksum
 *
 * @param data Data buffer to calculate CRC for
 * @param length Data length in bytes
 * @param initial_crc Initial CRC value (typically 0xFFFF)
 * @return Calculated CRC-16 value
 */
static uint16_t crc16_ccitt_calculate(const uint8_t *data, uint16_t length, uint16_t initial_crc)
{
    uint16_t crc = initial_crc;

    for (uint16_t i = 0; i < length; i++)
    {
        uint8_t tbl_idx = ((crc >> 8) ^ data[i]) & 0xFF;
        crc = ((crc << 8) ^ crc16_table[tbl_idx]) & 0xFFFF;
    }

    return crc;
}

// ===========================================================================
// COBS (CONSISTENT OVERHEAD BYTE STUFFING) IMPLEMENTATION
// ===========================================================================

/**
 * @brief COBS Encode Data
 *
 * Encodes data using Consistent Overhead Byte Stuffing to eliminate zero bytes.
 * Adds exactly one byte overhead plus one byte per 254-byte segment.
 *
 * @param src Source data buffer
 * @param src_len Source data length
 * @param dst Destination buffer (must be at least src_len + (src_len/254) + 1 bytes)
 * @param dst_len Destination buffer size
 * @return Number of encoded bytes, or 0 on error
 */
static uint16_t cobs_encode(const uint8_t *src, uint16_t src_len, uint8_t *dst, uint16_t dst_len)
{
    if (!src || !dst || src_len == 0)
    {
        ACP_LOG_ERROR("COBS encode: Invalid parameters");
        return 0;
    }

    // Calculate required output size: input + overhead + terminator
    uint16_t required_size = src_len + (src_len / 254) + 2;
    if (dst_len < required_size)
    {
        ACP_LOG_ERROR("COBS encode: Buffer too small (%u < %u)", dst_len, required_size);
        return 0;
    }

    uint16_t src_idx = 0;
    uint16_t dst_idx = 0;
    uint8_t code_idx = 0; // Index of current code byte
    uint8_t code = 1;     // Current code value

    // Reserve first byte for code
    dst[dst_idx++] = 0;
    code_idx = 0;

    while (src_idx < src_len)
    {
        if (src[src_idx] == 0)
        {
            // Found zero byte - finalize current code
            dst[code_idx] = code;
            code_idx = dst_idx++;
            dst[code_idx] = 0;
            code = 1;
        }
        else
        {
            // Copy non-zero byte
            dst[dst_idx++] = src[src_idx];
            code++;

            // Check if we need to split at 254 bytes
            if (code == 0xFF)
            {
                dst[code_idx] = code;
                code_idx = dst_idx++;
                dst[code_idx] = 0;
                code = 1;
            }
        }
        src_idx++;
    }

    // Finalize last code
    dst[code_idx] = code;

    // Add frame delimiter (zero byte)
    dst[dst_idx++] = 0;

    return dst_idx;
}

/**
 * @brief COBS Decode Data
 *
 * Decodes COBS-encoded data back to original form.
 *
 * @param src Source encoded buffer
 * @param src_len Source buffer length
 * @param dst Destination buffer (must be at least src_len bytes)
 * @param dst_len Destination buffer size
 * @return Number of decoded bytes, or 0 on error
 */
static uint16_t cobs_decode(const uint8_t *src, uint16_t src_len, uint8_t *dst, uint16_t dst_len)
{
    if (!src || !dst || src_len < 2)
    {
        ACP_LOG_ERROR("COBS decode: Invalid parameters");
        return 0;
    }

    // Check for frame delimiter at end
    if (src[src_len - 1] != 0)
    {
        ACP_LOG_ERROR("COBS decode: Missing frame delimiter");
        return 0;
    }

    uint16_t src_idx = 0;
    uint16_t dst_idx = 0;

    // Process until frame delimiter
    while (src_idx < src_len - 1)
    {
        uint8_t code = src[src_idx++];

        if (code == 0)
        {
            ACP_LOG_ERROR("COBS decode: Invalid code byte");
            return 0;
        }

        // Copy code-1 bytes
        for (uint8_t i = 1; i < code; i++)
        {
            if (src_idx >= src_len - 1)
            {
                ACP_LOG_ERROR("COBS decode: Premature end of data");
                return 0;
            }
            if (dst_idx >= dst_len)
            {
                ACP_LOG_ERROR("COBS decode: Output buffer overflow");
                return 0;
            }

            dst[dst_idx++] = src[src_idx++];
        }

        // Add zero byte if not at end and code was not 0xFF
        if (src_idx < src_len - 1 && code != 0xFF)
        {
            if (dst_idx >= dst_len)
            {
                ACP_LOG_ERROR("COBS decode: Output buffer overflow");
                return 0;
            }
            dst[dst_idx++] = 0;
        }
    }

    return dst_idx;
}

// ===========================================================================
// ACP FRAME VALIDATION AND PARSING
// ===========================================================================

/**
 * @brief Validate ACP Frame Header
 *
 * Validates frame header structure and field values.
 *
 * @param header Pointer to frame header
 * @return true if header is valid, false otherwise
 */
static bool validate_frame_header(const acp_frame_header_t *header)
{
    if (!header)
    {
        return false;
    }

    // Check sync bytes
    if (header->sync[0] != ACP_SYNC_BYTE1 || header->sync[1] != ACP_SYNC_BYTE2)
    {
        ACP_LOG_WARN("Invalid sync bytes: 0x%02X 0x%02X", header->sync[0], header->sync[1]);
        return false;
    }

    // Check protocol version
    if (header->version != ACP_VERSION_1_1)
    {
        ACP_LOG_WARN("Unsupported protocol version: 0x%02X", header->version);
        return false;
    }

    // Validate message type
    switch (header->msg_type)
    {
    case ACP_CMD_CONTROL:
    case ACP_CMD_GIMBAL:
    case ACP_CMD_CONFIG:
    case ACP_TLM_STATUS:
    case ACP_TLM_GIMBAL:
    case ACP_TLM_AUDIT:
    case ACP_TLM_MISSION:
    case ACP_ACK_RESPONSE:
    case ACP_ERR_RESPONSE:
        break;
    default:
        ACP_LOG_WARN("Unknown message type: 0x%02X", header->msg_type);
        return false;
    }

    // Validate payload length
    uint16_t payload_len = ntohs(header->length);
    if (payload_len > ACP_MAX_PAYLOAD_SIZE)
    {
        ACP_LOG_WARN("Payload too large: %u > %u", payload_len, ACP_MAX_PAYLOAD_SIZE);
        return false;
    }

    // Validate flags
    uint8_t reserved_flags = header->flags & ~(ACP_FLAG_AUTH_PRESENT | ACP_FLAG_COMPRESSED | ACP_FLAG_PRIORITY_MASK);
    if (reserved_flags != 0)
    {
        ACP_LOG_WARN("Reserved flags set: 0x%02X", reserved_flags);
        return false;
    }

    return true;
}

// ===========================================================================
// PUBLIC API IMPLEMENTATION
// ===========================================================================

/**
 * @brief Initialize ACP Framing Subsystem
 *
 * @return true if initialization successful
 */
bool acp_framer_init(void)
{
    ACP_LOG_INFO("Initializing ACP framing subsystem");

    // Verify CRC table integrity with known test vector
    uint8_t test_data[] = "123456789";
    uint16_t test_crc = crc16_ccitt_calculate(test_data, 9, 0xFFFF);
    if (test_crc != 0x29B1)
    {
        ACP_LOG_ERROR("CRC-16-CCITT self-test failed: 0x%04X != 0x29B1", test_crc);
        return false;
    }

    ACP_LOG_INFO("ACP framing initialization complete");
    return true;
}

/**
 * @brief Encode ACP Frame with COBS Framing
 *
 * @param header Frame header
 * @param payload Payload data (can be NULL if payload_len is 0)
 * @param payload_len Payload length
 * @param auth_tag Authentication tag (can be NULL if not present)
 * @param frame_out Output buffer for encoded frame
 * @param frame_len_max Maximum frame buffer size
 * @param frame_len_out Output: actual encoded frame length
 * @return true if encoding successful, false on error
 */
bool acp_framer_encode(const acp_frame_header_t *header,
                       const uint8_t *payload,
                       uint16_t payload_len,
                       const uint8_t *auth_tag,
                       uint8_t *frame_out,
                       uint16_t frame_len_max,
                       uint16_t *frame_len_out)
{

    if (!header || !frame_out || !frame_len_out)
    {
        ACP_LOG_ERROR("Encode: Invalid parameters");
        return false;
    }

    // Validate header
    if (!validate_frame_header(header))
    {
        ACP_LOG_ERROR("Encode: Invalid header");
        return false;
    }

    // Validate payload length matches header
    uint16_t expected_payload_len = ntohs(header->length);
    if (payload_len != expected_payload_len)
    {
        ACP_LOG_ERROR("Encode: Payload length mismatch (%u != %u)", payload_len, expected_payload_len);
        return false;
    }

    // Check if payload pointer is required
    if (payload_len > 0 && !payload)
    {
        ACP_LOG_ERROR("Encode: Payload required but NULL");
        return false;
    }

    // Check authentication tag requirement
    bool has_auth = ACP_HAS_AUTH(header->flags);
    if (has_auth && !auth_tag)
    {
        ACP_LOG_ERROR("Encode: Auth tag required but NULL");
        return false;
    }

    // Calculate frame size before COBS encoding
    uint16_t raw_frame_size = sizeof(acp_frame_header_t) + payload_len;
    if (has_auth)
    {
        raw_frame_size += ACP_AUTH_TAG_SIZE;
    }
    raw_frame_size += ACP_CRC16_SIZE;

    // Allocate temporary buffer for raw frame
    uint8_t raw_frame[ACP_MAX_FRAME_SIZE];
    if (raw_frame_size > sizeof(raw_frame))
    {
        ACP_LOG_ERROR("Encode: Frame too large (%u)", raw_frame_size);
        return false;
    }

    // Build raw frame
    uint16_t offset = 0;

    // Copy header
    memcpy(raw_frame + offset, header, sizeof(acp_frame_header_t));
    offset += sizeof(acp_frame_header_t);

    // Copy payload
    if (payload_len > 0)
    {
        memcpy(raw_frame + offset, payload, payload_len);
        offset += payload_len;
    }

    // Copy authentication tag
    if (has_auth)
    {
        memcpy(raw_frame + offset, auth_tag, ACP_AUTH_TAG_SIZE);
        offset += ACP_AUTH_TAG_SIZE;
    }

    // Calculate and append CRC (excluding CRC field itself)
    uint16_t crc = crc16_ccitt_calculate(raw_frame, offset, 0xFFFF);
    raw_frame[offset++] = (crc >> 8) & 0xFF; // CRC high byte
    raw_frame[offset++] = crc & 0xFF;        // CRC low byte

    // COBS encode the complete frame
    uint16_t encoded_len = cobs_encode(raw_frame, offset, frame_out, frame_len_max);
    if (encoded_len == 0)
    {
        ACP_LOG_ERROR("Encode: COBS encoding failed");
        return false;
    }

    *frame_len_out = encoded_len;

    ACP_LOG_DEBUG("Encoded frame: raw=%u, cobs=%u, type=0x%02X",
                  offset, encoded_len, header->msg_type);

    return true;
}

/**
 * @brief Decode ACP Frame from COBS Encoding
 *
 * @param frame_in Encoded frame buffer
 * @param frame_len Encoded frame length
 * @param header_out Output: decoded frame header
 * @param payload_out Output buffer for payload (can be NULL if not needed)
 * @param payload_len_max Maximum payload buffer size
 * @param payload_len_out Output: actual payload length
 * @param auth_tag_out Output buffer for auth tag (can be NULL if not needed)
 * @return true if decoding successful, false on error
 */
bool acp_framer_decode(const uint8_t *frame_in,
                       uint16_t frame_len,
                       acp_frame_header_t *header_out,
                       uint8_t *payload_out,
                       uint16_t payload_len_max,
                       uint16_t *payload_len_out,
                       uint8_t *auth_tag_out)
{

    if (!frame_in || !header_out || !payload_len_out)
    {
        ACP_LOG_ERROR("Decode: Invalid parameters");
        return false;
    }

    // COBS decode the frame
    uint8_t raw_frame[ACP_MAX_FRAME_SIZE];
    uint16_t raw_len = cobs_decode(frame_in, frame_len, raw_frame, sizeof(raw_frame));
    if (raw_len == 0)
    {
        ACP_LOG_ERROR("Decode: COBS decoding failed");
        return false;
    }

    // Minimum frame size check
    uint16_t min_frame_size = sizeof(acp_frame_header_t) + ACP_CRC16_SIZE;
    if (raw_len < min_frame_size)
    {
        ACP_LOG_ERROR("Decode: Frame too small (%u < %u)", raw_len, min_frame_size);
        return false;
    }

    // Extract and validate header
    memcpy(header_out, raw_frame, sizeof(acp_frame_header_t));
    if (!validate_frame_header(header_out))
    {
        ACP_LOG_ERROR("Decode: Header validation failed");
        return false;
    }

    // Extract payload length and validate frame size
    uint16_t payload_len = ntohs(header_out->length);
    bool has_auth = ACP_HAS_AUTH(header_out->flags);

    uint16_t expected_frame_size = sizeof(acp_frame_header_t) + payload_len;
    if (has_auth)
    {
        expected_frame_size += ACP_AUTH_TAG_SIZE;
    }
    expected_frame_size += ACP_CRC16_SIZE;

    if (raw_len != expected_frame_size)
    {
        ACP_LOG_ERROR("Decode: Frame size mismatch (%u != %u)", raw_len, expected_frame_size);
        return false;
    }

    // Verify CRC
    uint16_t crc_offset = raw_len - ACP_CRC16_SIZE;
    uint16_t frame_crc = (raw_frame[crc_offset] << 8) | raw_frame[crc_offset + 1];
    uint16_t calc_crc = crc16_ccitt_calculate(raw_frame, crc_offset, 0xFFFF);

    if (frame_crc != calc_crc)
    {
        ACP_LOG_ERROR("Decode: CRC mismatch (0x%04X != 0x%04X)", frame_crc, calc_crc);
        return false;
    }

    // Extract payload
    *payload_len_out = payload_len;
    if (payload_len > 0)
    {
        if (payload_out)
        {
            if (payload_len > payload_len_max)
            {
                ACP_LOG_ERROR("Decode: Payload buffer too small (%u > %u)", payload_len, payload_len_max);
                return false;
            }
            memcpy(payload_out, raw_frame + sizeof(acp_frame_header_t), payload_len);
        }
    }

    // Extract authentication tag
    if (has_auth && auth_tag_out)
    {
        uint16_t auth_offset = sizeof(acp_frame_header_t) + payload_len;
        memcpy(auth_tag_out, raw_frame + auth_offset, ACP_AUTH_TAG_SIZE);
    }

    ACP_LOG_DEBUG("Decoded frame: len=%u, type=0x%02X, payload=%u, auth=%s",
                  raw_len, header_out->msg_type, payload_len, has_auth ? "yes" : "no");

    return true;
}

/**
 * @brief Validate Frame CRC
 *
 * @param frame_data Raw frame data (after COBS decoding)
 * @param frame_len Frame length including CRC
 * @return true if CRC is valid, false otherwise
 */
bool acp_framer_validate_crc(const uint8_t *frame_data, uint16_t frame_len)
{
    if (!frame_data || frame_len < ACP_CRC16_SIZE)
    {
        return false;
    }

    uint16_t crc_offset = frame_len - ACP_CRC16_SIZE;
    uint16_t frame_crc = (frame_data[crc_offset] << 8) | frame_data[crc_offset + 1];
    uint16_t calc_crc = crc16_ccitt_calculate(frame_data, crc_offset, 0xFFFF);

    return (frame_crc == calc_crc);
}

/**
 * @brief Calculate Frame CRC
 *
 * @param frame_data Frame data (without CRC)
 * @param frame_len Frame length (without CRC)
 * @return Calculated CRC-16 value
 */
uint16_t acp_framer_calculate_crc(const uint8_t *frame_data, uint16_t frame_len)
{
    if (!frame_data)
    {
        return 0;
    }

    return crc16_ccitt_calculate(frame_data, frame_len, 0xFFFF);
}

/**
 * @brief Get Frame Overhead Size
 *
 * Calculates the total overhead bytes added by ACP framing.
 *
 * @param payload_len Payload length
 * @param has_auth Whether authentication tag is present
 * @return Total overhead bytes (header + auth + crc + cobs)
 */
uint16_t acp_framer_get_overhead(uint16_t payload_len, bool has_auth)
{
    uint16_t overhead = sizeof(acp_frame_header_t) + ACP_CRC16_SIZE;

    if (has_auth)
    {
        overhead += ACP_AUTH_TAG_SIZE;
    }

    // COBS overhead: 1 byte per 254 payload bytes + 2 bytes (code + delimiter)
    uint16_t raw_frame_len = overhead + payload_len;
    uint16_t cobs_overhead = (raw_frame_len / 254) + 2;

    return overhead + cobs_overhead;
}