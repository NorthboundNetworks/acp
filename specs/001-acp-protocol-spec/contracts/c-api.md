# C API Contract — ACP v0.3

This document describes the stable C API surface for ACP framing, authentication, and session management. Exact types and names may be refined in headers, but the contracts and error semantics are normative.

## Conventions

- All functions return int error codes (0 = success). Enumerated errors are prefixed with ACP_ERR_.
- Out-parameters use pointer arguments. Buffer sizes are provided explicitly.
- No dynamic allocation in default build; caller provides buffers.
- Multibyte fields are in network byte order on the wire.

## Types (conceptual)

- acp_frame_t: opaque/frame view used internally; encode/decode operate on raw buffers.
- acp_session_t: session state object; contains key id/material, nonce, counters, policy.

## Functions

### Framing and Integrity

```c
int acp_encode_frame(const uint8_t* payload, uint16_t payload_len,
                     uint8_t type, uint8_t flags,
                     const acp_session_t* session_or_null,
                     uint8_t* out_buf, size_t out_buf_size,
                     size_t* out_frame_len);
```

- Contract:
  - Encodes payload into an ACP frame, computes CRC16, applies COBS, appends delimiter.
  - If session provided and flags indicate auth, includes seq and HMAC.
  - Fails with ACP_ERR_PAYLOAD_TOO_LARGE if payload_len > 1024.
  - Fails with ACP_ERR_BUFFER_TOO_SMALL if out_buf_size is insufficient.

```c
int acp_decode_frame(const uint8_t* stream_buf, size_t stream_len,
                     uint8_t* out_payload, uint16_t* inout_payload_capacity,
                     uint8_t* out_type, uint8_t* out_flags,
                     acp_session_t* session_or_null,
                     size_t* out_consumed);
```

- Contract:
  - Attempts to extract one framed packet from stream_buf.
  - On success, writes payload (up to capacity), header fields, and bytes consumed.
  - Verifies CRC16; if auth-enabled, verifies HMAC and replay; returns ACP_ERR_AUTH_FAILED or ACP_ERR_REPLAY as needed.
  - If COBS malformed, returns ACP_ERR_COBS_DECODE.

### Session Management

```c
int acp_session_init(acp_session_t* s, uint32_t key_id, const uint8_t* key, size_t key_len,
                     uint64_t nonce, uint32_t start_seq, uint32_t policy_flags);
```

- Contract: Initializes a session; next_seq set to start_seq; fail-closed policy enabled if configured.

```c
int acp_session_rotate(acp_session_t* s, uint32_t new_key_id,
                       const uint8_t* new_key, size_t key_len,
                       uint64_t new_nonce, uint32_t new_start_seq);
```

- Contract: Rotates key/nonce; required prior to sequence wrap.

### Keystore Interface (platform shim)

```c
int acp_keystore_get_key(uint32_t key_id, uint8_t* out_key, size_t* inout_key_len);
```

- Contract: Retrieves key bytes for key_id; returns ACP_ERR_KEY_NOT_FOUND if unavailable.

## Error Semantics

- ACP_OK (0): success
- ACP_ERR_BUFFER_TOO_SMALL: output buffer insufficient
- ACP_ERR_PAYLOAD_TOO_LARGE: payload_len > 1024
- ACP_ERR_COBS_DECODE: invalid COBS stream
- ACP_ERR_CRC_MISMATCH: CRC check failed
- ACP_ERR_AUTH_FAILED: HMAC verification failed
- ACP_ERR_REPLAY: sequence not strictly increasing
- ACP_ERR_KEY_NOT_FOUND: keystore could not resolve key

## Versioning

- API is versioned according to semver. Additive changes are allowed in MINOR versions; breaking changes require MAJOR.
