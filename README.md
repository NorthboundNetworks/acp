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

## Build (planned targets)

Two build systems will be supported:

- Make: builds static and shared libraries on Linux/macOS; static on Windows (MinGW)
- CMake: portable configuration, install targets, and example builds

Artifacts:

- Linux/macOS: libacp.a and libacp.so/.dylib
- Windows (MinGW): static library for this release

Configuration flags:

- ACP_NO_HEAP=ON by default: disallow malloc/calloc/realloc/free in core paths
- Optional feature flags to enable heap-dependent paths explicitly

After T001/T002 are implemented, typical flows:

```sh
# Using Make (once Makefile is added)
make

# Using CMake (once CMakeLists.txt is added)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DACP_NO_HEAP=ON
cmake --build build --config Release
```

## Security policy (command vs telemetry)

- Command frames: MUST carry a 16-byte truncated HMAC-SHA256 tag and are subject to replay protection; unauthenticated or replayed command frames are rejected.
- Telemetry frames: MAY be unauthenticated if explicitly configured; authentication is recommended.
- Keystore: default file-based backend provided in `acp_nvs.c`; integrators may override via platform shim.

## Data model and wire format

- Network byte order (big-endian) on the wire
- Packed header with explicit version field
- CRC16-CCITT over framed content
- Optional HMAC tag (16 bytes) when authentication is enabled; mandatory for commands

See `docs/acp_comm_spec_v0-3.md` and `specs/001-acp-protocol-spec/spec.md` for details.

## Examples and tests

Planned and tracked in `specs/001-acp-protocol-spec/tasks.md`:

- Round-trip encode/decode tests (COBS + CRC16)
- CRC16 known vectors
- HMAC vectors and negative tests (bad/absent tags on commands)
- Replay rejection tests
- Byte-order conformance tests
- Example apps: `acp_client.c`, `mock_serial.c`

## API status

The public API header `acp_protocol.h` and versioning header will be introduced per tasks (T009–T012). A draft of the API is available in `specs/001-acp-protocol-spec/contracts/c-api.md`.

## License

MIT License

Copyright (c) 2025 Northbound Networks Pty. Ltd.
