/**
 * @file acp_nvs.c
 * @brief Non-volatile storage (keystore) implementation for ACP
 *
 * This module provides file-based keystore functionality for the ACP protocol.
 * Keys are stored in a simple binary format on disk.
 */

#include "acp_protocol.h"
#include "acp_platform_keystore.h"
#include "acp_errors.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

/**
 * @brief Default keystore file path
 */
#ifndef ACP_KEYSTORE_PATH
#define ACP_KEYSTORE_PATH "./acp_keystore.bin"
#endif

/**
 * @brief Keystore file magic number
 */
#define ACP_KEYSTORE_MAGIC 0x41435046 /* "ACPF" */

/**
 * @brief Keystore file version
 */
#define ACP_KEYSTORE_VERSION 1

/**
 * @brief Keystore file header
 */
typedef struct
{
    uint32_t magic;     /**< Magic number */
    uint32_t version;   /**< File format version */
    uint32_t key_count; /**< Number of keys stored */
    uint32_t reserved;  /**< Reserved for future use */
} acp_keystore_header_t;

/**
 * @brief Keystore entry
 */
typedef struct
{
    uint32_t key_id;                /**< Key identifier */
    uint8_t key_data[ACP_KEY_SIZE]; /**< Key data */
    uint32_t flags;                 /**< Key flags */
    uint32_t reserved;              /**< Reserved for future use */
} acp_keystore_entry_t;

acp_error_t acp_keystore_init(void)
{
    /* Check if keystore file exists, create if not */
    FILE *fp = fopen(ACP_KEYSTORE_PATH, "rb");
    if (fp != NULL)
    {
        fclose(fp);
        return ACP_OK;
    }

    /* Create new keystore file */
    fp = fopen(ACP_KEYSTORE_PATH, "wb");
    if (fp == NULL)
    {
        return ACP_ERROR_IO;
    }

    /* Write header */
    acp_keystore_header_t header = {
        .magic = ACP_KEYSTORE_MAGIC,
        .version = ACP_KEYSTORE_VERSION,
        .key_count = 0,
        .reserved = 0};

    if (fwrite(&header, sizeof(header), 1, fp) != 1)
    {
        fclose(fp);
        return ACP_ERROR_IO;
    }

    fclose(fp);
    return ACP_OK;
}

acp_error_t acp_keystore_get(uint32_t key_id, uint8_t *key_data, size_t key_size)
{
    if (key_data == NULL || key_size < ACP_KEY_SIZE)
    {
        return ACP_ERROR_INVALID_PARAM;
    }

    FILE *fp = fopen(ACP_KEYSTORE_PATH, "rb");
    if (fp == NULL)
    {
        return ACP_ERROR_NOT_FOUND;
    }

    /* Read header */
    acp_keystore_header_t header;
    if (fread(&header, sizeof(header), 1, fp) != 1)
    {
        fclose(fp);
        return ACP_ERROR_IO;
    }

    /* Validate header */
    if (header.magic != ACP_KEYSTORE_MAGIC || header.version != ACP_KEYSTORE_VERSION)
    {
        fclose(fp);
        return ACP_ERROR_INVALID_FORMAT;
    }

    /* Search for key */
    for (uint32_t i = 0; i < header.key_count; i++)
    {
        acp_keystore_entry_t entry;
        if (fread(&entry, sizeof(entry), 1, fp) != 1)
        {
            fclose(fp);
            return ACP_ERROR_IO;
        }

        if (entry.key_id == key_id)
        {
            memcpy(key_data, entry.key_data, ACP_KEY_SIZE);
            fclose(fp);
            return ACP_OK;
        }
    }

    fclose(fp);
    return ACP_ERROR_NOT_FOUND;
}

acp_error_t acp_keystore_set(uint32_t key_id, const uint8_t *key_data, size_t key_size)
{
    if (key_data == NULL || key_size != ACP_KEY_SIZE)
    {
        return ACP_ERROR_INVALID_PARAM;
    }

    /* Initialize keystore if it doesn't exist */
    acp_error_t result = acp_keystore_init();
    if (result != ACP_OK)
    {
        return result;
    }

    FILE *fp = fopen(ACP_KEYSTORE_PATH, "r+b");
    if (fp == NULL)
    {
        return ACP_ERROR_IO;
    }

    /* Read header */
    acp_keystore_header_t header;
    if (fread(&header, sizeof(header), 1, fp) != 1)
    {
        fclose(fp);
        return ACP_ERROR_IO;
    }

    /* Validate header */
    if (header.magic != ACP_KEYSTORE_MAGIC || header.version != ACP_KEYSTORE_VERSION)
    {
        fclose(fp);
        return ACP_ERROR_INVALID_FORMAT;
    }

    /* Search for existing key to update */
    for (uint32_t i = 0; i < header.key_count; i++)
    {
        long pos = ftell(fp);
        acp_keystore_entry_t entry;
        if (fread(&entry, sizeof(entry), 1, fp) != 1)
        {
            fclose(fp);
            return ACP_ERROR_IO;
        }

        if (entry.key_id == key_id)
        {
            /* Update existing key */
            memcpy(entry.key_data, key_data, ACP_KEY_SIZE);
            fseek(fp, pos, SEEK_SET);
            if (fwrite(&entry, sizeof(entry), 1, fp) != 1)
            {
                fclose(fp);
                return ACP_ERROR_IO;
            }
            fclose(fp);
            return ACP_OK;
        }
    }

    /* Add new key */
    acp_keystore_entry_t entry = {
        .key_id = key_id,
        .flags = 0,
        .reserved = 0};
    memcpy(entry.key_data, key_data, ACP_KEY_SIZE);

    /* Append new entry */
    fseek(fp, 0, SEEK_END);
    if (fwrite(&entry, sizeof(entry), 1, fp) != 1)
    {
        fclose(fp);
        return ACP_ERROR_IO;
    }

    /* Update header */
    header.key_count++;
    fseek(fp, 0, SEEK_SET);
    if (fwrite(&header, sizeof(header), 1, fp) != 1)
    {
        fclose(fp);
        return ACP_ERROR_IO;
    }

    fclose(fp);
    return ACP_OK;
}

acp_error_t acp_keystore_delete(uint32_t key_id)
{
    /* For simplicity, we don't implement key deletion in this version */
    (void)key_id;
    return ACP_ERROR_NOT_IMPLEMENTED;
}

acp_error_t acp_keystore_clear(void)
{
    /* Remove the keystore file */
    if (remove(ACP_KEYSTORE_PATH) != 0)
    {
        if (errno == ENOENT)
        {
            return ACP_OK; /* File doesn't exist, nothing to clear */
        }
        return ACP_ERROR_IO;
    }

    return ACP_OK;
}