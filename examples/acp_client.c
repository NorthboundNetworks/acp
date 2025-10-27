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
 * @file acp_client.c
 * @brief Example ACP client demonstrating frame encoding, authentication, and decoding
 *
 * This example shows how to:
 * 1. Initialize an ACP session with authentication
 * 2. Encode telemetry and command frames
 * 3. Decode received frames with integrity/authentication validation
 * 4. Handle errors and edge cases
 */

#include "acp_protocol.h"
#include "acp_errors.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Example telemetry payload structure
 */
typedef struct
{
    uint16_t voltage_mv;   /**< Battery voltage in millivolts */
    uint16_t current_ma;   /**< Current draw in milliamps */
    int16_t temperature_c; /**< Temperature in Celsius */
    uint8_t status_flags;  /**< System status bitmap */
} telemetry_data_t;

/**
 * @brief Example command payload structure
 */
typedef struct
{
    uint8_t command_id;    /**< Command identifier */
    uint8_t target_system; /**< Target system ID */
    uint16_t parameter;    /**< Command parameter */
} command_data_t;

/**
 * @brief Print frame contents for debugging
 */
static void print_frame_info(const acp_frame_t *frame)
{
    printf("Frame: type=%u, flags=0x%02x, len=%u, seq=%u\n",
           frame->type, frame->flags, frame->length, frame->sequence);

    printf("Payload (%u bytes): ", frame->length);
    for (size_t i = 0; i < frame->length && i < 16; i++)
    {
        printf("%02x ", frame->payload[i]);
    }
    if (frame->length > 16)
    {
        printf("...");
    }
    printf("\n");
}

/**
 * @brief Demonstrate telemetry frame encoding/decoding (unauthenticated)
 */
static int demo_telemetry_frame(void)
{
    printf("\n=== Telemetry Frame Demo ===\n");

    // Sample telemetry data
    telemetry_data_t telemetry = {
        .voltage_mv = 12500, // 12.5V
        .current_ma = 2300,  // 2.3A
        .temperature_c = 45, // 45°C
        .status_flags = 0x07 // Systems online
    };

    // Encode frame (no authentication for telemetry)
    uint8_t encoded_buffer[ACP_MAX_FRAME_SIZE];
    size_t encoded_len = sizeof(encoded_buffer);

    acp_result_t result = acp_encode_frame(
        ACP_FRAME_TYPE_TELEMETRY,
        0, // No auth flags for telemetry
        (uint8_t *)&telemetry,
        sizeof(telemetry),
        NULL, // No session (unauthenticated)
        encoded_buffer,
        &encoded_len);

    if (result != ACP_OK)
    {
        printf("Encode failed: %d\n", result);
        return -1;
    }

    printf("Encoded %zu bytes\n", encoded_len);

    // Decode frame
    acp_frame_t decoded_frame;
    size_t consumed = 0;

    result = acp_decode_frame(
        encoded_buffer,
        encoded_len,
        &decoded_frame,
        &consumed,
        NULL // No session (unauthenticated)
    );

    if (result != ACP_OK)
    {
        printf("Decode failed: %d\n", result);
        return -1;
    }

    printf("Decoded frame (%zu bytes consumed):\n", consumed);
    print_frame_info(&decoded_frame);

    // Verify payload integrity
    if (decoded_frame.length == sizeof(telemetry) &&
        memcmp(decoded_frame.payload, &telemetry, sizeof(telemetry)) == 0)
    {
        printf("✓ Telemetry payload verified\n");
    }
    else
    {
        printf("✗ Telemetry payload mismatch\n");
        return -1;
    }

    return 0;
}

/**
 * @brief Demonstrate authenticated command frame
 */
static int demo_authenticated_command(void)
{
    printf("\n=== Authenticated Command Demo ===\n");

    // Initialize session with test key
    acp_session_t session;
    uint8_t test_key[] = "test_key_32_bytes_for_hmac_demo!";
    uint64_t nonce = 0x1234567890ABCDEF;

    acp_result_t result = acp_session_init(&session, 1, test_key, sizeof(test_key), nonce);
    if (result != ACP_OK)
    {
        printf("Session init failed: %d\n", result);
        return -1;
    }

    printf("Session initialized with key_id=1, nonce=0x%016llx\n",
           (unsigned long long)nonce);

    // Sample command data
    command_data_t command = {
        .command_id = 0x42, // Set mode command
        .target_system = 1, // Target system 1
        .parameter = 1000   // Parameter value
    };

    // Encode authenticated command frame
    uint8_t encoded_buffer[ACP_MAX_FRAME_SIZE];
    size_t encoded_len = sizeof(encoded_buffer);

    result = acp_encode_frame(
        ACP_FRAME_TYPE_COMMAND,
        ACP_FLAG_AUTHENTICATED,
        (uint8_t *)&command,
        sizeof(command),
        &session,
        encoded_buffer,
        &encoded_len);

    if (result != ACP_OK)
    {
        printf("Authenticated encode failed: %d\n", result);
        return -1;
    }

    printf("Encoded authenticated frame: %zu bytes\n", encoded_len);

    // Create new session for decoding (simulating receiver)
    acp_session_t rx_session;
    result = acp_session_init(&rx_session, 1, test_key, sizeof(test_key), nonce);
    if (result != ACP_OK)
    {
        printf("RX session init failed: %d\n", result);
        return -1;
    }

    // Decode authenticated frame
    acp_frame_t decoded_frame;
    size_t consumed = 0;

    result = acp_decode_frame(
        encoded_buffer,
        encoded_len,
        &decoded_frame,
        &consumed,
        &rx_session);

    if (result != ACP_OK)
    {
        printf("Authenticated decode failed: %d\n", result);
        return -1;
    }

    printf("✓ Authentication verified\n");
    print_frame_info(&decoded_frame);

    // Test replay protection - try to decode same frame again
    printf("\nTesting replay protection...\n");
    result = acp_decode_frame(
        encoded_buffer,
        encoded_len,
        &decoded_frame,
        &consumed,
        &rx_session);

    if (result == ACP_ERR_REPLAY)
    {
        printf("✓ Replay protection working\n");
    }
    else
    {
        printf("✗ Replay protection failed: %d\n", result);
        return -1;
    }

    return 0;
}

/**
 * @brief Main example program
 */
int main(void)
{
    printf("ACP Client Example\n");
    printf("==================\n");

    // Initialize ACP library
    acp_result_t result = acp_init();
    if (result != ACP_OK)
    {
        printf("ACP initialization failed: %d\n", result);
        return 1;
    }

    printf("ACP library initialized\n");

    // Run demos
    if (demo_telemetry_frame() != 0)
    {
        printf("Telemetry demo failed\n");
        return 1;
    }

    if (demo_authenticated_command() != 0)
    {
        printf("Authentication demo failed\n");
        return 1;
    }

    printf("\n✓ All demos completed successfully\n");

    // Cleanup
    acp_cleanup();

    return 0;
}
