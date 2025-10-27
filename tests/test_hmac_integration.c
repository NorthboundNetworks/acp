/**
 * @file test_hmac_integration.c
 * @brief Test HMAC integration in high-level encode/decode functions
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "acp_protocol.h"

static void test_unauthenticated_telemetry(void)
{
    printf("Testing unauthenticated telemetry frame...\n");

    /* Initialize ACP library */
    acp_result_t result = acp_init();
    assert(result == ACP_OK);

    /* Create frame */
    uint8_t payload[] = "temperature:23.5C";
    uint8_t output[256];
    size_t output_len = sizeof(output);

    result = acp_encode_frame(ACP_FRAME_TYPE_TELEMETRY, 0, /* No auth flag */
                              payload, sizeof(payload) - 1,
                              NULL, /* No session for unauth */
                              output, &output_len);
    assert(result == ACP_OK);
    printf("  Encoded frame: %zu bytes\n", output_len);

    /* Decode frame */
    acp_frame_t decoded_frame;
    size_t consumed;

    result = acp_decode_frame(output, output_len, &decoded_frame, &consumed, NULL);
    assert(result == ACP_OK);
    assert(consumed == output_len);

    /* Verify frame contents */
    assert(decoded_frame.type == ACP_FRAME_TYPE_TELEMETRY);
    assert(decoded_frame.flags == 0);
    assert(decoded_frame.length == sizeof(payload) - 1);
    assert(decoded_frame.sequence == 0); /* No sequence for unauth */
    assert(memcmp(decoded_frame.payload, payload, sizeof(payload) - 1) == 0);

    printf("  ✓ Unauthenticated telemetry test passed\n");
}

static void test_authenticated_command(void)
{
    printf("Testing authenticated command frame...\n");

    /* Initialize session */
    acp_session_t session;
    uint8_t test_key[32] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20};

    acp_result_t result = acp_session_init(&session, 1, test_key, sizeof(test_key), 0x123456789ABCDEF0ULL);
    assert(result == ACP_OK);

    /* Create authenticated command */
    uint8_t payload[] = "SET_MODE:ACTIVE";
    uint8_t output[256];
    size_t output_len = sizeof(output);

    result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED,
                              payload, sizeof(payload) - 1,
                              &session,
                              output, &output_len);
    assert(result == ACP_OK);
    printf("  Encoded authenticated frame: %zu bytes\n", output_len);

    /* Verify session sequence was incremented */
    assert(session.next_sequence == 2); /* Started at 1, now should be 2 */

    /* Create fresh session for decoding (simulate receiver) */
    acp_session_t decode_session;
    result = acp_session_init(&decode_session, 1, test_key, sizeof(test_key), 0x123456789ABCDEF0ULL);
    assert(result == ACP_OK);

    /* Decode frame */
    acp_frame_t decoded_frame;
    size_t consumed;

    result = acp_decode_frame(output, output_len, &decoded_frame, &consumed, &decode_session);
    assert(result == ACP_OK);
    assert(consumed == output_len);

    /* Verify frame contents */
    assert(decoded_frame.type == ACP_FRAME_TYPE_COMMAND);
    assert(decoded_frame.flags & ACP_FLAG_AUTHENTICATED);
    assert(decoded_frame.length == sizeof(payload) - 1);
    assert(decoded_frame.sequence == 1); /* First sequence number */
    assert(memcmp(decoded_frame.payload, payload, sizeof(payload) - 1) == 0);

    /* Verify session state was updated */
    assert(decode_session.last_accepted_seq == 1);

    printf("  ✓ Authenticated command test passed\n");
}

static void test_replay_protection(void)
{
    printf("Testing replay protection...\n");

    /* Initialize session */
    acp_session_t encode_session, decode_session;
    uint8_t test_key[32] = {
        0xDE, 0xAD, 0xBE, 0xEF, 0xCA, 0xFE, 0xBA, 0xBE,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10,
        0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11};

    acp_result_t result = acp_session_init(&encode_session, 2, test_key, sizeof(test_key), 0xFEDCBA9876543210ULL);
    assert(result == ACP_OK);

    result = acp_session_init(&decode_session, 2, test_key, sizeof(test_key), 0xFEDCBA9876543210ULL);
    assert(result == ACP_OK);

    /* Encode and decode first frame */
    uint8_t payload1[] = "cmd1";
    uint8_t output1[256];
    size_t output1_len = sizeof(output1);

    result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED,
                              payload1, sizeof(payload1) - 1,
                              &encode_session, output1, &output1_len);
    assert(result == ACP_OK);

    acp_frame_t frame1;
    size_t consumed1;
    result = acp_decode_frame(output1, output1_len, &frame1, &consumed1, &decode_session);
    assert(result == ACP_OK);
    assert(frame1.sequence == 1);

    /* Encode second frame */
    uint8_t payload2[] = "cmd2";
    uint8_t output2[256];
    size_t output2_len = sizeof(output2);

    result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED,
                              payload2, sizeof(payload2) - 1,
                              &encode_session, output2, &output2_len);
    assert(result == ACP_OK);

    acp_frame_t frame2;
    size_t consumed2;
    result = acp_decode_frame(output2, output2_len, &frame2, &consumed2, &decode_session);
    assert(result == ACP_OK);
    assert(frame2.sequence == 2);

    /* Try to replay first frame - should be rejected */
    acp_frame_t replay_frame;
    size_t replay_consumed;
    result = acp_decode_frame(output1, output1_len, &replay_frame, &replay_consumed, &decode_session);
    assert(result == ACP_ERR_REPLAY);

    printf("  ✓ Replay protection test passed\n");
}

static void test_authentication_enforcement(void)
{
    printf("Testing authentication enforcement for commands...\n");

    /* Try to create unauthenticated command - should fail */
    uint8_t payload[] = "dangerous_cmd";
    uint8_t output[256];
    size_t output_len = sizeof(output);

    acp_result_t result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, 0, /* No auth flag */
                                           payload, sizeof(payload) - 1,
                                           NULL, output, &output_len);
    assert(result == ACP_ERR_AUTH_REQUIRED);

    /* Create a fake unauthenticated command frame manually */
    acp_frame_t fake_frame = {0};
    fake_frame.version = ACP_PROTOCOL_VERSION;
    fake_frame.type = ACP_FRAME_TYPE_COMMAND;
    fake_frame.flags = 0; /* No auth flag */
    fake_frame.length = sizeof(payload) - 1;
    memcpy(fake_frame.payload, payload, sizeof(payload) - 1);

    uint8_t fake_output[256];
    size_t fake_output_len;
    result = acp_frame_encode(&fake_frame, fake_output, sizeof(fake_output), &fake_output_len);
    assert(result == ACP_OK);

    /* Try to decode - should be rejected for missing auth */
    acp_frame_t decoded_frame;
    size_t consumed;
    result = acp_decode_frame(fake_output, fake_output_len, &decoded_frame, &consumed, NULL);
    assert(result == ACP_ERR_AUTH_REQUIRED);

    printf("  ✓ Authentication enforcement test passed\n");
}

int main(void)
{
    printf("ACP HMAC Integration Test\n");
    printf("=========================\n\n");

    test_unauthenticated_telemetry();
    printf("\n");

    test_authenticated_command();
    printf("\n");

    test_replay_protection();
    printf("\n");

    test_authentication_enforcement();
    printf("\n");

    printf("All HMAC integration tests passed!\n");

    acp_cleanup();
    return 0;
}