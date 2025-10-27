/**
 * @file test_hmac.c
 * @brief Test program for HMAC-SHA256 implementation
 */

#include "acp_crypto.h"
#include <stdio.h>
#include <string.h>

void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("%s: ", label);
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main(void) {
    printf("ACP Crypto Test - HMAC-SHA256 Implementation\n");
    printf("============================================\n\n");

    /* Test 1: SHA-256 self-test */
    printf("Running SHA-256 self-test...\n");
    int result = acp_sha256_self_test();
    printf("SHA-256 self-test: %s\n\n", result == 0 ? "PASS" : "FAIL");

    /* Test 2: HMAC-SHA256 self-test */
    printf("Running HMAC-SHA256 self-test...\n");
    result = acp_hmac_self_test();
    printf("HMAC-SHA256 self-test: %s\n\n", result == 0 ? "PASS" : "FAIL");

    /* Test 3: Full crypto self-test */
    printf("Running full crypto self-test...\n");
    result = acp_crypto_self_test();
    printf("Full crypto self-test: %s\n\n", result == 0 ? "PASS" : "FAIL");

    /* Test 4: Manual HMAC test with ACP key */
    printf("Manual HMAC-SHA256 test:\n");
    const uint8_t acp_key[32] = {
        0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x00, 0x11
    };
    
    const uint8_t test_data[] = "ACP Test Message";
    uint8_t hmac_result[ACP_HMAC_SIZE];
    
    acp_hmac_sha256(acp_key, sizeof(acp_key), test_data, strlen((char*)test_data), hmac_result);
    
    print_hex("Key", acp_key, 32);
    print_hex("Data", test_data, strlen((char*)test_data));
    print_hex("HMAC-SHA256 (16-byte truncated)", hmac_result, ACP_HMAC_SIZE);

    /* Test 5: Constant-time comparison */
    printf("\nTesting constant-time comparison:\n");
    uint8_t mac1[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
    uint8_t mac2[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10};
    uint8_t mac3[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x11}; /* Different last byte */

    int same = acp_crypto_memcmp_ct(mac1, mac2, 16);
    int different = acp_crypto_memcmp_ct(mac1, mac3, 16);
    
    printf("Same MACs comparison: %s\n", same == 0 ? "EQUAL" : "DIFFERENT");
    printf("Different MACs comparison: %s\n", different != 0 ? "DIFFERENT" : "EQUAL");

    printf("\nAll HMAC-SHA256 tests completed!\n");
    return 0;
}