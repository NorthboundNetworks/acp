# ACP Keystore Implementation and Management

## Overview

The ACP keystore provides secure key storage and management for HMAC-SHA256 authentication. This document describes the file format, key rotation procedures, and integration patterns.

## Keystore Architecture

### Design Principles

- **Simple File Format**: Human-readable for debugging, machine-parseable
- **Atomic Updates**: Key rotation operations are atomic
- **Security Boundaries**: Keys stored with appropriate access controls
- **Platform Abstraction**: Customizable storage backend

### Default Implementation

ACP includes a default file-based keystore (`acp_nvs.c`) that stores keys in:

**Storage Locations:**

- **Linux/macOS**: `$HOME/.acp/keystore` or `/etc/acp/keystore`
- **Windows**: `C:\ProgramData\ACP\keystore`
- **Embedded**: Custom path via platform shim

## File Format Specification

### Keystore File Structure

```text
# ACP Keystore v0.3
# Generated: 2025-10-27T10:30:00Z
# 
# Format: key_id:key_hex
# Keys are 32-byte HMAC-SHA256 keys in hexadecimal

# Production keys
0x00000001:0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF
0x00000002:FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210FEDCBA9876543210

# Test/development keys  
0x0000FFFF:DEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEFDEADBEEF
```

### File Format Details

**Header:**

- Comments start with `#`
- Version identifier for compatibility
- Optional timestamp and metadata

**Key Entries:**

- Format: `key_id:key_hex`
- Key ID: 32-bit unsigned integer (hex or decimal)
- Key Data: 64-character hexadecimal string (32 bytes)
- One key per line

**Validation Rules:**

- Key IDs must be unique within the file
- Keys must be exactly 32 bytes (64 hex characters)
- Invalid entries are ignored with warning logs
- Empty lines and comments are allowed

### Example Keystore File

```text
# ACP Keystore Configuration
# Project: sensor_network_v2
# Last updated: 2025-10-27
#
# Key ID assignments:
# 0x00000001-0x000000FF: Device keys
# 0x00000100-0x000001FF: Admin keys  
# 0x0000FF00-0x0000FFFF: Test keys

# Production device key
1:A1B2C3D4E5F6789012345678901234567890123456789012345678901234567890

# Admin master key
256:FFFFEEEEDDDDCCCCBBBBAAAA999988887777666655554444333322221111000

# Test key for unit tests
65535:0000111122223333444455556666777788889999AAAABBBBCCCCDDDDEEEEFFFF
```

## Key Management Operations

### Loading Keys

```c
// Initialize keystore (loads from file)
int result = acp_keystore_init();
if (result != ACP_OK) {
    printf("Keystore initialization failed: %d\n", result);
    return result;
}

// Get specific key
uint8_t key_buffer[32];
result = acp_keystore_get_key(0x00000001, key_buffer, sizeof(key_buffer));
if (result == ACP_OK) {
    // Use key for session initialization
    acp_session_init(&session, 0x00000001, key_buffer, 32, nonce);
}
```

### Adding New Keys

```c
// Generate new 32-byte key (use cryptographically secure random)
uint8_t new_key[32];
if (!generate_secure_random(new_key, 32)) {
    return ACP_ERR_CRYPTO;
}

// Store key with ID 0x100
result = acp_keystore_store_key(0x100, new_key, 32);
if (result != ACP_OK) {
    printf("Failed to store key: %d\n", result);
}

// Key is now available for sessions
acp_session_init(&session, 0x100, new_key, 32, nonce);
```

### Key Rotation Procedure

**Manual Key Rotation:**

```bash
# 1. Generate new key (use proper cryptographic tools)
openssl rand -hex 32 > new_key.txt

# 2. Add new key to keystore
echo "0x00000042:$(cat new_key.txt)" >> ~/.acp/keystore

# 3. Update application configuration to use new key ID
# 4. Verify new key works with test session
# 5. Remove old key from keystore (if desired)
```

**Programmatic Key Rotation:**

```c
int rotate_device_key(uint32_t old_key_id, uint32_t new_key_id) {
    uint8_t new_key[32];
    
    // Generate cryptographically secure key
    if (!platform_secure_random(new_key, 32)) {
        return ACP_ERR_CRYPTO;
    }
    
    // Store new key
    int result = acp_keystore_store_key(new_key_id, new_key, 32);
    if (result != ACP_OK) {
        return result;
    }
    
    // Test new key with temporary session
    acp_session_t test_session;
    result = acp_session_init(&test_session, new_key_id, new_key, 32, 12345);
    if (result != ACP_OK) {
        acp_keystore_delete(new_key_id);  // Cleanup on failure
        return result;
    }
    
    // Optionally remove old key
    // acp_keystore_delete(old_key_id);
    
    return ACP_OK;
}
```

## Security Considerations

### File Permissions

**Recommended file permissions:**

```bash
# Linux/macOS
chmod 600 ~/.acp/keystore        # Owner read/write only
chown $USER ~/.acp/keystore      # Owned by application user

# Windows (via PowerShell)
icacls C:\ProgramData\ACP\keystore /grant:r "%USERNAME%:(R,W)" /inheritance:r
```

### Key Generation

**Secure key generation requirements:**

```c
// Good: Use platform secure random
#ifdef __linux__
    int fd = open("/dev/urandom", O_RDONLY);
    read(fd, key_buffer, 32);
    close(fd);
#elif _WIN32
    // Use CryptGenRandom or BCryptGenRandom
#endif

// Bad: Do not use predictable sources
// time(NULL), rand(), getpid(), etc.
```

### Key Storage Best Practices

1. **Access Control**: Limit file access to application user only
2. **Backup Strategy**: Secure backup of keystore with proper encryption
3. **Key Rotation**: Regular rotation of cryptographic keys
4. **Audit Logging**: Log key access and modification events
5. **Environment Separation**: Different keys for dev/staging/production

## Platform-Specific Integration

### Custom Keystore Backend

To implement custom keystore (hardware security module, database, etc.):

```c
// Override platform keystore functions
int acp_platform_keystore_get_key(uint32_t key_id, uint8_t *key_buffer, size_t buffer_size) {
    // Your custom implementation:
    // - HSM integration
    // - Database lookup  
    // - Cloud key management service
    // - Hardware secure element
    
    return my_secure_key_lookup(key_id, key_buffer, buffer_size);
}

int acp_platform_keystore_store_key(uint32_t key_id, const uint8_t *key_data, size_t key_len) {
    // Your custom key storage implementation
    return my_secure_key_store(key_id, key_data, key_len);
}
```

### Embedded Systems Integration

**ROM/Flash-based keystore:**

```c
// Static key table in flash memory
static const struct {
    uint32_t key_id;
    uint8_t key_data[32];
} embedded_keys[] = {
    {0x00000001, {0x01, 0x23, 0x45, /* ... */}},
    {0x00000002, {0xFE, 0xDC, 0xBA, /* ... */}},
};

int acp_platform_keystore_get_key(uint32_t key_id, uint8_t *key_buffer, size_t buffer_size) {
    for (size_t i = 0; i < sizeof(embedded_keys)/sizeof(embedded_keys[0]); i++) {
        if (embedded_keys[i].key_id == key_id) {
            memcpy(key_buffer, embedded_keys[i].key_data, 32);
            return ACP_OK;
        }
    }
    return ACP_ERR_NOT_FOUND;
}
```

## Troubleshooting

### Common Issues

**Keystore file not found:**

```
[ERROR] Keystore file not found: /home/user/.acp/keystore
```

**Solution:** Create keystore directory and file with proper permissions

**Invalid key format:**

```
[WARN] Invalid key entry: key_id_123:invalid_hex_string
```

**Solution:** Ensure keys are exactly 64 hex characters (32 bytes)

**Permission denied:**

```
[ERROR] Cannot read keystore: Permission denied
```

**Solution:** Fix file permissions with `chmod 600` and correct ownership

### Diagnostic Commands

```bash
# Verify keystore file format
cat ~/.acp/keystore

# Check file permissions
ls -la ~/.acp/keystore

# Test key loading (if ACP tools available)
acp_feature_test --keystore-test

# Generate test key
openssl rand -hex 32
```

### Recovery Procedures

**Corrupted keystore recovery:**

1. Backup existing (corrupted) keystore
2. Recreate keystore file with known keys
3. Test with simple session initialization
4. Restore from secure backup if available

**Lost keystore recovery:**

1. Stop all ACP-dependent services
2. Generate new keys with proper entropy
3. Update all endpoints with new keys
4. Coordinate key distribution securely
5. Restart services with new keystore

## Integration Examples

### Production Deployment

```c
// Production keystore initialization
int initialize_production_keystore(void) {
    // Set secure keystore path
    setenv("ACP_KEYSTORE_PATH", "/etc/acp/production.keystore", 1);
    
    // Initialize with strict validation
    int result = acp_keystore_init();
    if (result != ACP_OK) {
        syslog(LOG_ERR, "Production keystore initialization failed: %d", result);
        return result;
    }
    
    // Verify critical keys are available
    uint8_t test_key[32];
    if (acp_keystore_get_key(PRODUCTION_MASTER_KEY_ID, test_key, 32) != ACP_OK) {
        syslog(LOG_CRIT, "Production master key not found");
        return ACP_ERR_NOT_FOUND;
    }
    
    return ACP_OK;
}
```

### Development/Testing

```c
// Development keystore with well-known test keys
int initialize_test_keystore(void) {
    // Use test keystore
    setenv("ACP_KEYSTORE_PATH", "./test_keystore", 1);
    
    // Create keystore if it doesn't exist
    FILE *f = fopen("./test_keystore", "a");
    if (f) {
        fprintf(f, "# Test keystore\n");
        fprintf(f, "65535:0000111122223333444455556666777788889999AAAABBBBCCCCDDDDEEEEFFFF\n");
        fclose(f);
    }
    
    return acp_keystore_init();
}
```

## References

1. ACP Protocol Specification v0.3 - Authentication Requirements
2. NIST SP 800-57 - Key Management Guidelines  
3. RFC 5869 - HMAC-based Extract-and-Expand Key Derivation Function
4. Platform-specific secure storage documentation
