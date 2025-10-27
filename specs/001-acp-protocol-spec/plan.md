# Implementation Plan: ACP Protocol Project Requirements and Deliverables

**Branch**: `001-acp-protocol-spec` | **Date**: 2025-10-27 | **Spec**: /Users/paulzanna/Github/acp/specs/001-acp-protocol-spec/spec.md
**Input**: Feature specification from `/specs/001-acp-protocol-spec/spec.md`

**Note**: Generated via speckit.plan workflow. See repo `.specify/scripts/bash/*` for automation.

## Summary

Deliver a portable C99 reference library implementing ACP framing (COBS + CRC16), optional HMAC-SHA256 authentication with replay protection, and platform shims (logging, time, mutex, keystore). Cross-platform builds target Linux, macOS, Windows (MinGW), and embedded. Artifacts include a stable C API, test vectors, and documentation enabling easy integration.

## Technical Context

**Language/Version**: C99
**Primary Dependencies**: Standard C library only; internal implementations for COBS, CRC16, HMAC-SHA256
**Storage**: N/A (keystore via platform shim; default file-based implementation)
**Testing**: Unit tests under `tests/` with vector-based checks for COBS, CRC16, HMAC, encode/decode; CI builds across OSes
**Target Platform**: Linux, macOS, Windows (MinGW), Embedded/RTOS via shims
**Project Type**: Single portable C library + examples
**Performance Goals**: Deterministic, zero heap by default; encode/decode bounded and linear in payload size; support up to 1024-byte payloads
**Constraints**: No dynamic allocation in default build; network byte order on wire; authenticated frames use truncated HMAC-SHA256; replay protection mandatory when auth enabled
**Scale/Scope**: Library API with core framing and session auth; examples and tests only (no transports)

## Constitution Check

Compliant with ACP Constitution (pre-design validation):

- Portability First: Targets Linux/macOS/Windows/embedded. Platform shims defined: logging (`acp_platform_log.h`), time (`acp_platform_time.h`), mutex (`acp_platform_mutex.h`), keystore (`acp_platform_keystore.h`). Default POSIX and Windows fallbacks planned.
- Minimal & Deterministic: Fixed header layout; COBS framing; CRC16 integrity; bounded linear-time encode/decode; no heap by default; packed structs and explicit endianness.
- Security by Design: Optional HMAC-SHA256 tag bound to session nonces/sequence; constant-time verification; replay protection enforced; key/nonce lifecycle documented.
- Extensible but Stable: Frame version included; additions compatible within MAJOR; negotiation via frame version; public API stabilized with semver.
- Open, Documented, and Testable: Spec (`docs/`, `specs/`) and conformance tests provided; cross-platform build targets and examples delivered.

Post-design validation: PASS — research.md, data-model.md, contracts, and quickstart reinforce compliance; no constitutional violations identified.

## Project Structure

### Documentation (this feature)

```text
specs/001-acp-protocol-spec/
├── plan.md              # This file (speckit.plan output)
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
└── contracts/           # Phase 1 output (C API contracts)
```

### Source Code (repository root)

```text
acp_constants.c
acp_crypto.c
acp_framer.c
acp_nvs.c
acp_platform_keystore.h
acp_platform_log.h
acp_platform_mutex.h
acp_platform_time.h
acp_session.c
docs/
examples/
specs/
tests/
```

**Structure Decision**: Single portable C library with platform shim headers at repo root; tests under `tests/`; documentation under `docs/` and `specs/`; examples under `examples/`. Build systems: make and CMake (to be confirmed/added where missing) producing static and shared libs.

## Complexity Tracking

N/A — no constitution violations identified at this stage.
