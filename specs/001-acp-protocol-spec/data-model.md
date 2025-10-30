# Data Model — ACP v0.3

This document captures the core entities, fields, relationships, and validation rules for the ACP reference implementation.

## Entities

### ACP Frame

- Fields (wire format order):
  - version: u8 (protocol version)
  - type: u8 (telemetry, command, system)  
  - flags: u8 (auth-enabled, reserved bits)
  - reserved: u8 (must be 0 in v0.3)
  - length: u16 (payload length in bytes; 0..1024)
  - seq: u32 (only present if auth-enabled flag set)
  - payload: bytes[length]
  - crc16: u16 (CRC16-CCITT over header + payload; excludes HMAC)
  - hmac16: 16 bytes (optional; HMAC-SHA256 truncated, only present if auth-enabled flag set)
- Relationships:
  - Bound to a Session when auth-enabled.
- Validation rules:
  - version matches supported set; flags consistent with presence of seq/hmac.
  - length <= 1024; payload length matches declared.
  - CRC16 verifies; if auth-enabled, HMAC verifies and seq is fresh (no replay).
  - Fields are encoded in network byte order on wire.

### Session

- Fields:
  - key_id: u32 (identifier for symmetric key in keystore)
  - key: bytes (secret key material; length suitable for HMAC-SHA256)
  - nonce: u64 (session nonce/random salt)
  - next_seq: u32 (monotonic counter for sender)
  - last_accepted_seq: u32 (receiver-side tracking)
  - policy: bitset (e.g., require_auth, fail_closed)
- Relationships:
  - Uses Keystore to resolve key from key_id.
- Validation rules:
  - Monotonic next_seq; rotation required before wrap.
  - Nonce must be unique per key/epoch.

### Keystore

- Fields:
  - storage_backend: enum (file, custom)
  - path or handle: string/handle
- Methods (contract-level):
  - get_key(key_id) -> key bytes or error
- Validation rules:
  - Read-only from core; platform shim implements secure retrieval.

### Platform Shims

- Logging: function pointers for levels; defaults to stderr on POSIX, OutputDebugStringA on Windows.
- Time: monotonic time source; used for optional auditing.
- Mutex: abstraction for multi-thread safety in optional builds.
- Keystore: as above.

## State Transitions (Session)

- init(key_id, nonce) → ready (next_seq=0)
- sign(frame) increments next_seq; verify(frame) updates last_accepted_seq on success
- rotate_key_or_nonce() resets counters; required before seq wrap

## Error Codes (excerpt)

- ACP_OK (0)
- ACP_ERR_PAYLOAD_TOO_LARGE
- ACP_ERR_COBS_DECODE
- ACP_ERR_CRC_MISMATCH
- ACP_ERR_AUTH_FAILED
- ACP_ERR_REPLAY
- ACP_ERR_KEY_NOT_FOUND
