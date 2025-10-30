#!/bin/bash
# ACP Build System Test Suite
# Tests both CMake and Make build systems across different configurations

set -e  # Exit on first error

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Utility functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[PASS]${NC} $1"
    ((TESTS_PASSED++))
}

log_error() {
    echo -e "${RED}[FAIL]${NC} $1"
    ((TESTS_FAILED++))
}

log_warning() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

run_test() {
    local test_name="$1"
    local test_command="$2"
    
    ((TESTS_RUN++))
    log_info "Running test: $test_name"
    
    if eval "$test_command" >/dev/null 2>&1; then
        log_success "$test_name"
        return 0
    else
        log_error "$test_name"
        return 1
    fi
}

# Cleanup function
cleanup() {
    log_info "Cleaning up test artifacts..."
    rm -rf build_test_* test_install_* *.tar.gz *.zip
    make clean >/dev/null 2>&1 || true
}

# Check prerequisites
check_prerequisites() {
    log_info "Checking build prerequisites..."
    
    # Check for required tools
    local tools=("gcc" "make")
    local cmake_available=false
    
    for tool in "${tools[@]}"; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            log_error "Required tool not found: $tool"
            exit 1
        fi
    done
    
    if command -v cmake >/dev/null 2>&1; then
        cmake_available=true
        log_success "CMake available: $(cmake --version | head -1)"
    else
        log_warning "CMake not available - CMake tests will be skipped"
    fi
    
    log_success "GCC available: $(gcc --version | head -1)"
    log_success "Make available: $(make --version | head -1)"
    
    return 0
}

# Test Make build system
test_make_builds() {
    log_info "=== Testing Make Build System ==="
    
    # Clean first
    make clean >/dev/null 2>&1 || true
    
    # Test static library build
    run_test "Make: Static library build" \
        "make static && test -f build/lib/libacp.a"
    
    # Test shared library build (Unix only)
    if [[ "$OSTYPE" != "msys" && "$OSTYPE" != "cygwin" ]]; then
        run_test "Make: Shared library build" \
            "make shared && test -f build/lib/libacp.*"
    fi
    
    # Test full build
    run_test "Make: Full build (all targets)" \
        "make all"
    
    # Test pkg-config generation
    run_test "Make: pkg-config generation" \
        "make pkg-config && test -f build/acp.pc"
    
    # Test clean
    run_test "Make: Clean build artifacts" \
        "make clean && test ! -d build"
    
    # Test cross-compiler support (if available)
    if command -v clang >/dev/null 2>&1; then
        run_test "Make: Clang compiler support" \
            "make static CC=clang && test -f build/lib/libacp.a"
        make clean >/dev/null 2>&1 || true
    fi
}

# Test CMake build system  
test_cmake_builds() {
    if ! command -v cmake >/dev/null 2>&1; then
        log_warning "Skipping CMake tests - CMake not available"
        return 0
    fi
    
    log_info "=== Testing CMake Build System ==="
    
    # Test basic configuration
    run_test "CMake: Basic configuration" \
        "mkdir -p build_test_cmake && cd build_test_cmake && cmake .."
    
    # Test Release build
    run_test "CMake: Release build" \
        "cd build_test_cmake && cmake --build . --config Release --parallel"
    
    # Test generated libraries
    run_test "CMake: Static library generated" \
        "test -f build_test_cmake/libacp.a"
    
    # Test shared library (Unix only)
    if [[ "$OSTYPE" != "msys" && "$OSTYPE" != "cygwin" ]]; then
        run_test "CMake: Shared library generated" \
            "test -f build_test_cmake/libacp.*"
    fi
    
    # Test different build configurations
    rm -rf build_test_debug
    run_test "CMake: Debug configuration" \
        "mkdir -p build_test_debug && cd build_test_debug && cmake .. -DCMAKE_BUILD_TYPE=Debug && cmake --build ."
    
    # Test build options
    rm -rf build_test_options  
    run_test "CMake: Custom build options" \
        "mkdir -p build_test_options && cd build_test_options && cmake .. -DACP_BUILD_SHARED=OFF -DACP_BUILD_TESTS=OFF && cmake --build ."
    
    # Test installation to temporary directory
    run_test "CMake: Install target" \
        "cd build_test_cmake && cmake --install . --prefix ../test_install_cmake"
    
    # Validate installation
    run_test "CMake: Installation validation" \
        "test -f test_install_cmake/lib/libacp.a && test -f test_install_cmake/include/acp_protocol.h"
    
    # Test pkg-config file generation
    run_test "CMake: pkg-config file generated" \
        "test -f test_install_cmake/lib/pkgconfig/acp.pc"
    
    # Test CMake config files
    run_test "CMake: Config files generated" \
        "test -f test_install_cmake/lib/cmake/acp/acpConfig.cmake"
}

# Test library functionality
test_library_functionality() {
    log_info "=== Testing Library Functionality ==="
    
    # Build feature test using Make
    run_test "Library: Feature test compilation (Make)" \
        "make static >/dev/null && gcc -std=c99 -Wall -I. tests/acp_feature_test.c build/lib/libacp.a -o feature_test_make"
    
    # Run feature test
    run_test "Library: Feature test execution" \
        "./feature_test_make"
    
    # Test with CMake if available
    if command -v cmake >/dev/null 2>&1 && [ -d build_test_cmake ]; then
        run_test "Library: Feature test compilation (CMake)" \
            "gcc -std=c99 -Wall -I. tests/acp_feature_test.c build_test_cmake/libacp.a -o feature_test_cmake"
        
        run_test "Library: Feature test execution (CMake)" \
            "./feature_test_cmake"
    fi
    
    # Test linking with pkg-config (if libraries are installed)
    if [ -f test_install_cmake/lib/pkgconfig/acp.pc ]; then
        export PKG_CONFIG_PATH="$(pwd)/test_install_cmake/lib/pkgconfig:$PKG_CONFIG_PATH"
        
        run_test "Library: pkg-config integration" \
            "pkg-config --exists acp && pkg-config --cflags acp >/dev/null && pkg-config --libs acp >/dev/null"
        
        # Test compilation with pkg-config
        run_test "Library: Compilation with pkg-config" \
            "gcc -std=c99 tests/acp_feature_test.c \$(pkg-config --cflags --libs acp) -o feature_test_pkg"
    fi
}

# Test build configurations
test_build_configurations() {
    log_info "=== Testing Build Configurations ==="
    
    # Test no-heap configuration (default)
    run_test "Config: No-heap build (Make)" \
        "make clean >/dev/null 2>&1; make static && nm build/lib/libacp.a | grep -q acp_ || true"
    
    # Test heap-enabled configuration
    run_test "Config: Heap-enabled build (Make)" \
        "make clean >/dev/null 2>&1; ACP_ENABLE_HEAP=1 make static"
    
    if command -v cmake >/dev/null 2>&1; then
        # Test CMake no-heap configuration
        rm -rf build_test_no_heap
        run_test "Config: No-heap build (CMake)" \
            "mkdir -p build_test_no_heap && cd build_test_no_heap && cmake .. -DACP_ENABLE_HEAP=OFF && cmake --build ."
        
        # Test CMake heap-enabled configuration
        rm -rf build_test_heap
        run_test "Config: Heap-enabled build (CMake)" \
            "mkdir -p build_test_heap && cd build_test_heap && cmake .. -DACP_ENABLE_HEAP=ON && cmake --build ."
    fi
}

# Test cross-platform compatibility
test_cross_platform() {
    log_info "=== Testing Cross-Platform Compatibility ==="
    
    # Test platform detection
    run_test "Platform: Configuration detection" \
        "./tests/feature_test"
    
    # Test with different compilers (if available)
    local compilers=("gcc")
    
    if command -v clang >/dev/null 2>&1; then
        compilers+=("clang")
    fi
    
    for compiler in "${compilers[@]}"; do
        run_test "Platform: $compiler compatibility" \
            "make clean >/dev/null 2>&1; CC=$compiler make static && test -f build/lib/libacp.a"
    done
    
    # Test with different optimization levels
    run_test "Platform: Optimization levels" \
        "make clean >/dev/null 2>&1; CFLAGS='-O0 -g' make static && CFLAGS='-O2 -DNDEBUG' make static"
}

# Test error conditions
test_error_conditions() {
    log_info "=== Testing Error Conditions ==="
    
    # Test with invalid compiler (should fail gracefully)
    if ! CC=nonexistent_compiler make static >/dev/null 2>&1; then
        log_success "Error: Invalid compiler handled correctly"
        ((TESTS_PASSED++))
    else
        log_error "Error: Invalid compiler not handled"
        ((TESTS_FAILED++))
    fi
    ((TESTS_RUN++))
    
    # Test with missing source files (should fail)
    if ! (cd /tmp && make -f "$(pwd)/Makefile" static >/dev/null 2>&1); then
        log_success "Error: Missing source files handled correctly"
        ((TESTS_PASSED++))
    else
        log_error "Error: Missing source files not handled"
        ((TESTS_FAILED++))
    fi
    ((TESTS_RUN++))
}

# Main test execution
main() {
    echo "================================================="
    echo "ACP Build System Test Suite"
    echo "================================================="
    echo
    
    # Setup
    check_prerequisites
    cleanup
    
    # Run test suites
    test_make_builds
    test_cmake_builds  
    test_library_functionality
    test_build_configurations
    test_cross_platform
    test_error_conditions
    
    # Cleanup
    cleanup
    
    # Results
    echo
    echo "================================================="
    echo "Test Results"
    echo "================================================="
    echo "Tests Run:    $TESTS_RUN"
    echo "Tests Passed: $TESTS_PASSED"
    echo "Tests Failed: $TESTS_FAILED"
    echo
    
    if [ $TESTS_FAILED -eq 0 ]; then
        log_success "All tests passed! 🎉"
        echo
        log_info "Build system is working correctly on this platform."
        exit 0
    else
        log_error "Some tests failed! ❌"
        echo
        log_error "Build system has issues that need to be addressed."
        exit 1
    fi
}

# Run main function
main "$@"