# Autonomous Command Protocol (ACP) v0.3 — Complete Specification

## Executive Summary

The Autonomous Command Protocol (ACP) v0.3 is a secure, lightweight binary communication protocol designed for embedded systems, IoT devices, and distributed applications. ACP provides authenticated message exchange with integrity validation, replay protection, and efficient framing suitable for resource-constrained environments.

## Design Principles

- **Security First**: HMAC-SHA256 authentication with constant-time verification
- **Embedded Ready**: Zero-heap operation by default, minimal resource requirements  
- **Platform Agnostic**: C99 standard compliance, cross-platform portability
- **Deterministic**: Predictable performance, bounded memory usage
- **Extensible**: Version negotiation and feature detection support

## Key Features

- **COBS Framing**: Consistent Overhead Byte Stuffing for reliable delimiter-based framing
- **CRC16 Integrity**: CRC16-CCITT validation for corruption detection
- **HMAC Authentication**: Optional HMAC-SHA256 with 16-byte truncation
- **Replay Protection**: Session-based sequence number enforcement  
- **Command Enforcement**: Mandatory authentication for command frames
- **Cross-Platform**: Linux, macOS, Windows, and embedded system support

## Protocol Architecture

### Wire Format

ACP v0.3 uses Consistent Overhead Byte Stuffing (COBS) for reliable framing:

```text
COBS_FRAME[CRC16 | PAYLOAD] | DELIMITER
```

Where:

- **COBS_FRAME**: COBS-encoded packet containing CRC16 + PAYLOAD
- **CRC16**: 2-byte CRC16-CCITT checksum (big-endian) of PAYLOAD
- **PAYLOAD**: Message content (telemetry data or authenticated command)
- **DELIMITER**: Single 0x00 byte marking frame boundary

### COBS Encoding Properties

- **Zero Elimination**: No 0x00 bytes within encoded frame
- **Overhead**: Maximum 1 byte per 254 bytes + 1 byte total
- **Self-Synchronizing**: Frame boundaries are unambiguous
- **Corruption Detection**: Encoding errors are detectable

### CRC16 Implementation

ACP uses CRC16-CCITT (polynomial 0x1021) with:

- **Initial Value**: 0xFFFF
- **Final XOR**: 0x0000
- **Bit Order**: MSB first
- **Byte Order**: Big-endian on wire

### Frame Types

ACP defines two primary frame types:

#### Telemetry Frames

```text
| CRC16 | PAYLOAD_DATA |
```

- **CRC16**: Integrity checksum of PAYLOAD_DATA
- **PAYLOAD_DATA**: Raw telemetry data (application-defined format)
- **Authentication**: Not required (telemetry is read-only)

#### Command Frames

```text
| CRC16 | HMAC_TAG | SEQUENCE | PAYLOAD_DATA |
```

- **CRC16**: Integrity checksum of (HMAC_TAG + SEQUENCE + PAYLOAD_DATA)
- **HMAC_TAG**: 16-byte HMAC-SHA256 authentication tag
- **SEQUENCE**: 4-byte sequence number (big-endian, replay protection)
- **PAYLOAD_DATA**: Command data (application-defined format)
- **Authentication**: Mandatory for all command frames

### Session Management

- **Keystore Integration**: Shared keys managed through platform keystore
- **Sequence Numbers**: Monotonic counters prevent replay attacks
- **Constant-Time Verification**: HMAC validation uses timing-safe comparison
- **Command Enforcement**: All command frames must include valid authentication

## Implementation Guidelines

### Payload Format

ACP is payload-agnostic. Applications define their own data formats within the PAYLOAD_DATA field. Common approaches include:

- **Binary Structs**: Fixed-size C structures (ensure portable packing)
- **TLV Encoding**: Type-Length-Value for extensible messages
- **Protocol Buffers**: For complex, versioned message schemas
- **JSON**: For human-readable debugging (though bandwidth-intensive)

### Error Handling

Implementations should handle:

- **CRC Mismatches**: Discard corrupted frames silently
- **COBS Decoding Errors**: Reset framing state and resynchronize  
- **HMAC Failures**: Log authentication errors, reject commands
- **Sequence Violations**: Detect replay attempts, maintain session state
- **Buffer Overflows**: Validate frame sizes against available memory

### Performance Considerations

- **Zero-Copy Operation**: Process frames in-place when possible
- **Bounded Memory**: All operations use stack allocation by default
- **Constant-Time Crypto**: HMAC verification prevents timing attacks
- **Incremental Processing**: Handle partial frames for streaming interfaces

## Security Model

### Authentication Architecture

ACP implements defense-in-depth security:

1. **Transport Security**: CRC16 ensures data integrity
2. **Application Security**: HMAC-SHA256 provides authentication  
3. **Replay Protection**: Sequence numbers prevent message replay
4. **Access Control**: Commands require authentication, telemetry does not

### Cryptographic Specifications

- **Algorithm**: HMAC-SHA256 per RFC 2104
- **Key Size**: 256-bit (32 bytes) shared secret
- **Tag Size**: 128-bit (16 bytes) truncated output
- **Verification**: Constant-time comparison prevents timing attacks

### Threat Model

ACP protects against:

- **Data Corruption**: CRC16 detects transmission errors
- **Command Injection**: HMAC authentication validates command source
- **Replay Attacks**: Sequence numbers ensure message freshness
- **Eavesdropping**: Telemetry remains readable (by design)

ACP does not protect against:

- **Key Compromise**: Shared keys must be managed securely
- **Traffic Analysis**: Frame timing and sizes are observable
- **Side-Channel Attacks**: Implementation must use constant-time operations

## API Reference

### Core Functions

```c
// Frame processing
int acp_frame_encode(const uint8_t *payload, size_t payload_len, 
                     uint8_t *output, size_t output_size, size_t *output_len);
int acp_frame_decode(const uint8_t *input, size_t input_len,
                     uint8_t *payload, size_t payload_size, size_t *payload_len);

// Authentication  
int acp_auth_set_key(const uint8_t *key, size_t key_len);
int acp_auth_sign_command(const uint8_t *payload, size_t payload_len,
                          uint32_t sequence, uint8_t *output, size_t output_size);
int acp_auth_verify_command(const uint8_t *frame, size_t frame_len,
                            uint32_t expected_sequence);
```

```c
typedef struct __attribute__((packed)) {
  ### Return Values

All ACP functions return standard error codes:

```c
typedef enum {
  ACP_OK = 0,              // Success
  ACP_ERR_INVALID_ARG,     // Invalid function argument
  ACP_ERR_BUFFER_SIZE,     // Insufficient buffer space
  ACP_ERR_CRC_MISMATCH,    // CRC validation failed
  ACP_ERR_AUTH_FAILED,     // HMAC authentication failed
  ACP_ERR_DECODE_ERROR,    // COBS decoding error
  ACP_ERR_SEQUENCE,        // Sequence number violation
  ACP_ERR_NOT_INITIALIZED  // Authentication not configured
} acp_result_t;
```

## Conformance Requirements

### Frame Processing

Conformant implementations must:

- **COBS Encoding**: Use RFC-compliant COBS for all frames
- **CRC16 Validation**: Compute CRC16-CCITT over payload data
- **Zero Delimiter**: Terminate all frames with single 0x00 byte
- **Buffer Management**: Handle frames up to 254 bytes payload
- **Error Recovery**: Resynchronize on invalid frames

### Authentication

For command processing:

- **HMAC-SHA256**: Implement per RFC 2104 specification
- **16-Byte Tags**: Truncate HMAC output to 128 bits
- **Sequence Enforcement**: Reject out-of-order messages
- **Constant-Time Verification**: Prevent timing attack vectors
- **Key Management**: Integrate with platform keystore

### Platform Integration

Implementations should provide:

- **Cross-Platform Builds**: Support major operating systems
- **Embedded Compatibility**: Function in resource-constrained environments
- **Zero-Heap Operation**: Use stack allocation by default
- **Thread Safety**: Provide synchronization primitives
- **Logging Integration**: Support platform-specific logging

## Reference Implementation

The reference implementation provides:

- **Complete C99 Library**: Full protocol implementation
- **Cross-Platform Support**: Linux, macOS, Windows, embedded
- **Comprehensive Tests**: Unit tests and integration validation
- **Example Applications**: Telemetry sender/receiver demos
- **Documentation**: API reference and integration guides
- **Build Systems**: CMake and Make support with installation

## Version History

### v0.3 (Current)

- COBS framing with 0x00 delimiters
- CRC16-CCITT integrity validation  
- HMAC-SHA256 authentication with 16-byte tags
- Sequence-based replay protection
- Command frame enforcement
- Cross-platform build system

### Previous Versions

- **v0.2**: SYNC-byte framing with length headers
- **v0.1**: Basic binary protocol without authentication

## Implementation Resources

### Library Structure

```text
acp_framer.c     - COBS encoding/decoding and frame processing
acp_crypto.c     - HMAC-SHA256 authentication and verification  
acp_session.c    - Session management and sequence tracking
acp_constants.c  - Protocol constants and configuration
acp_nvs.c        - Non-volatile storage abstraction
```

### Platform Headers

```text
acp_platform_keystore.h  - Keystore integration interface
acp_platform_time.h      - Time/timestamp services
acp_platform_mutex.h     - Thread synchronization primitives  
acp_platform_log.h       - Logging and debug output
```

### Testing

The reference implementation includes comprehensive test coverage:

- **Unit Tests**: Individual component validation
- **Integration Tests**: End-to-end protocol operation
- **Conformance Tests**: Standards compliance verification
- **Platform Tests**: Cross-platform compatibility
- **Security Tests**: Authentication and replay protection

### Build Requirements

- **C99 Compiler**: GCC, Clang, or MSVC
- **CMake 3.10+**: Cross-platform build system
- **Standard Library**: No external dependencies
- **Platform Layer**: Implementation-specific keystore/logging

## Conclusion

ACP v0.3 provides a production-ready protocol for secure embedded communication. The combination of COBS framing, CRC16 integrity, and HMAC authentication delivers reliable message exchange suitable for safety-critical applications while maintaining compatibility with resource-constrained environments.

For implementation guidance, API documentation, and integration examples, refer to the reference implementation repository and accompanying documentation suite.
