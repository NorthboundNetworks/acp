# ACP Library Build Guide

This guide covers building, installing, and integrating the ACP (Autonomous Command Protocol) library across different platforms and build systems.

## Table of Contents

1. [Quick Start](#quick-start)
2. [Build Systems](#build-systems)
3. [Platform Support](#platform-support)
4. [Build Options](#build-options)
5. [Installation](#installation)
6. [Integration](#integration)
7. [Cross-Compilation](#cross-compilation)
8. [Troubleshooting](#troubleshooting)

## Quick Start

### Prerequisites

- **C99 compliant compiler**: GCC 4.9+, Clang 3.5+, or MSVC 2015+
- **Build tools**: Make and/or CMake 3.16+
- **Platform**: Linux, macOS, Windows (MinGW), or embedded systems

### Fast Build (CMake)

```bash
# Clone and build
git clone https://github.com/pzanna/acp.git
cd acp
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel
```

### Fast Build (Make)

```bash
# Clone and build
git clone https://github.com/pzanna/acp.git
cd acp
make all
```

## Build Systems

ACP supports two build systems with feature parity:

### CMake (Recommended)

CMake provides the most portable and feature-rich build experience:

```bash
# Configure
mkdir build && cd build
cmake .. [OPTIONS]

# Build
cmake --build . --parallel

# Test
ctest --output-on-failure

# Install
cmake --install . --prefix /usr/local
```

**Key CMake Features:**

- Automatic platform detection
- Shared/static library generation
- Cross-platform install targets
- Find module generation
- MSVC support (future)
- Embedded system support

### Make

Make provides a lightweight alternative with platform detection:

```bash
# Build everything
make all

# Build specific targets
make static          # Static library only
make shared          # Shared library only (Unix)  
make examples        # Example programs
make tests          # Test programs

# Run tests
make check

# Install
make install PREFIX=/usr/local

# Clean
make clean
```

## Platform Support

### Linux (x86_64, ARM64)

**Supported Compilers:**

- GCC 4.9+ (recommended: 9.0+)
- Clang 3.5+ (recommended: 10.0+)

**Build Commands:**

```bash
# CMake
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel

# Make  
make all CC=gcc
make all CC=clang
```

**Generated Libraries:**

- `libacp.a` (static)
- `libacp.so` (shared with versioning)

### macOS (Intel, Apple Silicon)

**Supported Compilers:**

- Clang (Xcode Command Line Tools)
- GCC via Homebrew

**Prerequisites:**

```bash
# Install Xcode Command Line Tools
xcode-select --install

# Optional: Install additional tools via Homebrew
brew install cmake gcc doxygen
```

**Build Commands:**

```bash
# CMake (uses system clang by default)
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel

# Make
make all
```

**Generated Libraries:**

- `libacp.a` (static)
- `libacp.dylib` (shared with versioning)

### Windows (MinGW, MSVC)

**Supported Compilers:**

- MinGW-w64 (via MSYS2) - Recommended
- Microsoft Visual C++ 2015+ (partial support)
- Clang for Windows

**Prerequisites:**

- MSYS2 with MinGW-w64 toolchain (recommended)
- CMake (via MSYS2 or standalone installer)
- Windows SDK (for native Windows APIs)

**Setup MSYS2 (Recommended):**

```bash
# Install complete toolchain
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-make
pacman -S mingw-w64-x86_64-pkg-config
```

**Build Commands:**

```bash
# CMake with MinGW (in MSYS2 shell)
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel

# CMake with Visual Studio
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Release

# Make (in MSYS2 shell)
make all PLATFORM=windows
```

**Generated Libraries:**

- `libacp.a` (static)
- Windows shared library support planned for v0.4

**Windows Platform Features:**

- Native Win32 API time functions (QueryPerformanceCounter)
- Critical Section mutexes
- Windows-style logging with timestamps
- Keystore in `C:\ProgramData\ACP\acp_keystore.bin`
- Automatic WS2_32 linking for socket support

**Runtime Dependencies:**

- None (statically linked by default)
- WS2_32.dll (Windows Sockets - standard on all Windows)

**Installation Paths:**

- Headers: `C:\Program Files\ACP\include\`
- Libraries: `C:\Program Files\ACP\lib\`
- Documentation: `C:\Program Files\ACP\doc\`

### Embedded Systems

ACP is designed for embedded use with the `ACP_NO_HEAP` flag (default):

```bash
# CMake for embedded (example for ARM Cortex-M)
cmake .. -DCMAKE_SYSTEM_NAME=Generic \
         -DCMAKE_C_COMPILER=arm-none-eabi-gcc \
         -DACP_NO_HEAP=ON \
         -DACP_BUILD_SHARED=OFF \
         -DACP_BUILD_EXAMPLES=OFF

# Make for embedded
make static CC=arm-none-eabi-gcc CPPFLAGS="-DACP_NO_HEAP=1"
```

## Build Options

### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `ACP_BUILD_STATIC` | ON | Build static library (.a) |
| `ACP_BUILD_SHARED` | ON | Build shared library (.so/.dylib) |
| `ACP_BUILD_EXAMPLES` | ON | Build example programs |
| `ACP_BUILD_TESTS` | ON | Build test programs |
| `ACP_ENABLE_HEAP` | OFF | Allow heap allocation (embedded=OFF) |
| `CMAKE_BUILD_TYPE` | Debug | Debug, Release, RelWithDebInfo, MinSizeRel |
| `CMAKE_INSTALL_PREFIX` | /usr/local | Installation directory |

**Example Configurations:**

```bash
# Production build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DACP_BUILD_TESTS=OFF \
         -DACP_BUILD_EXAMPLES=OFF

# Development build  
cmake .. -DCMAKE_BUILD_TYPE=Debug \
         -DACP_BUILD_TESTS=ON \
         -DACP_ENABLE_HEAP=ON

# Embedded build
cmake .. -DCMAKE_BUILD_TYPE=MinSizeRel \
         -DACP_NO_HEAP=ON \
         -DACP_BUILD_SHARED=OFF \
         -DACP_BUILD_EXAMPLES=OFF \
         -DACP_BUILD_TESTS=OFF
```

### Make Options

| Variable | Default | Description |
|----------|---------|-------------|
| `CC` | gcc | Compiler to use |
| `PREFIX` | /usr/local | Install prefix |
| `ACP_ENABLE_HEAP` | (unset) | Set to 1 to enable heap |
| `CFLAGS` | (see Makefile) | Additional compiler flags |
| `LDFLAGS` | (empty) | Additional linker flags |

**Example Configurations:**

```bash
# Cross-compilation
make CC=aarch64-linux-gnu-gcc PREFIX=/opt/acp

# Debug build
make CFLAGS="-g -O0 -DDEBUG"

# Embedded build  
make CC=arm-none-eabi-gcc ACP_ENABLE_HEAP=0
```

## Installation

### System-wide Installation

**CMake:**

```bash
# Install to system directories (requires sudo on Unix)
cd build
sudo cmake --install . --prefix /usr/local

# Install to custom prefix
cmake --install . --prefix $HOME/local
```

**Make:**

```bash
# Install to system directories
sudo make install

# Install to custom prefix  
make install PREFIX=$HOME/local
```

### Package Manager Integration

**pkg-config Support:**

```bash
# After installation, use pkg-config
pkg-config --cflags acp
pkg-config --libs acp

# Compile with pkg-config
gcc myapp.c $(pkg-config --cflags --libs acp) -o myapp
```

**CMake Find Module:**

```cmake
# In your CMakeLists.txt
find_package(acp REQUIRED)
target_link_libraries(myapp acp::static)
# OR
target_link_libraries(myapp acp::shared)
```

## Integration

### Using Static Library

**With pkg-config:**

```bash
gcc -std=c99 myapp.c $(pkg-config --cflags --libs acp) -o myapp
```

**Manual linking:**

```bash
gcc -std=c99 -I/usr/local/include myapp.c -L/usr/local/lib -lacp -o myapp
```

**CMake integration:**

```cmake
find_package(acp REQUIRED)
add_executable(myapp myapp.c)
target_link_libraries(myapp acp::static)
```

### Using Shared Library

**Runtime linking (Unix):**

```bash
# Ensure library is in system path or set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
./myapp
```

**CMake integration:**

```cmake
find_package(acp REQUIRED)
add_executable(myapp myapp.c)
target_link_libraries(myapp acp::shared)
```

### Code Example

```c
#include <acp_protocol.h>
#include <stdio.h>

int main(void) {
    // Initialize ACP
    if (acp_init() != ACP_OK) {
        fprintf(stderr, "Failed to initialize ACP\n");
        return 1;
    }
    
    // Your ACP application code here
    
    // Cleanup
    acp_cleanup();
    return 0;
}
```

## Cross-Compilation

### ARM Linux (aarch64)

```bash
# CMake
cmake .. -DCMAKE_SYSTEM_NAME=Linux \
         -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
         -DCMAKE_BUILD_TYPE=Release

# Make  
make CC=aarch64-linux-gnu-gcc
```

### ARM Embedded (Cortex-M)

```bash
# CMake
cmake .. -DCMAKE_SYSTEM_NAME=Generic \
         -DCMAKE_C_COMPILER=arm-none-eabi-gcc \
         -DACP_NO_HEAP=ON \
         -DACP_BUILD_SHARED=OFF \
         -DCMAKE_C_FLAGS="-mcpu=cortex-m4 -mthumb"

# Make
make CC=arm-none-eabi-gcc \
     CFLAGS="-mcpu=cortex-m4 -mthumb -DACP_NO_HEAP=1" \
     static
```

### Windows from Linux (MinGW)

```bash
# CMake  
cmake .. -DCMAKE_SYSTEM_NAME=Windows \
         -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
         -DCMAKE_BUILD_TYPE=Release

# Make
make CC=x86_64-w64-mingw32-gcc
```

## Troubleshooting

### Common Build Issues

**1. Missing compiler features:**

```
Error: ACP requires C99 or later
```

Solution: Use `-std=c99` or newer compiler version.

**2. Heap usage in no-heap build:**

```
Undefined reference to __wrap_malloc
```

Solution: Remove heap allocations or set `ACP_ENABLE_HEAP=ON`.

**3. Missing dependencies:**

```
fatal error: 'acp_protocol.h' file not found
```

Solution: Add include path with `-I` or install library properly.

**4. Link errors with shared library:**

```
error while loading shared libraries: libacp.so.0
```

Solution: Set `LD_LIBRARY_PATH` or install to system directory.

### Platform-Specific Issues

**macOS: Code signing issues**

```bash
# Allow unsigned binaries in development
codesign --remove-signature ./build/libacp.dylib
```

**Windows: MSYS2 path issues**

```bash
# Use proper MSYS2 environment
export PATH="/mingw64/bin:$PATH"
```

**Linux: Permission issues**

```bash
# Install to user directory instead
cmake --install . --prefix $HOME/.local
```

### Debug Builds

Enable verbose output:

```bash
# CMake verbose build
cmake --build . --verbose

# Make verbose build  
make V=1

# Run feature test for diagnostics
./tests/acp_feature_test
```

### Getting Help

1. Check the [GitHub Issues](https://github.com/pzanna/acp/issues)
2. Run the feature validation: `./tests/acp_feature_test`  
3. Enable verbose builds for detailed error messages
4. Consult platform-specific documentation for your toolchain

---

For more information, see:

- [Protocol Specification](docs/acp_comm_spec_v0-3.md)
- [API Documentation](docs/) (generated with Doxygen)
- [Example Applications](examples/)
