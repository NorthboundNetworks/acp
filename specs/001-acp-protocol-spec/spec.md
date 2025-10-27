# Feature Specification: ACP Protocol Project Requirements and Deliverables

**Feature Branch**: `001-acp-protocol-spec`  
**Created**: 2025-10-27  
**Status**: Draft  
**Input**: User description captured from SpecKit.specify request (see repository context)

## Overview

This feature delivers a portable, C99 reference implementation of the Autonomous Command Protocol (ACP) with secure framing, authentication, and platform abstractions. The goal is to make it easy for teams to encode, transmit, and verify telemetry and command frames across Linux, macOS, Windows, and embedded systems, with stable headers, clear documentation, and reproducible builds. The result is a small, dependable library that downstream apps can integrate without taking on OS-specific complexity.

## Clarifications

### Session 2025-10-27

- Q: What is the maximum payload size for ACP frames? → A: 1024 bytes

## User Scenarios & Testing (mandatory)

### User Story 1 - Encode/Decode Telemetry Frame (Priority: P1)

Developers can encode a telemetry payload into an ACP frame, transmit it, and decode it back to the original payload using the public C API, with COBS framing and CRC16 integrity.

**Why this priority**: This is the core value of ACP: reliable portable framing and decoding across platforms.

**Independent Test**: Provide test vectors and a test harness that encodes a known payload, frames it, and decodes it to the original bytes; verify CRC and structural fields match.

**Acceptance Scenarios**:

1. Given a known payload and header fields, When encoded via `acp_framer` with COBS + CRC16, Then decoding returns identical payload and header, and CRC validates.
2. Given a byte stream with multiple COBS-framed ACP packets, When streaming decode is applied, Then each frame is extracted and validated independently without cross-frame corruption.

---

### User Story 2 - Authenticate Session With HMAC (Priority: P1)

Applications can initialize an ACP session, derive or load a symmetric key from the keystore, and authenticate frames using HMAC-SHA256 to prevent tampering and replays.

**Why this priority**: Security is a first-class requirement; correctness relies on authenticated frames and replay protection.

**Independent Test**: Using a fixed key and nonce/sequence inputs, compute a reference HMAC for a frame and verify the library reproduces it and rejects any modified frames.

**Acceptance Scenarios**:

1. Given a session with key K and sequence S, When a frame is signed, Then receiver verifies HMAC tag and accepts the frame.
2. Given a previously accepted frame (same S), When it is replayed, Then receiver rejects it as a replay.
3. Given a command frame without an HMAC tag, When it is processed, Then the receiver rejects it as unauthenticated.
4. Given a command frame with an incorrect (16-byte truncated) HMAC-SHA256 tag, When it is processed, Then the receiver rejects it.

---

### User Story 3 - Cross-Platform Build and Link (Priority: P2)

Developers can build static and shared libraries on Linux, macOS, and Windows using default platform shims; examples compile and link against the library.

**Why this priority**: Ensures portability and adoption across target environments.

**Independent Test**: CI builds produce `libacp.a` and `libacp.so/.dylib` artifacts (and Windows equivalent), and example programs link and run.

**Acceptance Scenarios**:

1. Given a clean checkout, When standard build steps are run per README, Then libraries are produced for each OS and examples execute successfully.

---

### User Story 4 - Platform Abstraction Fallbacks (Priority: P3)

Applications can use default logging, time, mutex, and keystore shims on supported OSes or provide their own implementations via the platform headers without modifying core.

**Why this priority**: Decouples core protocol from OS specifics and enables embedded reuse.

**Independent Test**: Build with default shims on POSIX and Windows; build again with custom shim stubs; all tests still pass.

**Acceptance Scenarios**:

1. Given no custom shims, When building on Linux/macOS, Then default POSIX backends are used and tests pass.
2. Given stubbed shim implementations, When running unit tests, Then core functionality works without OS dependencies.

### Edge Cases

- At 1024-byte payload boundary: frames at or below limit process successfully; larger payloads are rejected with an explicit error; CRC/HMAC still correct for accepted frames.
- COBS decode errors: invalid zero bytes or malformed runs are detected and reported without buffer overrun.
- CRC mismatch: decoder rejects frame and surfaces an explicit error code.
- Replay attempts: duplicate sequence or stale nonce is rejected; counter wrap behavior is defined.
- Endianness: multi-byte fields are encoded in network byte order; decoders handle both little/big endian hosts.
- Keystore unavailable: authentication gracefully degrades to unauthenticated mode only if explicitly configured; default is to fail closed.
- Authentication policy: Telemetry frames MAY be unauthenticated if explicitly configured (authentication is RECOMMENDED). Command frames MUST be authenticated; unauthenticated or incorrectly authenticated command frames are rejected.

## Requirements (mandatory)

### Functional Requirements

- FR-001: Provide a stable, portable C99 API for ACP core framing and encode/decode in `acp_protocol.h` with implementation in `acp.c`.
- FR-002: Implement COBS framing and CRC16 integrity in `acp_framer.c` with streaming-safe encode/decode APIs.
- FR-003: Implement frame authentication using HMAC-SHA256 truncated to 16 bytes (128 bits) in `acp_crypto.c` with constant-time verification.
- FR-003a: Command frames MUST include a valid 16-byte truncated HMAC-SHA256 tag and MUST be subject to replay protection; unauthenticated or replayed command frames MUST be rejected.
- FR-004: Implement session management and replay protection in `acp_session.c` using nonces and monotonic sequence numbers.
- FR-005: Provide platform abstraction headers and default backends: logging (`acp_platform_log.h`), time (`acp_platform_time.h`), mutexes (`acp_platform_mutex.h`), and persistent keystore (`acp_platform_keystore.h`).
- FR-006: Supply default POSIX backends for Linux/macOS and Windows backends or fallbacks as part of the distribution.
- FR-007: Provide compile-time tests and known-good vectors under `tests/` including CRC16, HMAC-SHA256, COBS framing, and round-trip frame encode/decode.
- FR-008: Package builds generate `libacp.a` and `libacp.so`/`.dylib`. On Windows, produce a static library only for this release; deferring `.dll` output and versioning to a later release.
- FR-009: Support standard build systems (make and CMake) with minimal dependencies; produce both static and shared libraries.
- FR-010: Provide complete documentation: Doxygen for all public APIs, `README.md` with architecture/usage/build/platform notes, protocol framing spec in `docs/acp_comm_spec_v0-3.md`.
- FR-011: Include example applications in `examples/` (e.g., `acp_client.c`, `mock_serial.c`) that demonstrate encoding, session auth, and decoding in a minimal loop.
- FR-012: Maintain backward-compatible versioning (semver) and a stable `acp_protocol.h` interface.
- FR-013: Use packed structs and explicit, endian-safe field definitions; wire format is network byte order; add protocol version tags to frames.
- FR-014: Default build avoids dynamic memory allocation (enforced via `ACP_NO_HEAP` compile flag, default ON); optional allocation features require explicit `ACP_ENABLE_HEAP` flag and must provide stub implementations that work without heap access for embedded/testing scenarios.
- FR-015: Windows toolchain support targets MinGW for official support in this release; MSVC support is explicitly out of scope for v0.3 and will be evaluated for v0.4+ based on community demand.
- FR-016: Keystore: default file-based keystore in the user configuration directory; manual key rotation documented for integrators (no automatic rotation in core). The default keystore backend is provided in `acp_nvs.c` (default file-based keystore).
- FR-017: Payload size limit: maximum payload size is 1024 bytes. Frames exceeding this limit MUST be rejected with an explicit error; segmentation, if needed, is the responsibility of calling applications.

### Key Entities (data-oriented)

- ACP Frame: version, type, flags, length, payload bytes, CRC16, optional HMAC tag (16-byte truncated; mandatory for command frames).
- Session: key identifier, symmetric key material, nonce/sequence, last-accepted sequence, policy flags.
- Keystore: persistent storage interface to retrieve key by ID; provides read-only API to core; platform-specific implementations.
- Platform Shims: logging/time/mutex/keystore interfaces with default and override behaviors.

## Success Criteria (mandatory)

### Measurable Outcomes

- SC-001: On Linux, macOS, and Windows, a clean build produces static and shared library artifacts and all example programs compile and run without modification.
- SC-002: Round-trip encode/decode for a reference telemetry frame reproduces the original payload and header fields exactly and passes CRC/HMAC verification.
- SC-003: Replay protection rejects 100% of duplicate frames in unit tests; no false positives on unique frames in test suites.
- SC-004: Public API headers contain Doxygen comments for 100% of public functions, types, and macros.
- SC-005: Default builds use no dynamic memory allocation in core paths; optional allocation features, if enabled, are documented and tested.
- SC-006: README contains protocol overview, cross-platform build instructions, example snippets, and versioning notes sufficient for a new developer to integrate in under 30 minutes as validated by a setup walkthrough.
- SC-007: Command frames without a 16-byte HMAC tag or with an incorrect tag are rejected in unit tests; telemetry unauthenticated frames are accepted only when explicitly configured.

## Assumptions and Dependencies

- Protocol fields use network byte order on the wire; host implementations handle local endianness.
- A symmetric HMAC key is provisioned via the keystore interface before authenticated sessions are used.
- Default builds avoid dynamic memory; any optional allocation-based features are off by default.
- Build systems supported are make and CMake; additional systems may be added later.
- Windows support targets MinGW as the official toolchain for this release; MSVC may be added later.
- No transport abstraction is provided; calling applications handle serial/TCP/USB I/O.
