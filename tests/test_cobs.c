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
 * @file test_cobs.c
 * @brief Simple test for COBS implementation
 */

#include "acp_cobs.h"
#include "acp_errors.h"
#include <stdio.h>
#include <string.h>

int test_cobs_basic(void)
{
    /* Test case: "Hello\0World" */
    const uint8_t input[] = {'H', 'e', 'l', 'l', 'o', 0, 'W', 'o', 'r', 'l', 'd'};
    uint8_t encoded[20];
    uint8_t decoded[20];
    size_t encoded_len, decoded_len;

    printf("Testing COBS encode/decode...\n");

    /* Encode */
    int result = acp_cobs_encode(input, sizeof(input), encoded, sizeof(encoded), &encoded_len);
    if (result != ACP_OK)
    {
        printf("COBS encode failed: %d\n", result);
        return -1;
    }

    printf("Input:   ");
    for (size_t i = 0; i < sizeof(input); i++)
    {
        printf("%02X ", input[i]);
    }
    printf("\n");

    printf("Encoded: ");
    for (size_t i = 0; i < encoded_len; i++)
    {
        printf("%02X ", encoded[i]);
    }
    printf("(length: %zu)\n", encoded_len);

    /* Decode */
    result = acp_cobs_decode(encoded, encoded_len, decoded, sizeof(decoded), &decoded_len);
    if (result != ACP_OK)
    {
        printf("COBS decode failed: %d\n", result);
        return -1;
    }

    printf("Decoded: ");
    for (size_t i = 0; i < decoded_len; i++)
    {
        printf("%02X ", decoded[i]);
    }
    printf("(length: %zu)\n", decoded_len);

    /* Verify */
    if (decoded_len != sizeof(input) || memcmp(input, decoded, sizeof(input)) != 0)
    {
        printf("COBS test FAILED - data mismatch\n");
        return -1;
    }

    printf("COBS test PASSED\n");
    return 0;
}

int test_cobs_streaming(void)
{
    printf("\nTesting COBS streaming decoder...\n");

    /* Create test frame: encode "Test" */
    const uint8_t test_data[] = "Test";
    uint8_t encoded[10];
    size_t encoded_len;

    int result = acp_cobs_encode(test_data, 4, encoded, sizeof(encoded), &encoded_len);
    if (result != ACP_OK)
    {
        printf("Failed to encode test data\n");
        return -1;
    }

    /* Create framed data (add delimiters) */
    uint8_t framed[12];
    framed[0] = ACP_COBS_DELIMITER; /* Frame start */
    memcpy(framed + 1, encoded, encoded_len);
    framed[encoded_len + 1] = ACP_COBS_DELIMITER; /* Frame end */

    /* Setup streaming decoder */
    uint8_t decoder_buffer[10];
    acp_cobs_decoder_t decoder;
    result = acp_cobs_decoder_init(&decoder, decoder_buffer, sizeof(decoder_buffer));
    if (result != ACP_OK)
    {
        printf("Failed to initialize decoder\n");
        return -1;
    }

    /* Feed bytes one by one */
    for (size_t i = 0; i < encoded_len + 2; i++)
    {
        int feed_result = acp_cobs_decoder_feed_byte(&decoder, framed[i]);
        if (feed_result < 0)
        {
            printf("Decoder error at byte %zu: %d\n", i, feed_result);
            return -1;
        }
        if (feed_result == 1)
        {
            printf("Frame complete at byte %zu\n", i);
            break;
        }
    }

    /* Get decoded frame */
    uint8_t decoded[10];
    size_t decoded_len;
    result = acp_cobs_decoder_get_frame(&decoder, decoded, sizeof(decoded), &decoded_len);
    if (result != ACP_OK)
    {
        printf("Failed to get decoded frame: %d\n", result);
        return -1;
    }

    /* Verify */
    if (decoded_len != 4 || memcmp(test_data, decoded, 4) != 0)
    {
        printf("Streaming decoder test FAILED\n");
        return -1;
    }

    printf("Streaming decoder test PASSED\n");
    return 0;
}

int main(void)
{
    printf("ACP COBS Implementation Test\n");
    printf("============================\n");

    if (test_cobs_basic() != 0)
    {
        return 1;
    }

    if (test_cobs_streaming() != 0)
    {
        return 1;
    }

    printf("\nAll COBS tests PASSED!\n");
    return 0;
}
