/**
 * @file test_session.c
 * @brief Test program for ACP session management
 */

#include "acp_session.h"
#include "acp_errors.h"
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
    printf("ACP Session Management Test\n");
    printf("==========================\n\n");

    /* Test data */
    const uint8_t test_key[32] = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
        0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
        0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20
    };
    
    const uint8_t test_nonce[16] = {
        0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8,
        0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8
    };

    acp_session_t session;
    acp_result_t result;

    /* Test 1: Session initialization */
    printf("Test 1: Session Initialization\n");
    result = acp_session_init(&session, 0x12345678, test_key, test_nonce);
    printf("Session init: %s\n", result == ACP_OK ? "PASS" : "FAIL");
    printf("Session ID: 0x%08x\n", session.session_id);
    printf("Active: %s\n", session.is_active ? "YES" : "NO");
    print_hex("Auth Key", session.auth_key, 32);
    print_hex("Nonce", session.nonce, 16);
    printf("\n");

    /* Test 2: Sequence number management */
    printf("Test 2: Sequence Number Management\n");
    uint32_t seq1, seq2, seq3;
    
    result = acp_session_get_tx_seq(&session, &seq1);
    printf("First TX seq: %u (%s)\n", seq1, result == ACP_OK ? "PASS" : "FAIL");
    
    result = acp_session_get_tx_seq(&session, &seq2);
    printf("Second TX seq: %u (%s)\n", seq2, result == ACP_OK ? "PASS" : "FAIL");
    
    result = acp_session_get_tx_seq(&session, &seq3);
    printf("Third TX seq: %u (%s)\n", seq3, result == ACP_OK ? "PASS" : "FAIL");
    
    printf("Sequence progression: %s\n", 
           (seq2 == seq1 + 1 && seq3 == seq2 + 1) ? "PASS" : "FAIL");
    printf("\n");

    /* Test 3: Replay protection */
    printf("Test 3: Replay Protection\n");
    
    /* First frame should be accepted */
    result = acp_session_check_rx_seq(&session, 100);
    printf("First RX seq 100: %s\n", result == ACP_OK ? "PASS (accepted)" : "FAIL (rejected)");
    
    /* Higher sequence should be accepted */
    result = acp_session_check_rx_seq(&session, 105);
    printf("Higher RX seq 105: %s\n", result == ACP_OK ? "PASS (accepted)" : "FAIL (rejected)");
    
    /* Replay of same sequence should be rejected */
    result = acp_session_check_rx_seq(&session, 105);
    printf("Replay seq 105: %s\n", result == ACP_ERR_REPLAY_ATTACK ? "PASS (rejected)" : "FAIL (accepted)");
    
    /* Within window should be accepted once */
    result = acp_session_check_rx_seq(&session, 102);
    printf("Within window seq 102: %s\n", result == ACP_OK ? "PASS (accepted)" : "FAIL (rejected)");
    
    /* Replay within window should be rejected */
    result = acp_session_check_rx_seq(&session, 102);
    printf("Replay within window seq 102: %s\n", result == ACP_ERR_REPLAY_ATTACK ? "PASS (rejected)" : "FAIL (accepted)");
    
    /* Too old sequence should be rejected */
    result = acp_session_check_rx_seq(&session, 10);
    printf("Too old seq 10: %s\n", result == ACP_ERR_REPLAY_ATTACK ? "PASS (rejected)" : "FAIL (accepted)");
    printf("\n");

    /* Test 4: Session statistics */
    printf("Test 4: Session Statistics\n");
    acp_session_stats_t stats;
    acp_session_get_stats(&session, &stats);
    
    printf("Stats - Session ID: 0x%08x\n", stats.session_id);
    printf("Stats - TX seq: %u\n", stats.tx_seq);
    printf("Stats - RX seq: %u\n", stats.rx_seq);
    printf("Stats - Active: %s\n", stats.is_active ? "YES" : "NO");
    printf("Stats - Replay window: 0x%016llx\n", (unsigned long long)stats.replay_window);
    printf("Stats - Replay counter: %u\n", stats.replay_counter);
    printf("\n");

    /* Test 5: Session rotation */
    printf("Test 5: Session Rotation\n");
    const uint8_t new_key[32] = {
        0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
        0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38,
        0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40
    };
    
    const uint8_t new_nonce[16] = {
        0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8,
        0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8
    };
    
    result = acp_session_rotate(&session, new_key, new_nonce);
    printf("Session rotate: %s\n", result == ACP_OK ? "PASS" : "FAIL");
    
    uint32_t rotated_seq;
    result = acp_session_get_tx_seq(&session, &rotated_seq);
    printf("First seq after rotate: %u (%s)\n", rotated_seq, 
           (result == ACP_OK && rotated_seq == 1) ? "PASS" : "FAIL");
    
    print_hex("New Auth Key", session.auth_key, 32);
    print_hex("New Nonce", session.nonce, 16);
    printf("\n");

    /* Test 6: Session termination */
    printf("Test 6: Session Termination\n");
    acp_session_terminate(&session);
    printf("Session terminated\n");
    printf("Active after termination: %s\n", 
           session.is_active ? "FAIL (still active)" : "PASS (inactive)");
    printf("Session ID cleared: %s\n", 
           session.session_id == 0 ? "PASS" : "FAIL");
    
    /* Check that key material was cleared */
    int key_cleared = 1;
    for (int i = 0; i < 32; i++) {
        if (session.auth_key[i] != 0) {
            key_cleared = 0;
            break;
        }
    }
    printf("Auth key cleared: %s\n", key_cleared ? "PASS" : "FAIL");

    printf("\nAll session management tests completed!\n");
    return 0;
}