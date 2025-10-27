/**
 * @file acp_session.h
 * @brief ACP Session Management - Authentication and Replay Protection
 * @version 0.3.0
 * @date 2025-10-27
 * 
 * This module manages ACP session state, sequence numbers, and replay protection.
 * It provides authentication context for secure frame processing.
 */

#ifndef ACP_SESSION_H
#define ACP_SESSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* Result type from protocol header */
#ifndef ACP_PROTOCOL_H
typedef int acp_result_t; /* Will be properly defined when acp_protocol.h is included */
#endif

/* ========================================================================== */
/*                              Constants                                     */
/* ========================================================================== */

/** @brief Authentication key size in bytes */
#define ACP_AUTH_KEY_SIZE 32

/** @brief Session nonce size in bytes */
#define ACP_NONCE_SIZE 16

/** @brief HMAC tag size in bytes (truncated) */
#define ACP_HMAC_SIZE 16

/** @brief Maximum replay window size (64 bits) */
#define ACP_REPLAY_WINDOW_SIZE 64

/* ========================================================================== */
/*                            Data Structures                                */
/* ========================================================================== */

/* Session type is defined in acp_protocol.h */

/**
 * @brief Session statistics for monitoring and debugging
 */
typedef struct {
    uint32_t session_id;        /**< Session identifier */
    uint32_t tx_seq;            /**< Current transmit sequence */
    uint32_t rx_seq;            /**< Current receive sequence */
    uint32_t created_time;      /**< Creation timestamp */
    uint8_t is_active;          /**< Active status */
    uint64_t replay_window;     /**< Current replay window */
    uint32_t replay_counter;    /**< Replay counter */
} acp_session_stats_t;

/* ========================================================================== */
/*                        Session Management Functions                       */
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
                              uint32_t session_id,
                              const uint8_t *key,
                              const uint8_t *nonce);

/**
 * @brief Get next sequence number for transmission
 * @param session Session context
 * @param seq_out Output sequence number
 * @return ACP_OK on success, error code on failure
 */
acp_result_t acp_session_get_tx_seq(acp_session_t *session, uint32_t *seq_out);

/**
 * @brief Validate received sequence number and update replay protection
 * @param session Session context  
 * @param rx_seq Received sequence number
 * @return ACP_OK if valid, ACP_ERR_REPLAY_ATTACK if replay detected
 */
acp_result_t acp_session_check_rx_seq(acp_session_t *session, uint32_t rx_seq);

/**
 * @brief Rotate session key and nonce
 * @param session Session context
 * @param new_key New authentication key (32 bytes)
 * @param new_nonce New session nonce (16 bytes)
 * @return ACP_OK on success, error code on failure
 */
acp_result_t acp_session_rotate(acp_session_t *session,
                                const uint8_t *new_key,
                                const uint8_t *new_nonce);

/**
 * @brief Terminate session and clear sensitive data
 * @param session Session context to terminate
 */
void acp_session_terminate(acp_session_t *session);

/**
 * @brief Check if session is expired based on time
 * @param session Session context
 * @param current_time Current system time
 * @param max_lifetime Maximum session lifetime in seconds
 * @return 1 if expired, 0 if still valid
 */
int acp_session_is_expired(const acp_session_t *session, 
                          uint32_t current_time,
                          uint32_t max_lifetime);

/* ========================================================================== */
/*                          Authentication Functions                         */
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
                                      uint8_t *hmac_out);

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
                                     const uint8_t *expected_hmac);

/* ========================================================================== */
/*                            Utility Functions                              */
/* ========================================================================== */

/**
 * @brief Get session statistics for monitoring
 * @param session Session context
 * @param stats Output statistics structure
 */
void acp_session_get_stats(const acp_session_t *session, acp_session_stats_t *stats);

/**
 * @brief Reset session replay protection (for testing/recovery)
 * @param session Session context
 */
void acp_session_reset_replay(acp_session_t *session);

#ifdef __cplusplus
}
#endif

#endif /* ACP_SESSION_H */