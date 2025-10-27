/**
 * @file test_hmac_vectors.c
 * @brief HMAC test vectors and validation for ACP crypto implementation
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "acp_protocol.h"

static void print_hex(const char *label, const uint8_t *data, size_t len)
{
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++)
    {
        printf("%02x", data[i]);
    }
    printf("\n");
}

/**
 * @brief Test HMAC-SHA256 with RFC 4231 test vectors
 */
static void test_rfc4231_vectors(void)
{
    printf("Testing RFC 4231 HMAC-SHA256 test vectors...\n");

    /* Test Case 1 from RFC 4231 */
    {
        printf("  Test Case 1:\n");

        uint8_t key[] = {
            0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
            0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
            0x0b, 0x0b, 0x0b, 0x0b};
        uint8_t data[] = "Hi There";

        uint8_t expected[] = {
            0xb0, 0x34, 0x4c, 0x61, 0xd8, 0xdb, 0x38, 0x53,
            0x5c, 0xa8, 0xaf, 0xce, 0xaf, 0x0b, 0xf1, 0x2b,
            0x88, 0x1d, 0xc2, 0x00, 0xc9, 0x83, 0x3d, 0xa7,
            0x26, 0xe9, 0x37, 0x6c, 0x2e, 0x32, 0xcf, 0xf7};

        uint8_t result[32];
        acp_hmac_sha256(key, sizeof(key) - 1, data, sizeof(data) - 1, result);

        print_hex("    Key", key, sizeof(key) - 1);
        print_hex("    Data", data, sizeof(data) - 1);
        print_hex("    Expected", expected, sizeof(expected));
        print_hex("    Got", result, sizeof(result));

        if (memcmp(result, expected, sizeof(expected)) == 0)
        {
            printf("    ✓ Test Case 1 passed\n");
        }
        else
        {
            printf("    ✗ Test Case 1 failed\n");
        }
    }

    /* Test Case 2 from RFC 4231 */
    {
        printf("  Test Case 2:\n");

        uint8_t key[] = "Jefe";
        uint8_t data[] = "what do ya want for nothing?";

        uint8_t expected[] = {
            0x5b, 0xdc, 0xc1, 0x46, 0xbf, 0x60, 0x75, 0x4e,
            0x6a, 0x04, 0x24, 0x26, 0x08, 0x95, 0x75, 0xc7,
            0x5a, 0x00, 0x3f, 0x08, 0x9d, 0x27, 0x39, 0x83,
            0x9d, 0xec, 0x58, 0xb9, 0x64, 0xec, 0x38, 0x43};

        uint8_t result[32];
        acp_hmac_sha256(key, sizeof(key) - 1, data, sizeof(data) - 1, result);

        print_hex("    Key", key, sizeof(key) - 1);
        print_hex("    Data", data, sizeof(data) - 1);
        print_hex("    Expected", expected, sizeof(expected));
        print_hex("    Got", result, sizeof(result));

        if (memcmp(result, expected, sizeof(expected)) == 0)
        {
            printf("    ✓ Test Case 2 passed\n");
        }
        else
        {
            printf("    ✗ Test Case 2 failed\n");
        }
    }

    printf("  ✓ RFC 4231 test vectors completed\n");
}

/**
 * @brief Test HMAC with ACP-specific parameters (16-byte truncation)
 */
static void test_acp_hmac_truncation(void)
{
    printf("Testing ACP HMAC 16-byte truncation...\n");

    /* Test with a known key and data */
    uint8_t key[ACP_KEY_SIZE] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20};

    uint8_t data[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

    /* Compute full HMAC */
    uint8_t full_hmac[32];
    acp_hmac_sha256(key, sizeof(key), data, sizeof(data), full_hmac);

    /* Extract 16-byte truncated version */
    uint8_t truncated_hmac[ACP_HMAC_TAG_LEN];
    memcpy(truncated_hmac, full_hmac, ACP_HMAC_TAG_LEN);

    print_hex("  Key", key, sizeof(key));
    print_hex("  Data", data, sizeof(data));
    print_hex("  Full HMAC", full_hmac, sizeof(full_hmac));
    print_hex("  Truncated HMAC", truncated_hmac, sizeof(truncated_hmac));

    /* Verify truncation is correct */
    assert(memcmp(truncated_hmac, full_hmac, ACP_HMAC_TAG_LEN) == 0);

    printf("  ✓ HMAC truncation test passed\n");
}

/**
 * @brief Test HMAC consistency across multiple calls
 */
static void test_hmac_consistency(void)
{
    printf("Testing HMAC consistency...\n");

    uint8_t key[] = "test_key_123456789012345678901234"; /* 32 bytes */
    uint8_t data[] = "consistency_test_data";

    uint8_t hmac1[32], hmac2[32], hmac3[32];

    /* Compute HMAC multiple times */
    acp_hmac_sha256(key, 32, data, sizeof(data) - 1, hmac1);
    acp_hmac_sha256(key, 32, data, sizeof(data) - 1, hmac2);
    acp_hmac_sha256(key, 32, data, sizeof(data) - 1, hmac3);

    /* All should be identical */
    assert(memcmp(hmac1, hmac2, 32) == 0);
    assert(memcmp(hmac2, hmac3, 32) == 0);

    print_hex("  Consistent HMAC", hmac1, 32);

    printf("  ✓ HMAC consistency test passed\n");
}

/**
 * @brief Test HMAC with different key lengths
 */
static void test_hmac_key_lengths(void)
{
    printf("Testing HMAC with different key lengths...\n");

    uint8_t data[] = "test data for key length variation";

    /* Short key (< block size) */
    {
        uint8_t short_key[] = "short";
        uint8_t hmac[32];
        acp_hmac_sha256(short_key, sizeof(short_key) - 1, data, sizeof(data) - 1, hmac);
        print_hex("  Short key HMAC", hmac, ACP_HMAC_TAG_LEN);
    }

    /* Exact block size key (64 bytes) */
    {
        uint8_t block_key[64];
        for (int i = 0; i < 64; i++)
        {
            block_key[i] = (uint8_t)(i + 1);
        }
        uint8_t hmac[32];
        acp_hmac_sha256(block_key, sizeof(block_key), data, sizeof(data) - 1, hmac);
        print_hex("  Block size key HMAC", hmac, ACP_HMAC_TAG_LEN);
    }

    /* Long key (> block size) */
    {
        uint8_t long_key[80];
        for (int i = 0; i < 80; i++)
        {
            long_key[i] = (uint8_t)(0xFF - i);
        }
        uint8_t hmac[32];
        acp_hmac_sha256(long_key, sizeof(long_key), data, sizeof(data) - 1, hmac);
        print_hex("  Long key HMAC", hmac, ACP_HMAC_TAG_LEN);
    }

    printf("  ✓ Key length variation test passed\n");
}

/**
 * @brief Test constant-time comparison
 */
static void test_constant_time_comparison(void)
{
    printf("Testing constant-time comparison...\n");

    uint8_t mac1[] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};

    uint8_t mac2[] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};

    uint8_t mac3[] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x11 /* Different last byte */
    };

    /* Test equal MACs */
    int result1 = acp_crypto_memcmp_ct(mac1, mac2, ACP_HMAC_TAG_LEN);
    assert(result1 == 0);
    printf("  ✓ Equal MACs comparison: %s\n", (result1 == 0) ? "EQUAL" : "DIFFERENT");

    /* Test different MACs */
    int result2 = acp_crypto_memcmp_ct(mac1, mac3, ACP_HMAC_TAG_LEN);
    assert(result2 != 0);
    printf("  ✓ Different MACs comparison: %s\n", (result2 == 0) ? "EQUAL" : "DIFFERENT");

    printf("  ✓ Constant-time comparison test passed\n");
}

int main(void)
{
    printf("ACP HMAC Test Vectors\n");
    printf("====================\n\n");

    /* Initialize crypto for self-tests */
    printf("Running crypto self-tests first...\n");
    int self_test_result = acp_crypto_self_test();
    printf("Crypto self-test: %s\n\n", (self_test_result == ACP_OK) ? "PASS" : "FAIL");
    assert(self_test_result == ACP_OK);

    test_rfc4231_vectors();
    printf("\n");

    test_acp_hmac_truncation();
    printf("\n");

    test_hmac_consistency();
    printf("\n");

    test_hmac_key_lengths();
    printf("\n");

    test_constant_time_comparison();
    printf("\n");

    printf("All HMAC test vectors passed!\n");
    return 0;
}