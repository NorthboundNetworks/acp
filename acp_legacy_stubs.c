/**
 * @file acp_legacy_stubs.c
 * @brief Compatibility stubs for legacy ACP implementation
 *
 * This file provides stub implementations of functions from the existing
 * ACP codebase to allow compilation while we implement the new specification.
 *
 * @version 0.3.0
 * @date 2025-10-27
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ========================================================================== */
/*                            Legacy Type Stubs                              */
/* ========================================================================== */

/* Legacy frame header structure (from existing code) */
typedef struct
{
    uint8_t sync[2];
    uint8_t version;
    uint8_t msg_type;
    uint16_t length;
    uint8_t flags;
    uint8_t seq_id;
} acp_frame_header_t;

/* Legacy command structures */
typedef struct
{
    uint8_t action;
    uint8_t mode;
    uint16_t reserved;
} acp_cmd_control_t;
typedef struct
{
    int16_t pan;
    int16_t tilt;
    uint8_t mode;
    uint8_t reserved[3];
} acp_cmd_gimbal_t;
typedef struct
{
    uint8_t config_type;
    uint8_t data[7];
} acp_cmd_config_t;

/* Legacy telemetry structures */
typedef struct
{
    uint8_t status;
    uint8_t mode;
    uint16_t battery;
    uint32_t timestamp;
} acp_tlm_status_t;
typedef struct
{
    int16_t pan;
    int16_t tilt;
    uint8_t status;
    uint8_t reserved[3];
} acp_tlm_gimbal_t;
typedef struct
{
    uint8_t event_type;
    uint8_t severity;
    uint16_t timestamp;
    uint32_t data;
} acp_tlm_audit_t;

/* Legacy response structures */
typedef struct
{
    uint8_t msg_type;
    uint8_t seq_id;
    uint8_t result;
    uint8_t reserved;
} acp_ack_response_t;
typedef struct
{
    uint8_t msg_type;
    uint8_t seq_id;
    uint8_t error;
    uint8_t reserved;
} acp_err_response_t;

/* ========================================================================== */
/*                          Legacy Constants                                  */
/* ========================================================================== */

#define ACP_SYNC_BYTE1 0xAA
#define ACP_SYNC_BYTE2 0x55
#define ACP_VERSION_1_1 0x11
#define ACP_MAX_PAYLOAD_SIZE 512
#define ACP_MAX_FRAME_SIZE 1024
#define ACP_AUTH_TAG_SIZE 16
#define ACP_CRC16_SIZE 2
#define ACP_FLAG_AUTH_PRESENT 0x01
#define ACP_FLAG_COMPRESSED 0x02
#define ACP_FLAG_PRIORITY_MASK 0x0C

/* Message types */
#define ACP_CMD_CONTROL 0x01
#define ACP_CMD_GIMBAL 0x02
#define ACP_CMD_CONFIG 0x03
#define ACP_TLM_STATUS 0x10
#define ACP_TLM_GIMBAL 0x11
#define ACP_TLM_AUDIT 0x12
#define ACP_TLM_MISSION 0x13
#define ACP_ACK_RESPONSE 0x20
#define ACP_ERR_RESPONSE 0x21

/* Error codes */
#define ACP_ERR_OK 0x00
#define ACP_ERR_UNKNOWN 0x01
#define ACP_ERR_CRC_FAIL 0x02
#define ACP_ERR_SAFETY_REJECT 0x03
#define ACP_ERR_AUTH_FAIL 0x04
#define ACP_ERR_RESOURCE 0x05
#define ACP_ERR_REPLAY 0x06
#define ACP_ERR_ROLE_DENIED 0x07
#define ACP_ERR_RATE_LIMIT 0x08

/* Result codes */
#define ACP_RESULT_SUCCESS 0x00
#define ACP_RESULT_QUEUED 0x01
#define ACP_RESULT_RATE_LIMITED 0x02

/* Priorities, roles, etc. */
#define ACP_PRIORITY_EMERGENCY 0x00
#define ACP_PRIORITY_HIGH 0x01
#define ACP_PRIORITY_NORMAL 0x02
#define ACP_PRIORITY_LOW 0x03

#define ACP_ROLE_OBSERVER 0x00
#define ACP_ROLE_PILOT 0x01
#define ACP_ROLE_COMMANDER 0x02

#define ACP_CONN_UART 0x00
#define ACP_CONN_USB 0x01
#define ACP_CONN_SPI 0x02

#define ACP_ACTION_ARM 0x01
#define ACP_ACTION_DISARM 0x02
#define ACP_ACTION_MODE_CHANGE 0x03

#define ACP_MODE_HOVER 0x01
#define ACP_MODE_LOITER 0x02
#define ACP_MODE_FORWARD 0x03

#define ACP_AUDIT_COMMAND 0x01
#define ACP_AUDIT_AUTH_FAIL 0x02
#define ACP_AUDIT_SESSION 0x03
#define ACP_AUDIT_SECURITY 0x04

#define ACP_CONFIG_TELEMETRY 0x01
#define ACP_CONFIG_SECURITY 0x02
#define ACP_CONFIG_SYSTEM 0x03

#define ACP_MAX_SESSIONS 4
#define ACP_SESSION_KEY_SIZE 32
#define ACP_RATE_SUSTAINED_CPS 10
#define ACP_RATE_BURST_CPS 20
#define ACP_SESSION_TIMEOUT 30000

#define ACP_VERSION_MAJOR 0
#define ACP_VERSION_MINOR 3

/* Macros */
#define ACP_GET_PRIORITY(flags) (((flags) & ACP_FLAG_PRIORITY_MASK) >> 2)
#define ntohs(x) ((uint16_t)(((x) << 8) | ((x) >> 8)))

/* ========================================================================== */
/*                           Stub Functions                                   */
/* ========================================================================== */

/* These are minimal stub implementations to allow compilation */

const char *acp_error_to_string(uint8_t error_code)
{
    (void)error_code;
    return "Legacy error stub";
}

const char *acp_result_to_string(uint8_t result_code)
{
    (void)result_code;
    return "Legacy result stub";
}

const char *acp_message_type_to_string(uint8_t msg_type)
{
    (void)msg_type;
    return "Legacy message type stub";
}

const char *acp_action_to_string(uint8_t action)
{
    (void)action;
    return "Legacy action stub";
}

const char *acp_mode_to_string(uint8_t mode)
{
    (void)mode;
    return "Legacy mode stub";
}

const char *acp_priority_to_string(uint8_t priority)
{
    (void)priority;
    return "Legacy priority stub";
}

const char *acp_audit_event_to_string(uint8_t event_type)
{
    (void)event_type;
    return "Legacy audit event stub";
}

const char *acp_config_type_to_string(uint8_t config_type)
{
    (void)config_type;
    return "Legacy config type stub";
}

bool acp_is_valid_message_type(uint8_t msg_type)
{
    (void)msg_type;
    return true;
}

bool acp_is_command_message(uint8_t msg_type)
{
    (void)msg_type;
    return false;
}

bool acp_is_telemetry_message(uint8_t msg_type)
{
    (void)msg_type;
    return false;
}

bool acp_is_response_message(uint8_t msg_type)
{
    (void)msg_type;
    return false;
}

bool acp_is_valid_role(uint8_t role)
{
    (void)role;
    return true;
}

bool acp_is_valid_connection_type(uint8_t conn_type)
{
    (void)conn_type;
    return true;
}

bool acp_is_valid_priority(uint8_t priority)
{
    (void)priority;
    return true;
}

bool acp_is_valid_action(uint8_t action)
{
    (void)action;
    return true;
}

bool acp_is_valid_mode(uint8_t mode)
{
    (void)mode;
    return true;
}

uint16_t acp_get_expected_payload_size(uint8_t msg_type)
{
    (void)msg_type;
    return 64;
}

bool acp_has_fixed_payload_size(uint8_t msg_type)
{
    (void)msg_type;
    return true;
}

bool acp_validate_frame_limits(const acp_frame_header_t *header)
{
    (void)header;
    return true;
}

uint16_t acp_get_max_frame_size(uint16_t payload_size, bool has_auth)
{
    (void)has_auth;
    return payload_size + 64;
}

uint32_t acp_get_expected_processing_time_us(uint8_t msg_type)
{
    (void)msg_type;
    return 1000;
}

bool acp_is_processing_time_acceptable(uint8_t msg_type, uint32_t actual_time_us)
{
    (void)msg_type;
    (void)actual_time_us;
    return true;
}

bool acp_constants_init(void)
{
    return true;
}

void acp_log_protocol_info(void)
{
    /* No-op stub */
}