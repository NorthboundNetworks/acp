/**
 * @file crc_mismatch_test.c
 * @brief CRC validation and corruption detection tests for ACP
 *
 * Tests that corrupt frames are properly rejected and valid frames
 * are accepted. Validates CRC16 integrity checking behavior.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "acp_protocol.h"

/* Test corruption patterns */
typedef struct
{
    const char *description;
    void (*corrupt_func)(uint8_t *buffer, size_t len);
} corruption_pattern_t;

/* Corruption functions */
static void corrupt_single_bit(uint8_t *buffer, size_t len)
{
    if (len > 5)
    {
        buffer[5] ^= 0x01; /* Flip one bit in middle of frame */
    }
}

static void corrupt_single_byte(uint8_t *buffer, size_t len)
{
    if (len > 7)
    {
        buffer[7] = ~buffer[7]; /* Invert entire byte */
    }
}

static void corrupt_multiple_bytes(uint8_t *buffer, size_t len)
{
    if (len > 10)
    {
        for (size_t i = 8; i < len && i < 12; i++)
        {
            buffer[i] ^= 0xFF; /* Corrupt several consecutive bytes */
        }
    }
}

static void corrupt_header_byte(uint8_t *buffer, size_t len)
{
    if (len > 3)
    {
        buffer[3] ^= 0x80; /* Corrupt header field */
    }
}

static void corrupt_payload_start(uint8_t *buffer, size_t len)
{
    if (len > 10)
    {
        buffer[8] = 0x00; /* Zero out payload start */
        buffer[9] = 0xFF; /* Set to different pattern */
    }
}

static void corrupt_payload_end(uint8_t *buffer, size_t len)
{
    if (len > 5)
    {
        buffer[len - 5] ^= 0xAA; /* Corrupt near end before CRC */
    }
}

static void corrupt_crc_field(uint8_t *buffer, size_t len)
{
    if (len > 2)
    {
        /* Corrupt the CRC field itself (last 2 bytes before COBS delimiter) */
        buffer[len - 3] = 0x00;
        buffer[len - 2] = 0x00;
    }
}

static void corrupt_random_pattern(uint8_t *buffer, size_t len)
{
    /* Corrupt multiple random positions */
    if (len > 10)
    {
        buffer[2] ^= 0x42;
        buffer[6] ^= 0x19;
        buffer[len / 2] ^= 0x7D;
    }
}

/* Array of corruption patterns */
static const corruption_pattern_t corruption_patterns[] = {
    {"Single bit flip", corrupt_single_bit},
    {"Single byte invert", corrupt_single_byte},
    {"Multiple consecutive bytes", corrupt_multiple_bytes},
    {"Header corruption", corrupt_header_byte},
    {"Payload start corruption", corrupt_payload_start},
    {"Payload end corruption", corrupt_payload_end},
    {"CRC field corruption", corrupt_crc_field},
    {"Random pattern corruption", corrupt_random_pattern}};

/* Test CRC validation with various corruption patterns */
static int test_corruption_detection(void)
{
    printf("\nTest 1: CRC Corruption Detection\n");
    printf("=================================\n");

    int tests_passed = 0;
    int total_tests = 0;

    /* Create test frame with known payload */
    uint8_t test_payload[] = "Hello, ACP CRC test! This is a longer payload to provide more data for corruption testing.";
    acp_frame_t original_frame;

    int result = acp_frame_create_telemetry(&original_frame, ACP_FRAME_TYPE_TELEMETRY, 42,
                                            test_payload, sizeof(test_payload) - 1);
    if (result != ACP_OK)
    {
        printf("✗ Failed to create test frame: %d\n", result);
        return 0;
    }

    /* Encode the original frame */
    uint8_t original_buffer[512];
    size_t original_len;
    result = acp_frame_encode(&original_frame, original_buffer, sizeof(original_buffer), &original_len);
    if (result != ACP_OK)
    {
        printf("✗ Failed to encode test frame: %d\n", result);
        return 0;
    }

    printf("Original frame encoded to %zu bytes\n", original_len);

    /* Test each corruption pattern */
    for (size_t i = 0; i < sizeof(corruption_patterns) / sizeof(corruption_patterns[0]); i++)
    {
        total_tests++;
        printf("Testing %s... ", corruption_patterns[i].description);

        /* Make a copy of the original encoded frame */
        uint8_t corrupted_buffer[512];
        memcpy(corrupted_buffer, original_buffer, original_len);

        /* Apply corruption */
        corruption_patterns[i].corrupt_func(corrupted_buffer, original_len);

        /* Try to decode the corrupted frame */
        acp_frame_t decoded_frame;
        size_t consumed;
        result = acp_frame_decode(corrupted_buffer, original_len, &decoded_frame, &consumed);

        /* Corrupted frame should be rejected */
        if (result != ACP_OK)
        {
            printf("✓ PASS (correctly rejected, error=%d)\n", result);
            tests_passed++;
        }
        else
        {
            printf("✗ FAIL (corrupted frame was accepted)\n");
        }
    }

    printf("\nCorruption detection tests: %d/%d passed\n", tests_passed, total_tests);
    return tests_passed == total_tests;
}

/* Test that valid frames are still accepted */
static int test_valid_frame_acceptance(void)
{
    printf("\nTest 2: Valid Frame Acceptance\n");
    printf("===============================\n");

    int tests_passed = 0;

    /* Test various valid frame sizes and contents */
    struct
    {
        const char *description;
        const char *payload;
        size_t payload_len;
    } test_cases[] = {
        {"Empty payload", "", 0},
        {"Small payload", "Hi", 2},
        {"ASCII text", "The quick brown fox jumps over the lazy dog", 43},
        {"Binary data", "\x00\x01\x02\x03\xFE\xFF", 6},
        {"Large payload", NULL, 1000} /* Will be filled with pattern */
    };

    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        printf("Testing %s... ", test_cases[i].description);

        /* Prepare payload */
        uint8_t *payload_data = NULL;
        size_t payload_size = test_cases[i].payload_len;

        if (test_cases[i].payload)
        {
            payload_data = (uint8_t *)test_cases[i].payload;
        }
        else if (payload_size > 0)
        {
            /* Generate large test pattern */
            payload_data = malloc(payload_size);
            if (!payload_data)
            {
                printf("✗ FAIL (allocation failed)\n");
                continue;
            }
            for (size_t j = 0; j < payload_size; j++)
            {
                payload_data[j] = (uint8_t)(j % 256);
            }
        }

        /* Create frame */
        acp_frame_t frame;
        int result = acp_frame_create_telemetry(&frame, ACP_FRAME_TYPE_TELEMETRY, i,
                                                payload_data, payload_size);
        if (result != ACP_OK)
        {
            printf("✗ FAIL (frame creation failed: %d)\n", result);
            if (payload_data && !test_cases[i].payload)
                free(payload_data);
            continue;
        }

        /* Encode frame */
        uint8_t buffer[2048];
        size_t encoded_len;
        result = acp_frame_encode(&frame, buffer, sizeof(buffer), &encoded_len);
        if (result != ACP_OK)
        {
            printf("✗ FAIL (encoding failed: %d)\n", result);
            if (payload_data && !test_cases[i].payload)
                free(payload_data);
            continue;
        }

        /* Decode frame back */
        acp_frame_t decoded_frame;
        size_t consumed;
        result = acp_frame_decode(buffer, encoded_len, &decoded_frame, &consumed);

        /* Verify successful decode and data integrity */
        if (result == ACP_OK &&
            decoded_frame.length == payload_size &&
            decoded_frame.type == ACP_FRAME_TYPE_TELEMETRY)
        {

            /* Check payload integrity */
            bool payload_ok = true;
            if (payload_size > 0 && payload_data)
            {
                payload_ok = (memcmp(decoded_frame.payload, payload_data, payload_size) == 0);
            }

            if (payload_ok)
            {
                printf("✓ PASS\n");
                tests_passed++;
            }
            else
            {
                printf("✗ FAIL (payload corrupted)\n");
            }
        }
        else
        {
            printf("✗ FAIL (decode error: %d, len: %u->%u, type: %u->%u)\n",
                   result, (unsigned)payload_size, decoded_frame.length,
                   ACP_FRAME_TYPE_TELEMETRY, decoded_frame.type);
        }

        if (payload_data && !test_cases[i].payload)
        {
            free(payload_data);
        }
    }

    printf("\nValid frame acceptance tests: %d/5 passed\n", tests_passed);
    return tests_passed == 5;
}

/* Test edge case corruption scenarios */
static int test_edge_case_corruptions(void)
{
    printf("\nTest 3: Edge Case Corruptions\n");
    printf("==============================\n");

    int tests_passed = 0;
    int total_tests = 0;

    /* Create test frame */
    uint8_t payload[] = "Edge case test";
    acp_frame_t frame;
    int result = acp_frame_create_telemetry(&frame, ACP_FRAME_TYPE_TELEMETRY, 99,
                                            payload, sizeof(payload) - 1);
    if (result != ACP_OK)
    {
        printf("✗ Test setup failed\n");
        return 0;
    }

    uint8_t buffer[256];
    size_t encoded_len;
    result = acp_frame_encode(&frame, buffer, sizeof(buffer), &encoded_len);
    if (result != ACP_OK)
    {
        printf("✗ Test setup failed\n");
        return 0;
    }

    /* Test 1: Truncated frame */
    total_tests++;
    printf("Testing truncated frame... ");
    acp_frame_t decoded;
    size_t consumed;
    result = acp_frame_decode(buffer, encoded_len - 3, &decoded, &consumed); /* Remove last 3 bytes */
    if (result != ACP_OK)
    {
        printf("✓ PASS (correctly rejected: %d)\n", result);
        tests_passed++;
    }
    else
    {
        printf("✗ FAIL (truncated frame accepted)\n");
    }

    /* Test 2: Empty buffer */
    total_tests++;
    printf("Testing empty buffer... ");
    result = acp_frame_decode(NULL, 0, &decoded, &consumed);
    if (result != ACP_OK)
    {
        printf("✓ PASS (correctly rejected: %d)\n", result);
        tests_passed++;
    }
    else
    {
        printf("✗ FAIL (empty buffer accepted)\n");
    }

    /* Test 3: Single byte buffer */
    total_tests++;
    printf("Testing single byte... ");
    uint8_t single_byte = 0x42;
    result = acp_frame_decode(&single_byte, 1, &decoded, &consumed);
    if (result != ACP_OK)
    {
        printf("✓ PASS (correctly rejected: %d)\n", result);
        tests_passed++;
    }
    else
    {
        printf("✗ FAIL (single byte accepted)\n");
    }

    /* Test 4: All zeros frame */
    total_tests++;
    printf("Testing all-zeros frame... ");
    uint8_t zeros[32];
    memset(zeros, 0, sizeof(zeros));
    result = acp_frame_decode(zeros, sizeof(zeros), &decoded, &consumed);
    if (result != ACP_OK)
    {
        printf("✓ PASS (correctly rejected: %d)\n", result);
        tests_passed++;
    }
    else
    {
        printf("✗ FAIL (all-zeros frame accepted)\n");
    }

    /* Test 5: All 0xFF frame */
    total_tests++;
    printf("Testing all-0xFF frame... ");
    uint8_t ones[32];
    memset(ones, 0xFF, sizeof(ones));
    result = acp_frame_decode(ones, sizeof(ones), &decoded, &consumed);
    if (result != ACP_OK)
    {
        printf("✓ PASS (correctly rejected: %d)\n", result);
        tests_passed++;
    }
    else
    {
        printf("✗ FAIL (all-0xFF frame accepted)\n");
    }

    printf("\nEdge case corruption tests: %d/%d passed\n", tests_passed, total_tests);
    return tests_passed == total_tests;
}

/* Test multiple frames with different corruption levels */
static int test_bulk_corruption_resistance(void)
{
    printf("\nTest 4: Bulk Corruption Resistance\n");
    printf("===================================\n");

    int valid_frames = 0;
    int corrupted_frames = 0;
    int correctly_rejected = 0;

    /* Generate and test many frames */
    for (int i = 0; i < 50; i++)
    {
        /* Create frame with varying payload */
        char payload_text[64];
        int payload_len = snprintf(payload_text, sizeof(payload_text), "Frame %d test data", i);

        acp_frame_t frame;
        int result = acp_frame_create_telemetry(&frame, ACP_FRAME_TYPE_TELEMETRY, i,
                                                payload_text, payload_len);
        if (result != ACP_OK)
            continue;

        uint8_t buffer[256];
        size_t encoded_len;
        result = acp_frame_encode(&frame, buffer, sizeof(buffer), &encoded_len);
        if (result != ACP_OK)
            continue;

        /* Test valid frame */
        acp_frame_t decoded;
        size_t consumed;
        result = acp_frame_decode(buffer, encoded_len, &decoded, &consumed);
        if (result == ACP_OK)
        {
            valid_frames++;
        }

        /* Test corrupted version */
        buffer[encoded_len / 2] ^= 0x55; /* Corrupt middle byte */
        result = acp_frame_decode(buffer, encoded_len, &decoded, &consumed);
        corrupted_frames++;
        if (result != ACP_OK)
        {
            correctly_rejected++;
        }
    }

    printf("Valid frames processed: %d\n", valid_frames);
    printf("Corrupted frames tested: %d\n", corrupted_frames);
    printf("Correctly rejected: %d\n", correctly_rejected);

    /* All corrupted frames should be rejected */
    bool all_rejected = (correctly_rejected == corrupted_frames);
    bool some_valid = (valid_frames > 40); /* At least 80% should work */

    if (all_rejected && some_valid)
    {
        printf("✓ PASS - All corrupted frames rejected, valid frames accepted\n");
        return 1;
    }
    else
    {
        printf("✗ FAIL - Rejection rate: %d/%d, Valid rate: %d/50\n",
               correctly_rejected, corrupted_frames, valid_frames);
        return 0;
    }
}

/* Main test runner */
int main(void)
{
    printf("ACP CRC Validation and Corruption Detection Tests\n");
    printf("=================================================\n");

    /* Initialize ACP */
    if (acp_init() != ACP_OK)
    {
        printf("Failed to initialize ACP\n");
        return 1;
    }

    printf("Testing CRC16 integrity checking and corruption resistance...\n");

    int tests_passed = 0;
    int total_tests = 4;

    /* Run all CRC tests */
    if (test_corruption_detection())
        tests_passed++;
    if (test_valid_frame_acceptance())
        tests_passed++;
    if (test_edge_case_corruptions())
        tests_passed++;
    if (test_bulk_corruption_resistance())
        tests_passed++;

    /* Cleanup */
    acp_cleanup();

    /* Results */
    printf("\n=================================================\n");
    printf("CRC Mismatch Test Results: %d/%d passed\n", tests_passed, total_tests);

    if (tests_passed == total_tests)
    {
        printf("✅ All CRC validation tests PASSED\n");
        printf("   Frame corruption is properly detected and rejected\n");
        printf("   Valid frames are accepted without issues\n");
        printf("   CRC16 integrity checking works correctly\n");
        return 0;
    }
    else
    {
        printf("❌ Some CRC validation tests FAILED\n");
        printf("   Check CRC16 calculation and validation implementation\n");
        return 1;
    }
}