/**
 * @file test_conditional_sequence.c
 * @brief Test conditional sequence field in ACP frames
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "acp_protocol.h"

static void test_unauthenticated_frame(void)
{
    printf("Testing unauthenticated frame (no sequence field)...\n");

    acp_frame_t frame = {0};
    frame.version = ACP_PROTOCOL_VERSION;
    frame.type = ACP_FRAME_TYPE_TELEMETRY;
    frame.flags = 0;        /* No ACP_FLAG_AUTHENTICATED */
    frame.sequence = 12345; /* Should be ignored */
    frame.length = 4;
    memcpy(frame.payload, "test", 4);

    uint8_t encoded[256];
    size_t encoded_len;

    int result = acp_frame_encode(&frame, encoded, sizeof(encoded), &encoded_len);
    assert(result == ACP_OK);

    printf("  Encoded frame: %zu bytes\n", encoded_len);

    /* Decode and verify */
    acp_frame_t decoded_frame = {0};
    size_t consumed;

    result = acp_frame_decode(encoded, encoded_len, &decoded_frame, &consumed);
    assert(result == ACP_OK);
    assert(consumed == encoded_len);

    /* Verify frame contents */
    assert(decoded_frame.version == frame.version);
    assert(decoded_frame.type == frame.type);
    assert(decoded_frame.flags == frame.flags);
    assert(decoded_frame.length == frame.length);
    assert(decoded_frame.sequence == 0); /* Should be 0 for unauthenticated frames */
    assert(memcmp(decoded_frame.payload, frame.payload, frame.length) == 0);

    printf("  ✓ Unauthenticated frame test passed\n");
}

static void test_authenticated_frame(void)
{
    printf("Testing authenticated frame (with sequence field)...\n");

    acp_frame_t frame = {0};
    frame.version = ACP_PROTOCOL_VERSION;
    frame.type = ACP_FRAME_TYPE_COMMAND;
    frame.flags = ACP_FLAG_AUTHENTICATED;
    frame.sequence = 0x12345678; /* Should be preserved */
    frame.length = 6;
    memcpy(frame.payload, "secure", 6);

    uint8_t encoded[256];
    size_t encoded_len;

    int result = acp_frame_encode(&frame, encoded, sizeof(encoded), &encoded_len);
    assert(result == ACP_OK);

    printf("  Encoded frame: %zu bytes\n", encoded_len);

    /* Decode and verify */
    acp_frame_t decoded_frame = {0};
    size_t consumed;

    result = acp_frame_decode(encoded, encoded_len, &decoded_frame, &consumed);
    assert(result == ACP_OK);
    assert(consumed == encoded_len);

    /* Verify frame contents */
    assert(decoded_frame.version == frame.version);
    assert(decoded_frame.type == frame.type);
    assert(decoded_frame.flags == frame.flags);
    assert(decoded_frame.length == frame.length);
    assert(decoded_frame.sequence == frame.sequence); /* Should be preserved */
    assert(memcmp(decoded_frame.payload, frame.payload, frame.length) == 0);

    printf("  ✓ Authenticated frame test passed\n");
}

static void test_header_size_calculation(void)
{
    printf("Testing header size calculation...\n");

    /* Unauthenticated frame should have smaller header */
    size_t unauth_size = acp_wire_header_size(0);
    printf("  Unauthenticated header size: %zu bytes\n", unauth_size);
    assert(unauth_size == sizeof(acp_wire_header_base_t));

    /* Authenticated frame should have larger header */
    size_t auth_size = acp_wire_header_size(ACP_FLAG_AUTHENTICATED);
    printf("  Authenticated header size: %zu bytes\n", auth_size);
    assert(auth_size == sizeof(acp_wire_header_base_t) + sizeof(uint32_t));

    /* Difference should be sequence field size */
    assert(auth_size - unauth_size == sizeof(uint32_t));

    printf("  ✓ Header size calculation test passed\n");
}

int main(void)
{
    printf("ACP Conditional Sequence Field Test\n");
    printf("===================================\n\n");

    test_header_size_calculation();
    printf("\n");

    test_unauthenticated_frame();
    printf("\n");

    test_authenticated_frame();
    printf("\n");

    printf("All conditional sequence field tests passed!\n");
    return 0;
}