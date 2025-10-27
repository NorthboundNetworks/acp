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
 * @file acp_constants.c
 * @brief ACP Protocol Constants and Utility Functions
 *
 * This module defines the enumerations, mappings, constraints, and validation
 * functions for ACP v0.3. It provides conversion utilities for message types,
 * error/result codes, roles, priorities, and limits.
 *
 * @version 0.3
 * @date 2025-10-26
 * @author Paul Zanna
 */

#include "acp_protocol.h"
#include "acp_session.h"

static const char *TAG = "acp_constants";

// ===========================================================================
// ERROR CODE TO STRING MAPPINGS
// ===========================================================================

/**
 * @brief Convert ACP error code to human-readable string
 *
 * @param error_code ACP error code (ACP_ERR_*)
 * @return Human-readable error description string, or "Unknown error code" if invalid
 */
const char *acp_error_to_string(uint8_t error_code)
{
    switch (error_code)
    {
    case ACP_ERR_OK:
        return "No error";
    case ACP_ERR_UNKNOWN:
        return "Unknown error";
    case ACP_ERR_CRC_FAIL:
        return "Frame CRC validation failed";
    case ACP_ERR_SAFETY_REJECT:
        return "Safety system rejected command";
    case ACP_ERR_AUTH_FAIL:
        return "Authentication failed";
    case ACP_ERR_RESOURCE:
        return "Insufficient resources";
    case ACP_ERR_REPLAY:
        return "Replay attack detected";
    case ACP_ERR_ROLE_DENIED:
        return "Operator role insufficient";
    case ACP_ERR_RATE_LIMIT:
        return "Rate limit exceeded";
    default:
        return "Unknown error code";
    }
}

/**
 * @brief Convert ACP result code to human-readable string
 *
 * @param result_code ACP result code (ACP_RESULT_*)
 * @return Human-readable result description string, or "Unknown result code" if invalid
 */
const char *acp_result_to_string(uint8_t result_code)
{
    switch (result_code)
    {
    case ACP_RESULT_SUCCESS:
        return "Command executed successfully";
    case ACP_RESULT_QUEUED:
        return "Command queued for execution";
    case ACP_RESULT_RATE_LIMITED:
        return "Command accepted but rate limited";
    default:
        return "Unknown result code";
    }
}

/**
 * @brief Convert ACP message type to human-readable string
 *
 * @param msg_type ACP message type (ACP_CMD_*, ACP_TLM_*, etc.)
 * @return Human-readable message type description string, or "Unknown message type" if invalid
 */
const char *acp_message_type_to_string(uint8_t msg_type)
{
    switch (msg_type)
    {
    // Command messages
    case ACP_CMD_CONTROL:
        return "Control Command";
    case ACP_CMD_GIMBAL:
        return "Gimbal Command";
    case ACP_CMD_CONFIG:
        return "Configuration Command";

    // Telemetry messages
    case ACP_TLM_STATUS:
        return "Status Telemetry";
    case ACP_TLM_GIMBAL:
        return "Gimbal Telemetry";
    case ACP_TLM_AUDIT:
        return "Audit Telemetry";
    case ACP_TLM_MISSION:
        return "Mission Telemetry";

    // Response messages
    case ACP_ACK_RESPONSE:
        return "Acknowledgment Response";
    case ACP_ERR_RESPONSE:
        return "Error Response";

    default:
        return "Unknown message type";
    }
}

/**
 * @brief Convert ACP action type to human-readable string
 *
 * @param action ACP action code (ACP_ACTION_*)
 * @return Human-readable action description string, or "Unknown action" if invalid
 */
const char *acp_action_to_string(uint8_t action)
{
    switch (action)
    {
    case ACP_ACTION_ARM:
        return "ARM";
    case ACP_ACTION_DISARM:
        return "DISARM";
    case ACP_ACTION_MODE_CHANGE:
        return "MODE_CHANGE";
    default:
        return "Unknown action";
    }
}

/**
 * @brief Convert ACP flight mode to human-readable string
 *
 * @param mode ACP mode code (ACP_MODE_*)
 * @return Human-readable mode description string, or "Unknown mode" if invalid
 */
const char *acp_mode_to_string(uint8_t mode)
{
    switch (mode)
    {
    case ACP_MODE_HOVER:
        return "HOVER";
    case ACP_MODE_LOITER:
        return "LOITER";
    case ACP_MODE_FORWARD:
        return "FORWARD";
    default:
        return "Unknown mode";
    }
}

/**
 * @brief Convert ACP priority to human-readable string
 *
 * @param priority ACP priority level (ACP_PRIORITY_*)
 * @return Human-readable priority description string, or "Unknown priority" if invalid
 */
const char *acp_priority_to_string(uint8_t priority)
{
    switch (priority)
    {
    case ACP_PRIORITY_EMERGENCY:
        return "EMERGENCY";
    case ACP_PRIORITY_HIGH:
        return "HIGH";
    case ACP_PRIORITY_NORMAL:
        return "NORMAL";
    case ACP_PRIORITY_LOW:
        return "LOW";
    default:
        return "Unknown priority";
    }
}

/**
 * @brief Convert audit event type to human-readable string
 *
 * @param event_type ACP audit event type (ACP_AUDIT_*)
 * @return Human-readable audit event type description string, or "Unknown audit event" if invalid
 */
const char *acp_audit_event_to_string(uint8_t event_type)
{
    switch (event_type)
    {
    case ACP_AUDIT_COMMAND:
        return "Command Event";
    case ACP_AUDIT_AUTH_FAIL:
        return "Authentication Failure";
    case ACP_AUDIT_SESSION:
        return "Session Event";
    case ACP_AUDIT_SECURITY:
        return "Security Event";
    default:
        return "Unknown audit event";
    }
}

/**
 * @brief Convert configuration type to human-readable string
 *
 * @param config_type ACP config type (ACP_CONFIG_*)
 * @return Human-readable configuration type description string, or "Unknown configuration type" if invalid
 */
const char *acp_config_type_to_string(uint8_t config_type)
{
    switch (config_type)
    {
    case ACP_CONFIG_TELEMETRY:
        return "Telemetry Configuration";
    case ACP_CONFIG_SECURITY:
        return "Security Configuration";
    case ACP_CONFIG_SYSTEM:
        return "System Configuration";
    default:
        return "Unknown configuration type";
    }
}

// ===========================================================================
// VALIDATION FUNCTIONS
// ===========================================================================

/**
 * @brief Validate ACP message type
 *
 * @param msg_type Message type to validate
 * @return true if valid message type, false otherwise
 */
bool acp_is_valid_message_type(uint8_t msg_type)
{
    switch (msg_type)
    {
    case ACP_CMD_CONTROL:
    case ACP_CMD_GIMBAL:
    case ACP_CMD_CONFIG:
    case ACP_TLM_STATUS:
    case ACP_TLM_GIMBAL:
    case ACP_TLM_AUDIT:
    case ACP_TLM_MISSION:
    case ACP_ACK_RESPONSE:
    case ACP_ERR_RESPONSE:
        return true;
    default:
        return false;
    }
}

/**
 * @brief Check if message type is a command message
 *
 * @param msg_type Message type to check
 * @return true if command message, false otherwise
 */
bool acp_is_command_message(uint8_t msg_type)
{
    return (msg_type >= 0x01 && msg_type <= 0x0F);
}

/**
 * @brief Check if message type is a telemetry message
 *
 * @param msg_type Message type to check
 * @return true if telemetry message, false otherwise
 */
bool acp_is_telemetry_message(uint8_t msg_type)
{
    return (msg_type >= 0x10 && msg_type <= 0x1F);
}

/**
 * @brief Check if message type is a response message
 *
 * @param msg_type Message type to check
 * @return true if response message, false otherwise
 */
bool acp_is_response_message(uint8_t msg_type)
{
    return (msg_type >= 0x20 && msg_type <= 0x2F);
}

/**
 * @brief Validate ACP operator role
 *
 * @param role Role to validate
 * @return true if valid role, false otherwise
 */
bool acp_is_valid_role(uint8_t role)
{
    // Explicit set membership avoids type-limit warnings with unsigned types
    return (role == ACP_ROLE_OBSERVER || role == ACP_ROLE_PILOT || role == ACP_ROLE_COMMANDER);
}

/**
 * @brief Validate ACP connection type
 *
 * @param conn_type Connection type to validate
 * @return true if valid connection type, false otherwise
 */
bool acp_is_valid_connection_type(uint8_t conn_type)
{
    // Explicit set membership avoids always-true comparison against 0
    return (conn_type == ACP_CONN_UART || conn_type == ACP_CONN_USB || conn_type == ACP_CONN_SPI);
}

/**
 * @brief Validate ACP priority level
 *
 * @param priority Priority level to validate
 * @return true if valid priority, false otherwise
 */
bool acp_is_valid_priority(uint8_t priority)
{
    return (priority == ACP_PRIORITY_EMERGENCY || priority == ACP_PRIORITY_HIGH ||
            priority == ACP_PRIORITY_NORMAL || priority == ACP_PRIORITY_LOW);
}

/**
 * @brief Validate ACP action type
 *
 * @param action Action type to validate
 * @return true if valid action, false otherwise
 */
bool acp_is_valid_action(uint8_t action)
{
    return (action == ACP_ACTION_ARM || action == ACP_ACTION_DISARM || action == ACP_ACTION_MODE_CHANGE);
}

/**
 * @brief Validate ACP flight mode
 *
 * @param mode Flight mode to validate
 * @return true if valid mode, false otherwise
 */
bool acp_is_valid_mode(uint8_t mode)
{
    return (mode == ACP_MODE_HOVER || mode == ACP_MODE_LOITER || mode == ACP_MODE_FORWARD);
}

// ===========================================================================
// MESSAGE SIZE FUNCTIONS
// ===========================================================================

/**
 * @brief Get expected payload size for message type
 *
 * @param msg_type ACP message type
 * @return Expected payload size in bytes, or 0 if unknown or variable size
 */
uint16_t acp_get_expected_payload_size(uint8_t msg_type)
{
    switch (msg_type)
    {
    case ACP_CMD_CONTROL:
        // sizeof(acp_cmd_control_t)
        return sizeof(acp_cmd_control_t);
    case ACP_CMD_GIMBAL:
        // sizeof(acp_cmd_gimbal_t)
        return sizeof(acp_cmd_gimbal_t);
    case ACP_CMD_CONFIG:
        // sizeof(acp_cmd_config_t)
        return sizeof(acp_cmd_config_t);
    case ACP_TLM_STATUS:
        // sizeof(acp_tlm_status_t)
        return sizeof(acp_tlm_status_t);
    case ACP_TLM_GIMBAL:
        // sizeof(acp_tlm_gimbal_t)
        return sizeof(acp_tlm_gimbal_t);
    case ACP_TLM_AUDIT:
        // sizeof(acp_tlm_audit_t)
        return sizeof(acp_tlm_audit_t);
    case ACP_ACK_RESPONSE:
        // sizeof(acp_ack_response_t)
        return sizeof(acp_ack_response_t);
    case ACP_ERR_RESPONSE:
        // sizeof(acp_err_response_t)
        return sizeof(acp_err_response_t);
    case ACP_TLM_MISSION:
        // Variable size (e.g., mission upload/download)
        return 0;
    default:
        // Unknown or variable size
        return 0;
    }
}

/**
 * @brief Check if message type has fixed payload size
 *
 * @param msg_type ACP message type
 * @return true if fixed size, false if variable
 */
bool acp_has_fixed_payload_size(uint8_t msg_type)
{
    return (acp_get_expected_payload_size(msg_type) > 0);
}

// ===========================================================================
// PROTOCOL LIMITS AND CONSTRAINTS
// ===========================================================================

/**
 * @brief Validate frame header constraints
 *
 * @param header Frame header to validate
 * @return true if within protocol limits, false otherwise
 */
bool acp_validate_frame_limits(const acp_frame_header_t *header)
{
    if (!header)
    {
        return false;
    }

    // Check sync pattern
    if (header->sync[0] != ACP_SYNC_BYTE1 || header->sync[1] != ACP_SYNC_BYTE2)
    {
        return false;
    }

    // Check protocol version
    if (header->version != ACP_VERSION_1_1)
    {
        return false;
    }

    // Check message type
    if (!acp_is_valid_message_type(header->msg_type))
    {
        return false;
    }

    // Check payload length
    uint16_t payload_len = ntohs(header->length);
    if (payload_len > ACP_MAX_PAYLOAD_SIZE)
    {
        return false;
    }

    // Check priority in flags (inline)
    if (!acp_is_valid_priority(ACP_GET_PRIORITY(header->flags)))
    {
        return false;
    }

    // Check for reserved flag bits
    uint8_t reserved_flags = header->flags & ~(ACP_FLAG_AUTH_PRESENT |
                                               ACP_FLAG_COMPRESSED |
                                               ACP_FLAG_PRIORITY_MASK);
    if (reserved_flags != 0)
    {
        return false;
    }

    return true;
}

/**
 * @brief Get maximum frame size for given payload
 *
 * @param payload_size Payload size in bytes
 * @param has_auth Whether authentication tag is present
 * @return Maximum encoded frame size including all overhead
 */
uint16_t acp_get_max_frame_size(uint16_t payload_size, bool has_auth)
{
    uint16_t raw_size = sizeof(acp_frame_header_t) + payload_size + ACP_CRC16_SIZE;

    if (has_auth)
    {
        raw_size += ACP_AUTH_TAG_SIZE;
    }

    // COBS: 1 byte per 254 + 2 bytes for framing
    uint16_t cobs_overhead = (raw_size / 254) + 2;

    return raw_size + cobs_overhead;
}

// ===========================================================================
// TIMING AND PERFORMANCE CONSTANTS
// ===========================================================================

/**
 * @brief Get expected processing time for message type
 *
 * Performance targets for different message types to ensure
 * real-time constraints are met.
 *
 * @param msg_type ACP message type
 * @return Expected processing time in microseconds
 */
uint32_t acp_get_expected_processing_time_us(uint8_t msg_type)
{
    switch (msg_type)
    {
    case ACP_CMD_CONTROL:
        return 3000; // 3ms for critical control commands
    case ACP_CMD_GIMBAL:
        return 2000; // 2ms for gimbal commands
    case ACP_CMD_CONFIG:
        return 5000; // 5ms for configuration commands
    case ACP_TLM_STATUS:
        return 1000; // 1ms for status telemetry
    case ACP_TLM_GIMBAL:
        return 800; // 0.8ms for gimbal telemetry
    case ACP_TLM_AUDIT:
        return 1500; // 1.5ms for audit events
    case ACP_ACK_RESPONSE:
        return 500; // 0.5ms for acknowledgments
    case ACP_ERR_RESPONSE:
        return 600; // 0.6ms for error responses
    default:
        return 5000; // 5ms default for unknown types
    }
}

/**
 * @brief Check if processing time is within acceptable limits
 *
 * @param msg_type Message type
 * @param actual_time_us Actual processing time in microseconds
 * @return true if within limits, false if exceeded
 */
bool acp_is_processing_time_acceptable(uint8_t msg_type, uint32_t actual_time_us)
{
    uint32_t expected_time = acp_get_expected_processing_time_us(msg_type);
    uint32_t max_allowed = expected_time * 2; // Allow 2x expected as maximum

    return (actual_time_us <= max_allowed);
}

// ===========================================================================
// INITIALIZATION AND VALIDATION
// ===========================================================================

/**
 * @brief Initialize and validate ACP protocol constants
 *
 * Performs any required initialization for the constants module.
 * Currently just validates critical constants.
 *
 * @return true if initialization successful
 */
bool acp_constants_init(void)
{
    ESP_LOGI(TAG, "Initializing ACP constants");

    // Validate critical protocol constants
    static_assert(ACP_MAX_FRAME_SIZE >= 256, "Frame size too small");
    static_assert(ACP_MAX_PAYLOAD_SIZE < ACP_MAX_FRAME_SIZE, "Payload size inconsistent");
    static_assert(ACP_AUTH_TAG_SIZE == 16, "Auth tag size must be 16 bytes");
    static_assert(ACP_CRC16_SIZE == 2, "CRC size must be 2 bytes");

    // Validate session constants
    static_assert(ACP_MAX_SESSIONS >= 1 && ACP_MAX_SESSIONS <= 8, "Invalid session count");
    static_assert(ACP_SESSION_KEY_SIZE == 32, "Session key must be 32 bytes");

    // Validate rate limiting constants
    static_assert(ACP_RATE_SUSTAINED_CPS > 0, "Sustained rate must be positive");
    static_assert(ACP_RATE_BURST_CPS >= ACP_RATE_SUSTAINED_CPS, "Burst rate must be >= sustained");
    static_assert(ACP_SESSION_TIMEOUT > 0, "Session timeout must be positive");

    ESP_LOGI(TAG, "ACP constants validation passed");
    return true;
}

/**
 * @brief Log ACP protocol configuration parameters
 *
 * Logs key protocol parameters for verification during initialization.
 */
void acp_log_protocol_info(void)
{
    ESP_LOGI(TAG, "ACP Protocol v%d.%d Configuration:", ACP_VERSION_MAJOR, ACP_VERSION_MINOR);
    ESP_LOGI(TAG, "  Max frame size: %u bytes", ACP_MAX_FRAME_SIZE);
    ESP_LOGI(TAG, "  Max payload size: %u bytes", ACP_MAX_PAYLOAD_SIZE);
    ESP_LOGI(TAG, "  Authentication tag: %u bytes", ACP_AUTH_TAG_SIZE);
    ESP_LOGI(TAG, "  Max sessions: %u", ACP_MAX_SESSIONS);
    ESP_LOGI(TAG, "  Rate limits: %u sustained, %u burst CPS",
             ACP_RATE_SUSTAINED_CPS, ACP_RATE_BURST_CPS);
    ESP_LOGI(TAG, "  Session timeout: %u seconds", ACP_SESSION_TIMEOUT / 1000);
}