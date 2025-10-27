/**
 * @file acp_crc16.c
 * @brief CRC16-CCITT implementation for ACP frame integrity
 *
 * This module implements CRC16-CCITT (ITU-T X.25) with polynomial 0x1021,
 * initial value 0xFFFF, and no final XOR.
 *
 * @version 0.3.0
 * @date 2025-10-27
 */

#include "acp_crc16.h"
#include <string.h>

/* ========================================================================== */
/*                              CRC16 Lookup Table                           */
/* ========================================================================== */

/** @brief CRC16-CCITT lookup table (computed at runtime or compile-time) */
static uint16_t crc16_table[256];
static int table_initialized = 0;

/**
 * @brief Initialize CRC16 lookup table
 */
void acp_crc16_init_table(void)
{
    if (table_initialized)
    {
        return; /* Already initialized */
    }

    for (int i = 0; i < 256; i++)
    {
        uint16_t crc = (uint16_t)(i << 8);

        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ ACP_CRC16_POLY;
            }
            else
            {
                crc = crc << 1;
            }
        }

        crc16_table[i] = crc;
    }

    table_initialized = 1;
}

/**
 * @brief Get CRC16 lookup table
 */
const uint16_t *acp_crc16_get_table(void)
{
    acp_crc16_init_table();
    return crc16_table;
}

/* ========================================================================== */
/*                             CRC16 Functions                               */
/* ========================================================================== */

/**
 * @brief Initialize CRC16 calculation state
 */
uint16_t acp_crc16_init(void)
{
    return ACP_CRC16_INIT;
}

/**
 * @brief Update CRC16 with a single byte
 */
uint16_t acp_crc16_update_byte(uint16_t crc, uint8_t byte)
{
    acp_crc16_init_table(); /* Ensure table is initialized */

    uint8_t table_index = (uint8_t)((crc >> 8) ^ byte);
    return (crc << 8) ^ crc16_table[table_index];
}

/**
 * @brief Update CRC16 calculation with new data
 */
uint16_t acp_crc16_update(uint16_t crc, const uint8_t *data, size_t length)
{
    if (data == NULL || length == 0)
    {
        return crc;
    }

    acp_crc16_init_table(); /* Ensure table is initialized */

    for (size_t i = 0; i < length; i++)
    {
        crc = acp_crc16_update_byte(crc, data[i]);
    }

    return crc;
}

/**
 * @brief Finalize CRC16 calculation
 */
uint16_t acp_crc16_finalize(uint16_t crc)
{
    /* For CRC16-CCITT, no final XOR is applied */
    return crc ^ ACP_CRC16_FINAL_XOR;
}

/**
 * @brief Calculate CRC16-CCITT for a buffer
 */
uint16_t acp_crc16_calculate(const uint8_t *data, size_t length)
{
    if (data == NULL)
    {
        return 0;
    }

    uint16_t crc = acp_crc16_init();
    crc = acp_crc16_update(crc, data, length);
    return acp_crc16_finalize(crc);
}

/**
 * @brief Verify CRC16 checksum
 */
int acp_crc16_verify(const uint8_t *data, size_t length, uint16_t expected_crc)
{
    uint16_t calculated_crc = acp_crc16_calculate(data, length);
    return (calculated_crc == expected_crc) ? 1 : 0;
}

/* ========================================================================== */
/*                              Test Vectors                                 */
/* ========================================================================== */

/** @brief Test data strings */
static const char test_empty[] = "";
static const char test_a[] = "A";
static const char test_123[] = "123456789";
static const char test_hello[] = "Hello, World!";
static const char test_acp[] = "ACP Protocol Test Vector";

/** @brief CRC16-CCITT test vectors */
static const acp_crc16_test_vector_t test_vectors[] = {
    {
        .name = "Empty string",
        .data = (const uint8_t *)test_empty,
        .length = 0,
        .expected_crc = 0xFFFF /* Initial value with no data */
    },
    {
        .name = "Single byte 'A'",
        .data = (const uint8_t *)test_a,
        .length = 1,
        .expected_crc = 0xB915 /* CRC16-CCITT of 'A' */
    },
    {
        .name = "ASCII '123456789'",
        .data = (const uint8_t *)test_123,
        .length = 9,
        .expected_crc = 0x29B1 /* Standard CRC16-CCITT test vector */
    },
    {
        .name = "Hello, World!",
        .data = (const uint8_t *)test_hello,
        .length = 13,
        .expected_crc = 0x4B37 /* CRC16-CCITT of "Hello, World!" */
    },
    {
        .name = "ACP Protocol Test Vector",
        .data = (const uint8_t *)test_acp,
        .length = 24,
        .expected_crc = 0x8F5D /* CRC16-CCITT of test string */
    }};

/**
 * @brief Get CRC16 test vectors
 */
const acp_crc16_test_vector_t *acp_crc16_get_test_vectors(size_t *count)
{
    if (count != NULL)
    {
        *count = sizeof(test_vectors) / sizeof(test_vectors[0]);
    }
    return test_vectors;
}

/**
 * @brief Run CRC16 self-test
 */
int acp_crc16_self_test(void)
{
    size_t test_count;
    const acp_crc16_test_vector_t *vectors = acp_crc16_get_test_vectors(&test_count);
    int failures = 0;

    for (size_t i = 0; i < test_count; i++)
    {
        uint16_t calculated = acp_crc16_calculate(vectors[i].data, vectors[i].length);

        if (calculated != vectors[i].expected_crc)
        {
            failures++;
        }
    }

    return failures;
}