/**
 * @file test_replay_protection.c
 * @brief Comprehensive replay protection tests for ACP
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "acp_protocol.h"

static void test_basic_replay_rejection(void)
{
    printf("Testing basic replay rejection...\n");

    /* Initialize ACP */
    acp_result_t result = acp_init();
    assert(result == ACP_OK);

    /* Setup sessions */
    acp_session_t sender_session, receiver_session;
    uint8_t test_key[ACP_KEY_SIZE] = {
        0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11,
        0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

    result = acp_session_init(&sender_session, 1, test_key, ACP_KEY_SIZE, 0x1122334455667788ULL);
    assert(result == ACP_OK);

    result = acp_session_init(&receiver_session, 1, test_key, ACP_KEY_SIZE, 0x1122334455667788ULL);
    assert(result == ACP_OK);

    /* Encode first frame */
    uint8_t payload1[] = "first command";
    uint8_t output1[256];
    size_t output1_len = sizeof(output1);

    result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED,
                              payload1, sizeof(payload1) - 1,
                              &sender_session, output1, &output1_len);
    assert(result == ACP_OK);

    /* Decode first frame successfully */
    acp_frame_t frame1;
    size_t consumed1;
    result = acp_decode_frame(output1, output1_len, &frame1, &consumed1, &receiver_session);
    assert(result == ACP_OK);
    assert(frame1.sequence == 1);
    printf("  ✓ First frame (seq=%u) accepted\n", frame1.sequence);

    /* Try to replay the same frame - should be rejected */
    acp_frame_t replay_frame;
    size_t replay_consumed;
    result = acp_decode_frame(output1, output1_len, &replay_frame, &replay_consumed, &receiver_session);
    assert(result == ACP_ERR_REPLAY);
    printf("  ✓ Replayed frame (seq=%u) rejected with ACP_ERR_REPLAY\n", frame1.sequence);

    printf("  ✓ Basic replay rejection test passed\n");
}

static void test_sequence_ordering(void)
{
    printf("Testing sequence number ordering...\n");

    /* Setup sessions */
    acp_session_t sender_session, receiver_session;
    uint8_t test_key[ACP_KEY_SIZE];
    for (int i = 0; i < ACP_KEY_SIZE; i++)
    {
        test_key[i] = (uint8_t)(i * 7 + 13); /* Pseudo-random key */
    }

    acp_result_t result = acp_session_init(&sender_session, 2, test_key, ACP_KEY_SIZE, 0x9876543210FEDCBAULL);
    assert(result == ACP_OK);

    result = acp_session_init(&receiver_session, 2, test_key, ACP_KEY_SIZE, 0x9876543210FEDCBAULL);
    assert(result == ACP_OK);

    /* Send frames in order: seq 1, 2, 3 */
    for (int i = 1; i <= 3; i++)
    {
        uint8_t payload[32];
        snprintf((char *)payload, sizeof(payload), "command_%d", i);

        uint8_t output[256];
        size_t output_len = sizeof(output);

        result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED,
                                  payload, strlen((char *)payload),
                                  &sender_session, output, &output_len);
        assert(result == ACP_OK);

        acp_frame_t frame;
        size_t consumed;
        result = acp_decode_frame(output, output_len, &frame, &consumed, &receiver_session);
        assert(result == ACP_OK);
        assert(frame.sequence == (uint32_t)i);
        printf("  ✓ Frame sequence %d accepted in order\n", i);
    }

    /* Now try to send an out-of-order frame (seq 2) - should be rejected */
    sender_session.next_sequence = 2; /* Reset sender to simulate out-of-order */

    uint8_t old_payload[] = "old_command_2";
    uint8_t old_output[256];
    size_t old_output_len = sizeof(old_output);

    result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED,
                              old_payload, sizeof(old_payload) - 1,
                              &sender_session, old_output, &old_output_len);
    assert(result == ACP_OK);

    acp_frame_t old_frame;
    size_t old_consumed;
    result = acp_decode_frame(old_output, old_output_len, &old_frame, &old_consumed, &receiver_session);
    assert(result == ACP_ERR_REPLAY);
    printf("  ✓ Out-of-order frame (seq=%u) rejected\n", old_frame.sequence);

    printf("  ✓ Sequence ordering test passed\n");
}

static void test_replay_window(void)
{
    printf("Testing replay protection window...\n");

    /* Setup sessions */
    acp_session_t sender_session, receiver_session;
    uint8_t test_key[ACP_KEY_SIZE];
    memset(test_key, 0xA5, sizeof(test_key)); /* Pattern fill */

    acp_result_t result = acp_session_init(&sender_session, 3, test_key, ACP_KEY_SIZE, 0x1111111111111111ULL);
    assert(result == ACP_OK);

    result = acp_session_init(&receiver_session, 3, test_key, ACP_KEY_SIZE, 0x1111111111111111ULL);
    assert(result == ACP_OK);

    /* Send multiple frames to advance sequence numbers */
    uint8_t outputs[10][256];
    size_t output_lens[10];

    for (int i = 1; i <= 10; i++)
    {
        uint8_t payload[32];
        snprintf((char *)payload, sizeof(payload), "msg_%d", i);

        output_lens[i - 1] = sizeof(outputs[i - 1]);
        result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED,
                                  payload, strlen((char *)payload),
                                  &sender_session, outputs[i - 1], &output_lens[i - 1]);
        assert(result == ACP_OK);

        acp_frame_t frame;
        size_t consumed;
        result = acp_decode_frame(outputs[i - 1], output_lens[i - 1], &frame, &consumed, &receiver_session);
        assert(result == ACP_OK);
        assert(frame.sequence == (uint32_t)i);
        printf("  ✓ Frame %d (seq=%u) accepted\n", i, frame.sequence);
    }

    /* Now try to replay old frames - should all be rejected */
    for (int i = 1; i <= 10; i++)
    {
        acp_frame_t replay_frame;
        size_t replay_consumed;
        result = acp_decode_frame(outputs[i - 1], output_lens[i - 1], &replay_frame, &replay_consumed, &receiver_session);
        assert(result == ACP_ERR_REPLAY);
        printf("  ✓ Replay of frame %d (seq=%u) rejected\n", i, replay_frame.sequence);
    }

    printf("  ✓ Replay protection window test passed\n");
}

static void test_mixed_auth_frames(void)
{
    printf("Testing mixed authenticated and unauthenticated frames...\n");

    /* Setup session for authenticated frames */
    acp_session_t auth_session;
    uint8_t test_key[ACP_KEY_SIZE];
    for (int i = 0; i < ACP_KEY_SIZE; i++)
    {
        test_key[i] = (uint8_t)(0xFF - i);
    }

    acp_result_t result = acp_session_init(&auth_session, 4, test_key, ACP_KEY_SIZE, 0xCAFEBABEDEADBEEFULL);
    assert(result == ACP_OK);

    /* Send unauthenticated telemetry frame - should work */
    uint8_t telem_payload[] = "sensor_data_123";
    uint8_t telem_output[256];
    size_t telem_output_len = sizeof(telem_output);

    result = acp_encode_frame(ACP_FRAME_TYPE_TELEMETRY, 0, /* No auth */
                              telem_payload, sizeof(telem_payload) - 1,
                              NULL, telem_output, &telem_output_len);
    assert(result == ACP_OK);

    acp_frame_t telem_frame;
    size_t telem_consumed;
    result = acp_decode_frame(telem_output, telem_output_len, &telem_frame, &telem_consumed, NULL);
    assert(result == ACP_OK);
    printf("  ✓ Unauthenticated telemetry frame accepted\n");

    /* Send authenticated command frame - should work */
    uint8_t cmd_payload[] = "authenticated_command";
    uint8_t cmd_output[256];
    size_t cmd_output_len = sizeof(cmd_output);

    result = acp_encode_frame(ACP_FRAME_TYPE_COMMAND, ACP_FLAG_AUTHENTICATED,
                              cmd_payload, sizeof(cmd_payload) - 1,
                              &auth_session, cmd_output, &cmd_output_len);
    assert(result == ACP_OK);

    acp_frame_t cmd_frame;
    size_t cmd_consumed;
    result = acp_decode_frame(cmd_output, cmd_output_len, &cmd_frame, &cmd_consumed, &auth_session);
    assert(result == ACP_OK);
    assert(cmd_frame.sequence == 1);
    printf("  ✓ Authenticated command frame accepted\n");

    /* Try to replay the authenticated command - should be rejected */
    acp_frame_t replay_cmd_frame;
    size_t replay_cmd_consumed;
    result = acp_decode_frame(cmd_output, cmd_output_len, &replay_cmd_frame, &replay_cmd_consumed, &auth_session);
    assert(result == ACP_ERR_REPLAY);
    printf("  ✓ Replayed authenticated command rejected\n");

    /* Unauthenticated frames can be replayed (no sequence numbers) */
    acp_frame_t replay_telem_frame;
    size_t replay_telem_consumed;
    result = acp_decode_frame(telem_output, telem_output_len, &replay_telem_frame, &replay_telem_consumed, NULL);
    assert(result == ACP_OK);
    printf("  ✓ Unauthenticated telemetry replay allowed (no sequence tracking)\n");

    printf("  ✓ Mixed authentication frames test passed\n");
}

int main(void)
{
    printf("ACP Replay Protection Tests\n");
    printf("===========================\n\n");

    test_basic_replay_rejection();
    printf("\n");

    test_sequence_ordering();
    printf("\n");

    test_replay_window();
    printf("\n");

    test_mixed_auth_frames();
    printf("\n");

    printf("All replay protection tests passed!\n");

    acp_cleanup();
    return 0;
}