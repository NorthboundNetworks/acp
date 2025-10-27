/**
 * @file acp_session.c
 * @brief ACP Session Management - Authentication and Replay Protection
 * @version 0.3.0
 * @date 2025-10-27
 *
 * This module manages ACP session state, sequence numbers, and replay protection.
 * It provides authentication context for secure frame processing.
 */

#include "acp_protocol.h" /* Contains session types and declarations */
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
                              size_t key_len,
                              uint64_t nonce)
{
    if (!session || !key || key_len == 0)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    /* Initialize session state */
    memset(session, 0, sizeof(acp_session_t));

    session->key_id = key_id;
    session->next_sequence = 1; /* Start at 1, 0 is reserved for unauthenticated */
    session->last_accepted_seq = 0;
    session->initialized = true;

    /* Copy authentication key (truncate to 32 bytes if needed) */
    size_t copy_len = (key_len > 32) ? 32 : key_len;
    memcpy(session->key, key, copy_len);
    session->nonce = nonce;

    return ACP_OK;
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
                                size_t new_key_len,
                                uint64_t new_nonce)
{
    if (!session)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    if (!session->initialized)
    {
        return ACP_ERR_SESSION_NOT_INIT;
    }

    /* Clear old key material */
    acp_crypto_clear(session->key, sizeof(session->key));

    /* Install new key material if provided */
    if (new_key && new_key_len > 0)
    {
        size_t copy_len = (new_key_len > 32) ? 32 : new_key_len;
        memcpy(session->key, new_key, copy_len);
    }

    session->nonce = new_nonce;

    /* Reset sequence numbers */
    session->next_sequence = 1;
    session->last_accepted_seq = 0;

    return ACP_OK;
}

/**
 * @brief Terminate session and clear sensitive data
 * @param session Session context to terminate
 */
void acp_session_terminate(acp_session_t *session)
{
    if (!session)
        return;

    session->initialized = false;

    /* Clear sensitive key material */
    acp_crypto_clear(session->key, sizeof(session->key));

    /* Clear session state */
    session->key_id = 0;
    session->next_sequence = 0;
    session->last_accepted_seq = 0;
    session->nonce = 0;
    session->policy_flags = 0;
}

/**
 * @brief Check if session is expired (simplified)
 * @param session Session context
 * @return 1 if expired, 0 if still valid
 */
int acp_session_is_expired(const acp_session_t *session)
{
    if (!session || !session->initialized)
    {
        return 1; /* Uninitialized sessions are expired */
    }

    return 0; /* Simple implementation - sessions don't expire */
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
    if (!session || !frame_data || !hmac_out)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    if (!session->initialized)
    {
        return ACP_ERR_SESSION_NOT_INIT;
    }

    /* Compute HMAC-SHA256 truncated to 16 bytes */
    acp_hmac_sha256(session->key, 32, frame_data, frame_len, hmac_out);

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
    if (!session || !frame_data || !expected_hmac)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    if (!session->initialized)
    {
        return ACP_ERR_SESSION_NOT_INIT;
    }

    uint8_t computed_hmac[ACP_HMAC_SIZE];

    /* Compute HMAC for comparison */
    acp_result_t result = acp_session_compute_hmac(session, frame_data,
                                                   frame_len, computed_hmac);
    if (result != ACP_OK)
    {
        return result;
    }

    /* Constant-time comparison to prevent timing attacks */
    if (acp_hmac_verify(expected_hmac, computed_hmac, ACP_HMAC_SIZE) == 0)
    {
        return ACP_OK;
    }

    return ACP_ERR_AUTH_FAILED;
}

/* ========================================================================== */
/*                            Session Utilities                              */
/* ========================================================================== */

/* ========================================================================== */
/*                            Additional Functions                            */
/* ========================================================================== */

/**
 * @brief Get next sequence number for transmission
 */
acp_result_t acp_session_get_tx_seq(acp_session_t *session, uint32_t *seq_out)
{
    if (!session || !seq_out)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    if (!session->initialized)
    {
        return ACP_ERR_SESSION_NOT_INIT;
    }

    *seq_out = session->next_sequence;
    session->next_sequence++;

    /* Check for sequence number rollover */
    if (session->next_sequence == 0)
    {
        session->next_sequence = 1; /* Skip 0 */
    }

    return ACP_OK;
}

/**
 * @brief Simple sequence validation without full replay protection
 */
acp_result_t acp_session_check_rx_seq(acp_session_t *session, uint32_t rx_seq)
{
    if (!session)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    if (!session->initialized)
    {
        return ACP_ERR_SESSION_NOT_INIT;
    }

    /* Sequence 0 is reserved for unauthenticated frames */
    if (rx_seq == 0)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    /* Simple forward progression check */
    if (rx_seq <= session->last_accepted_seq)
    {
        return ACP_ERR_REPLAY; /* Replay or out-of-order */
    }

    session->last_accepted_seq = rx_seq;
    return ACP_OK;
}

/**
 * @brief Check if session is initialized
 */
int acp_session_is_initialized(const acp_session_t *session)
{
    return (session && session->initialized) ? 1 : 0;
}
