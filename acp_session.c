/**
 * @file acp_session.c
 * @brief ACP Session Management - Authentication and Replay Protection
 * @version 0.3.0
 * @date 2025-10-27
 * 
 * This module manages ACP session state, sequence numbers, and replay protection.
 * It provides authentication context for secure frame processing.
 */

#include "acp_protocol.h"  /* Must be first for acp_result_t */
#include "acp_session.h"
#include "acp_crypto.h"
#include <string.h>

/* ========================================================================== */
/*                            Session Management                             */
/* ========================================================================== */

/**
 * @brief Initialize a new ACP session
 * @param session Session context to initialize
 * @param session_id Unique session identifier
 * @param key Authentication key (32 bytes)
 * @param nonce Session nonce (16 bytes)
 * @return ACP_OK on success, error code on failure
 */
acp_result_t acp_session_init(acp_session_t *session, 
                              uint32_t key_id,
                              const uint8_t *key,
                              const uint8_t *nonce)
{
    if (!session || !key || !nonce) {
        return ACP_ERR_INVALID_PARAM;
    }

    /* Initialize session state */
    memset(session, 0, sizeof(acp_session_t));
    
    session->key_id = key_id;
    session->next_sequence = 1;  /* Start at 1, 0 is reserved for unauthenticated */
    session->last_accepted_seq = 0;
    session->initialized = true;
    
    /* Copy authentication key (use existing 32-byte key field) */
    memcpy(session->key, key, 32);
    session->nonce = *(const uint64_t*)nonce;  /* Use first 8 bytes of nonce */
    
    return ACP_OK;
}

/**
 * @brief Get next sequence number for transmission
 * @param session Session context
 * @param seq_out Output sequence number
 * @return ACP_OK on success, error code on failure
 */
acp_result_t acp_session_get_tx_seq(acp_session_t *session, uint32_t *seq_out)
{
    if (!session || !seq_out) {
        return ACP_ERR_INVALID_PARAM;
    }
    
    if (!session->initialized) {
        return ACP_ERR_SESSION_NOT_INIT;
    }
    
    *seq_out = session->next_sequence;
    session->next_sequence++;
    
    /* Check for sequence number rollover */
    if (session->next_sequence == 0) {
        session->next_sequence = 1; /* Skip 0 */
        /* TODO: Consider session rotation on rollover */
    }
    
    return ACP_OK;
}

/**
 * @brief Validate received sequence number and update replay protection
 * @param session Session context  
 * @param rx_seq Received sequence number
 * @return ACP_OK if valid, ACP_ERR_REPLAY_ATTACK if replay detected
 */
acp_result_t acp_session_check_rx_seq(acp_session_t *session, uint32_t rx_seq)
{
    if (!session) {
        return ACP_ERR_INVALID_PARAM;
    }
    
    if (!session->is_active) {
        return ACP_ERR_SESSION_EXPIRED;
    }
    
    /* Sequence 0 is reserved for unauthenticated frames */
    if (rx_seq == 0) {
        return ACP_ERR_INVALID_PARAM;
    }
    
    /* First authenticated frame in session */
    if (session->rx_seq == 0) {
        session->rx_seq = rx_seq;
        session->replay_window = 1ULL << 0; /* Mark bit 0 as received */
        session->replay_counter = rx_seq;
        return ACP_OK;
    }
    
    /* Check if sequence is too old (outside replay window) */
    if (rx_seq < session->replay_counter && 
        (session->replay_counter - rx_seq) > 64) {
        return ACP_ERR_REPLAY_ATTACK;
    }
    
    /* Check if sequence is in the future */
    if (rx_seq > session->replay_counter) {
        /* Advance window */
        uint32_t shift = rx_seq - session->replay_counter;
        if (shift >= 64) {
            session->replay_window = 1; /* Only current bit set */
        } else {
            session->replay_window <<= shift;
            session->replay_window |= 1; /* Set current bit */
        }
        session->replay_counter = rx_seq;
        session->rx_seq = rx_seq;
        return ACP_OK;
    }
    
    /* Check if sequence is within current window */
    if (rx_seq <= session->replay_counter) {
        uint32_t bit_pos = session->replay_counter - rx_seq;
        if (bit_pos >= 64) {
            return ACP_ERR_REPLAY_ATTACK; /* Too old */
        }
        
        uint64_t bit_mask = 1ULL << bit_pos;
        if (session->replay_window & bit_mask) {
            return ACP_ERR_REPLAY_ATTACK; /* Already received */
        }
        
        /* Mark as received */
        session->replay_window |= bit_mask;
        return ACP_OK;
    }
    
    return ACP_ERR_REPLAY_ATTACK;
}

/**
 * @brief Rotate session key and nonce
 * @param session Session context
 * @param new_key New authentication key (32 bytes)
 * @param new_nonce New session nonce (16 bytes)
 * @return ACP_OK on success, error code on failure
 */
acp_result_t acp_session_rotate(acp_session_t *session,
                                const uint8_t *new_key,
                                const uint8_t *new_nonce)
{
    if (!session || !new_key || !new_nonce) {
        return ACP_ERR_INVALID_PARAM;
    }
    
    if (!session->is_active) {
        return ACP_ERR_SESSION_EXPIRED;
    }
    
    /* Clear old key material */
    acp_crypto_clear(session->auth_key, sizeof(session->auth_key));
    acp_crypto_clear(session->nonce, sizeof(session->nonce));
    
    /* Install new key material */
    memcpy(session->auth_key, new_key, ACP_AUTH_KEY_SIZE);
    memcpy(session->nonce, new_nonce, ACP_NONCE_SIZE);
    
    /* Reset sequence numbers and replay protection */
    session->tx_seq = 1;
    session->rx_seq = 0;
    session->replay_window = 0;
    session->replay_counter = 0;
    
    return ACP_OK;
}

/**
 * @brief Terminate session and clear sensitive data
 * @param session Session context to terminate
 */
void acp_session_terminate(acp_session_t *session)
{
    if (!session) return;
    
    session->is_active = 0;
    
    /* Clear sensitive key material */
    acp_crypto_clear(session->auth_key, sizeof(session->auth_key));
    acp_crypto_clear(session->nonce, sizeof(session->nonce));
    
    /* Clear session state */
    session->session_id = 0;
    session->tx_seq = 0;
    session->rx_seq = 0;
    session->replay_window = 0;
    session->replay_counter = 0;
    session->created_time = 0;
}

/**
 * @brief Check if session is expired based on time
 * @param session Session context
 * @param current_time Current system time
 * @param max_lifetime Maximum session lifetime in seconds
 * @return 1 if expired, 0 if still valid
 */
int acp_session_is_expired(const acp_session_t *session, 
                          uint32_t current_time,
                          uint32_t max_lifetime)
{
    if (!session || !session->is_active) {
        return 1; /* Inactive sessions are expired */
    }
    
    if (max_lifetime == 0) {
        return 0; /* No expiration */
    }
    
    if (current_time < session->created_time) {
        return 1; /* Time went backwards, assume expired */
    }
    
    return (current_time - session->created_time) > max_lifetime;
}

/* ========================================================================== */
/*                          Authentication Helpers                           */
/* ========================================================================== */

/**
 * @brief Compute HMAC for frame authentication
 * @param session Session context containing auth key
 * @param frame_data Frame data to authenticate
 * @param frame_len Length of frame data
 * @param hmac_out Output HMAC buffer (16 bytes)
 * @return ACP_OK on success, error code on failure
 */
acp_result_t acp_session_compute_hmac(const acp_session_t *session,
                                      const uint8_t *frame_data,
                                      size_t frame_len,
                                      uint8_t *hmac_out)
{
    if (!session || !frame_data || !hmac_out) {
        return ACP_ERR_INVALID_PARAM;
    }
    
    if (!session->is_active) {
        return ACP_ERR_SESSION_EXPIRED;
    }
    
    /* Compute HMAC-SHA256 truncated to 16 bytes */
    acp_hmac_sha256(session->auth_key, ACP_AUTH_KEY_SIZE,
                    frame_data, frame_len, hmac_out);
    
    return ACP_OK;
}

/**
 * @brief Verify HMAC for frame authentication
 * @param session Session context containing auth key
 * @param frame_data Frame data to verify
 * @param frame_len Length of frame data
 * @param expected_hmac Expected HMAC value (16 bytes)
 * @return ACP_OK if verification succeeds, error code on failure
 */
acp_result_t acp_session_verify_hmac(const acp_session_t *session,
                                     const uint8_t *frame_data,
                                     size_t frame_len,
                                     const uint8_t *expected_hmac)
{
    if (!session || !frame_data || !expected_hmac) {
        return ACP_ERR_INVALID_PARAM;
    }
    
    if (!session->is_active) {
        return ACP_ERR_SESSION_EXPIRED;
    }
    
    uint8_t computed_hmac[ACP_HMAC_SIZE];
    
    /* Compute HMAC for comparison */
    acp_result_t result = acp_session_compute_hmac(session, frame_data, 
                                                   frame_len, computed_hmac);
    if (result != ACP_OK) {
        return result;
    }
    
    /* Constant-time comparison to prevent timing attacks */
    if (acp_hmac_verify(expected_hmac, computed_hmac, ACP_HMAC_SIZE) == 0) {
        return ACP_OK;
    }
    
    return ACP_ERR_AUTH_FAILED;
}

/* ========================================================================== */
/*                            Session Utilities                              */
/* ========================================================================== */

/**
 * @brief Get session statistics for monitoring
 * @param session Session context
 * @param stats Output statistics structure
 */
void acp_session_get_stats(const acp_session_t *session, acp_session_stats_t *stats)
{
    if (!session || !stats) return;
    
    memset(stats, 0, sizeof(acp_session_stats_t));
    
    stats->session_id = session->session_id;
    stats->tx_seq = session->tx_seq;
    stats->rx_seq = session->rx_seq;
    stats->is_active = session->is_active;
    stats->created_time = session->created_time;
    stats->replay_window = session->replay_window;
    stats->replay_counter = session->replay_counter;
}

/**
 * @brief Reset session replay protection (for testing/recovery)
 * @param session Session context
 */
void acp_session_reset_replay(acp_session_t *session)
{
    if (!session) return;
    
    session->rx_seq = 0;
    session->replay_window = 0;
    session->replay_counter = 0;
}