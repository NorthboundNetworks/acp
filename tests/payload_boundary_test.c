/**
 * @file payload_boundary_test.c
 * @brief Payload boundary condition tests for ACP
 *
 * Tests various payload sizes including edge cases at boundaries
 * (0, 1, 1023, 1024 bytes) and validates proper error handling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "acp_protocol.h"

/* Test payload sizes */
static const size_t test_sizes[] = {
    0,    /* Empty payload */
    1,    /* Minimum non-empty payload */
    2,    /* Small payload */
    255,  /* Single-byte length boundary */
    256,  /* Just over byte boundary */
    512,  /* Medium payload */
    1022, /* Near max minus 2 */
    1023, /* Near max minus 1 */
    1024, /* At max boundary */
    1025, /* Over max (should fail) */
    1500, /* Way over max (should fail) */
    2048, /* Much larger (should fail) */
    65535 /* Very large (should fail) */
};

/* Generate test pattern for payload */
static void fill_test_pattern(uint8_t *buffer, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        buffer[i] = (uint8_t)(i & 0xFF);
    }
}

/* Test frame creation with various payload sizes */
static int test_frame_creation_boundaries(void)
{
    printf("\nTest 1: Frame Creation Payload Boundaries\n");
    printf("===========================================\n");

    int tests_passed = 0;
    int total_tests = sizeof(test_sizes) / sizeof(test_sizes[0]);

    for (size_t i = 0; i < sizeof(test_sizes) / sizeof(test_sizes[0]); i++)
    {
        size_t payload_size = test_sizes[i];
        printf("Testing payload size: %zu bytes... ", payload_size);

        /* Allocate payload buffer */
        uint8_t *payload = NULL;
        if (payload_size > 0)
        {
            payload = malloc(payload_size);
            if (!payload && payload_size > 0)
            {
                printf("✗ Failed to allocate test payload\n");
                continue;
            }
            fill_test_pattern(payload, payload_size);
        }

        /* Try to create frame */
        acp_frame_t frame;
        int result = acp_frame_create_telemetry(&frame, ACP_FRAME_TYPE_TELEMETRY, 42,
                                                payload, payload_size);

        /* Expected results based on size */
        bool should_succeed = (payload_size <= ACP_MAX_PAYLOAD_SIZE);

        if (should_succeed)
        {
            if (result == ACP_OK)
            {
                if (frame.length == payload_size)
                {
                    printf("✓ PASS (length=%u)\n", frame.length);
                    tests_passed++;
                }
                else
                {
                    printf("✗ FAIL - length mismatch: expected %zu, got %u\n",
                           payload_size, frame.length);
                }
            }
            else
            {
                printf("✗ FAIL - creation failed with error %d\n", result);
            }
        }
        else
        {
            if (result != ACP_OK)
            {
                printf("✓ PASS (correctly rejected, error=%d)\n", result);
                tests_passed++;
            }
            else
            {
                printf("✗ FAIL - should have been rejected\n");
            }
        }

        if (payload)
        {
            free(payload);
        }
    }

    printf("\nFrame creation boundary tests: %d/%d passed\n", tests_passed, total_tests);
    return tests_passed == total_tests;
}

/* Test encoding with various payload sizes */
static int test_encoding_boundaries(void)
{
    printf("\nTest 2: Encoding Payload Boundaries\n");
    printf("====================================\n");

    int tests_passed = 0;
    int total_tests = 0;

    /* Test valid sizes only (that should encode successfully) */
    for (size_t i = 0; i < sizeof(test_sizes) / sizeof(test_sizes[0]); i++)
    {
        size_t payload_size = test_sizes[i];

        /* Skip sizes that are too large for frame creation */
        if (payload_size > ACP_MAX_PAYLOAD_SIZE)
        {
            continue;
        }

        total_tests++;
        printf("Encoding payload size: %zu bytes... ", payload_size);

        /* Create frame */
        uint8_t *payload = NULL;
        if (payload_size > 0)
        {
            payload = malloc(payload_size);
            if (!payload)
            {
                printf("✗ Failed to allocate payload\n");
                continue;
            }
            fill_test_pattern(payload, payload_size);
        }

        acp_frame_t frame;
        int result = acp_frame_create_telemetry(&frame, ACP_FRAME_TYPE_TELEMETRY, 42,
                                                payload, payload_size);
        if (result != ACP_OK)
        {
            printf("✗ Frame creation failed: %d\n", result);
            if (payload)
                free(payload);
            continue;
        }

        /* Encode frame */
        uint8_t buffer[4096]; /* Large buffer for encoding */
        size_t encoded_len;
        result = acp_frame_encode(&frame, buffer, sizeof(buffer), &encoded_len);

        if (result == ACP_OK)
        {
            printf("✓ PASS (encoded to %zu bytes)\n", encoded_len);
            tests_passed++;
        }
        else
        {
            printf("✗ FAIL - encoding failed with error %d\n", result);
        }

        if (payload)
        {
            free(payload);
        }
    }

    printf("\nEncoding boundary tests: %d/%d passed\n", tests_passed, total_tests);
    return tests_passed == total_tests;
}

/* Test round-trip encode/decode with various sizes */
static int test_roundtrip_boundaries(void)
{
    printf("\nTest 3: Round-trip Encode/Decode Boundaries\n");
    printf("============================================\n");

    int tests_passed = 0;
    int total_tests = 0;

    /* Test reasonable sizes for round-trip */
    size_t roundtrip_sizes[] = {0, 1, 2, 10, 100, 255, 256, 512, 1023, 1024};

    for (size_t i = 0; i < sizeof(roundtrip_sizes) / sizeof(roundtrip_sizes[0]); i++)
    {
        size_t payload_size = roundtrip_sizes[i];

        /* Skip if too large */
        if (payload_size > ACP_MAX_PAYLOAD_SIZE)
        {
            continue;
        }

        total_tests++;
        printf("Round-trip size %zu bytes... ", payload_size);

        /* Create original payload */
        uint8_t *orig_payload = NULL;
        if (payload_size > 0)
        {
            orig_payload = malloc(payload_size);
            if (!orig_payload)
            {
                printf("✗ Allocation failed\n");
                continue;
            }
            fill_test_pattern(orig_payload, payload_size);
        }

        /* Create frame */
        acp_frame_t orig_frame;
        int result = acp_frame_create_telemetry(&orig_frame, ACP_FRAME_TYPE_TELEMETRY, 123,
                                                orig_payload, payload_size);
        if (result != ACP_OK)
        {
            printf("✗ Frame creation failed\n");
            if (orig_payload)
                free(orig_payload);
            continue;
        }

        /* Encode */
        uint8_t buffer[4096];
        size_t encoded_len;
        result = acp_frame_encode(&orig_frame, buffer, sizeof(buffer), &encoded_len);
        if (result != ACP_OK)
        {
            printf("✗ Encoding failed\n");
            if (orig_payload)
                free(orig_payload);
            continue;
        }

        /* Decode */
        acp_frame_t decoded_frame;
        size_t consumed;
        result = acp_frame_decode(buffer, encoded_len, &decoded_frame, &consumed);
        if (result != ACP_OK)
        {
            printf("✗ Decoding failed\n");
            if (orig_payload)
                free(orig_payload);
            continue;
        }

        /* Verify round-trip integrity */
        bool payload_match = true;
        if (decoded_frame.length != payload_size)
        {
            payload_match = false;
        }
        else if (payload_size > 0)
        {
            payload_match = (memcmp(orig_payload, decoded_frame.payload, payload_size) == 0);
        }

        if (payload_match && decoded_frame.type == ACP_FRAME_TYPE_TELEMETRY)
        {
            printf("✓ PASS\n");
            tests_passed++;
        }
        else
        {
            printf("✗ FAIL - data mismatch (len: %u->%u, type: %u->%u)\n",
                   (unsigned)payload_size, decoded_frame.length,
                   ACP_FRAME_TYPE_TELEMETRY, decoded_frame.type);
        }

        if (orig_payload)
        {
            free(orig_payload);
        }
    }

    printf("\nRound-trip boundary tests: %d/%d passed\n", tests_passed, total_tests);
    return tests_passed == total_tests;
}

/* Test buffer size limits during encoding */
static int test_buffer_size_limits(void)
{
    printf("\nTest 4: Output Buffer Size Limits\n");
    printf("==================================\n");

    int tests_passed = 0;

    /* Create a frame with reasonable payload */
    uint8_t payload[100];
    fill_test_pattern(payload, sizeof(payload));

    acp_frame_t frame;
    int result = acp_frame_create_telemetry(&frame, ACP_FRAME_TYPE_TELEMETRY, 42,
                                            payload, sizeof(payload));
    if (result != ACP_OK)
    {
        printf("✗ Test setup failed\n");
        return 0;
    }

    /* Test with insufficient buffer sizes */
    size_t buffer_sizes[] = {0, 1, 5, 10, 50}; /* Too small buffers */

    for (size_t i = 0; i < sizeof(buffer_sizes) / sizeof(buffer_sizes[0]); i++)
    {
        size_t buffer_size = buffer_sizes[i];
        printf("Testing buffer size %zu bytes... ", buffer_size);

        uint8_t *small_buffer = malloc(buffer_size);
        if (!small_buffer && buffer_size > 0)
        {
            printf("✗ Allocation failed\n");
            continue;
        }

        size_t encoded_len;
        result = acp_frame_encode(&frame, small_buffer, buffer_size, &encoded_len);

        /* Should fail with buffer too small error */
        if (result == ACP_ERR_BUFFER_TOO_SMALL)
        {
            printf("✓ PASS (correctly rejected)\n");
            tests_passed++;
        }
        else if (result == ACP_OK)
        {
            printf("✗ FAIL - should have failed with small buffer\n");
        }
        else
        {
            printf("✗ FAIL - unexpected error %d\n", result);
        }

        if (small_buffer)
        {
            free(small_buffer);
        }
    }

    /* Test with adequate buffer */
    printf("Testing adequate buffer size... ");
    uint8_t adequate_buffer[1024];
    size_t encoded_len;
    result = acp_frame_encode(&frame, adequate_buffer, sizeof(adequate_buffer), &encoded_len);

    if (result == ACP_OK)
    {
        printf("✓ PASS (encoded %zu bytes)\n", encoded_len);
        tests_passed++;
    }
    else
    {
        printf("✗ FAIL - encoding failed: %d\n", result);
    }

    printf("\nBuffer size limit tests: %d/6 passed\n", tests_passed);
    return tests_passed == 6;
}

/* Main test runner */
int main(void)
{
    printf("ACP Payload Boundary Tests\n");
    printf("==========================\n");

    /* Initialize ACP */
    if (acp_init() != ACP_OK)
    {
        printf("Failed to initialize ACP\n");
        return 1;
    }

    printf("Testing payload size boundaries and edge cases...\n");
    printf("ACP_MAX_PAYLOAD_SIZE = %d bytes\n", ACP_MAX_PAYLOAD_SIZE);

    int tests_passed = 0;
    int total_tests = 4;

    /* Run all boundary tests */
    if (test_frame_creation_boundaries())
        tests_passed++;
    if (test_encoding_boundaries())
        tests_passed++;
    if (test_roundtrip_boundaries())
        tests_passed++;
    if (test_buffer_size_limits())
        tests_passed++;

    /* Cleanup */
    acp_cleanup();

    /* Results */
    printf("\n==========================\n");
    printf("Payload Boundary Test Results: %d/%d passed\n", tests_passed, total_tests);

    if (tests_passed == total_tests)
    {
        printf("✅ All payload boundary tests PASSED\n");
        printf("   Frame creation, encoding, and decoding work correctly\n");
        printf("   at all payload size boundaries including edge cases\n");
        return 0;
    }
    else
    {
        printf("❌ Some payload boundary tests FAILED\n");
        printf("   Check payload size handling implementation\n");
        return 1;
    }
}