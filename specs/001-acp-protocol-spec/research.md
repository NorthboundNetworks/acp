# Phase 0 Research — ACP Protocol Implementation Details

This document consolidates design decisions and resolves prior unknowns for the ACP reference C99 implementation.

Format per task: Decision, Rationale, Alternatives considered.

## Framing and Integrity

- Decision: Use COBS framing with 0x00 delimiter between packets.
  - Rationale: COBS guarantees no zero bytes within framed payloads, enabling robust streaming extraction with clear packet boundaries.
  - Alternatives considered: SLIP (worse overhead on random data), HDLC-like bit-stuffing (more complex and CPU-heavy), delimiter-only framing (fragile under payload collisions).

- Decision: CRC16-CCITT (0x1021), initial value 0xFFFF, reflect-in/out = false, XOR-out = 0x0000.
  - Rationale: Widely used, good burst error detection, common in embedded; deterministic and portable.
  - Alternatives considered: CRC-16/IBM (0xA001 reflected), CRC-32 (heavier and bigger tag), Fletcher-16 (weaker error detection).

## Authentication and Replay Protection

- Decision: HMAC-SHA256 with 16-byte (128-bit) truncated tag appended to authenticated frames.
  - Rationale: 128-bit tag balances security and frame size; HMAC-SHA256 is broadly available and vetted; truncation is standard practice.
  - Alternatives considered: Full 32-byte tag (larger frames), Poly1305 (requires one-time keying and adds constraints), CMAC-AES (requires AES dependency).

- Decision: Session-bound nonce + monotonic 32-bit sequence number; receiver tracks last-accepted sequence per session key ID.
  - Rationale: Simplicity and sufficient headroom; monotonic counter prevents replays; 32-bit covers long sessions; wrap behavior defined below.
  - Alternatives considered: 64-bit sequence (larger header), timestamp-based windows (clock sync dependency), nonces only (harder receiver state mgmt).

- Decision: Sequence wrap policy is to require explicit key/nonce rotation before wrap; frames at or beyond wrap are rejected.
  - Rationale: Avoids ambiguity and replay edge cases; encourages operational hygiene.
  - Alternatives considered: Modular arithmetic windows (complex corner cases), unlimited counters (wider fields increase overhead).

## Frame Header and Endianness

- Decision: Network byte order (big-endian) for all multibyte fields.
  - Rationale: Standard for interoperability; explicitly called out in spec.
  - Alternatives considered: Little-endian (host-native), variable.

- Decision: Header fields (tentative v0.3):
  - version (u8), type (u8), flags (u8), reserved (u8), length (u16), seq (u32 when auth enabled; omitted otherwise), crc16 (u16), optional hmac16 (16 bytes)
  - Rationale: Keeps base header compact; seq present only when needed to reduce overhead in unauthenticated frames; aligns with constraints and tests.
  - Alternatives considered: Always include seq (more overhead), embed seq in flags (not enough bits for growth).

## Payload Constraints and Errors

- Decision: Maximum payload size 1024 bytes; reject larger with explicit error.
  - Rationale: Clarified in spec; ensures bounded resource usage and deterministic paths.
  - Alternatives considered: Larger default limit (higher memory needs), negotiated MTU (adds complexity).

- Decision: COBS decode errors return explicit error codes and never overrun buffers; decoding stops at frame boundary.
  - Rationale: Safety and determinism; aligns with testability goals.
  - Alternatives considered: Best-effort salvage (riskier behavior).

## Keystore

- Decision: Default keystore is file-based in user config directory, read-only API from core; key identified by key ID.
  - Rationale: Portable and simple; avoids adding DB deps; allows platform overrides.
  - Alternatives considered: OS keychains (platform-specific), custom secure elements (out of scope for core).

## Build and Toolchains

- Decision: Provide make and CMake builds that produce libacp.a and libacp.so/.dylib; on Windows (MinGW) provide static library; shared deferred.
  - Rationale: Matches spec; covers primary consumers and CI.
  - Alternatives considered: Meson/Bazel-only (less ubiquitous), Windows DLL in v0.3 (more maintenance).

## Testing

- Decision: Include vector tests for CRC16, HMAC, COBS; round-trip frame tests (with/without auth); replay rejection tests.
  - Rationale: Ensures correctness and prevents regressions.
  - Alternatives considered: Ad-hoc tests (insufficient coverage), property-only tests (hard to debug failures).

## Open Questions (resolved here)

- HMAC tag length → 16 bytes truncation: resolved as above.
- CRC parameters → CCITT-FALSE profile: resolved as above.
- Sequence width and wrap policy → 32-bit; no wrap without rotation: resolved as above.
- Header structure → compact base; conditional seq: resolved as above.

All previously identified clarifications are resolved in this document.
