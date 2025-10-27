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
 * @file acp_cobs.c
 * @brief COBS (Consistent Overhead Byte Stuffing) implementation
 *
 * @version 0.3.0
 * @date 2025-10-27
 */

#include "acp_cobs.h"
#include "acp_errors.h"
#include <string.h>

/* ========================================================================== */
/*                           Encoding Functions                              */
/* ========================================================================== */

int acp_cobs_encode(const uint8_t *input, size_t input_len,
                    uint8_t *output, size_t output_size,
                    size_t *encoded_len)
{
    if (!input || !output || !encoded_len)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    if (output_size < acp_cobs_max_encoded_size(input_len))
    {
        return ACP_ERR_BUFFER_TOO_SMALL;
    }

    const uint8_t *src = input;
    uint8_t *dst = output;
    size_t remaining = input_len;
    size_t encoded = 0;

    while (remaining > 0)
    {
        /* Find next zero byte or end of block */
        size_t block_len = 0;
        while (block_len < remaining && block_len < ACP_COBS_BLOCK_SIZE && src[block_len] != 0)
        {
            block_len++;
        }

        /* Write block length code */
        *dst++ = (uint8_t)(block_len + 1);
        encoded++;

        /* Copy non-zero bytes */
        memcpy(dst, src, block_len);
        dst += block_len;
        src += block_len;
        encoded += block_len;
        remaining -= block_len;

        /* Skip zero byte if present */
        if (remaining > 0 && *src == 0)
        {
            src++;
            remaining--;
        }
    }

    *encoded_len = encoded;
    return ACP_OK;
}

/* ========================================================================== */
/*                           Decoding Functions                              */
/* ========================================================================== */

int acp_cobs_decode(const uint8_t *input, size_t input_len,
                    uint8_t *output, size_t output_size,
                    size_t *decoded_len)
{
    if (!input || !output || !decoded_len)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    if (input_len == 0)
    {
        *decoded_len = 0;
        return ACP_OK;
    }

    const uint8_t *src = input;
    uint8_t *dst = output;
    size_t remaining = input_len;
    size_t decoded = 0;

    while (remaining > 0)
    {
        uint8_t code = *src++;
        remaining--;

        /* Validate code byte */
        if (code == 0 || code > (ACP_COBS_BLOCK_SIZE + 1))
        {
            return ACP_ERR_COBS_DECODE;
        }

        size_t block_len = code - 1;

        /* Check if we have enough input data */
        if (block_len > remaining)
        {
            return ACP_ERR_COBS_DECODE;
        }

        /* Check if we have enough output space */
        if (decoded + block_len + (remaining > block_len ? 1 : 0) > output_size)
        {
            return ACP_ERR_BUFFER_TOO_SMALL;
        }

        /* Copy block data */
        memcpy(dst, src, block_len);
        src += block_len;
        dst += block_len;
        decoded += block_len;
        remaining -= block_len;

        /* Add zero byte if this wasn't the last block */
        if (remaining > 0 && code != (ACP_COBS_BLOCK_SIZE + 1))
        {
            *dst++ = 0;
            decoded++;
        }
    }

    *decoded_len = decoded;
    return ACP_OK;
}

/* ========================================================================== */
/*                           Utility Functions                               */
/* ========================================================================== */

size_t acp_cobs_max_encoded_size(size_t input_len)
{
    if (input_len == 0)
    {
        return 1; /* At least one overhead byte */
    }
    return input_len + ACP_COBS_OVERHEAD(input_len);
}

size_t acp_cobs_max_decoded_size(size_t encoded_len)
{
    if (encoded_len == 0)
    {
        return 0;
    }
    /* Worst case: every overhead byte represents maximum block */
    return encoded_len - 1;
}

int acp_cobs_validate(const uint8_t *data, size_t len)
{
    if (!data || len == 0)
    {
        return 0;
    }

    size_t pos = 0;

    while (pos < len)
    {
        uint8_t code = data[pos];

        /* Code must be 1-255 */
        if (code == 0 || code > (ACP_COBS_BLOCK_SIZE + 1))
        {
            return 0;
        }

        size_t block_len = code - 1;
        pos += 1 + block_len;

        /* Check bounds */
        if (pos > len)
        {
            return 0;
        }

        /* Verify no zero bytes in block */
        for (size_t i = pos - block_len; i < pos; i++)
        {
            if (data[i] == 0)
            {
                return 0;
            }
        }
    }

    return 1;
}

/* ========================================================================== */
/*                         Streaming Decoder                                 */
/* ========================================================================== */

int acp_cobs_decoder_init(acp_cobs_decoder_t *decoder,
                          uint8_t *buffer, size_t buffer_size)
{
    if (!decoder || !buffer || buffer_size == 0)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    decoder->buffer = buffer;
    decoder->buffer_size = buffer_size;
    decoder->buffer_pos = 0;
    decoder->state = ACP_COBS_DECODER_IDLE;
    decoder->error_code = ACP_OK;

    return ACP_OK;
}

void acp_cobs_decoder_reset(acp_cobs_decoder_t *decoder)
{
    if (decoder)
    {
        decoder->buffer_pos = 0;
        decoder->state = ACP_COBS_DECODER_IDLE;
        decoder->error_code = ACP_OK;
    }
}

int acp_cobs_decoder_feed_byte(acp_cobs_decoder_t *decoder, uint8_t byte)
{
    if (!decoder)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    switch (decoder->state)
    {
    case ACP_COBS_DECODER_IDLE:
        if (byte == ACP_COBS_DELIMITER)
        {
            /* Stay idle, waiting for first data byte */
            return 0;
        }
        /* First non-delimiter byte starts frame */
        decoder->buffer[0] = byte;
        decoder->buffer_pos = 1;
        decoder->state = ACP_COBS_DECODER_RECEIVING;
        return 0;

    case ACP_COBS_DECODER_RECEIVING:
        if (byte == ACP_COBS_DELIMITER)
        {
            /* Frame delimiter found - frame complete */
            decoder->state = ACP_COBS_DECODER_COMPLETE;
            return 1; /* Frame ready */
        }

        /* Check buffer overflow */
        if (decoder->buffer_pos >= decoder->buffer_size)
        {
            decoder->state = ACP_COBS_DECODER_ERROR;
            decoder->error_code = ACP_ERR_BUFFER_TOO_SMALL;
            return ACP_ERR_BUFFER_TOO_SMALL;
        }

        /* Accumulate byte */
        decoder->buffer[decoder->buffer_pos++] = byte;
        return 0;

    case ACP_COBS_DECODER_COMPLETE:
        /* Already have complete frame, ignore bytes until reset */
        return 0;

    case ACP_COBS_DECODER_ERROR:
        /* In error state until reset */
        return decoder->error_code;

    default:
        decoder->state = ACP_COBS_DECODER_ERROR;
        decoder->error_code = ACP_ERR_INTERNAL;
        return ACP_ERR_INTERNAL;
    }
}

int acp_cobs_decoder_get_frame(acp_cobs_decoder_t *decoder,
                               uint8_t *output, size_t output_size,
                               size_t *decoded_len)
{
    if (!decoder || !output || !decoded_len)
    {
        return ACP_ERR_INVALID_PARAM;
    }

    if (decoder->state != ACP_COBS_DECODER_COMPLETE)
    {
        return ACP_ERR_INVALID_STATE;
    }

    int result = acp_cobs_decode(decoder->buffer, decoder->buffer_pos,
                                 output, output_size, decoded_len);

    /* Reset decoder for next frame */
    acp_cobs_decoder_reset(decoder);

    return result;
}

acp_cobs_decoder_state_t acp_cobs_decoder_get_state(const acp_cobs_decoder_t *decoder)
{
    return decoder ? decoder->state : ACP_COBS_DECODER_ERROR;
}
