#!/bin/bash
# ACP Build Validation Script - Quick build system check

set -e

echo "ACP Build System Validation"
echo "==========================="

# Check if we're in the right directory
if [ ! -f acp_protocol.h ]; then
    echo "❌ Error: Run this script from the ACP project root directory"
    exit 1
fi

echo "✓ In ACP project directory"

# Test Make build
echo "Testing Make build..."
make clean >/dev/null 2>&1 || true
if make static >/dev/null 2>&1; then
    echo "✓ Make static build: SUCCESS"
else
    echo "❌ Make static build: FAILED"
    exit 1
fi

# Check library file
if [ -f build/lib/libacp.a ]; then
    echo "✓ Static library generated: $(du -h build/lib/libacp.a | cut -f1)"
else
    echo "❌ Static library not found"
    exit 1
fi

# Test shared library (Unix only)
if [[ "$OSTYPE" != "msys" && "$OSTYPE" != "cygwin" ]]; then
    if make shared >/dev/null 2>&1; then
        echo "✓ Make shared build: SUCCESS"
    else
        echo "❌ Make shared build: FAILED"
        exit 1
    fi
fi

# Test CMake if available
if command -v cmake >/dev/null 2>&1; then
    echo "Testing CMake build..."
    rm -rf build_validate
    mkdir build_validate
    cd build_validate
    
    if cmake .. >/dev/null 2>&1 && cmake --build . >/dev/null 2>&1; then
        echo "✓ CMake build: SUCCESS"
        if [ -f libacp.a ]; then
            echo "✓ CMake static library: $(du -h libacp.a | cut -f1)"
        fi
    else
        echo "❌ CMake build: FAILED"
        cd ..
        exit 1
    fi
    cd ..
else
    echo "ℹ CMake not available - skipped"
fi

# Test feature validation
echo "Testing library functionality..."
if gcc -std=c99 -Wall -I. tests/acp_feature_test.c build/lib/libacp.a -o validate_test 2>/dev/null; then
    echo "✓ Feature test compilation: SUCCESS"
    if ./validate_test >/dev/null 2>&1; then
        echo "✓ Feature test execution: SUCCESS"
    else
        echo "❌ Feature test execution: FAILED"
        exit 1
    fi
else
    echo "❌ Feature test compilation: FAILED"
    exit 1
fi

# Cleanup
rm -f validate_test
rm -rf build_validate
make clean >/dev/null 2>&1 || true

echo
echo "🎉 Build system validation: PASSED"
echo "   The ACP library build system is working correctly!"
echo