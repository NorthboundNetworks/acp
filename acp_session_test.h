/**
 * @file acp_session_test.h
 * @brief Session helper functions for testing
 */

#ifndef ACP_SESSION_TEST_H
#define ACP_SESSION_TEST_H

#include "acp_protocol.h"

/* Additional session functions not in main API */
acp_result_t acp_session_get_tx_seq(acp_session_t *session, uint32_t *seq_out);
acp_result_t acp_session_check_rx_seq(acp_session_t *session, uint32_t rx_seq);
void acp_session_terminate(acp_session_t *session);
int acp_session_is_initialized(const acp_session_t *session);

#endif /* ACP_SESSION_TEST_H */