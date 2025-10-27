/**
 * @file test_keystore_integration.c
 * @brief Test keystore integration with session management
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "acp_protocol.h"

static void test_keystore_session_init(void)
{
    printf("Testing keystore session initialization...\n");

    /* Initialize ACP library */
    acp_result_t result = acp_init();
    assert(result == ACP_OK);

    /* Clear keystore for clean test */
    acp_keystore_clear();

    /* Initialize keystore */
    result = acp_keystore_init();
    assert(result == ACP_OK);

    /* Store a test key */
    uint32_t test_key_id = 42;
    uint8_t test_key[32] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};

    result = acp_keystore_set(test_key_id, test_key, sizeof(test_key));
    assert(result == ACP_OK);
    printf("  ✓ Key stored in keystore\n");

    /* Initialize session using keystore */
    acp_session_t session;
    uint64_t test_nonce = 0x123456789ABCDEF0ULL;

    result = acp_keystore_init_session(&session, test_key_id, test_nonce);
    assert(result == ACP_OK);
    printf("  ✓ Session initialized from keystore\n");

    /* Verify session was initialized correctly */
    assert(session.initialized == true);
    assert(session.key_id == test_key_id);
    assert(session.nonce == test_nonce);
    assert(session.next_sequence == 1);
    assert(session.last_accepted_seq == 0);

    /* Verify key material was loaded correctly */
    assert(memcmp(session.key, test_key, sizeof(test_key)) == 0);
    printf("  ✓ Session contains correct key material\n");

    /* Test with non-existent key */
    acp_session_t bad_session;
    result = acp_keystore_init_session(&bad_session, 9999, test_nonce);
    assert(result == ACP_ERR_KEY_NOT_FOUND);
    printf("  ✓ Correctly rejected non-existent key\n");

    printf("  ✓ Keystore session initialization test passed\n");
}

static void test_keystore_authentication_flow(void)
{
    printf("Testing end-to-end authentication with keystore...\n");

    /* Setup keystore with test key */
    acp_keystore_clear();
    acp_keystore_init();

    uint32_t key_id = 100;
    uint8_t master_key[32] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
        0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};

    acp_result_t result = acp_keystore_set(key_id, master_key, sizeof(master_key));
    assert(result == ACP_OK);

    /* Initialize sender session from keystore */
    acp_session_t sender_session;
    result = acp_keystore_init_session(&sender_session, key_id, 0x1111222233334444ULL);
    assert(result == ACP_OK);

    /* Initialize receiver session from keystore */
    acp_session_t receiver_session;
    result = acp_keystore_init_session(&receiver_session, key_id, 0x1111222233334444ULL);
    assert(result == ACP_OK);

    /* Encode authenticated command using keystore-initialized session */
    uint8_t payload[] = "KEYSTORE_AUTH_TEST";
    uint8_t output[256];
    size_t output_len = sizeof(output);

    result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED,
                              payload, sizeof(payload) - 1,
                              &sender_session, output, &output_len);
    assert(result == ACP_OK);
    printf("  ✓ Authenticated frame encoded using keystore session\n");

    /* Decode frame using receiver session (also from keystore) */
    acp_frame_t decoded_frame;
    size_t consumed;

    result = acp_decode_frame(output, output_len, &decoded_frame, &consumed, &receiver_session);
    assert(result == ACP_OK);
    assert(consumed == output_len);

    /* Verify frame contents */
    assert(decoded_frame.type == ACP_FRAME_TYPE_COMMAND);
    assert(decoded_frame.flags & ACP_FLAG_AUTHENTICATED);
    assert(decoded_frame.length == sizeof(payload) - 1);
    assert(decoded_frame.sequence == 1);
    assert(memcmp(decoded_frame.payload, payload, sizeof(payload) - 1) == 0);

    printf("  ✓ Frame successfully authenticated using keystore\n");
    printf("  ✓ End-to-end keystore authentication test passed\n");
}

int main(void)
{
    printf("ACP Keystore Integration Test\n");
    printf("=============================\n\n");

    test_keystore_session_init();
    printf("\n");

    test_keystore_authentication_flow();
    printf("\n");

    printf("All keystore integration tests passed!\n");

    acp_cleanup();
    return 0;
}