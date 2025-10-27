/**
 * @file test_command_auth_rejection.c
 * @brief Test that unauthenticated command frames are properly rejected
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "acp_protocol.h"

static void test_encode_rejects_unauth_commands(void)
{
    printf("Testing encode rejection of unauthenticated commands...\n");

    /* Initialize ACP */
    acp_result_t result = acp_init();
    assert(result == ACP_OK);

    /* Try to encode unauthenticated command - should fail at encode time */
    uint8_t payload[] = "unauthorized_command";
    uint8_t output[256];
    size_t output_len = sizeof(output);

    result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, 0, /* No auth flag */
                              payload, sizeof(payload) - 1,
                              NULL, output, &output_len);

    /* Should be rejected with AUTH_REQUIRED */
    assert(result == ACP_ERR_AUTH_REQUIRED);
    printf("  ✓ Encoder correctly rejected unauthenticated command with ACP_ERR_AUTH_REQUIRED\n");

    /* Verify telemetry can still be unauthenticated */
    result = acp_encode_frame(ACP_FRAME_TYPE_TELEMETRY, 0, /* No auth flag */
                              payload, sizeof(payload) - 1,
                              NULL, output, &output_len);
    assert(result == ACP_OK);
    printf("  ✓ Encoder correctly allowed unauthenticated telemetry\n");

    /* Verify system frames can be unauthenticated */
    result = acp_encode_frame(ACP_FRAME_TYPE_SYSTEM, 0, /* No auth flag */
                              payload, sizeof(payload) - 1,
                              NULL, output, &output_len);
    assert(result == ACP_OK);
    printf("  ✓ Encoder correctly allowed unauthenticated system frame\n");

    printf("  ✓ Encode rejection test passed\n");
}

static void test_decode_rejects_unauth_commands(void)
{
    printf("Testing decode rejection of unauthenticated commands...\n");

    /* Create a fake unauthenticated command frame by bypassing high-level API */
    acp_frame_t fake_command = {0};
    fake_command.version = ACP_PROTOCOL_VERSION;
    fake_command.type = ACP_FRAME_TYPE_COMMAND;
    fake_command.flags = 0;    /* No authentication flag */
    fake_command.sequence = 0; /* No sequence for unauth */
    fake_command.length = 15;
    memcpy(fake_command.payload, "malicious_cmd", fake_command.length);

    /* Encode using low-level framer (bypasses auth checks) */
    uint8_t fake_output[256];
    size_t fake_len;
    int result = acp_frame_encode(&fake_command, fake_output, sizeof(fake_output), &fake_len);
    assert(result == ACP_OK);
    printf("  ✓ Created fake unauthenticated command frame\n");

    /* Try to decode with high-level decoder - should be rejected */
    acp_frame_t decoded_frame;
    size_t consumed;
    acp_result_t decode_result = acp_decode_frame(fake_output, fake_len, &decoded_frame, &consumed, NULL);

    /* Should be rejected with AUTH_REQUIRED */
    assert(decode_result == ACP_ERR_AUTH_REQUIRED);
    printf("  ✓ Decoder correctly rejected unauthenticated command with ACP_ERR_AUTH_REQUIRED\n");

    /* Verify that authenticated commands work */
    acp_session_t session;
    uint8_t test_key[ACP_KEY_SIZE];
    memset(test_key, 0x42, sizeof(test_key));

    acp_result_t init_result = acp_session_init(&session, 1, test_key, ACP_KEY_SIZE, 0x123456789ABCDEF0ULL);
    assert(init_result == ACP_OK);

    uint8_t auth_payload[] = "authorized_cmd";
    uint8_t auth_output[256];
    size_t auth_len = sizeof(auth_output);

    acp_result_t auth_encode_result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED,
                                                       auth_payload, sizeof(auth_payload) - 1,
                                                       &session, auth_output, &auth_len);
    assert(auth_encode_result == ACP_OK);

    acp_frame_t auth_frame;
    size_t auth_consumed;
    acp_result_t auth_decode_result = acp_decode_frame(auth_output, auth_len, &auth_frame, &auth_consumed, &session);
    assert(auth_decode_result == ACP_OK);
    printf("  ✓ Decoder correctly accepted authenticated command\n");

    printf("  ✓ Decode rejection test passed\n");
}

static void test_frame_type_requirements(void)
{
    printf("Testing frame type authentication requirements...\n");

    /* Test the frame type requirement function directly */
    bool cmd_requires_auth = acp_frame_requires_auth(ACP_FRAME_TYPE_COMMAND);
    assert(cmd_requires_auth == true);
    printf("  ✓ Command frames require authentication: %s\n", cmd_requires_auth ? "YES" : "NO");

    bool telem_requires_auth = acp_frame_requires_auth(ACP_FRAME_TYPE_TELEMETRY);
    assert(telem_requires_auth == false);
    printf("  ✓ Telemetry frames require authentication: %s\n", telem_requires_auth ? "YES" : "NO");

    bool system_requires_auth = acp_frame_requires_auth(ACP_FRAME_TYPE_SYSTEM);
    assert(system_requires_auth == false);
    printf("  ✓ System frames require authentication: %s\n", system_requires_auth ? "YES" : "NO");

    /* Test invalid frame type */
    bool invalid_requires_auth = acp_frame_requires_auth(0xFF);
    assert(invalid_requires_auth == false);
    printf("  ✓ Invalid frame types require authentication: %s\n", invalid_requires_auth ? "YES" : "NO");

    printf("  ✓ Frame type requirements test passed\n");
}

static void test_mixed_authenticated_frames(void)
{
    printf("Testing mixed authenticated and unauthenticated frames...\n");

    acp_session_t session;
    uint8_t test_key[ACP_KEY_SIZE];
    for (int i = 0; i < ACP_KEY_SIZE; i++)
    {
        test_key[i] = (uint8_t)(i ^ 0xAA);
    }

    acp_result_t result = acp_session_init(&session, 99, test_key, ACP_KEY_SIZE, 0xFEDCBA9876543210ULL);
    assert(result == ACP_OK);

    /* Test combinations */
    struct
    {
        uint8_t frame_type;
        uint8_t flags;
        acp_session_t *session_ptr;
        int expected_encode_result;
        int expected_decode_result;
        const char *description;
    } test_cases[] = {
        {ACP_FRAME_TYPE_TELEMETRY, 0, NULL, ACP_OK, ACP_OK, "Unauth telemetry"},
        {ACP_FRAME_TYPE_TELEMETRY, ACP_FLAG_AUTHENTICATED, &session, ACP_OK, ACP_OK, "Auth telemetry"},
        {ACP_FRAME_TYPE_SYSTEM, 0, NULL, ACP_OK, ACP_OK, "Unauth system"},
        {ACP_FRAME_TYPE_SYSTEM, ACP_FLAG_AUTHENTICATED, &session, ACP_OK, ACP_OK, "Auth system"},
        {ACP_FRAME_TYPE_COMMAND, 0, NULL, ACP_ERR_AUTH_REQUIRED, -1, "Unauth command (encode fail)"},
        {ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED, &session, ACP_OK, ACP_OK, "Auth command"},
    };

    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++)
    {
        uint8_t payload[32];
        snprintf((char *)payload, sizeof(payload), "test_%zu", i);

        uint8_t output[256];
        size_t output_len = sizeof(output);

        /* Test encoding */
        acp_result_t encode_result = acp_encode_frame(test_cases[i].frame_type, test_cases[i].flags,
                                                      payload, strlen((char *)payload),
                                                      test_cases[i].session_ptr, output, &output_len);

        if (test_cases[i].expected_encode_result == ACP_ERR_AUTH_REQUIRED)
        {
            assert(encode_result == ACP_ERR_AUTH_REQUIRED);
            printf("    ✓ %s: encode rejected as expected\n", test_cases[i].description);
            continue; /* Skip decode test */
        }

        assert(encode_result == test_cases[i].expected_encode_result);

        /* Test decoding */
        acp_frame_t frame;
        size_t consumed;
        acp_result_t decode_result = acp_decode_frame(output, output_len, &frame, &consumed, test_cases[i].session_ptr);

        assert(decode_result == test_cases[i].expected_decode_result);
        printf("    ✓ %s: encode/decode succeeded\n", test_cases[i].description);
    }

    printf("  ✓ Mixed frame authentication test passed\n");
}

int main(void)
{
    printf("ACP Command Authentication Rejection Test\n");
    printf("=========================================\n\n");

    test_encode_rejects_unauth_commands();
    printf("\n");

    test_decode_rejects_unauth_commands();
    printf("\n");

    test_frame_type_requirements();
    printf("\n");

    test_mixed_authenticated_frames();
    printf("\n");

    printf("All command authentication rejection tests passed!\n");

    acp_cleanup();
    return 0;
}