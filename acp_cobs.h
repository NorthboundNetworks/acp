/**
 * @file acp_cobs.h
 * @brief COBS (Consistent Overhead Byte Stuffing) implementation for ACP
 *
 * Provides frame boundary detection and encoding/decoding for the ACP protocol.
 * COBS allows reliable framing over unreliable byte streams by eliminating
 * zero bytes and using them as frame delimiters.
 *
 * @version 0.3.0
 * @date 2025-10-27
 */

#ifndef ACP_COBS_H
#define ACP_COBS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stddef.h>

/* ========================================================================== */
/*                              Constants                                     */
/* ========================================================================== */

/** @brief COBS frame delimiter (zero byte) */
#define ACP_COBS_DELIMITER 0x00

/** @brief Maximum overhead per COBS block (254 bytes) */
#define ACP_COBS_BLOCK_SIZE 254

/** @brief Calculate COBS overhead for given data length */
#define ACP_COBS_OVERHEAD(len) (((len) / ACP_COBS_BLOCK_SIZE) + 1)

    /* ========================================================================== */
    /*                              Functions                                     */
    /* ========================================================================== */

    /**
     * @brief Encode data using COBS algorithm
     *
     * Encodes input data to eliminate zero bytes, allowing zero to be used
     * as a reliable frame delimiter. The encoded data is guaranteed to contain
     * no zero bytes.
     *
     * @param input Input data to encode
     * @param input_len Length of input data in bytes
     * @param output Output buffer for encoded data (must be large enough)
     * @param output_size Size of output buffer
     * @param encoded_len Returns the actual encoded length
     * @return 0 on success, negative error code on failure
     */
    int acp_cobs_encode(const uint8_t *input, size_t input_len,
                        uint8_t *output, size_t output_size,
                        size_t *encoded_len);

    /**
     * @brief Decode COBS-encoded data
     *
     * Decodes COBS-encoded data back to original form. The input must be
     * a complete COBS-encoded block without frame delimiters.
     *
     * @param input COBS-encoded input data
     * @param input_len Length of encoded input data
     * @param output Output buffer for decoded data
     * @param output_size Size of output buffer
     * @param decoded_len Returns the actual decoded length
     * @return 0 on success, negative error code on failure
     */
    int acp_cobs_decode(const uint8_t *input, size_t input_len,
                        uint8_t *output, size_t output_size,
                        size_t *decoded_len);

    /**
     * @brief Calculate maximum encoded size for given input length
     *
     * @param input_len Input data length
     * @return Maximum possible encoded length (including overhead)
     */
    size_t acp_cobs_max_encoded_size(size_t input_len);

    /**
     * @brief Calculate maximum decoded size for given encoded length
     *
     * @param encoded_len Encoded data length
     * @return Maximum possible decoded length
     */
    size_t acp_cobs_max_decoded_size(size_t encoded_len);

    /**
     * @brief Validate COBS-encoded data format
     *
     * Checks if the provided data appears to be valid COBS-encoded data
     * by verifying the overhead bytes are within valid ranges.
     *
     * @param data COBS-encoded data to validate
     * @param len Length of data
     * @return 1 if valid, 0 if invalid
     */
    int acp_cobs_validate(const uint8_t *data, size_t len);

    /* ========================================================================== */
    /*                          Decoder State Machine                            */
    /* ========================================================================== */

    /**
     * @brief COBS decoder states for streaming decode
     */
    typedef enum
    {
        ACP_COBS_DECODER_IDLE,      /**< Waiting for frame start */
        ACP_COBS_DECODER_RECEIVING, /**< Receiving frame data */
        ACP_COBS_DECODER_COMPLETE,  /**< Frame complete, ready to decode */
        ACP_COBS_DECODER_ERROR      /**< Decoder error state */
    } acp_cobs_decoder_state_t;

    /**
     * @brief COBS streaming decoder context
     */
    typedef struct
    {
        uint8_t *buffer;                /**< Frame buffer */
        size_t buffer_size;             /**< Buffer size */
        size_t buffer_pos;              /**< Current position in buffer */
        acp_cobs_decoder_state_t state; /**< Decoder state */
        int error_code;                 /**< Last error code */
    } acp_cobs_decoder_t;

    /**
     * @brief Initialize COBS streaming decoder
     *
     * @param decoder Decoder context to initialize
     * @param buffer Frame buffer for accumulating data
     * @param buffer_size Size of frame buffer
     * @return 0 on success, negative error code on failure
     */
    int acp_cobs_decoder_init(acp_cobs_decoder_t *decoder,
                              uint8_t *buffer, size_t buffer_size);

    /**
     * @brief Reset COBS decoder to idle state
     *
     * @param decoder Decoder context to reset
     */
    void acp_cobs_decoder_reset(acp_cobs_decoder_t *decoder);

    /**
     * @brief Feed bytes to COBS streaming decoder
     *
     * Processes incoming bytes and accumulates frame data. When a complete
     * frame is received (delimiter found), the decoder state becomes COMPLETE.
     *
     * @param decoder Decoder context
     * @param byte Input byte to process
     * @return 1 if frame complete, 0 if more data needed, negative on error
     */
    int acp_cobs_decoder_feed_byte(acp_cobs_decoder_t *decoder, uint8_t byte);

    /**
     * @brief Get decoded frame from streaming decoder
     *
     * Should be called when decoder state is COMPLETE to retrieve the
     * decoded frame data.
     *
     * @param decoder Decoder context
     * @param output Output buffer for decoded frame
     * @param output_size Size of output buffer
     * @param decoded_len Returns decoded frame length
     * @return 0 on success, negative error code on failure
     */
    int acp_cobs_decoder_get_frame(acp_cobs_decoder_t *decoder,
                                   uint8_t *output, size_t output_size,
                                   size_t *decoded_len);

    /**
     * @brief Get current decoder state
     *
     * @param decoder Decoder context
     * @return Current decoder state
     */
    acp_cobs_decoder_state_t acp_cobs_decoder_get_state(const acp_cobs_decoder_t *decoder);

#ifdef __cplusplus
}
#endif

#endif /* ACP_COBS_H */
