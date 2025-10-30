<!--
Sync Impact Report
- Version change: N/A → 1.0.0
- Modified principles:
	- [template placeholders] → Portability First
	- [template placeholders] → Minimal & Deterministic
	- [template placeholders] → Security by Design
	- [template placeholders] → Extensible but Stable
	- [template placeholders] → Open, Documented, and Testable
- Added sections:
	- Functional Scope and Constraints
	- Versioning, Testing, and Ecosystem (includes Design Mantras)
- Removed sections: None
- Templates requiring updates:
	- .specify/templates/plan-template.md ✅ updated (Constitution Check gates aligned with ACP principles)
	- .specify/templates/spec-template.md ✅ reviewed (no changes needed)
	- .specify/templates/tasks-template.md ✅ reviewed (no changes needed)
	- .specify/templates/commands/*.md ⚠ pending (no command templates present in repo)
- Deferred TODOs: None
-->

# Autonomous Command Protocol (ACP) Constitution

## Core Principles

### Portability First

ACP MUST run on bare-metal, embedded RTOS, and desktop OS (Linux/macOS/Windows), and be
adaptable to future microcontroller platforms via thin shims. Rationale: ACP targets
heterogeneous systems; portability ensures one spec, many runtimes.

### Minimal & Deterministic

Messages MUST be fixed-format or efficiently encoded to ensure consistent memory layout,
size, and parsing characteristics across platforms. Rationale: determinism underpins
predictability, bounded latency, and ease of verification.

### Security by Design

Authenticated command messages MUST use truncated HMAC-SHA256, bind to session nonces,
and include replay protection. Rationale: cryptographic integrity and anti-replay are
non-negotiable for command and control safety.

### Extensible but Stable

Framing and field layouts MUST remain backward-compatible within a major version.
Rationale: ecosystems evolve, but stability is required for interoperable upgrades.

### Open, Documented, and Testable

Every ACP version MUST ship with a complete public framing spec, a portable reference
implementation, and conformance test vectors. Rationale: transparency and testing drive
adoption and reliability.

## Functional Scope and Constraints

The ACP project governs the protocol specification and reference C library.

In scope:

- Message framing and parsing
- Frame-level authentication (HMAC) for commands
- Telemetry, command, and system message support
- Audit logging (e.g., tlm_audit)
- Session management (nonces, tokens, replay protection)
- Platform abstraction shims: logging, time, mutex, keystore (POSIX and embedded)

Out of scope:

- Transport layers (serial, USB, CAN, TCP) — handled by host system
- UI, GUI, or Web interface logic
- Application-specific payload families (e.g., MAVLink, ROS)
- File transfer, encryption, compression (unless ratified in a future spec)

## Versioning, Testing, and Ecosystem

### Versioning & Compatibility

- ACP uses semantic versioning (MAJOR.MINOR.PATCH).
- Minor versions MAY add message types, fields, or constants, and MUST remain backward-compatible.
- Major versions MAY alter framing or authentication requirements.
- All ACP frames MUST include a version field in the header to enable negotiation.

### Test & Validation

- Implementations MUST include vector-based encode/decode tests and CRC/HMAC test cases.
- Cross-platform builds MUST be exercised for supported targets.
- Protocol spec and test vectors MUST be updated together in every change set.

### Ecosystem Growth

- Future language bindings MAY include Python (acp.py), Rust (acp-rs), and Go (acp-go).
- Integrations MAY target companion systems (GCS, firmware, security layers).
- ACP MAY be registered as a formal protocol (optional).

### Design Mantras

- "Framed, not streamed."
- "Secure by default."
- "If it compiles on Linux and builds on a drone, it's good enough."

## Governance

- Maintainer holds authority for merging new ACP versions.
- Protocol-breaking changes REQUIRE a major version bump and documented justification.
- Reference implementations MUST match published specs at the same version.
- Community contributions are welcome under the MIT license and contributor agreements if applicable.
- Amendments: Any change to principles, scope, or governance MUST update this constitution,
  bump the version per semantic rules, and record Last Amended date (ISO YYYY-MM-DD).
- Compliance: PRs and reviews MUST verify conformance to principles and required tests/vectors.

**Version**: 1.0.0 | **Ratified**: 2025-10-27 | **Last Amended**: 2025-10-27
