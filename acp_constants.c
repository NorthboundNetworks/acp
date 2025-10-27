/**
 * @file acp_constants.c
 * @brief ACP protocol constants and lookup tables
 *
 * This module provides protocol constants and precomputed lookup tables
 * for the ACP protocol implementation.
 */

#include "acp_protocol.h"
#include "acp_errors.h"

/**
 * @brief Protocol version string
 */
const char *acp_version_string = "0.3.0";

/**
 * @brief Default frame types names for debugging
 */
const char *acp_frame_type_names[] = {
    "UNKNOWN",     /* 0x00 */
    "TELEMETRY",   /* 0x01 */
    "COMMAND",     /* 0x02 */
    "RESPONSE",    /* 0x03 */
    "HEARTBEAT",   /* 0x04 */
    "RESERVED_05", /* 0x05 */
    "RESERVED_06", /* 0x06 */
    "RESERVED_07", /* 0x07 */
    "RESERVED_08", /* 0x08 */
    "RESERVED_09", /* 0x09 */
    "RESERVED_0A", /* 0x0A */
    "RESERVED_0B", /* 0x0B */
    "RESERVED_0C", /* 0x0C */
    "RESERVED_0D", /* 0x0D */
    "RESERVED_0E", /* 0x0E */
    "RESERVED_0F"  /* 0x0F */
};

/**
 * @brief Default error code names for debugging
 */
const char *acp_error_names[] = {
    "ACP_OK",                     /* 0 */
    "ACP_ERROR_INVALID_PARAM",    /* 1 */
    "ACP_ERROR_BUFFER_TOO_SMALL", /* 2 */
    "ACP_ERROR_INVALID_FORMAT",   /* 3 */
    "ACP_ERROR_CRC_MISMATCH",     /* 4 */
    "ACP_ERROR_COBS_INVALID",     /* 5 */
    "ACP_ERROR_FRAME_TOO_LARGE",  /* 6 */
    "ACP_ERROR_NOT_FOUND",        /* 7 */
    "ACP_ERROR_IO",               /* 8 */
    "ACP_ERROR_NO_MEMORY",        /* 9 */
    "ACP_ERROR_TIMEOUT",          /* 10 */
    "ACP_ERROR_NOT_IMPLEMENTED",  /* 11 */
    "ACP_ERROR_AUTH_FAILED",      /* 12 */
    "ACP_ERROR_REPLAY_ATTACK",    /* 13 */
    "ACP_ERROR_SESSION_NOT_INIT", /* 14 */
    "ACP_ERROR_UNSUPPORTED"       /* 15 */
};

/**
 * @brief Number of frame type names
 */
const size_t acp_frame_type_count = sizeof(acp_frame_type_names) / sizeof(acp_frame_type_names[0]);

/**
 * @brief Number of error names
 */
const size_t acp_error_count = sizeof(acp_error_names) / sizeof(acp_error_names[0]);

/**
 * @brief Get frame type name for debugging
 */
const char *acp_get_frame_type_name(uint8_t frame_type)
{
    if (frame_type < acp_frame_type_count)
    {
        return acp_frame_type_names[frame_type];
    }
    return "INVALID";
}

/**
 * @brief Get error name for debugging
 */
const char *acp_get_error_name(acp_error_t error)
{
    if (error < acp_error_count)
    {
        return acp_error_names[error];
    }
    return "UNKNOWN_ERROR";
}
