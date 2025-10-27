/**
 * @file acp_framer_stub.c
 * @brief Minimal stub implementation of ACP framing functions
 *
 * This provides stub implementations to allow compilation while the proper
 * framing implementation is being developed according to the specification.
 *
 * @version 0.3.0
 * @date 2025-10-27
 */

#include "acp_protocol.h"
#include "acp_errors.h"
#include "acp_crc16.h"
#include "acp_platform_log.h"

#include <string.h>

/* ========================================================================== */
/*                         Framing Stub Functions                            */
/* ========================================================================== */

/**
 * @brief Encode frame to wire format (stub)
 */
int acp_frame_encode(const acp_frame_t *frame, uint8_t *output, size_t output_size, size_t *bytes_written)
{
    (void)frame;
    (void)output;
    (void)output_size;
    if (bytes_written)
        *bytes_written = 0;
    return ACP_ERR_NOT_IMPLEMENTED;
}

/**
 * @brief Decode wire data to frame (stub)
 */
int acp_frame_decode(const uint8_t *input, size_t input_size, acp_frame_t *frame, size_t *bytes_consumed)
{
    (void)input;
    (void)input_size;
    (void)frame;
    if (bytes_consumed)
        *bytes_consumed = 0;
    return ACP_ERR_NOT_IMPLEMENTED;
}

/**
 * @brief Calculate encoded frame size (stub)
 */
size_t acp_frame_encoded_size(const acp_frame_t *frame)
{
    (void)frame;
    return 64; /* Placeholder size */
}

/**
 * @brief Frame encoder state handling (stub)
 */
typedef struct
{
    int dummy;
} acp_frame_encoder_t;

int acp_frame_encoder_init(acp_frame_encoder_t *encoder)
{
    (void)encoder;
    return ACP_ERR_NOT_IMPLEMENTED;
}

int acp_frame_encoder_reset(acp_frame_encoder_t *encoder)
{
    (void)encoder;
    return ACP_ERR_NOT_IMPLEMENTED;
}

/**
 * @brief Frame decoder state handling (stub)
 */
typedef struct
{
    int dummy;
} acp_frame_decoder_t;

int acp_frame_decoder_init(acp_frame_decoder_t *decoder)
{
    (void)decoder;
    return ACP_ERR_NOT_IMPLEMENTED;
}

int acp_frame_decoder_reset(acp_frame_decoder_t *decoder)
{
    (void)decoder;
    return ACP_ERR_NOT_IMPLEMENTED;
}

int acp_frame_decoder_push_bytes(acp_frame_decoder_t *decoder, const uint8_t *data, size_t len)
{
    (void)decoder;
    (void)data;
    (void)len;
    return ACP_ERR_NOT_IMPLEMENTED;
}

int acp_frame_decoder_get_frame(acp_frame_decoder_t *decoder, acp_frame_t *frame)
{
    (void)decoder;
    (void)frame;
    return ACP_ERR_NOT_IMPLEMENTED;
}