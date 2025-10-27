/**
 * @file acp_crc16.h
 * @brief CRC16-CCITT implementation for ACP frame integrity
 *
 * This module implements CRC16-CCITT (ITU-T X.25) with polynomial 0x1021,
 * initial value 0xFFFF, and no final XOR. This provides frame integrity
 * checking for ACP protocol frames.
 *
 * @version 0.3.0
 * @date 2025-10-27
 * @copyright Copyright (c) 2025 Northbound Networks Pty. Ltd. MIT License.
 */

#ifndef ACP_CRC16_H
#define ACP_CRC16_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* ========================================================================== */
/*                               Constants                                    */
/* ========================================================================== */

/** @brief CRC16-CCITT polynomial (x^16 + x^12 + x^5 + 1) */
#define ACP_CRC16_POLY 0x1021

/** @brief CRC16 initial value */
#define ACP_CRC16_INIT 0xFFFF

/** @brief CRC16 final XOR value (no XOR for CCITT) */
#define ACP_CRC16_FINAL_XOR 0x0000

/** @brief CRC16 result size in bytes */
#define ACP_CRC16_SIZE 2

    /* ========================================================================== */
    /*                            CRC16 Functions                                */
    /* ========================================================================== */

    /**
     * @brief Calculate CRC16-CCITT for a buffer
     *
     * Calculates CRC16-CCITT checksum for the given data buffer using
     * polynomial 0x1021, initial value 0xFFFF.
     *
     * @param[in] data    Pointer to data buffer
     * @param[in] length  Length of data in bytes
     *
     * @return CRC16 checksum (16-bit value)
     */
    uint16_t acp_crc16_calculate(const uint8_t *data, size_t length);

    /**
     * @brief Initialize CRC16 calculation state
     *
     * Initializes CRC16 state for incremental calculation.
     *
     * @return Initial CRC16 state value
     */
    uint16_t acp_crc16_init(void);

    /**
     * @brief Update CRC16 calculation with new data
     *
     * Updates CRC16 checksum with additional data. This allows for
     * incremental calculation over multiple data chunks.
     *
     * @param[in] crc     Current CRC16 state
     * @param[in] data    Pointer to data buffer
     * @param[in] length  Length of data in bytes
     *
     * @return Updated CRC16 state
     */
    uint16_t acp_crc16_update(uint16_t crc, const uint8_t *data, size_t length);

    /**
     * @brief Finalize CRC16 calculation
     *
     * Applies final XOR to complete CRC16 calculation.
     * For CRC16-CCITT, this is a no-op (final XOR is 0).
     *
     * @param[in] crc CRC16 state from update operations
     *
     * @return Final CRC16 checksum
     */
    uint16_t acp_crc16_finalize(uint16_t crc);

    /**
     * @brief Update CRC16 with a single byte
     *
     * Optimized function to update CRC16 with a single byte.
     *
     * @param[in] crc  Current CRC16 state
     * @param[in] byte Data byte to process
     *
     * @return Updated CRC16 state
     */
    uint16_t acp_crc16_update_byte(uint16_t crc, uint8_t byte);

    /**
     * @brief Verify CRC16 checksum
     *
     * Verifies that the CRC16 checksum of data matches the expected value.
     *
     * @param[in] data         Pointer to data buffer
     * @param[in] length       Length of data in bytes
     * @param[in] expected_crc Expected CRC16 value
     *
     * @return 1 if CRC matches, 0 if mismatch
     */
    int acp_crc16_verify(const uint8_t *data, size_t length, uint16_t expected_crc);

    /* ========================================================================== */
    /*                           Table-Based Implementation                       */
    /* ========================================================================== */

    /**
     * @brief Get CRC16 lookup table
     *
     * Returns pointer to the 256-entry CRC16 lookup table for table-based
     * implementations. The table is computed at compile time or runtime.
     *
     * @return Pointer to CRC16 lookup table (256 entries)
     */
    const uint16_t *acp_crc16_get_table(void);

    /**
     * @brief Initialize CRC16 lookup table
     *
     * Initializes the CRC16 lookup table. Called automatically by other
     * CRC functions, but can be called explicitly for performance.
     *
     * This function is thread-safe and idempotent.
     */
    void acp_crc16_init_table(void);

    /* ========================================================================== */
    /*                              Test Vectors                                 */
    /* ========================================================================== */

    /**
     * @brief CRC16 test vector structure
     */
    typedef struct
    {
        const char *name;      /**< Test case name */
        const uint8_t *data;   /**< Test data */
        size_t length;         /**< Data length */
        uint16_t expected_crc; /**< Expected CRC16 result */
    } acp_crc16_test_vector_t;

    /**
     * @brief Get CRC16 test vectors
     *
     * Returns array of test vectors for validating CRC16 implementation.
     *
     * @param[out] count Number of test vectors returned
     *
     * @return Pointer to array of test vectors
     */
    const acp_crc16_test_vector_t *acp_crc16_get_test_vectors(size_t *count);

    /**
     * @brief Run CRC16 self-test
     *
     * Runs built-in test vectors to validate CRC16 implementation.
     *
     * @return Number of test failures (0 = all tests passed)
     */
    int acp_crc16_self_test(void);

/* ========================================================================== */
/*                            Utility Macros                                 */
/* ========================================================================== */

/**
 * @brief Calculate CRC16 for a string literal
 *
 * Compile-time macro for calculating CRC16 of string literals.
 * Note: This is a compile-time approximation; use acp_crc16_calculate()
 * for runtime calculations.
 */
#define ACP_CRC16_STRING(str) acp_crc16_calculate((const uint8_t *)(str), sizeof(str) - 1)

#ifdef __cplusplus
}
#endif

#endif /* ACP_CRC16_H */
