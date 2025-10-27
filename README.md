# ACP — Autonomous Command Protocol

**ACP** is a lightweight, cross-platform communication protocol designed for secure, deterministic data exchange between autonomous systems, such as unmanned vehicles, ground control stations, and diagnostic interfaces.

This repository provides the **official C reference implementation** of ACP, suitable for embedded firmware, CLI telemetry tools, and cross-platform integration (e.g., Linux, RTOS, or macOS).

---

## 🎯 Purpose

The goal of ACP is to define a **simple, robust binary protocol** that ensures:

- ✅ Low-latency telemetry streaming
- ✅ Reliable field diagnostics over serial or radio links
- ✅ Cross-language, cross-platform compatibility
- ✅ Integration into mission-critical systems (e.g., GCS, drones, Lattice OS)

---

## 🔧 Features

- 🧱 Fixed-size binary frame format
- 🛰️ Support for telemetry, command, and system messages
- 🧩 Lightweight and dependency-free C implementation
- 🔐 Optional CRC / future support for authenticated frames
- 🧪 Test vectors and mock ACP generator included

---

## 📦 Repository Structure

```plaintext
acp/
├── acp.h            # Frame definition, structs, constants
├── acp.c            # Encoding/decoding logic
├── test_acp.c       # CLI tool for decoding test frames
├── spec/
│   ├── acp-v1.0.md  # Protocol documentation
│   └── examples/    # Example frame diagrams and field notes
├── tests/
│   └── vectors.json # Reference vectors for conformance
├── Makefile         # Build for Linux/macOS
├── LICENSE          # MIT License
└── README.md        # This file


⸻

🚀 Quick Start

🧪 Build and run decoder tool:

git clone https://github.com/pzanna/acp.git
cd acp
make
./test_acp vectors/test-frame.bin

Note: this repo is pure C. No external dependencies.

⸻

💡 Usage

Include in your C project:

#include "acp.h"

acp_telemetry_t frame;
if (acp_decode_telemetry(buffer, len, &frame)) {
    printf("CPU Temp: %.2f C\n", frame.cpu_temp_c);
}

Link:

Add acp.c to your project and include acp.h. No dynamic allocation or platform dependencies.

⸻

📘 Protocol Overview

ACP frames are:
 • Versioned (uint8_t version)
 • Typed (e.g., ACP_FRAME_TELEMETRY)
 • Packed binary for minimal overhead
 • CRC protected (optional)
 • Little or big endian (TBD; configurable)

See docs/acp_comm_spec_v0-3.md for full framing structure.

⸻

📜 License

This project is licensed under the MIT License.

Copyright (c) 2025 Northbound Networks Pty. Ltd.
