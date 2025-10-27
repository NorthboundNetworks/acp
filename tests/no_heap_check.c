/**
 * @file no_heap_check.c
 * @brief No-heap allocation verification for ACP library
 *
 * Verifies that the ACP library operates without heap allocation
 * when compiled with ACP_NO_HEAP=1, making it suitable for
 * embedded systems with static memory requirements.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "acp_protocol.h"
#include "acp_config.h"

/* Heap allocation tracking */
static size_t malloc_call_count = 0;
static size_t calloc_call_count = 0;
static size_t realloc_call_count = 0;
static size_t free_call_count = 0;
static bool heap_tracking_enabled = false;

/* Original function pointers (if we want to do real allocation) */
static void *(*original_malloc)(size_t) = NULL;
static void *(*original_calloc)(size_t, size_t) = NULL;
static void *(*original_realloc)(void *, size_t) = NULL;
static void (*original_free)(void *) = NULL;

/* Heap allocation interceptors */
void *malloc(size_t size)
{
    if (heap_tracking_enabled)
    {
        malloc_call_count++;
        printf("❌ HEAP VIOLATION: malloc(%zu) called!\n", size);
        return NULL; /* Force failure to detect heap usage */
    }

    /* If tracking disabled, use system malloc (for test infrastructure) */
    if (original_malloc)
    {
        return original_malloc(size);
    }

    /* Fallback - this shouldn't happen in our test */
    return NULL;
}

void *calloc(size_t nmemb, size_t size)
{
    if (heap_tracking_enabled)
    {
        calloc_call_count++;
        printf("❌ HEAP VIOLATION: calloc(%zu, %zu) called!\n", nmemb, size);
        return NULL;
    }

    if (original_calloc)
    {
        return original_calloc(nmemb, size);
    }

    return NULL;
}

void *realloc(void *ptr, size_t size)
{
    if (heap_tracking_enabled)
    {
        realloc_call_count++;
        printf("❌ HEAP VIOLATION: realloc(%p, %zu) called!\n", ptr, size);
        return NULL;
    }

    if (original_realloc)
    {
        return original_realloc(ptr, size);
    }

    return NULL;
}

void free(void *ptr)
{
    if (heap_tracking_enabled)
    {
        free_call_count++;
        printf("❌ HEAP VIOLATION: free(%p) called!\n", ptr);
    }

    if (original_free)
    {
        original_free(ptr);
    }
}

/* Reset heap allocation counters */
static void reset_heap_counters(void)
{
    malloc_call_count = 0;
    calloc_call_count = 0;
    realloc_call_count = 0;
    free_call_count = 0;
}

/* Check if any heap allocation occurred */
static bool heap_used(void)
{
    return (malloc_call_count > 0 || calloc_call_count > 0 ||
            realloc_call_count > 0 || free_call_count > 0);
}

/* Print heap usage statistics */
static void print_heap_stats(void)
{
    printf("Heap allocation statistics:\n");
    printf("  malloc() calls: %zu\n", malloc_call_count);
    printf("  calloc() calls: %zu\n", calloc_call_count);
    printf("  realloc() calls: %zu\n", realloc_call_count);
    printf("  free() calls: %zu\n", free_call_count);
}

/* Test basic ACP operations without heap */
static int test_basic_operations_no_heap(void)
{
    printf("\nTest 1: Basic Operations Without Heap\n");
    printf("======================================\n");

    /* Enable heap tracking */
    heap_tracking_enabled = true;
    reset_heap_counters();

    /* Initialize ACP */
    printf("Testing acp_init()... ");
    int result = acp_init();
    if (result != ACP_OK)
    {
        printf("✗ FAIL (init failed: %d)\n", result);
        heap_tracking_enabled = false;
        return 0;
    }
    if (heap_used())
    {
        printf("✗ FAIL (heap used during init)\n");
        print_heap_stats();
        heap_tracking_enabled = false;
        return 0;
    }
    printf("✓ PASS\n");

    /* Test frame creation */
    printf("Testing frame creation... ");
    reset_heap_counters();
    acp_frame_t frame;
    uint8_t test_payload[] = "No heap test payload";
    result = acp_frame_create_telemetry(&frame, ACP_FRAME_TYPE_TELEMETRY, 42,
                                        test_payload, sizeof(test_payload) - 1);
    if (result != ACP_OK)
    {
        printf("✗ FAIL (creation failed: %d)\n", result);
        heap_tracking_enabled = false;
        return 0;
    }
    if (heap_used())
    {
        printf("✗ FAIL (heap used during frame creation)\n");
        print_heap_stats();
        heap_tracking_enabled = false;
        return 0;
    }
    printf("✓ PASS\n");

    /* Test frame encoding */
    printf("Testing frame encoding... ");
    reset_heap_counters();
    uint8_t buffer[256];
    size_t encoded_len;
    result = acp_frame_encode(&frame, buffer, sizeof(buffer), &encoded_len);
    if (result != ACP_OK)
    {
        printf("✗ FAIL (encoding failed: %d)\n", result);
        heap_tracking_enabled = false;
        return 0;
    }
    if (heap_used())
    {
        printf("✗ FAIL (heap used during encoding)\n");
        print_heap_stats();
        heap_tracking_enabled = false;
        return 0;
    }
    printf("✓ PASS (encoded to %zu bytes)\n", encoded_len);

    /* Test frame decoding */
    printf("Testing frame decoding... ");
    reset_heap_counters();
    acp_frame_t decoded_frame;
    size_t consumed;
    result = acp_frame_decode(buffer, encoded_len, &decoded_frame, &consumed);
    if (result != ACP_OK)
    {
        printf("✗ FAIL (decoding failed: %d)\n", result);
        heap_tracking_enabled = false;
        return 0;
    }
    if (heap_used())
    {
        printf("✗ FAIL (heap used during decoding)\n");
        print_heap_stats();
        heap_tracking_enabled = false;
        return 0;
    }
    printf("✓ PASS\n");

    /* Test cleanup */
    printf("Testing acp_cleanup()... ");
    reset_heap_counters();
    acp_cleanup();
    if (heap_used())
    {
        printf("✗ FAIL (heap used during cleanup)\n");
        print_heap_stats();
        heap_tracking_enabled = false;
        return 0;
    }
    printf("✓ PASS\n");

    heap_tracking_enabled = false;
    return 1;
}

/* Test intensive operations to ensure no hidden allocations */
static int test_intensive_operations_no_heap(void)
{
    printf("\nTest 2: Intensive Operations Without Heap\n");
    printf("==========================================\n");

    heap_tracking_enabled = true;
    reset_heap_counters();

    /* Initialize */
    if (acp_init() != ACP_OK)
    {
        printf("✗ Initialization failed\n");
        heap_tracking_enabled = false;
        return 0;
    }

    printf("Creating and processing 100 frames...\n");

    /* Process many frames */
    for (int i = 0; i < 100; i++)
    {
        /* Create frame with varying payload sizes */
        char payload_data[128];
        int payload_len = snprintf(payload_data, sizeof(payload_data),
                                   "Frame %d test data with varying length", i);

        acp_frame_t frame;
        int result = acp_frame_create_telemetry(&frame, ACP_FRAME_TYPE_TELEMETRY, i,
                                                payload_data, payload_len);
        if (result != ACP_OK)
        {
            printf("✗ Frame %d creation failed: %d\n", i, result);
            break;
        }

        /* Encode frame */
        uint8_t buffer[512];
        size_t encoded_len;
        result = acp_frame_encode(&frame, buffer, sizeof(buffer), &encoded_len);
        if (result != ACP_OK)
        {
            printf("✗ Frame %d encoding failed: %d\n", i, result);
            break;
        }

        /* Decode frame back */
        acp_frame_t decoded_frame;
        size_t consumed;
        result = acp_frame_decode(buffer, encoded_len, &decoded_frame, &consumed);
        if (result != ACP_OK)
        {
            printf("✗ Frame %d decoding failed: %d\n", i, result);
            break;
        }

        /* Verify no heap usage every 25 frames */
        if ((i + 1) % 25 == 0)
        {
            if (heap_used())
            {
                printf("✗ FAIL (heap used after %d frames)\n", i + 1);
                print_heap_stats();
                heap_tracking_enabled = false;
                acp_cleanup();
                return 0;
            }
            printf("  ✓ %d frames processed, no heap usage\n", i + 1);
        }
    }

    /* Final check */
    if (heap_used())
    {
        printf("✗ FAIL (heap used during intensive operations)\n");
        print_heap_stats();
        heap_tracking_enabled = false;
        acp_cleanup();
        return 0;
    }

    printf("✓ PASS - All 100 frames processed without heap allocation\n");

    acp_cleanup();
    heap_tracking_enabled = false;
    return 1;
}

/* Test maximum payload size without heap */
static int test_max_payload_no_heap(void)
{
    printf("\nTest 3: Maximum Payload Size Without Heap\n");
    printf("==========================================\n");

    heap_tracking_enabled = true;
    reset_heap_counters();

    if (acp_init() != ACP_OK)
    {
        printf("✗ Initialization failed\n");
        heap_tracking_enabled = false;
        return 0;
    }

    printf("Testing maximum payload size (%d bytes)...\n", ACP_MAX_PAYLOAD_SIZE);

    /* Create maximum size payload on stack */
    uint8_t max_payload[ACP_MAX_PAYLOAD_SIZE];
    for (size_t i = 0; i < ACP_MAX_PAYLOAD_SIZE; i++)
    {
        max_payload[i] = (uint8_t)(i & 0xFF);
    }

    /* Create frame with max payload */
    acp_frame_t frame;
    int result = acp_frame_create_telemetry(&frame, ACP_FRAME_TYPE_TELEMETRY, 999,
                                            max_payload, ACP_MAX_PAYLOAD_SIZE);
    if (result != ACP_OK)
    {
        printf("✗ Max payload frame creation failed: %d\n", result);
        heap_tracking_enabled = false;
        acp_cleanup();
        return 0;
    }

    /* Encode max frame */
    uint8_t large_buffer[2048]; /* Large stack buffer */
    size_t encoded_len;
    result = acp_frame_encode(&frame, large_buffer, sizeof(large_buffer), &encoded_len);
    if (result != ACP_OK)
    {
        printf("✗ Max payload encoding failed: %d\n", result);
        heap_tracking_enabled = false;
        acp_cleanup();
        return 0;
    }

    printf("Max frame encoded to %zu bytes\n", encoded_len);

    /* Decode max frame */
    acp_frame_t decoded_frame;
    size_t consumed;
    result = acp_frame_decode(large_buffer, encoded_len, &decoded_frame, &consumed);
    if (result != ACP_OK)
    {
        printf("✗ Max payload decoding failed: %d\n", result);
        heap_tracking_enabled = false;
        acp_cleanup();
        return 0;
    }

    /* Verify payload integrity */
    if (decoded_frame.length != ACP_MAX_PAYLOAD_SIZE ||
        memcmp(decoded_frame.payload, max_payload, ACP_MAX_PAYLOAD_SIZE) != 0)
    {
        printf("✗ Max payload data integrity failed\n");
        heap_tracking_enabled = false;
        acp_cleanup();
        return 0;
    }

    /* Check heap usage */
    if (heap_used())
    {
        printf("✗ FAIL (heap used for maximum payload)\n");
        print_heap_stats();
        heap_tracking_enabled = false;
        acp_cleanup();
        return 0;
    }

    printf("✓ PASS - Maximum payload processed without heap allocation\n");

    acp_cleanup();
    heap_tracking_enabled = false;
    return 1;
}

/* Check ACP_NO_HEAP configuration */
static int test_configuration_check(void)
{
    printf("\nTest 4: No-Heap Configuration Check\n");
    printf("====================================\n");

#ifdef ACP_NO_HEAP
    printf("✓ ACP_NO_HEAP is defined (value: %d)\n", ACP_NO_HEAP);
    if (ACP_NO_HEAP == 1)
    {
        printf("✓ ACP_NO_HEAP correctly set to 1\n");
    }
    else
    {
        printf("⚠ ACP_NO_HEAP is defined but not set to 1 (value: %d)\n", ACP_NO_HEAP);
    }
#else
    printf("❌ ACP_NO_HEAP is not defined - library may use heap!\n");
    printf("   This test should be run with ACP_NO_HEAP=1\n");
    return 0;
#endif

    /* Check other related configuration */
    printf("Configuration status:\n");
    printf("  ACP_MAX_PAYLOAD_SIZE: %d\n", ACP_MAX_PAYLOAD_SIZE);

#ifdef ACP_STATIC_ONLY
    printf("  ACP_STATIC_ONLY: defined\n");
#else
    printf("  ACP_STATIC_ONLY: not defined\n");
#endif

    return 1;
}

/* Main test runner */
int main(void)
{
    printf("ACP No-Heap Allocation Verification\n");
    printf("===================================\n");

    printf("This test verifies that ACP operates without heap allocation\n");
    printf("when compiled with ACP_NO_HEAP=1 for embedded systems.\n");

    int tests_passed = 0;
    int total_tests = 4;

    /* Run configuration check first */
    if (test_configuration_check())
        tests_passed++;

/* Only run operational tests if properly configured */
#ifdef ACP_NO_HEAP
    if (test_basic_operations_no_heap())
        tests_passed++;
    if (test_intensive_operations_no_heap())
        tests_passed++;
    if (test_max_payload_no_heap())
        tests_passed++;
#else
    printf("\n⚠ Skipping operational tests - ACP_NO_HEAP not defined\n");
    total_tests = 1; /* Only config test */
#endif

    /* Results */
    printf("\n===================================\n");
    printf("No-Heap Verification Results: %d/%d passed\n", tests_passed, total_tests);

    if (tests_passed == total_tests)
    {
        printf("✅ All no-heap tests PASSED\n");
        printf("   ACP library operates without heap allocation\n");
        printf("   Suitable for embedded systems with static memory\n");
        return 0;
    }
    else
    {
        printf("❌ Some no-heap tests FAILED\n");
        printf("   Library may not be suitable for heap-constrained environments\n");
        return 1;
    }
}
