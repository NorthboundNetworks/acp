# Quickstart — ACP Reference C Library

This guide shows how to build the ACP library and run a minimal encode/decode example on Linux/macOS. Windows (MinGW) is similar.

## Prerequisites

- C toolchain (clang or gcc)
- make and/or CMake 3.16+

## Build

Option A: make

```sh
# from repo root
make
```

Option B: CMake

```sh
# from repo root
mkdir -p build && cd build
cmake ..
cmake --build . --config Release
```

Outputs:

- Static library: libacp.a
- Shared library: libacp.so (Linux) or libacp.dylib (macOS)

## Minimal usage

```c
#include <stdint.h>
#include <stdio.h>
// #include "acp_protocol.h" // placeholder for final public header

int main(void) {
  const uint8_t payload[] = {0x01, 0x02, 0x03};
  uint8_t frame[256];
  size_t frame_len = 0;

  // acp_encode_frame(payload, sizeof(payload), /*type=*/1, /*flags=*/0,
  //                  /*session=*/NULL, frame, sizeof(frame), &frame_len);

  // Normally: transmit frame and decode on receiver side

  printf("Encoded %zu bytes (demo)\n", frame_len);
  return 0;
}
```

## Tests

- Vector tests for CRC16, HMAC, and COBS live under `tests/`.
- Round-trip frame encode/decode tests validate correctness and replay protection.

## Platform shims

- Defaults provided for logging, time, mutex, and keystore.
- To customize, implement functions declared in:
  - `acp_platform_log.h`
  - `acp_platform_time.h`
  - `acp_platform_mutex.h`
  - `acp_platform_keystore.h`

## Notes

- Wire format uses network byte order.
- Maximum payload is 1024 bytes; larger inputs are rejected.
- When authentication is enabled, HMAC-SHA256 (16-byte tag) and replay protection are enforced.
