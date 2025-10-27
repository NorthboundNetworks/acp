# Autonomous Command Protocol v0.3 — Specification

## Purpose

Define a secure, auditable, and robust binary communication protocol for command and telemetry exchange between embedded devices, control stations, or companion systems. ACP provides a lightweight and resilient data layer for any autonomous or remotely operated platform, supporting authentication, replay protection, and efficient framing across serial, USB, or network transports.

---

## Goals

- Enable full mission control and telemetry using a compact, tamper-resistant binary format
- Mediate all incoming commands through the existing safety and audit infrastructure
- Support operation over UART, USB CDC, or SPI
- Add support for message authentication, replay protection, and improved framing
- Prepare for STANAG 4586 compatibility at the GCS layer

---

## Protocol Frame Structure (v0.3)

| Field   | Size (bytes) | Description                                          |
| ------- | ------------ | ---------------------------------------------------- |
| SYNC    | 2            | Start marker: `0xAA 0x55`                            |
| Version | 1            | Protocol version (e.g. `0x01`)                       |
| MsgType | 1            | Message identifier                                   |
| SeqNum  | 1            | Per-session sequence number                          |
| Flags   | 1            | Bitfield: 0x01 = auth present, 0x02 = reserved (future compression) |
| Length  | 2            | Payload size (including `auth_tag` if present)       |
| Payload | N            | COBS-encoded message body (see below)                |
| AuthTag | 16 (opt.)    | HMAC-SHA256 tag (if auth flag set)                   |
| CRC16   | 2            | CRC-16-CCITT (header + payload + auth)               |

The full frame (excluding SYNC) is COBS-encoded and terminated by a single 0x00 byte.

- Framing: COBS (Consistent Overhead Byte Stuffing) encoding
- Authentication: Optional per-message HMAC-SHA256 (truncated to 16 bytes)
- Replay Protection: Session nonce + SeqNum enforcement. The session nonce is established during connection via `ack_response` and is required for all authenticated messages. It is not transmitted in each frame but is cached locally by both ends.

---

## Message Types

### GCS → Drone Commands

| MsgType | Name          | Description                   |
| ------- | ------------- | ----------------------------- |
| 0x01    | `cmd_control` | Arm, disarm, mode set         |
| 0x02    | `cmd_gimbal`  | Gimbal movement commands      |
| 0x03    | `cmd_config`  | Field or system config update |

### Drone → GCS Telemetry / Events

| MsgType | Name          | Description                  |
| ------- | ------------- | ---------------------------- |
| 0x10    | `tlm_status`  | Basic telemetry (state, GPS) |
| 0x11    | `tlm_gimbal`  | Gimbal angle/status feedback |
| 0x12    | `tlm_audit`   | Command audit log event      |
| 0x13    | `tlm_mission` | Summary of mission log       |

### Responses

| MsgType | Name           | Description                   |
| ------- | -------------- | ----------------------------- |
| 0x20    | `ack_response` | Acknowledgement w/ session ID |
| 0x21    | `err_response` | Rejection reason / error code |

### Response Structs

```c
typedef struct __attribute__((packed)) {
  uint32_t session_id;
  uint8_t seq_num;
} ack_response_t;

typedef struct __attribute__((packed)) {
  uint8_t error_code;      // see acp_err_t
  uint8_t offending_type;  // original MsgType
  uint8_t seq_num;
} err_response_t;
```

---

## Example Structs (Packed C Representations)

All multi-byte fields are encoded in little-endian format unless otherwise specified.

### `cmd_control_t`

```c
typedef struct __attribute__((packed)) {
  uint8_t arm;         // 0 = disarm, 1 = arm
  uint8_t mode;        // 0 = hover, 1 = loiter, 2 = forward
  uint32_t session_id; // GCS session identifier
} cmd_control_t;
```

### `tlm_status_t`

```c
typedef struct __attribute__((packed)) {
  uint8_t armed;
  uint8_t mode;
  uint8_t gps_fix;
  uint8_t battery_percent;
  int16_t roll, pitch, yaw;     // in degrees × 100
  int32_t lat, lon, alt;        // in cm
  uint32_t uptime_ms;
  uint32_t timestamp_ms;
  uint8_t seq_num;
} tlm_status_t;
```

### `tlm_audit_t`

```c
typedef struct __attribute__((packed)) {
  uint8_t source;     // 0 = GCS, 1 = Companion
  uint8_t action;     // e.g., CMD_MODE_CHANGE
  uint8_t result;     // 0 = reject, 1 = accept
  uint32_t session_id;
  uint32_t timestamp_ms;
} tlm_audit_t;
```

---

## Safety & Mediation Requirements

- All GCS commands must pass through the same audit + safety gates as companion inputs
- Audit logs must capture:
  - Command source (GCS/Companion)
  - Result (accepted/rejected)
  - Reason (if rejected)
  - Session ID and timestamp
- The `tlm_audit` message must be emitted immediately after every command evaluation, whether accepted or rejected, and precedes any `err_response`.
- Unsupported MsgTypes must return `err_response` with expanded error codes

### Error Codes

```c
typedef enum {
  ERR_OK = 0x00,
  ERR_UNKNOWN = 0x01,
  ERR_CRC_FAIL = 0x02,
  ERR_SAFETY_REJECT = 0x03,
  ERR_AUTH_FAIL = 0x04,
  ERR_RESOURCE = 0x05,
  ERR_REPLAY = 0x06
} acp_err_t;
```

---

## Extensibility & Future Features

- Session management (auto-expiry after 15 min inactivity)
- Dynamic telemetry rate configuration via `cmd_config`
- Compression support for telemetry (bit-packed)
- STANAG 4586 mapping table in GCS layer

---

## Implementation Notes

- COBS decoding and framing logic in `src/utils/acp_framer.c`
- Authentication handled by `src/utils/acp_crypto.c` (HMAC-SHA256)
- Framing logic should be encapsulated in `acp_framer.h`, authentication in `acp_crypto.h`.
- Unit tests must verify:
  - Round-trip encode/decode
  - CRC mismatch rejection
  - Replay rejection
  - Auth tag verification
  - Audit trail correctness

---

## Success Criteria

- ✅ Commands accepted only after safety checks and HMAC validation
- ✅ Commands rejected produce `err_response` and `tlm_audit` entry
- ✅ `tlm_status` transmitted at configured rate (1–10 Hz) with stable timing
- ✅ Replay attempts rejected (sequence/nonce mismatch)
- ✅ CRC failures drop packets with error log
- ✅ Protocol backward-compatible with v1.0 (header interpreted safely)

---

## Out of Scope

- ACP is the sole production protocol
- No direct Lattice OS integration
- No in-flight OTA configuration changes
