# Implementation Tasks — ACP Protocol Project Requirements and Deliverables

Feature: 001-acp-protocol-spec
Feature Dir: /Users/paulzanna/Github/acp/specs/001-acp-protocol-spec
Repo Root: /Users/paulzanna/Github/acp

This tasks file is organized by phases and user stories. Tasks are immediately executable and follow the strict checklist format.

## Phase 1 — Setup (project initialization)

- [ ] T001 Create Makefile in /Users/paulzanna/Github/acp/Makefile
- [ ] T002 Create CMakeLists.txt with targets libacp (static/shared) in /Users/paulzanna/Github/acp/CMakeLists.txt
- [ ] T003 Create GitHub Actions CI workflow for Linux/macOS/Windows (MinGW) in /Users/paulzanna/Github/acp/.github/workflows/build.yml
- [ ] T004 Create Doxygen config in /Users/paulzanna/Github/acp/docs/Doxyfile
- [ ] T005 Create examples skeleton in /Users/paulzanna/Github/acp/examples/acp_client.c
- [ ] T006 Create examples skeleton in /Users/paulzanna/Github/acp/examples/mock_serial.c
- [ ] T007 Create tests build config (CMakeLists or Makefile) in /Users/paulzanna/Github/acp/tests/CMakeLists.txt
- [ ] T008 [P] Add README build matrix section in /Users/paulzanna/Github/acp/README.md
- [ ] T057 [Build] Add compile flag ACP_NO_HEAP (default ON) to fail builds on heap usage in /Users/paulzanna/Github/acp/CMakeLists.txt and /Users/paulzanna/Github/acp/Makefile

## Phase 2 — Foundational (blocking prerequisites)

- [ ] T009 Define public API header in /Users/paulzanna/Github/acp/acp_protocol.h
- [ ] T010 Implement core API stub (wiring only) in /Users/paulzanna/Github/acp/acp.c
- [ ] T011 Create version header (semver) in /Users/paulzanna/Github/acp/acp_version.h
- [ ] T012 Define error codes in /Users/paulzanna/Github/acp/acp_errors.h
- [ ] T013 Implement CRC16-CCITT in /Users/paulzanna/Github/acp/acp_crc16.c
- [ ] T014 [P] Add CRC16 header in /Users/paulzanna/Github/acp/acp_crc16.h
- [ ] T015 Implement COBS encode/decode scaffolding in /Users/paulzanna/Github/acp/acp_framer.c
- [ ] T016 Implement HMAC-SHA256 (portable) in /Users/paulzanna/Github/acp/acp_crypto.c
- [ ] T017 Implement session state and replay tracking in /Users/paulzanna/Github/acp/acp_session.c
- [ ] T018 Implement default file-based keystore in /Users/paulzanna/Github/acp/acp_nvs.c
- [ ] T020 [P] Provide Windows (MinGW) shim fallback in /Users/paulzanna/Github/acp/acp_platform_windows.c
- [ ] T062 [Core] Add wire header struct with explicit packing and static asserts in /Users/paulzanna/Github/acp/acp_protocol.h
- [ ] T063 [Core] Implement host↔network helpers (u16/u32) and use them consistently in /Users/paulzanna/Github/acp/acp.c
- [ ] T064 [Tests] Add byte-order conformance test (known header→wire bytes) in /Users/paulzanna/Github/acp/tests/byte_order_test.c
- [ ] T065 [Core] Ensure version field is present and encoded in wire header in /Users/paulzanna/Github/acp/acp.c

## Phase 3 — User Story 1 (P1): Encode/Decode Telemetry Frame

Goal: Encode telemetry payload into ACP frame with COBS + CRC16; streaming decode extracts frames; headers/payload round-trip exactly.

Independent Test Criteria:

- Vector-based round-trip: known payload → frame → decode → identical payload/header; CRC validates.
- Streaming decode: from concatenated frames stream, extract each frame correctly; handle malformed COBS with explicit error.

Tasks:

- [ ] T021 [US1] Implement acp_encode_frame (no auth path) in /Users/paulzanna/Github/acp/acp.c
- [ ] T022 [US1] Implement acp_decode_frame (no auth path, streaming) in /Users/paulzanna/Github/acp/acp.c
- [ ] T023 [P] [US1] Implement COBS encode function in /Users/paulzanna/Github/acp/acp_framer.c
- [ ] T024 [P] [US1] Implement COBS decode function in /Users/paulzanna/Github/acp/acp_framer.c
- [ ] T025 [US1] Integrate CRC16 calculation into frame encode/decode in /Users/paulzanna/Github/acp/acp_framer.c
- [ ] T026 [US1] Add round-trip test in /Users/paulzanna/Github/acp/tests/frame_roundtrip_test.c
- [ ] T027 [P] [US1] Add streaming multi-frame decode test in /Users/paulzanna/Github/acp/tests/stream_decode_test.c
- [ ] T028 [P] [US1] Add COBS test vectors in /Users/paulzanna/Github/acp/tests/cobs_test.c
- [ ] T060 [US1] Add CRC16-CCITT known vectors test in /Users/paulzanna/Github/acp/tests/crc16_test.c
- [ ] T061 [US1] Document CRC16 polynomial/initial values and vector sources in /Users/paulzanna/Github/acp/docs/crc16.md

## Phase 4 — User Story 2 (P1): Authenticate Session With HMAC

Goal: Initialize session with key/nonce; authenticate frames with truncated HMAC-SHA256; enforce replay protection.

Independent Test Criteria:

- Given fixed key and nonce/seq, HMAC tag matches reference; frame tampering is rejected.
- Replay of same seq is rejected; unique seq accepted.

Tasks:

- [ ] T029 [US2] Implement session init/rotate APIs in /Users/paulzanna/Github/acp/acp_session.c
- [ ] T030 [US2] Add seq handling and conditional header fields in /Users/paulzanna/Github/acp/acp_framer.c
- [ ] T031 [P] [US2] Implement HMAC compute function in /Users/paulzanna/Github/acp/acp_crypto.c
- [ ] T032 [P] [US2] Implement constant-time tag compare in /Users/paulzanna/Github/acp/acp_crypto.c
- [ ] T033 [US2] Integrate HMAC into acp_encode_frame/acp_decode_frame in /Users/paulzanna/Github/acp/acp.c
- [ ] T034 [US2] Add HMAC test vectors in /Users/paulzanna/Github/acp/tests/hmac_test.c
- [ ] T035 [P] [US2] Add replay rejection tests in /Users/paulzanna/Github/acp/tests/replay_test.c
- [ ] T036 [US2] Implement default keystore get_key() in /Users/paulzanna/Github/acp/acp_nvs.c
- [ ] T051 [US2] Enforce command-frame auth in decoder: reject unauthenticated command frames in /Users/paulzanna/Github/acp/acp.c
- [ ] T052 [US2] Add test: unauthenticated command frame is rejected in /Users/paulzanna/Github/acp/tests/command_auth_reject_test.c
- [ ] T053 [US2] Add test: incorrect HMAC tag (16-byte truncated) is rejected in /Users/paulzanna/Github/acp/tests/command_auth_bad_tag_test.c
- [ ] T054 [US2] Define HMAC truncation length constant (ACP_HMAC_TAG_LEN=16) in /Users/paulzanna/Github/acp/acp_protocol.h
- [ ] T055 [US2] Update encoder/decoder to use ACP_HMAC_TAG_LEN for tag length in /Users/paulzanna/Github/acp/acp.c
- [ ] T056 [US2] Add test vectors including 16-byte truncated tags in /Users/paulzanna/Github/acp/tests/hmac_test.c

## Phase 5 — User Story 3 (P2): Cross-Platform Build and Link

Goal: Build static/shared libraries on Linux/macOS and static on Windows (MinGW); examples compile and run.

Independent Test Criteria:

- CI produces libacp.a and libacp.so/.dylib (Linux/macOS); MinGW produces static lib; examples link and run.

Tasks:

- [ ] T037 [US3] Add CMake targets for shared/static libs and install in /Users/paulzanna/Github/acp/CMakeLists.txt
- [ ] T038 [US3] Add example build targets in /Users/paulzanna/Github/acp/examples/CMakeLists.txt
- [ ] T039 [P] [US3] Add Makefile rules for libs and examples in /Users/paulzanna/Github/acp/Makefile
- [ ] T040 [US3] Configure CI matrix for gcc/clang and MinGW in /Users/paulzanna/Github/acp/.github/workflows/build.yml
- [ ] T041 [P] [US3] Add packaging step to publish artifacts in /Users/paulzanna/Github/acp/.github/workflows/build.yml

## Phase 6 — User Story 4 (P3): Platform Abstraction Fallbacks

Goal: Default POSIX and Windows shims for logging, time, mutex, keystore; allow stub overrides; tests pass with both default and stubs.

Independent Test Criteria:

- Build passes using defaults on POSIX/Windows; Build passes with stubbed shims; core tests still pass.

Tasks:

- [ ] T042 [US4] Implement logging/time/mutex/keystore functions in /Users/paulzanna/Github/acp/acp_platform_posix.c
- [ ] T043 [P] [US4] Implement Windows shim fallbacks in /Users/paulzanna/Github/acp/acp_platform_windows.c
- [ ] T044 [US4] Provide stub shim implementation in /Users/paulzanna/Github/acp/tests/stubs/acp_platform_stubs.c
- [ ] T045 [P] [US4] Add build variant to use stubs in /Users/paulzanna/Github/acp/tests/CMakeLists.txt
- [ ] T046 [US4] Document shim override instructions in /Users/paulzanna/Github/acp/README.md

## Final Phase — Polish & Cross-Cutting

- [ ] T047 Add Doxygen comments for all public APIs in /Users/paulzanna/Github/acp/acp_protocol.h
- [ ] T048 [P] Add payload boundary tests (1024 bytes) in /Users/paulzanna/Github/acp/tests/payload_boundary_test.c
- [ ] T049 [P] Add CRC mismatch tests in /Users/paulzanna/Github/acp/tests/crc_mismatch_test.c
- [ ] T050 Update docs with protocol versioning and framing details in /Users/paulzanna/Github/acp/docs/acp_comm_spec_v0-3.md
- [ ] T058 [Tests] Add no-heap verification (link wrap or symbol scan) in /Users/paulzanna/Github/acp/tests/no_heap_check.c
- [ ] T059 [Docs] Document no-heap default and enabling heap features in /Users/paulzanna/Github/acp/README.md
- [ ] T066 [Docs] Document keystore file format and manual key rotation steps in /Users/paulzanna/Github/acp/docs/keystore.md

## Dependencies (story completion order)

- Phase 1 → Phase 2 → US1 and US2 (P1) → US3 (P2) → US4 (P3)
- US1 depends on: COBS + CRC (T013–T015)
- US2 depends on: US1 core + HMAC/session/keystore (T016–T018)
- US3 depends on: Setup + core build targets (T001–T008, T009–T012)
- US4 can run after Foundational; may proceed in parallel with US3

## Parallel execution examples

- US1: T023, T024, T028 can run in parallel once framing files exist.
- US2: T031, T032, T035 can run in parallel after acp_crypto.c skeleton exists.
- US3: T039 and T041 can run in parallel with T037/T038.
- US4: T043 and T045 can run in parallel with T042.

## Implementation strategy

- MVP: Deliver US1 end-to-end (encode/decode + tests). Then add US2 authentication, followed by US3 build artifacts and US4 shims.
- Keep zero-heap default; use explicit buffers and lengths in APIs.
- Enforce network byte order in wire format functions.
- Add CI early to avoid drift across platforms.

## Validation

- Checklist format: All tasks follow `- [ ] T### [P]? [USn]? Description with file path`.
- File paths are absolute.
- Each user story includes goal, independent test criteria, and tasks that make the story independently testable.
