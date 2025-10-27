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
 * @file acp_nvs.c
 * @brief ACP NVS Storage Integration
 *
 * Provides NVS storage integration for ACP session keys, configuration,
 * and persistent state management.
 *
 * @version 0.3
 * @date 2025-10-26
 * @author Paul Zanna
 */

#include <string.h>
#include "acp_platform_log.h"
#include "acp_platform_keystore.h"
#include "acp_protocol.h"
#include "acp_session.h"
#include "acp_nvs.h"

// ===========================================================================
// NVS NAMESPACE AND KEY CONSTANTS
// ===========================================================================

#define ACP_NVS_NAMESPACE "acp_config"
#define ACP_NVS_KEY_TELEMETRY_RATE "tlm_rate"
#define ACP_NVS_KEY_SESSION_TIMEOUT "sess_timeout"
#define ACP_NVS_KEY_RATE_LIMIT_SUSTAINED "rate_sust"
#define ACP_NVS_KEY_RATE_LIMIT_BURST "rate_burst"
#define ACP_NVS_KEY_DEBUG_ENABLED "debug_en"
#define ACP_NVS_KEY_PROTOCOL_VERSION "proto_ver"
#define ACP_NVS_KEY_CONFIG_CHECKSUM "cfg_crc"

// Configuration structure for NVS storage
typedef struct
{
    uint8_t telemetry_rate_hz;    // Telemetry transmission rate (1-10 Hz)
    uint32_t session_timeout_ms;  // Session timeout in milliseconds
    uint8_t rate_limit_sustained; // Sustained command rate (commands/sec)
    uint8_t rate_limit_burst;     // Burst command rate (commands/sec)
    bool debug_logging_enabled;   // Debug logging flag
    uint8_t protocol_version;     // Protocol version
    uint32_t config_checksum;     // Configuration integrity checksum
} acp_config_t;

// Global configuration state
static acp_config_t current_config;
static nvs_handle_t config_nvs_handle;
static bool nvs_initialized = false;

// ===========================================================================
// CONFIGURATION DEFAULTS
// ===========================================================================

/**
 * @brief Initialize configuration with safe defaults
 */
static void init_default_config(acp_config_t *config)
{
    memset(config, 0, sizeof(acp_config_t));

    config->telemetry_rate_hz = 1;                         // 1 Hz default (safe)
    config->session_timeout_ms = ACP_SESSION_TIMEOUT;      // 15 minutes
    config->rate_limit_sustained = ACP_RATE_SUSTAINED_CPS; // 5 commands/sec
    config->rate_limit_burst = ACP_RATE_BURST_CPS;         // 20 commands/sec
    config->debug_logging_enabled = false;                 // Debug off by default
    config->protocol_version = ACP_VERSION_1_1;            // Current protocol version
}

/**
 * @brief Calculate configuration checksum for integrity verification
 */
static uint32_t calculate_config_checksum(const acp_config_t *config)
{
    if (!config)
        return 0;

    uint32_t checksum = 0x5C501234; // ACP magic number
    const uint8_t *data = (const uint8_t *)config;

    // Calculate checksum excluding the checksum field itself
    size_t size = sizeof(acp_config_t) - sizeof(uint32_t);

    for (size_t i = 0; i < size; i++)
    {
        checksum = checksum * 31 + data[i];
    }

    return checksum;
}

/**
 * @brief Validate configuration parameters
 */
static bool validate_config(const acp_config_t *config)
{
    if (!config)
        return false;

    // Validate telemetry rate
    if (config->telemetry_rate_hz == 0 || config->telemetry_rate_hz > 10)
    {
        ACP_LOG_ERROR("Invalid telemetry rate: %u Hz (must be 1-10)", config->telemetry_rate_hz);
        return false;
    }

    // Validate session timeout
    if (config->session_timeout_ms < 60000 || config->session_timeout_ms > 3600000)
    {
        ACP_LOG_ERROR("Invalid session timeout: %u ms (must be 60s-1h)", config->session_timeout_ms);
        return false;
    }

    // Validate rate limiting
    if (config->rate_limit_sustained == 0 || config->rate_limit_sustained > 50)
    {
        ACP_LOG_ERROR("Invalid sustained rate: %u (must be 1-50)", config->rate_limit_sustained);
        return false;
    }

    if (config->rate_limit_burst < config->rate_limit_sustained || config->rate_limit_burst > 100)
    {
        ACP_LOG_ERROR("Invalid burst rate: %u (must be >= sustained and <= 100)", config->rate_limit_burst);
        return false;
    }

    // Validate protocol version
    if (config->protocol_version != ACP_VERSION_1_1)
    {
        ACP_LOG_ERROR("Unsupported protocol version: 0x%02X", config->protocol_version);
        return false;
    }

    // Verify checksum
    uint32_t expected_checksum = calculate_config_checksum(config);
    if (config->config_checksum != expected_checksum)
    {
        ACP_LOG_ERROR("Configuration checksum mismatch: 0x%08X != 0x%08X",
                      config->config_checksum, expected_checksum);
        return false;
    }

    return true;
}

// ===========================================================================
// NVS STORAGE OPERATIONS
// ===========================================================================

/**
 * @brief Initialize NVS storage for ACP configuration
 */
bool acp_nvs_init(void)
{
    if (nvs_initialized)
    {
        ACP_LOG_WARN("ACP NVS already initialized");
        return true;
    }

    ACP_LOG_INFO("Initializing ACP NVS storage");

    // Initialize default configuration
    init_default_config(&current_config);
    current_config.config_checksum = calculate_config_checksum(&current_config);

    // TODO: Replace with platform_keystore_open(...)
    int err = 0; // platform_keystore_open(ACP_NVS_NAMESPACE, &config_nvs_handle);
    if (err != ACP_PLATFORM_OK)
    {
        ACP_LOG_ERROR("Failed to open keystore namespace: %s", "platform_error");
        return false;
    }

    nvs_initialized = true;

    // Try to load existing configuration
    if (acp_nvs_load_config())
    {
        ACP_LOG_INFO("Loaded existing ACP configuration from keystore");
    }
    else
    {
        ACP_LOG_INFO("Using default ACP configuration");
        // Save default configuration to keystore
        acp_nvs_save_config();
    }

    ACP_LOG_INFO("ACP keystore initialization complete");
    return true;
}

/**
 * @brief Load ACP configuration from NVS
 */
bool acp_nvs_load_config(void)
{
    if (!nvs_initialized)
    {
        ACP_LOG_ERROR("Keystore not initialized");
        return false;
    }

    acp_config_t loaded_config;
    size_t required_size = sizeof(acp_config_t);

    // TODO: Replace with platform_keystore_get_blob(...)
    int err = 0; // platform_keystore_get_blob(config_nvs_handle, "config", &loaded_config, &required_size);
    if (err != ACP_PLATFORM_OK)
    {
        ACP_LOG_INFO("No existing configuration found in keystore");
        return false;
    }

    // Validate loaded configuration
    if (!validate_config(&loaded_config))
    {
        ACP_LOG_ERROR("Loaded configuration is invalid, using defaults");
        return false;
    }

    // Configuration is valid, update current config
    memcpy(&current_config, &loaded_config, sizeof(acp_config_t));

    ACP_LOG_INFO("Configuration loaded: rate=%u Hz, timeout=%u ms, debug=%s",
                 current_config.telemetry_rate_hz,
                 current_config.session_timeout_ms,
                 current_config.debug_logging_enabled ? "ON" : "OFF");

    return true;
}

/**
 * @brief Save ACP configuration to NVS
 */
bool acp_nvs_save_config(void)
{
    if (!nvs_initialized)
    {
        ACP_LOG_ERROR("Keystore not initialized");
        return false;
    }

    // Update checksum before saving
    current_config.config_checksum = calculate_config_checksum(&current_config);

    // Validate before saving
    if (!validate_config(&current_config))
    {
        ACP_LOG_ERROR("Cannot save invalid configuration");
        return false;
    }

    // TODO: Replace with platform_keystore_set_blob(...) and platform_keystore_commit(...)
    int err = 0; // platform_keystore_set_blob(config_nvs_handle, "config", &current_config, sizeof(acp_config_t));
    if (err != ACP_PLATFORM_OK)
    {
        ACP_LOG_ERROR("Failed to save configuration: %s", "platform_error");
        return false;
    }
    err = 0; // platform_keystore_commit(config_nvs_handle);
    if (err != ACP_PLATFORM_OK)
    {
        ACP_LOG_ERROR("Failed to commit configuration: %s", "platform_error");
        return false;
    }

    ACP_LOG_INFO("Configuration saved to keystore");
    return true;
}

/**
 * @brief Reset configuration to defaults and save
 */
bool acp_nvs_reset_config(void)
{
    if (!nvs_initialized)
    {
        ACP_LOG_ERROR("Keystore not initialized");
        return false;
    }

    ACP_LOG_INFO("Resetting ACP configuration to defaults");

    init_default_config(&current_config);
    current_config.config_checksum = calculate_config_checksum(&current_config);

    return acp_nvs_save_config();
}

// ===========================================================================
// CONFIGURATION ACCESSORS
// ===========================================================================

/**
 * @brief Get current telemetry rate
 */
uint8_t acp_nvs_get_telemetry_rate(void)
{
    return nvs_initialized ? current_config.telemetry_rate_hz : 1;
}

/**
 * @brief Set telemetry rate
 */
bool acp_nvs_set_telemetry_rate(uint8_t rate_hz)
{
    if (!nvs_initialized)
        return false;

    if (rate_hz == 0 || rate_hz > 10)
    {
        ACP_LOG_ERROR("Invalid telemetry rate: %u (must be 1-10)", rate_hz);
        return false;
    }

    current_config.telemetry_rate_hz = rate_hz;
    ACP_LOG_INFO("Telemetry rate updated to %u Hz", rate_hz);

    return acp_nvs_save_config();
}

/**
 * @brief Get current session timeout
 */
uint32_t acp_nvs_get_session_timeout(void)
{
    return nvs_initialized ? current_config.session_timeout_ms : ACP_SESSION_TIMEOUT;
}

/**
 * @brief Set session timeout
 */
bool acp_nvs_set_session_timeout(uint32_t timeout_ms)
{
    if (!nvs_initialized)
        return false;

    if (timeout_ms < 60000 || timeout_ms > 3600000)
    {
        ACP_LOG_ERROR("Invalid session timeout: %u ms (must be 60s-1h)", timeout_ms);
        return false;
    }

    current_config.session_timeout_ms = timeout_ms;
    ACP_LOG_INFO("Session timeout updated to %u ms", timeout_ms);

    return acp_nvs_save_config();
}

/**
 * @brief Get sustained rate limit
 */
uint8_t acp_nvs_get_rate_limit_sustained(void)
{
    return nvs_initialized ? current_config.rate_limit_sustained : ACP_RATE_SUSTAINED_CPS;
}

/**
 * @brief Set sustained rate limit
 */
bool acp_nvs_set_rate_limit_sustained(uint8_t rate_cps)
{
    if (!nvs_initialized)
        return false;

    if (rate_cps == 0 || rate_cps > 50)
    {
        ACP_LOG_ERROR("Invalid sustained rate: %u (must be 1-50)", rate_cps);
        return false;
    }

    if (rate_cps > current_config.rate_limit_burst)
    {
        ACP_LOG_ERROR("Sustained rate cannot exceed burst rate (%u)", current_config.rate_limit_burst);
        return false;
    }

    current_config.rate_limit_sustained = rate_cps;
    ACP_LOG_INFO("Sustained rate limit updated to %u CPS", rate_cps);

    return acp_nvs_save_config();
}

/**
 * @brief Get burst rate limit
 */
uint8_t acp_nvs_get_rate_limit_burst(void)
{
    return nvs_initialized ? current_config.rate_limit_burst : ACP_RATE_BURST_CPS;
}

/**
 * @brief Set burst rate limit
 */
bool acp_nvs_set_rate_limit_burst(uint8_t rate_cps)
{
    if (!nvs_initialized)
        return false;

    if (rate_cps < current_config.rate_limit_sustained || rate_cps > 100)
    {
        ACP_LOG_ERROR("Invalid burst rate: %u (must be >= %u and <= 100)",
                      rate_cps, current_config.rate_limit_sustained);
        return false;
    }

    current_config.rate_limit_burst = rate_cps;
    ACP_LOG_INFO("Burst rate limit updated to %u CPS", rate_cps);

    return acp_nvs_save_config();
}

/**
 * @brief Get debug logging state
 */
bool acp_nvs_get_debug_logging(void)
{
    return nvs_initialized ? current_config.debug_logging_enabled : false;
}

/**
 * @brief Set debug logging state
 */
bool acp_nvs_set_debug_logging(bool enabled)
{
    if (!nvs_initialized)
        return false;

    current_config.debug_logging_enabled = enabled;
    ACP_LOG_INFO("Debug logging %s", enabled ? "ENABLED" : "DISABLED");

    return acp_nvs_save_config();
}

/**
 * @brief Get protocol version
 */
uint8_t acp_nvs_get_protocol_version(void)
{
    return nvs_initialized ? current_config.protocol_version : ACP_VERSION_1_1;
}

// ===========================================================================
// CONFIGURATION DUMP AND DIAGNOSTICS
// ===========================================================================

/**
 * @brief Log current configuration
 */
void acp_nvs_dump_config(void)
{
    if (!nvs_initialized)
    {
        ACP_LOG_WARN("Keystore not initialized - cannot dump configuration");
        return;
    }

    ACP_LOG_INFO("=== ACP Configuration ===");
    ACP_LOG_INFO("  Protocol Version: v%d.%d (0x%02X)",
                 ACP_VERSION_MAJOR, ACP_VERSION_MINOR, current_config.protocol_version);
    ACP_LOG_INFO("  Telemetry Rate: %u Hz", current_config.telemetry_rate_hz);
    ACP_LOG_INFO("  Session Timeout: %u ms (%u minutes)",
                 current_config.session_timeout_ms, current_config.session_timeout_ms / 60000);
    ACP_LOG_INFO("  Rate Limits: %u sustained, %u burst CPS",
                 current_config.rate_limit_sustained, current_config.rate_limit_burst);
    ACP_LOG_INFO("  Debug Logging: %s", current_config.debug_logging_enabled ? "ENABLED" : "DISABLED");
    ACP_LOG_INFO("  Config Checksum: 0x%08X", current_config.config_checksum);
    ACP_LOG_INFO("========================");
}

/**
 * @brief Get configuration as read-only structure
 */
const acp_config_t *acp_nvs_get_config(void)
{
    return nvs_initialized ? &current_config : NULL;
}

/**
 * @brief Validate NVS storage integrity
 */
bool acp_nvs_validate_storage(void)
{
    if (!nvs_initialized)
    {
        ACP_LOG_ERROR("Keystore not initialized");
        return false;
    }

    // Try to read configuration and validate
    acp_config_t test_config;
    size_t required_size = sizeof(acp_config_t);

    // TODO: Replace with platform_keystore_get_blob(...)
    int err = 0; // platform_keystore_get_blob(config_nvs_handle, "config", &test_config, &required_size);
    if (err != ACP_PLATFORM_OK)
    {
        ACP_LOG_ERROR("Failed to read stored configuration: %s", "platform_error");
        return false;
    }

    bool valid = validate_config(&test_config);

    if (valid)
    {
        ACP_LOG_INFO("Keystore storage validation passed");
    }
    else
    {
        ACP_LOG_ERROR("Keystore storage validation failed");
    }

    return valid;
}

/**
 * @brief Erase all ACP configuration from NVS
 */
bool acp_nvs_erase_all(void)
{
    if (!nvs_initialized)
    {
        ACP_LOG_ERROR("Keystore not initialized");
        return false;
    }

    ACP_LOG_WARN("Erasing all ACP configuration from keystore");

    // TODO: Replace with platform_keystore_erase_all(...) and platform_keystore_commit(...)
    int err = 0; // platform_keystore_erase_all(config_nvs_handle);
    if (err != ACP_PLATFORM_OK)
    {
        ACP_LOG_ERROR("Failed to erase keystore data: %s", "platform_error");
        return false;
    }
    err = 0; // platform_keystore_commit(config_nvs_handle);
    if (err != ACP_PLATFORM_OK)
    {
        ACP_LOG_ERROR("Failed to commit keystore erase: %s", "platform_error");
        return false;
    }

    // Reset to defaults
    init_default_config(&current_config);
    current_config.config_checksum = calculate_config_checksum(&current_config);

    ACP_LOG_INFO("Keystore data erased, configuration reset to defaults");
    return true;
}