/**
 * @file test_hmac_tag_rejection.c
 * @brief Test that incorrect HMAC tags (16-byte truncated) are properly rejected
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

static void test_correct_hmac_acceptance(void)
{
    printf("Testing correct HMAC acceptance...\n");

    /* Initialize ACP */
    acp_result_t result = acp_init();
    assert(result == ACP_OK);

    /* Setup sessions with same key */
    acp_session_t sender_session, receiver_session;
    uint8_t test_key[ACP_KEY_SIZE] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

    result = acp_session_init(&sender_session, 1, test_key, ACP_KEY_SIZE, 0x1234567890ABCDEFULL);
    assert(result == ACP_OK);

    result = acp_session_init(&receiver_session, 1, test_key, ACP_KEY_SIZE, 0x1234567890ABCDEFULL);
    assert(result == ACP_OK);

    /* Encode authenticated frame */
    uint8_t payload[] = "test_hmac_verification";
    uint8_t output[256];
    size_t output_len = sizeof(output);

    result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED,
                              payload, sizeof(payload) - 1,
                              &sender_session, output, &output_len);
    assert(result == ACP_OK);

    printf("  Original frame (%zu bytes):\n", output_len);
    print_hex("    Frame data", output, output_len);
    printf("    Last 16 bytes (HMAC tag): ");
    print_hex("", output + output_len - 16, 16);

    /* Decode with correct HMAC - should succeed */
    acp_frame_t frame;
    size_t consumed;
    result = acp_decode_frame(output, output_len, &frame, &consumed, &receiver_session);
    assert(result == ACP_OK);
    assert(consumed == output_len);

    printf("  ✓ Frame with correct HMAC tag accepted\n");
    printf("  ✓ Correct HMAC acceptance test passed\n");
}

static void test_corrupted_hmac_rejection(void)
{
    printf("Testing corrupted HMAC rejection...\n");

    /* Setup sessions */
    acp_session_t sender_session, receiver_session;
    uint8_t test_key[ACP_KEY_SIZE];
    for (int i = 0; i < ACP_KEY_SIZE; i++)
    {
        test_key[i] = (uint8_t)(i * 3 + 17);
    }

    acp_result_t result = acp_session_init(&sender_session, 2, test_key, ACP_KEY_SIZE, 0xFEDCBA9876543210ULL);
    assert(result == ACP_OK);

    result = acp_session_init(&receiver_session, 2, test_key, ACP_KEY_SIZE, 0xFEDCBA9876543210ULL);
    assert(result == ACP_OK);

    /* Encode authenticated frame */
    uint8_t payload[] = "hmac_corruption_test";
    uint8_t output[256];
    size_t output_len = sizeof(output);

    result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED,
                              payload, sizeof(payload) - 1,
                              &sender_session, output, &output_len);
    assert(result == ACP_OK);

    printf("  Original HMAC tag: ");
    print_hex("", output + output_len - 16, 16);

    /* Test different types of corruption */

    /* 1. Corrupt single byte in HMAC */
    {
        uint8_t corrupted[256];
        memcpy(corrupted, output, output_len);
        corrupted[output_len - 1] ^= 0x01; /* Flip last bit */

        printf("  Corrupted HMAC (last byte): ");
        print_hex("", corrupted + output_len - 16, 16);

        acp_frame_t frame;
        size_t consumed;
        result = acp_decode_frame(corrupted, output_len, &frame, &consumed, &receiver_session);
        assert(result == ACP_ERR_AUTH_FAILED);
        printf("  ✓ Single byte corruption rejected with ACP_ERR_AUTH_FAILED\n");
    }

    /* 2. Corrupt first byte of HMAC */
    {
        uint8_t corrupted[256];
        memcpy(corrupted, output, output_len);
        corrupted[output_len - 16] ^= 0xFF; /* Flip first byte of HMAC */

        acp_frame_t frame;
        size_t consumed;
        result = acp_decode_frame(corrupted, output_len, &frame, &consumed, &receiver_session);
        assert(result == ACP_ERR_AUTH_FAILED);
        printf("  ✓ First byte corruption rejected\n");
    }

    /* 3. Corrupt middle byte of HMAC */
    {
        uint8_t corrupted[256];
        memcpy(corrupted, output, output_len);
        corrupted[output_len - 8] ^= 0xAA; /* Flip middle byte */

        acp_frame_t frame;
        size_t consumed;
        result = acp_decode_frame(corrupted, output_len, &frame, &consumed, &receiver_session);
        assert(result == ACP_ERR_AUTH_FAILED);
        printf("  ✓ Middle byte corruption rejected\n");
    }

    /* 4. Zero out entire HMAC */
    {
        uint8_t corrupted[256];
        memcpy(corrupted, output, output_len);
        memset(corrupted + output_len - 16, 0, 16);

        printf("  Zeroed HMAC: ");
        print_hex("", corrupted + output_len - 16, 16);

        acp_frame_t frame;
        size_t consumed;
        result = acp_decode_frame(corrupted, output_len, &frame, &consumed, &receiver_session);
        assert(result == ACP_ERR_AUTH_FAILED);
        printf("  ✓ Zeroed HMAC rejected\n");
    }

    /* 5. Replace with random HMAC */
    {
        uint8_t corrupted[256];
        memcpy(corrupted, output, output_len);

        uint8_t random_hmac[16] = {
            0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
            0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0};
        memcpy(corrupted + output_len - 16, random_hmac, 16);

        printf("  Random HMAC: ");
        print_hex("", corrupted + output_len - 16, 16);

        acp_frame_t frame;
        size_t consumed;
        result = acp_decode_frame(corrupted, output_len, &frame, &consumed, &receiver_session);
        assert(result == ACP_ERR_AUTH_FAILED);
        printf("  ✓ Random HMAC rejected\n");
    }

    printf("  ✓ HMAC corruption rejection test passed\n");
}

static void test_wrong_key_rejection(void)
{
    printf("Testing wrong key HMAC rejection...\n");

    /* Setup sender and receiver with different keys */
    acp_session_t sender_session, receiver_session;

    uint8_t sender_key[ACP_KEY_SIZE] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};

    uint8_t receiver_key[ACP_KEY_SIZE] = {
        0x20, 0x1F, 0x1E, 0x1D, 0x1C, 0x1B, 0x1A, 0x19,
        0x18, 0x17, 0x16, 0x15, 0x14, 0x13, 0x12, 0x11,
        0x10, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09,
        0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};

    acp_result_t result = acp_session_init(&sender_session, 3, sender_key, ACP_KEY_SIZE, 0xAAAABBBBCCCCDDDDULL);
    assert(result == ACP_OK);

    result = acp_session_init(&receiver_session, 3, receiver_key, ACP_KEY_SIZE, 0xAAAABBBBCCCCDDDDULL);
    assert(result == ACP_OK);

    print_hex("  Sender key", sender_key, ACP_KEY_SIZE);
    print_hex("  Receiver key", receiver_key, ACP_KEY_SIZE);

    /* Encode with sender key */
    uint8_t payload[] = "wrong_key_test";
    uint8_t output[256];
    size_t output_len = sizeof(output);

    result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED,
                              payload, sizeof(payload) - 1,
                              &sender_session, output, &output_len);
    assert(result == ACP_OK);

    printf("  Sender HMAC: ");
    print_hex("", output + output_len - 16, 16);

    /* Try to decode with receiver key - should fail */
    acp_frame_t frame;
    size_t consumed;
    result = acp_decode_frame(output, output_len, &frame, &consumed, &receiver_session);
    assert(result == ACP_ERR_AUTH_FAILED);
    printf("  ✓ Frame with wrong key HMAC rejected with ACP_ERR_AUTH_FAILED\n");

    printf("  ✓ Wrong key rejection test passed\n");
}

static void test_truncated_hmac_properties(void)
{
    printf("Testing 16-byte HMAC truncation properties...\n");

    /* Generate full HMAC and truncated version */
    uint8_t key[ACP_KEY_SIZE];
    for (int i = 0; i < ACP_KEY_SIZE; i++)
    {
        key[i] = (uint8_t)(i + 100);
    }

    uint8_t data[] = "truncation_test_data_12345";
    uint8_t full_hmac[32];

    acp_hmac_sha256(key, sizeof(key), data, sizeof(data) - 1, full_hmac);

    print_hex("  Full HMAC (32 bytes)", full_hmac, 32);
    print_hex("  Truncated HMAC (16 bytes)", full_hmac, ACP_HMAC_TAG_LEN);

    /* Verify truncation properties */
    assert(ACP_HMAC_TAG_LEN == 16);
    printf("  ✓ ACP_HMAC_TAG_LEN is correctly set to 16 bytes\n");

    /* Verify that different data produces different truncated HMACs */
    uint8_t data2[] = "different_test_data_67890";
    uint8_t full_hmac2[32];

    acp_hmac_sha256(key, sizeof(key), data2, sizeof(data2) - 1, full_hmac2);

    print_hex("  Different data HMAC (16 bytes)", full_hmac2, ACP_HMAC_TAG_LEN);

    /* The truncated HMACs should be different */
    int same = (memcmp(full_hmac, full_hmac2, ACP_HMAC_TAG_LEN) == 0);
    assert(same == 0);
    printf("  ✓ Different data produces different truncated HMACs\n");

    printf("  ✓ HMAC truncation properties test passed\n");
}

int main(void)
{
    printf("ACP HMAC Tag Rejection Test\n");
    printf("===========================\n\n");

    test_correct_hmac_acceptance();
    printf("\n");

    test_corrupted_hmac_rejection();
    printf("\n");

    test_wrong_key_rejection();
    printf("\n");

    test_truncated_hmac_properties();
    printf("\n");

    printf("All HMAC tag rejection tests passed!\n");

    acp_cleanup();
    return 0;
}