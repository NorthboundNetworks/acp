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
 * @file mock_serial.c
 * @brief Mock serial transport example for ACP frame streaming
 *
 * This example demonstrates:
 * 1. Streaming frame decode from a simulated serial port
 * 2. COBS framing boundary detection
 * 3. Multiple frame handling in a continuous stream
 * 4. Error handling for malformed frames
 */

#include "acp_protocol.h"
#include "acp_errors.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

/**
 * @brief Mock serial port state
 */
typedef struct
{
    uint8_t *buffer;    /**< Circular buffer */
    size_t buffer_size; /**< Total buffer size */
    size_t write_pos;   /**< Write position */
    size_t read_pos;    /**< Read position */
    size_t available;   /**< Bytes available to read */
} mock_serial_t;

/**
 * @brief Initialize mock serial port
 */
static void mock_serial_init(mock_serial_t *serial, size_t buffer_size)
{
    serial->buffer = malloc(buffer_size);
    serial->buffer_size = buffer_size;
    serial->write_pos = 0;
    serial->read_pos = 0;
    serial->available = 0;
}

/**
 * @brief Cleanup mock serial port
 */
static void mock_serial_cleanup(mock_serial_t *serial)
{
    free(serial->buffer);
    memset(serial, 0, sizeof(*serial));
}

/**
 * @brief Write data to mock serial port
 */
static size_t mock_serial_write(mock_serial_t *serial, const uint8_t *data, size_t len)
{
    size_t written = 0;

    while (written < len && serial->available < serial->buffer_size)
    {
        serial->buffer[serial->write_pos] = data[written];
        serial->write_pos = (serial->write_pos + 1) % serial->buffer_size;
        serial->available++;
        written++;
    }

    return written;
}

/**
 * @brief Read data from mock serial port
 */
static size_t mock_serial_read(mock_serial_t *serial, uint8_t *data, size_t len)
{
    size_t read_count = 0;

    while (read_count < len && serial->available > 0)
    {
        data[read_count] = serial->buffer[serial->read_pos];
        serial->read_pos = (serial->read_pos + 1) % serial->buffer_size;
        serial->available--;
        read_count++;
    }

    return read_count;
}

/**
 * @brief Get available bytes in mock serial port
 */
static size_t mock_serial_available(const mock_serial_t *serial)
{
    return serial->available;
}

/**
 * @brief Create sample frames and send to mock serial
 */
static int send_sample_frames(mock_serial_t *serial)
{
    printf("Sending sample frames...\n");

    // Sample payload data
    uint8_t telemetry_payload[] = {0x12, 0x34, 0x56, 0x78, 0xAB, 0xCD};
    uint8_t command_payload[] = {0xFF, 0xEE, 0xDD, 0xCC};

    // Encode first frame (telemetry)
    uint8_t frame1_buffer[128];
    size_t frame1_len = sizeof(frame1_buffer);

    acp_result_t result = acp_encode_frame(
        ACP_FRAME_TYPE_TELEMETRY,
        0, // No authentication
        telemetry_payload,
        sizeof(telemetry_payload),
        NULL,
        frame1_buffer,
        &frame1_len);

    if (result != ACP_OK)
    {
        printf("Frame 1 encode failed: %d\n", result);
        return -1;
    }

    // Encode second frame (command)
    uint8_t frame2_buffer[128];
    size_t frame2_len = sizeof(frame2_buffer);

    result = acp_encode_frame(
        ACP_FRAME_TYPE_COMMAND,
        0, // No authentication for this test
        command_payload,
        sizeof(command_payload),
        NULL,
        frame2_buffer,
        &frame2_len);

    if (result != ACP_OK)
    {
        printf("Frame 2 encode failed: %d\n", result);
        return -1;
    }

    // Send frames to mock serial (simulate transmission)
    printf("Sending frame 1 (%zu bytes)...\n", frame1_len);
    size_t sent = mock_serial_write(serial, frame1_buffer, frame1_len);
    if (sent != frame1_len)
    {
        printf("Warning: Only sent %zu/%zu bytes of frame 1\n", sent, frame1_len);
    }

    // Add some inter-frame delay simulation
    usleep(1000); // 1ms delay

    printf("Sending frame 2 (%zu bytes)...\n", frame2_len);
    sent = mock_serial_write(serial, frame2_buffer, frame2_len);
    if (sent != frame2_len)
    {
        printf("Warning: Only sent %zu/%zu bytes of frame 2\n", sent, frame2_len);
    }

    // Simulate some noise/corrupt data
    uint8_t noise[] = {0x00, 0x55, 0xAA, 0x00};
    printf("Sending noise (%zu bytes)...\n", sizeof(noise));
    mock_serial_write(serial, noise, sizeof(noise));

    printf("Total data in serial buffer: %zu bytes\n", mock_serial_available(serial));

    return 0;
}

/**
 * @brief Receive and decode frames from mock serial
 */
static int receive_frames(mock_serial_t *serial)
{
    printf("\nReceiving frames...\n");

    uint8_t receive_buffer[512];
    size_t buffer_pos = 0;
    int frame_count = 0;

    // Stream processing loop
    while (mock_serial_available(serial) > 0)
    {
        // Read more data from serial port
        size_t available_space = sizeof(receive_buffer) - buffer_pos;
        if (available_space == 0)
        {
            printf("Receive buffer full, resetting\n");
            buffer_pos = 0;
            available_space = sizeof(receive_buffer);
        }

        size_t bytes_read = mock_serial_read(serial,
                                             &receive_buffer[buffer_pos],
                                             available_space);

        if (bytes_read == 0)
        {
            break; // No more data
        }

        buffer_pos += bytes_read;

        printf("Read %zu bytes, buffer now has %zu bytes\n", bytes_read, buffer_pos);

        // Try to decode frames from current buffer
        size_t processed = 0;

        while (processed < buffer_pos)
        {
            acp_frame_t frame;
            size_t consumed = 0;

            acp_result_t result = acp_decode_frame(
                &receive_buffer[processed],
                buffer_pos - processed,
                &frame,
                &consumed,
                NULL // No authentication for this test
            );

            if (result == ACP_OK)
            {
                // Successfully decoded a frame
                frame_count++;
                printf("\n✓ Frame %d decoded (%zu bytes consumed):\n",
                       frame_count, consumed);
                printf("  Type: %u, Flags: 0x%02x, Length: %u\n",
                       frame.type, frame.flags, frame.length);
                printf("  Payload: ");
                for (size_t i = 0; i < frame.length; i++)
                {
                    printf("%02x ", frame.payload[i]);
                }
                printf("\n");

                processed += consumed;
            }
            else if (result == ACP_ERR_NEED_MORE_DATA)
            {
                // Need more data to complete frame
                printf("Need more data (have %zu bytes)\n", buffer_pos - processed);
                break;
            }
            else
            {
                // Frame decode error - skip one byte and try again
                printf("✗ Frame decode error %d, skipping byte 0x%02x\n",
                       result, receive_buffer[processed]);
                processed++;
            }
        }

        // Move remaining data to start of buffer
        if (processed > 0 && processed < buffer_pos)
        {
            memmove(receive_buffer, &receive_buffer[processed], buffer_pos - processed);
            buffer_pos -= processed;
        }
        else if (processed >= buffer_pos)
        {
            buffer_pos = 0;
        }
    }

    printf("\nTotal frames received: %d\n", frame_count);

    if (buffer_pos > 0)
    {
        printf("Remaining buffer data (%zu bytes): ", buffer_pos);
        for (size_t i = 0; i < buffer_pos && i < 16; i++)
        {
            printf("%02x ", receive_buffer[i]);
        }
        printf("\n");
    }

    return frame_count;
}

/**
 * @brief Test frame boundary detection with partial reads
 */
static int test_partial_reads(mock_serial_t *serial)
{
    printf("\n=== Partial Read Test ===\n");

    // Send a test frame
    uint8_t test_payload[] = "Hello ACP World!";
    uint8_t frame_buffer[128];
    size_t frame_len = sizeof(frame_buffer);

    acp_result_t result = acp_encode_frame(
        ACP_FRAME_TYPE_SYSTEM,
        0,
        test_payload,
        sizeof(test_payload) - 1, // Exclude null terminator
        NULL,
        frame_buffer,
        &frame_len);

    if (result != ACP_OK)
    {
        printf("Test frame encode failed: %d\n", result);
        return -1;
    }

    printf("Sending test frame in chunks...\n");

    // Send frame in small chunks to test partial read handling
    for (size_t i = 0; i < frame_len; i += 3)
    {
        size_t chunk_size = (i + 3 < frame_len) ? 3 : frame_len - i;
        mock_serial_write(serial, &frame_buffer[i], chunk_size);
        printf("Sent chunk %zu: %zu bytes\n", i / 3 + 1, chunk_size);

        // Try to decode with each partial chunk
        uint8_t read_buffer[128];
        size_t total_read = 0;

        while (mock_serial_available(serial) > 0 && total_read < sizeof(read_buffer))
        {
            size_t read_this_time = mock_serial_read(serial,
                                                     &read_buffer[total_read], 1);
            total_read += read_this_time;
        }

        acp_frame_t frame;
        size_t consumed = 0;
        result = acp_decode_frame(read_buffer, total_read, &frame, &consumed, NULL);

        if (result == ACP_OK)
        {
            printf("✓ Complete frame decoded on chunk %zu\n", i / 3 + 1);
            break;
        }
        else if (result == ACP_ERR_NEED_MORE_DATA)
        {
            printf("Partial frame, need more data\n");
            // Put data back for next iteration
            mock_serial_write(serial, read_buffer, total_read);
        }
        else
        {
            printf("Decode error: %d\n", result);
        }
    }

    return 0;
}

/**
 * @brief Main mock serial example
 */
int main(void)
{
    printf("Mock Serial Transport Example\n");
    printf("=============================\n");

    // Initialize ACP library
    acp_result_t result = acp_init();
    if (result != ACP_OK)
    {
        printf("ACP initialization failed: %d\n", result);
        return 1;
    }

    // Initialize mock serial port with 1KB buffer
    mock_serial_t serial;
    mock_serial_init(&serial, 1024);

    printf("Mock serial port initialized (1024 byte buffer)\n");

    // Test normal frame streaming
    if (send_sample_frames(&serial) != 0)
    {
        printf("Send frames failed\n");
        goto cleanup;
    }

    int frames_received = receive_frames(&serial);
    if (frames_received < 2)
    {
        printf("Expected at least 2 frames, got %d\n", frames_received);
        goto cleanup;
    }

    // Test partial read handling
    if (test_partial_reads(&serial) != 0)
    {
        printf("Partial read test failed\n");
        goto cleanup;
    }

    printf("\n✓ Mock serial example completed successfully\n");

cleanup:
    mock_serial_cleanup(&serial);
    acp_cleanup();

    return 0;
}
