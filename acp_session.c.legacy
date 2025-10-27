/*
 * Autonomous Command Protocol (ACP)
 * Reference C Implementation
 *
 * Copyright (c) 2025 Northbound Networks
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
 * OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file acp_session.c
 * @brief ACP Session Management Implementation
 *
 * Manages operator sessions with role-based access control, rate limiting,
 * and cryptographic authentication for the Autonomous Control Protocol (ACP) v0.3.
 *
 * Provides NVS storage integration for ACP session keys, configuration,
 * and persistent state management.
 *
 * @version 0.3
 * @date 2025-10-26
 * @author Paul Zanna
 */

#include <string.h>
#include <stddef.h>
#include "acp_platform_log.h"
#include "acp_platform_mutex.h"
#include "acp_platform_time.h"
#include "acp_session.h"
#include "acp_crypto.h"

/**
 * @file acp_session.c
 * @brief ACP Session Management Implementation
 *
 * Manages operator sessions with role-based access control, rate limiting,
 * and cryptographic authentication for the Secure Control Protocol (ACP) v1.1.
 *
 * @author Scout ISR Project
 * @version 1.1
 * @date 2025-10-26
 */

// ===========================================================================
// GLOBAL STATE AND SYNCHRONIZATION
// ===========================================================================

/**
 * @brief Session manager global state
 */
static acp_session_manager_t session_manager;
static void *session_mutex = NULL;
static bool session_system_initialized = false;

// Session statistics
static acp_session_stats_t global_stats = {0};

// ===========================================================================
// INTERNAL HELPER FUNCTIONS
// ===========================================================================

/**
 * @brief Get current system time in milliseconds
 */
static uint32_t get_current_time_ms(void)
{
    return (uint32_t)(acp_platform_get_time_ms());
}

/**
 * @brief Find session by ID (must hold mutex)
 */
static acp_session_t *find_session_by_id(uint32_t session_id)
{
    if (session_id == ACP_SESSION_ID_INVALID)
    {
        return NULL;
    }

    for (uint8_t i = 0; i < ACP_MAX_SESSIONS; i++)
    {
        if (session_manager.sessions[i].session_id == session_id &&
            session_manager.sessions[i].state != ACP_SESSION_STATE_INACTIVE)
        {
            return &session_manager.sessions[i];
        }
    }

    return NULL;
}

/**
 * @brief Find free session slot (must hold mutex)
 */
static acp_session_t *find_free_session_slot(void)
{
    for (uint8_t i = 0; i < ACP_MAX_SESSIONS; i++)
    {
        if (session_manager.sessions[i].state == ACP_SESSION_STATE_INACTIVE)
        {
            return &session_manager.sessions[i];
        }
    }
    return NULL;
}

/**
 * @brief Initialize session structure
 */
static void init_session(acp_session_t *session,
                         uint32_t session_id,
                         uint8_t operator_role,
                         uint8_t connection_type,
                         const uint8_t *auth_key)
{

    memset(session, 0, sizeof(acp_session_t));

    uint32_t current_time = get_current_time_ms();

    session->session_id = session_id;
    session->state = ACP_SESSION_STATE_ACTIVE;
    session->operator_role = operator_role;
    session->connection_type = connection_type;

    memcpy(session->auth_key, auth_key, ACP_SESSION_KEY_SIZE);
    session->last_seq_num = 0;

    session->created_time = current_time;
    session->last_activity = current_time;
    session->expires_at = current_time + ACP_SESSION_TIMEOUT;

    // Initialize rate limiting
    session->rate_limit.command_count = 0;
    session->rate_limit.window_start = current_time;
    session->rate_limit.burst_used = 0;
    session->rate_limit.penalty_until = 0;

    // Initialize statistics
    session->messages_received = 0;
    session->commands_executed = 0;
    session->auth_failures = 0;
    session->rate_limit_hits = 0;
    ACP_LOG_INFO("Initialized session %u for role %u", session_id, operator_role);
}

/**
 * @brief Check if sequence number is valid (replay protection)
 */
static bool is_sequence_valid(acp_session_t *session, uint8_t seq_num)
{
    // For simplicity, we use a simple forward-only sequence check
    // In production, a sliding window approach would be more robust

    if (seq_num == session->last_seq_num)
    {
        // Duplicate sequence number
        ACP_LOG_WARN("Duplicate sequence number %u for session %u",
                     seq_num, session->session_id);
        return false;
    }

    // Allow sequence number wrapping (0-255)
    uint8_t expected_min = (session->last_seq_num + 1) & 0xFF;
    uint8_t expected_max = (session->last_seq_num + 10) & 0xFF; // Allow up to 10 ahead

    if (expected_max > expected_min)
    {
        // No wrap-around
        if (seq_num >= expected_min && seq_num <= expected_max)
        {
            return true;
        }
    }
    else
    {
        // Wrap-around case
        if (seq_num >= expected_min || seq_num <= expected_max)
        {
            return true;
        }
    }

    ESP_LOGW(TAG, "Invalid sequence number %u for session %u (last: %u)",
             seq_num, session->session_id, session->last_seq_num);
    ACP_LOG_WARN("Invalid sequence number %u for session %u (last: %u)",
                 seq_num, session->session_id, session->last_seq_num);
    return false;
}

// ===========================================================================
// RATE LIMITING IMPLEMENTATION
// ===========================================================================

/**
 * @brief Update rate limiting state
 */
static bool update_rate_limit(acp_session_t *session, uint32_t current_time)
{
    acp_rate_limit_t *rl = &session->rate_limit;

    // Check if we're in penalty timeout
    if (rl->penalty_until > current_time)
    {
        ACP_LOG_DEBUG("Session %u in penalty timeout", session->session_id);
        return false;
    }

    // Reset penalty state if expired
    if (rl->penalty_until > 0 && current_time >= rl->penalty_until)
    {
        rl->penalty_until = 0;
        rl->command_count = 0;
        rl->burst_used = 0;
        ACP_LOG_INFO("Penalty timeout expired for session %u", session->session_id);
    }

    // Check if we need to start a new rate window
    uint32_t window_age = current_time - rl->window_start;
    if (window_age >= 1000)
    { // 1 second window
        rl->window_start = current_time;
        rl->command_count = 0;
        // Don't reset burst_used - it accumulates over burst window
    }

    // Check burst window reset
    if (window_age >= ACP_RATE_BURST_WINDOW)
    {
        rl->burst_used = 0;
    }

    // Check sustained rate limit
    if (rl->command_count >= ACP_RATE_SUSTAINED_CPS)
    {
        // Check if we can use burst allowance
        if (rl->burst_used >= ACP_RATE_BURST_CPS)
        {
            // Rate limit exceeded - apply penalty
            rl->penalty_until = current_time + ACP_RATE_PENALTY_TIME;
            session->rate_limit_hits++;
            global_stats.rate_limit_events++;

            ESP_LOGW(TAG, "Rate limit exceeded for session %u (penalty: %u ms)",
                     session->session_id, ACP_RATE_PENALTY_TIME);
            ACP_LOG_WARN("Rate limit exceeded for session %u (penalty: %u ms)",
                         session->session_id, ACP_RATE_PENALTY_TIME);
            return false;
        }
    }

    // Allow command - update counters
    rl->command_count++;
    if (rl->command_count > ACP_RATE_SUSTAINED_CPS)
    {
        rl->burst_used++;
    }

    return true;
}

// ===========================================================================
// ROLE-BASED AUTHORIZATION
// ===========================================================================

bool acp_role_can_execute(uint8_t role, uint8_t msg_type)
{
    switch (role)
    {
    case ACP_ROLE_COMMANDER:
        // Commanders can execute all commands
        switch (msg_type)
        {
        case ACP_CMD_CONTROL:
        case ACP_CMD_GIMBAL:
        case ACP_CMD_CONFIG:
            return true;
        default:
            return false;
        }

    case ACP_ROLE_PILOT:
        // Pilots can execute flight control but not configuration
        switch (msg_type)
        {
        case ACP_CMD_CONTROL:
        case ACP_CMD_GIMBAL:
            return true;
        case ACP_CMD_CONFIG:
            return false; // No configuration changes
        default:
            return false;
        }

    case ACP_ROLE_OBSERVER:
        // Observers cannot execute any commands (telemetry only)
        return false;

    default:
        return false;
    }
}

const char *acp_role_get_name(uint8_t role)
{
    switch (role)
    {
    case ACP_ROLE_COMMANDER:
        return "Commander";
    case ACP_ROLE_PILOT:
        return "Pilot";
    case ACP_ROLE_OBSERVER:
        return "Observer";
    default:
        return "Unknown";
    }
}

const char *acp_connection_get_name(uint8_t conn_type)
{
    switch (conn_type)
    {
    case ACP_CONN_UART:
        return "UART";
    case ACP_CONN_USB:
        return "USB";
    case ACP_CONN_SPI:
        return "SPI";
    default:
        return "Unknown";
    }
}

// ===========================================================================
// PUBLIC API IMPLEMENTATION
// ===========================================================================

bool acp_session_init(void)
{
    if (session_system_initialized)
    {
        ACP_LOG_WARN("Session system already initialized");
        return true;
    }
    ACP_LOG_INFO("Initializing ACP session management");

    // Create mutex for thread safety
    session_mutex = acp_mutex_create();
    if (!session_mutex)
    {
        ACP_LOG_ERROR("Failed to create session mutex");
        return false;
    }

    // Initialize session manager
    memset(&session_manager, 0, sizeof(session_manager));
    session_manager.active_count = 0;
    session_manager.next_session_id = 1; // Start with ID 1 (0 is invalid)

    // Initialize all sessions as inactive
    for (uint8_t i = 0; i < ACP_MAX_SESSIONS; i++)
    {
        session_manager.sessions[i].state = ACP_SESSION_STATE_INACTIVE;
    }

    // Initialize global statistics
    memset(&global_stats, 0, sizeof(global_stats));

    session_system_initialized = true;
    ACP_LOG_INFO("Session system initialization complete");

    return true;
}

bool acp_session_create(uint8_t operator_role,
                        uint8_t connection_type,
                        const uint8_t *auth_key,
                        uint32_t *session_id_out)
{

    if (!session_system_initialized || !auth_key || !session_id_out)
    {
        ACP_LOG_ERROR("Invalid parameters for session creation");
        return false;
    }

    // Validate role (explicit membership to avoid unsigned range warnings)
    if (!(operator_role == ACP_ROLE_OBSERVER || operator_role == ACP_ROLE_PILOT || operator_role == ACP_ROLE_COMMANDER))
    {
        ACP_LOG_ERROR("Invalid operator role: %u", operator_role);
        return false;
    }

    // Validate connection type (explicit membership)
    if (!(connection_type == ACP_CONN_UART || connection_type == ACP_CONN_USB || connection_type == ACP_CONN_SPI))
    {
        ACP_LOG_ERROR("Invalid connection type: %u", connection_type);
        return false;
    }

    acp_mutex_lock(session_mutex);

    bool success = false;

    // Find free session slot
    acp_session_t *session = find_free_session_slot();
    if (!session)
    {
        ACP_LOG_WARN("No free session slots available");
        goto cleanup;
    }

    // Allocate session ID
    uint32_t new_session_id = session_manager.next_session_id++;
    if (session_manager.next_session_id == ACP_SESSION_ID_INVALID)
    {
        session_manager.next_session_id = 1; // Skip invalid ID
    }

    // Initialize session
    init_session(session, new_session_id, operator_role, connection_type, auth_key);

    // Update manager state
    session_manager.active_count++;
    session_manager.system_uptime_ms = get_current_time_ms();

    // Update global statistics
    global_stats.total_sessions++;
    switch (operator_role)
    {
    case ACP_ROLE_COMMANDER:
        global_stats.commander_sessions++;
        break;
    case ACP_ROLE_PILOT:
        global_stats.pilot_sessions++;
        break;
    case ACP_ROLE_OBSERVER:
        global_stats.observer_sessions++;
        break;
    }

    *session_id_out = new_session_id;
    success = true;

    ESP_LOGI(TAG, "Created session %u: role=%s, conn=%s, active=%u/%u",
             new_session_id,
             acp_role_get_name(operator_role),
             acp_connection_get_name(connection_type),
             session_manager.active_count,
             ACP_MAX_SESSIONS);
    ACP_LOG_INFO("Created session %u: role=%s, conn=%s, active=%u/%u",
                 new_session_id,
                 acp_role_get_name(operator_role),
                 acp_connection_get_name(connection_type),
                 session_manager.active_count,
                 ACP_MAX_SESSIONS);

cleanup:
    acp_mutex_unlock(session_mutex);
    return success;
}

bool acp_session_authenticate(uint32_t session_id,
                              uint8_t seq_num,
                              const uint8_t *auth_tag,
                              const uint8_t *message_data,
                              uint16_t message_len)
{

    if (!session_system_initialized || !message_data)
    {
        return false;
    }

    if (xSemaphoreTake(session_mutex, portMAX_DELAY) != pdTRUE)
    {
        ACP_LOG_ERROR("Failed to acquire session mutex for auth");
        return false;
    }

    bool authenticated = false;
    acp_session_t *session = find_session_by_id(session_id);

    if (!session)
    {
        ACP_LOG_WARN("Authentication failed: session %u not found", session_id);
        goto cleanup;
    }

    if (session->state != ACP_SESSION_STATE_ACTIVE)
    {
        ACP_LOG_WARN("Authentication failed: session %u not active (state=%u)",
                     session_id, session->state);
        goto cleanup;
    }

    // Check sequence number (replay protection)
    if (!is_sequence_valid(session, seq_num))
    {
        session->auth_failures++;
        global_stats.auth_failures++;
        ACP_LOG_WARN("Authentication failed: invalid sequence number");
        goto cleanup;
    }

    // Verify HMAC if present
    if (auth_tag)
    {
        int crypto_result = acp_crypto_hmac_verify(session->auth_key,
                                                   message_data,
                                                   message_len,
                                                   auth_tag);
        if (crypto_result != ACP_CRYPTO_OK)
        {
            session->auth_failures++;
            global_stats.auth_failures++;
            ACP_LOG_WARN("Authentication failed: HMAC verification failed");
            goto cleanup;
        }
    }

    // Update session activity
    uint32_t current_time = get_current_time_ms();
    session->last_activity = current_time;
    session->last_seq_num = seq_num;
    session->messages_received++;
    session->expires_at = current_time + ACP_SESSION_TIMEOUT;

    authenticated = true;
    ACP_LOG_DEBUG("Authentication successful for session %u", session_id);

cleanup:
    acp_mutex_unlock(session_mutex);
    return authenticated;
}

bool acp_session_check_rate_limit(uint32_t session_id, uint32_t current_time_ms)
{
    if (!session_system_initialized)
    {
        return false;
    }

    if (xSemaphoreTake(session_mutex, portMAX_DELAY) != pdTRUE)
    {
        return false;
    }
    bool allowed = false;
    acp_session_t *session = find_session_by_id(session_id);
    if (session && session->state == ACP_SESSION_STATE_ACTIVE)
    {
        allowed = update_rate_limit(session, current_time_ms);
    }
    acp_mutex_unlock(session_mutex);
    return allowed;
}

bool acp_session_authorize_command(uint32_t session_id,
                                   uint8_t msg_type,
                                   const void *command_data)
{

    if (!session_system_initialized)
    {
        return false;
    }

    if (xSemaphoreTake(session_mutex, portMAX_DELAY) != pdTRUE)
    {
        return false;
    }
    bool authorized = false;
    acp_session_t *session = find_session_by_id(session_id);
    if (session && session->state == ACP_SESSION_STATE_ACTIVE)
    {
        // Check role-based authorization
        authorized = acp_role_can_execute(session->operator_role, msg_type);
        if (!authorized)
        {
            ACP_LOG_WARN("Command 0x%02X denied for role %s (session %u)",
                         msg_type, acp_role_get_name(session->operator_role), session_id);
        }
        // TODO: Add additional authorization checks based on command_data
        // For example, certain flight modes might require higher privileges
    }
    acp_mutex_unlock(session_mutex);
    return authorized;
}

bool acp_session_update_activity(uint32_t session_id, uint32_t current_time_ms)
{
    if (!session_system_initialized)
    {
        return false;
    }

    if (xSemaphoreTake(session_mutex, portMAX_DELAY) != pdTRUE)
    {
        return false;
    }
    bool updated = false;
    acp_session_t *session = find_session_by_id(session_id);
    if (session && session->state == ACP_SESSION_STATE_ACTIVE)
    {
        session->last_activity = current_time_ms;
        session->expires_at = current_time_ms + ACP_SESSION_TIMEOUT;
        updated = true;
    }
    acp_mutex_unlock(session_mutex);
    return updated;
}

bool acp_session_close(uint32_t session_id)
{
    if (!session_system_initialized)
    {
        return false;
    }

    if (xSemaphoreTake(session_mutex, portMAX_DELAY) != pdTRUE)
    {
        return false;
    }
    bool closed = false;
    acp_session_t *session = find_session_by_id(session_id);
    if (session)
    {
        uint8_t old_role = session->operator_role;

        // Clear sensitive data
        acp_crypto_secure_memclear(session->auth_key, sizeof(session->auth_key));

        // Mark as inactive
        session->state = ACP_SESSION_STATE_INACTIVE;
        session->session_id = ACP_SESSION_ID_INVALID;

        // Update counters
        if (session_manager.active_count > 0)
        {
            session_manager.active_count--;
        }

        // Update role statistics
        switch (old_role)
        {
        case ACP_ROLE_COMMANDER:
            if (global_stats.commander_sessions > 0)
                global_stats.commander_sessions--;
            break;
        case ACP_ROLE_PILOT:
            if (global_stats.pilot_sessions > 0)
                global_stats.pilot_sessions--;
            break;
        case ACP_ROLE_OBSERVER:
            if (global_stats.observer_sessions > 0)
                global_stats.observer_sessions--;
            break;
        }

        closed = true;
        ACP_LOG_INFO("Closed session %u (%s), active sessions: %u",
                     session_id, acp_role_get_name(old_role), session_manager.active_count);
    }
    acp_mutex_unlock(session_mutex);
    return closed;
}

uint8_t acp_session_cleanup_expired(uint32_t current_time_ms)
{
    if (!session_system_initialized)
    {
        return 0;
    }

    if (xSemaphoreTake(session_mutex, portMAX_DELAY) != pdTRUE)
    {
        return 0;
    }
    uint8_t cleaned_count = 0;
    for (uint8_t i = 0; i < ACP_MAX_SESSIONS; i++)
    {
        acp_session_t *session = &session_manager.sessions[i];
        if (session->state != ACP_SESSION_STATE_INACTIVE &&
            current_time_ms >= session->expires_at)
        {
            ACP_LOG_INFO("Session %u expired (inactive for %u ms)",
                         session->session_id,
                         current_time_ms - session->last_activity);
            // Close expired session
            uint32_t expired_id = session->session_id;
            acp_mutex_unlock(session_mutex); // Release mutex for close call
            acp_session_close(expired_id);
            acp_mutex_lock(session_mutex); // Re-acquire
            cleaned_count++;
            global_stats.expired_sessions++;
        }
    }
    if (cleaned_count > 0)
    {
        ACP_LOG_INFO("Cleaned up %u expired sessions", cleaned_count);
    }
    acp_mutex_unlock(session_mutex);
    return cleaned_count;
}

// ===========================================================================
// QUERY API IMPLEMENTATION
// ===========================================================================

acp_session_t *acp_session_get_by_id(uint32_t session_id)
{
    if (!session_system_initialized)
    {
        return NULL;
    }

    // Note: This returns a pointer to internal data, caller must handle synchronization
    return find_session_by_id(session_id);
}

uint8_t acp_session_get_active_count(void)
{
    if (!session_system_initialized)
    {
        return 0;
    }

    return session_manager.active_count;
}

uint8_t acp_session_get_list(acp_session_t *sessions_out, uint8_t max_sessions)
{
    if (!session_system_initialized || !sessions_out || max_sessions == 0)
    {
        return 0;
    }

    if (xSemaphoreTake(session_mutex, portMAX_DELAY) != pdTRUE)
    {
        return 0;
    }
    uint8_t count = 0;
    for (uint8_t i = 0; i < ACP_MAX_SESSIONS && count < max_sessions; i++)
    {
        if (session_manager.sessions[i].state != ACP_SESSION_STATE_INACTIVE)
        {
            memcpy(&sessions_out[count], &session_manager.sessions[i], sizeof(acp_session_t));
            count++;
        }
    }
    acp_mutex_unlock(session_mutex);
    return count;
}

uint8_t acp_session_get_priority(uint32_t session_id, uint8_t msg_type)
{
    if (!session_system_initialized)
    {
        return ACP_PRIORITY_LOW;
    }

    acp_session_t *session = find_session_by_id(session_id);
    if (!session)
    {
        return ACP_PRIORITY_LOW;
    }

    // Determine priority based on role and message type
    switch (session->operator_role)
    {
    case ACP_ROLE_COMMANDER:
        // Commanders get high priority for all commands
        switch (msg_type)
        {
        case ACP_CMD_CONTROL:
            return ACP_PRIORITY_EMERGENCY; // ARM/DISARM commands
        case ACP_CMD_CONFIG:
            return ACP_PRIORITY_HIGH;
        default:
            return ACP_PRIORITY_NORMAL;
        }

    case ACP_ROLE_PILOT:
        // Pilots get normal priority for flight commands
        switch (msg_type)
        {
        case ACP_CMD_CONTROL:
            return ACP_PRIORITY_HIGH;
        case ACP_CMD_GIMBAL:
            return ACP_PRIORITY_NORMAL;
        default:
            return ACP_PRIORITY_LOW;
        }

    case ACP_ROLE_OBSERVER:
        // Observers get low priority (telemetry only)
        return ACP_PRIORITY_LOW;

    default:
        return ACP_PRIORITY_LOW;
    }
}

// ===========================================================================
// STATISTICS API
// ===========================================================================

void acp_session_get_stats(acp_session_stats_t *stats_out)
{
    if (!stats_out || !session_system_initialized)
    {
        return;
    }

    if (xSemaphoreTake(session_mutex, portMAX_DELAY) != pdTRUE)
    {
        return;
    }
    // Copy global stats and update current counts
    memcpy(stats_out, &global_stats, sizeof(acp_session_stats_t));
    stats_out->active_sessions = session_manager.active_count;
    acp_mutex_unlock(session_mutex);
}

void acp_session_reset_stats(void)
{
    if (!session_system_initialized)
    {
        return;
    }

    if (xSemaphoreTake(session_mutex, portMAX_DELAY) != pdTRUE)
    {
        return;
    }
    // Reset global statistics but keep current session counts
    uint8_t active = global_stats.active_sessions;
    uint8_t commanders = global_stats.commander_sessions;
    uint8_t pilots = global_stats.pilot_sessions;
    uint8_t observers = global_stats.observer_sessions;

    memset(&global_stats, 0, sizeof(global_stats));

    global_stats.active_sessions = active;
    global_stats.commander_sessions = commanders;
    global_stats.pilot_sessions = pilots;
    global_stats.observer_sessions = observers;

    acp_mutex_unlock(session_mutex);
    ACP_LOG_INFO("Session statistics reset");
}