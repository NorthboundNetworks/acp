# ACP — Autonomous Command Protocol (C99 Reference)

ACP is a small, portable protocol and C99 library for framing, authenticating, and parsing telemetry and command messages across Linux, macOS, Windows (MinGW), and embedded systems.

This repository contains the reference C implementation with platform shims, tests, and documentation. It follows the ACP Constitution: portability first, minimal and deterministic, security by design, extensible but stable, open and testable.

## Highlights

- COBS framing + CRC16-CCITT integrity
- HMAC-SHA256 authentication (16-byte truncated) with constant-time verification
- Command frames MUST be authenticated and replay-protected
- Zero-heap default (compile-time opt-in for heap if needed)
- Network byte order on the wire; packed headers; explicit versioning
- Max payload: 1024 bytes per frame
- Platform shims for logging, time, mutex, keystore (POSIX and Windows fallbacks)

## Repository structure

```text
.
├── acp_constants.c
├── acp_crypto.c                # HMAC-SHA256 and constant-time utilities
├── acp_framer.c                # COBS framing + CRC16 integration
├── acp_nvs.c                   # Default file-based keystore backend ("NVS")
├── acp_platform_keystore.h     # Platform shim headers
├── acp_platform_log.h
├── acp_platform_mutex.h
├── acp_platform_time.h
├── acp_session.c               # Session state + replay protection
├── docs/
│   └── acp_comm_spec_v0-3.md   # Protocol framing spec
├── examples/                   # Example apps (to be added)
└── tests/                      # Unit tests and vectors (to be added)
```

Note: Build system files (Makefile, CMakeLists.txt) and example sources are planned in tasks and may not exist yet on this branch.

## Build

ACP supports both CMake and Make build systems with full cross-platform support:

### Quick Start

**CMake (Recommended):**

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

**Make:**

```bash
make all
```

### Supported Platforms & Artifacts

- **Linux/macOS**: `libacp.a` (static) and `libacp.so/.dylib` (shared)
- **Windows (MinGW)**: `libacp.a` (static library)

### Build Options

- `ACP_NO_HEAP=ON` (default): Embedded-friendly, no heap allocation
- `ACP_BUILD_SHARED=ON`: Build shared libraries (Linux/macOS)
- `ACP_BUILD_TESTS=ON`: Build test suite
- `ACP_BUILD_EXAMPLES=ON`: Build example programs

### Installation

```bash
# CMake
cmake --install . --prefix /usr/local

# Make
make install PREFIX=/usr/local
```

**For detailed build instructions, cross-compilation, and troubleshooting, see [BUILD.md](docs/BUILD.md).**

## Security policy (command vs telemetry)

- Command frames: MUST carry a 16-byte truncated HMAC-SHA256 tag and are subject to replay protection; unauthenticated or replayed command frames are rejected.
- Telemetry frames: MAY be unauthenticated if explicitly configured; authentication is recommended.
- Keystore: default file-based backend provided in `acp_nvs.c`; integrators may override via platform shim.

## Memory Management and Embedded Support

**ACP is designed for embedded systems with zero heap allocation by default.**

### No-Heap Default (Recommended)

ACP operates without heap allocation by default (`ACP_NO_HEAP=ON`):

- **Static Memory Only**: All operations use stack or static storage
- **Embedded Ready**: Suitable for microcontrollers and real-time systems
- **Deterministic**: Predictable memory usage without malloc/free
- **Validated**: Comprehensive no-heap verification testing

### Enabling Heap Features (Optional)

To enable optional heap-based features:

```bash
# CMake
cmake .. -DACP_NO_HEAP=OFF

# Make
make ACP_ENABLE_HEAP=1
```

**Heap features (when enabled):**

- Dynamic keystore entries (vs fixed-size arrays)
- Larger internal buffers for complex operations
- Optional performance optimizations

**Note:** All core protocol features work without heap allocation. Heap mode is only needed for advanced integrations.

### Memory Requirements

**Stack usage (no-heap mode):**

- Frame encode/decode: ~1KB stack
- HMAC operations: ~300 bytes
- Session state: ~100 bytes

**Static storage:**

- Frame buffers: ~1KB per concurrent operation  
- Keystore: ~2KB for default file backend
- Total footprint: <10KB for typical embedded use

## Data model and wire format

- Network byte order (big-endian) on the wire
- Packed header with explicit version field
- CRC16-CCITT over framed content
- Optional HMAC tag (16 bytes) when authentication is enabled; mandatory for commands

See `docs/acp_comm_spec_v0-3.md` and `specs/001-acp-protocol-spec/spec.md` for details.

## Documentation

**Full API documentation is available in the generated HTML docs:**

To generate locally:

```bash
# Install doxygen (if not already installed)
# On macOS: brew install doxygen
# On Ubuntu: apt-get install doxygen
# On Windows: download from https://doxygen.nl

doxygen docs/Doxyfile
open docs/html/index.html
```

The documentation includes:

- Complete API reference with function documentation
- Data structure details and relationships  
- Usage examples and integration guides
- Protocol specification and wire format details

## Examples and tests

Comprehensive test suite includes:

- ✅ Round-trip encode/decode tests (COBS + CRC16)
- ✅ CRC16 known vectors
- ✅ HMAC vectors and negative tests (bad/absent tags on commands)
- ✅ Replay rejection tests
- ✅ Byte-order conformance tests
- ✅ No-heap verification for embedded systems
- ⚠️ Example apps: `acp_client.c`, `mock_serial.c` (planned)

Run tests with:

```bash
# CMake
cd build && ctest

# Make  
make test
```

## API Reference

The complete public API is defined in `acp_protocol.h` with full type safety and documentation. Key components:

- **Frame Management**: `acp_frame_encode()`, `acp_frame_decode()`
- **Session Management**: `acp_session_init()`, `acp_session_rotate()`
- **Cryptography**: `acp_hmac_sha256()`, `acp_crypto_memcmp_ct()`
- **Platform Integration**: Keystore, logging, time, and mutex abstractions

See the generated HTML documentation or `specs/001-acp-protocol-spec/contracts/c-api.md` for detailed API contracts.

## Platform Integration and Customization

ACP provides platform abstraction layers that can be customized for specific environments.

### Default Platform Support

**Built-in implementations provided:**

- **POSIX Systems**: Full implementation in `acp_platform_posix.c`
- **Windows**: Native Win32 implementation in `acp_platform_windows.c`
- **Embedded**: Minimal stubs with override capability

### Platform Shim Override

To customize platform behavior for your system, override the platform shim functions:

**Required Platform Functions:**

```c
// Time functions (acp_platform_time.h)
uint64_t acp_platform_get_time_ms(void);
uint64_t acp_platform_get_monotonic_us(void);
void acp_platform_sleep_ms(uint32_t ms);

// Mutex functions (acp_platform_mutex.h)  
acp_mutex_t* acp_platform_mutex_create(void);
int acp_platform_mutex_lock(acp_mutex_t *mutex);
int acp_platform_mutex_unlock(acp_mutex_t *mutex);
void acp_platform_mutex_destroy(acp_mutex_t *mutex);

// Keystore functions (acp_platform_keystore.h)
int acp_platform_keystore_get_key(uint32_t key_id, uint8_t *key_buffer, size_t buffer_size);
int acp_platform_keystore_store_key(uint32_t key_id, const uint8_t *key_data, size_t key_len);

// Logging functions (acp_platform_log.h)
void acp_platform_log(int level, const char *format, ...);
```

### Custom Platform Integration

#### Method 1: Replace platform files

```bash
# Replace default implementation with your custom version
cp my_platform_impl.c acp_platform_posix.c

# Build normally
make all
```

#### Method 2: Compile-time override

```c
// In your project, define ACP_CUSTOM_PLATFORM before including ACP
#define ACP_CUSTOM_PLATFORM 1
#include "acp_protocol.h"

// Provide your implementations
uint64_t acp_platform_get_time_ms(void) {
    // Your custom time implementation
    return my_get_milliseconds();
}

// ... implement other required functions
```

#### Method 3: Link-time override

```bash
# Compile ACP without default platform
make ACP_NO_PLATFORM=1

# Link with your platform implementation
gcc -o myapp myapp.c libacp.a my_platform.o
```

### Minimal Embedded Integration

For resource-constrained systems:

```c
// Minimal time implementation (required)
uint64_t acp_platform_get_time_ms(void) {
    return system_tick_count_ms;  // Your system tick counter
}

// Stub mutex implementation (if no threading)
acp_mutex_t* acp_platform_mutex_create(void) { return (acp_mutex_t*)1; }
int acp_platform_mutex_lock(acp_mutex_t *mutex) { return 0; }
int acp_platform_mutex_unlock(acp_mutex_t *mutex) { return 0; }
void acp_platform_mutex_destroy(acp_mutex_t *mutex) { }

// Minimal keystore (if no persistent storage)
int acp_platform_keystore_get_key(uint32_t key_id, uint8_t *key_buffer, size_t buffer_size) {
    // Return hardcoded key or error
    return ACP_ERR_NOT_FOUND;
}
```

**Note:** Only time functions are strictly required. Other functions can return appropriate error codes if not needed.

## Build Matrix

| Platform | Compiler | Static Lib | Shared Lib | CI Status |
|----------|----------|------------|------------|-----------|
| Linux x64 | GCC 9+ | ✅ | ✅ | [![Build Status](https://github.com/pzanna/acp/workflows/Build%20and%20Test/badge.svg)](https://github.com/pzanna/acp/actions) |
| Linux x64 | Clang 10+ | ✅ | ✅ | [![Build Status](https://github.com/pzanna/acp/workflows/Build%20and%20Test/badge.svg)](https://github.com/pzanna/acp/actions) |
| macOS (Intel/M1) | Clang (Xcode) | ✅ | ✅ | [![Build Status](https://github.com/pzanna/acp/workflows/Build%20and%20Test/badge.svg)](https://github.com/pzanna/acp/actions) |
| Windows x64 | MinGW-w64 | ✅ | ⚠️ v0.4+ | [![Build Status](https://github.com/pzanna/acp/workflows/Build%20and%20Test/badge.svg)](https://github.com/pzanna/acp/actions) |
| Windows x64 | MSVC | ⚠️ v0.4+ | ⚠️ v0.4+ | Not supported in v0.3 |
| ARM64 Linux | GCC/Clang | ✅ | ✅ | Cross-compilation tested |
| ARM Embedded | arm-none-eabi-gcc | ✅ | N/A | Cross-compilation support |

**Legend:**

- ✅ Fully supported and tested
- ⚠️ Planned for future release  
- ❌ Not supported

**Dependencies:** Standard C library only (C99 compliant)

## License

MIT License

Copyright (c) 2025 Northbound Networks Pty. Ltd.
