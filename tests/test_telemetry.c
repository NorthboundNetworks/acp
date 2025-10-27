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
 * @file test_telemetry.c
 * @brief Test User Story 1 - Telemetry Frame Encoding/Decoding
 *
 * This test validates the complete telemetry frame processing chain:
 * 1. Create a telemetry frame with payload
 * 2. Encode it with COBS + CRC16
 * 3. Decode it back and verify integrity
 */

#include "acp_protocol.h"
#include "acp_errors.h"
#include <stdio.h>
#include <string.h>

/* Sample telemetry payload structure */
typedef struct
{
    uint32_t timestamp;
    int16_t temperature;
    int16_t voltage;
    uint8_t status;
    uint8_t reserved;
} telemetry_data_t;

int test_telemetry_frame_processing(void)
{
    printf("=== Testing ACP Telemetry Frame Processing ===\n");

    /* Create sample telemetry data */
    telemetry_data_t telem_data = {
        .timestamp = 1234567890,
        .temperature = 2150, /* 21.5°C */
        .voltage = 1205,     /* 12.05V */
        .status = 0x01,      /* Online */
        .reserved = 0};

    printf("Sample telemetry data:\n");
    printf("  Timestamp:   %u\n", telem_data.timestamp);
    printf("  Temperature: %d (%.1f°C)\n", telem_data.temperature, telem_data.temperature / 100.0);
    printf("  Voltage:     %d (%.2fV)\n", telem_data.voltage, telem_data.voltage / 100.0);
    printf("  Status:      0x%02X\n", telem_data.status);

    /* Step 1: Create ACP frame */
    acp_frame_t frame;
    int result = acp_frame_create_telemetry(&frame, ACP_FRAME_TYPE_TELEMETRY, 42,
                                            &telem_data, sizeof(telem_data));
    if (result != ACP_OK)
    {
        printf("ERROR: Failed to create telemetry frame: %d\n", result);
        return -1;
    }

    printf("\nCreated ACP frame:\n");
    printf("  Version:  0x%02X\n", frame.version);
    printf("  Type:     0x%02X\n", frame.type);
    printf("  Flags:    0x%02X\n", frame.flags);
    printf("  Sequence: %u\n", frame.sequence);
    printf("  Length:   %u bytes\n", frame.length);

    /* Step 2: Encode frame */
    uint8_t encoded_buffer[256];
    size_t encoded_len;
    result = acp_frame_encode(&frame, encoded_buffer, sizeof(encoded_buffer), &encoded_len);
    if (result != ACP_OK)
    {
        printf("ERROR: Failed to encode frame: %d\n", result);
        return -1;
    }

    printf("\nEncoded frame (%zu bytes):\n", encoded_len);
    printf("  Raw bytes: ");
    for (size_t i = 0; i < encoded_len && i < 32; i++)
    {
        printf("%02X ", encoded_buffer[i]);
        if ((i + 1) % 16 == 0)
            printf("\n             ");
    }
    if (encoded_len > 32)
        printf("... (%zu more)", encoded_len - 32);
    printf("\n");

    /* Verify frame has delimiters */
    if (encoded_buffer[0] != 0x00 || encoded_buffer[encoded_len - 1] != 0x00)
    {
        printf("ERROR: Frame missing COBS delimiters\n");
        return -1;
    }
    printf("  Frame has proper COBS delimiters: ✓\n");

    /* Step 3: Decode frame */
    acp_frame_t decoded_frame;
    size_t consumed_bytes;
    result = acp_frame_decode(encoded_buffer, encoded_len, &decoded_frame, &consumed_bytes);
    if (result != ACP_OK)
    {
        printf("ERROR: Failed to decode frame: %d\n", result);
        return -1;
    }

    printf("\nDecoded frame:\n");
    printf("  Version:  0x%02X\n", decoded_frame.version);
    printf("  Type:     0x%02X\n", decoded_frame.type);
    printf("  Flags:    0x%02X\n", decoded_frame.flags);
    printf("  Sequence: %u\n", decoded_frame.sequence);
    printf("  Length:   %u bytes\n", decoded_frame.length);
    printf("  Consumed: %zu bytes\n", consumed_bytes);

    /* Step 4: Verify integrity */
    if (decoded_frame.version != frame.version ||
        decoded_frame.type != frame.type ||
        decoded_frame.flags != frame.flags ||
        decoded_frame.sequence != frame.sequence ||
        decoded_frame.length != frame.length)
    {
        printf("ERROR: Frame header mismatch\n");
        return -1;
    }

    if (memcmp(decoded_frame.payload, frame.payload, frame.length) != 0)
    {
        printf("ERROR: Payload data mismatch\n");
        return -1;
    }

    printf("  Frame header verification: ✓\n");
    printf("  Payload integrity check: ✓\n");

    /* Step 5: Verify telemetry data */
    const telemetry_data_t *decoded_telem = (const telemetry_data_t *)decoded_frame.payload;

    printf("\nDecoded telemetry data:\n");
    printf("  Timestamp:   %u\n", decoded_telem->timestamp);
    printf("  Temperature: %d (%.1f°C)\n", decoded_telem->temperature, decoded_telem->temperature / 100.0);
    printf("  Voltage:     %d (%.2fV)\n", decoded_telem->voltage, decoded_telem->voltage / 100.0);
    printf("  Status:      0x%02X\n", decoded_telem->status);

    if (memcmp(&telem_data, decoded_telem, sizeof(telemetry_data_t)) != 0)
    {
        printf("ERROR: Telemetry data corruption\n");
        return -1;
    }

    printf("  Telemetry data integrity: ✓\n");

    printf("\n✅ User Story 1 - Telemetry Frame Processing: SUCCESS\n");
    return 0;
}

int test_frame_size_calculation(void)
{
    printf("\n=== Testing Frame Size Calculations ===\n");

    acp_frame_t frame;
    acp_frame_create_telemetry(&frame, ACP_FRAME_TYPE_TELEMETRY, 1, "Hello", 5);

    size_t predicted_size = acp_frame_encoded_size(&frame);

    uint8_t buffer[256];
    size_t actual_size;
    int result = acp_frame_encode(&frame, buffer, sizeof(buffer), &actual_size);

    if (result != ACP_OK)
    {
        printf("ERROR: Frame encoding failed: %d\n", result);
        return -1;
    }

    printf("  Predicted encoded size: %zu bytes\n", predicted_size);
    printf("  Actual encoded size:    %zu bytes\n", actual_size);

    if (actual_size > predicted_size)
    {
        printf("ERROR: Actual size exceeds prediction\n");
        return -1;
    }

    printf("  Size prediction: ✓\n");
    return 0;
}

int main(void)
{
    printf("ACP Library - User Story 1 Test\n");
    printf("================================\n");

    if (test_telemetry_frame_processing() != 0)
    {
        return 1;
    }

    if (test_frame_size_calculation() != 0)
    {
        return 1;
    }

    printf("\n🎉 All User Story 1 tests PASSED!\n");
    printf("   The ACP library successfully:\n");
    printf("   • Creates telemetry frames\n");
    printf("   • Encodes with COBS + CRC16\n");
    printf("   • Decodes with integrity verification\n");
    printf("   • Preserves data through encode/decode cycle\n");

    return 0;
}
