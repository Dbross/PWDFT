#!/bin/bash

# Aurora Build Script for SYCL Tests
# This script builds the SYCL test programs on Aurora

set -e

echo "=== Building Aurora SYCL Test Programs ==="
echo ""

# Check for Intel oneAPI environment
if [ -z "$ONEAPI_ROOT" ]; then
    # Try to find Intel oneAPI installation
    if [ -d "/opt/intel/oneapi" ]; then
        export ONEAPI_ROOT="/opt/intel/oneapi"
        echo "Found Intel oneAPI at: $ONEAPI_ROOT"
    elif [ -d "/usr/local/intel/oneapi" ]; then
        export ONEAPI_ROOT="/usr/local/intel/oneapi"
        echo "Found Intel oneAPI at: $ONEAPI_ROOT"
    else
        echo "Warning: ONEAPI_ROOT not set and Intel oneAPI not found in standard locations"
        echo "Trying to find dpcpp compiler in PATH..."
    fi
fi

# Set up environment if ONEAPI_ROOT is available
if [ -n "$ONEAPI_ROOT" ]; then
    echo "Setting up Intel oneAPI environment..."
    source "$ONEAPI_ROOT/setvars.sh" > /dev/null 2>&1 || echo "Warning: setvars.sh not found"
fi

# Check for dpcpp compiler
if command -v dpcpp >/dev/null 2>&1; then
    echo "✓ Found dpcpp compiler: $(which dpcpp)"
    DPCPP_COMPILER="dpcpp"
elif command -v clang++ >/dev/null 2>&1; then
    echo "✓ Found clang++ compiler: $(which clang++)"
    DPCPP_COMPILER="clang++"
else
    echo "Error: No suitable SYCL compiler found (dpcpp or clang++)"
    echo "Please set up Intel oneAPI environment"
    exit 1
fi

# Build test_sycl_device
echo ""
echo "Building test_sycl_device..."
if [ -f "test_sycl_device.cpp" ]; then
    $DPCPP_COMPILER -fsycl -fsycl-device-code-split=per_kernel -sycl-std=2020 \
        -g -O0 -Wall -Wextra -o test_sycl_device test_sycl_device.cpp
    
    if [ $? -eq 0 ]; then
        echo "✓ test_sycl_device built successfully"
    else
        echo "✗ Failed to build test_sycl_device"
        exit 1
    fi
else
    echo "Error: test_sycl_device.cpp not found"
    exit 1
fi

# Test the built program
echo ""
echo "Testing built program..."
if [ -f "test_sycl_device" ]; then
    ./test_sycl_device
    if [ $? -eq 0 ]; then
        echo "✓ test_sycl_device runs successfully"
    else
        echo "✗ test_sycl_device failed to run"
        exit 1
    fi
fi

echo ""
echo "=== Build completed successfully ==="
echo "Available executables:"
echo "  - test_sycl_device"
echo ""
echo "Ready to run Aurora tests!" 