#!/bin/bash

# Build script for SYCL device selection testing
set -e

echo "=== Building SYCL Test Programs ==="
echo

# Set up environment variables for Intel oneAPI
if [ -z "$ONEAPI_ROOT" ]; then
    echo "Warning: ONEAPI_ROOT not set, trying to find Intel oneAPI..."
    if [ -d "/opt/intel/oneapi" ]; then
        export ONEAPI_ROOT="/opt/intel/oneapi"
    elif [ -d "/opt/aurora/24.347.0/spack/unified/0.9.2/install/linux-sles15-x86_64/gcc-13.3.0/intel-oneapi-compilers-2024.1.0" ]; then
        export ONEAPI_ROOT="/opt/aurora/24.347.0/spack/unified/0.9.2/install/linux-sles15-x86_64/gcc-13.3.0/intel-oneapi-compilers-2024.1.0"
    else
        echo "Error: Could not find Intel oneAPI installation"
        exit 1
    fi
fi

echo "Using ONEAPI_ROOT: $ONEAPI_ROOT"

# Set up compiler environment
export SYCL_CXX_COMPILE_FLAGS="-fsycl -fsycl-device-code-split=per_kernel -sycl-std=2020"
export CXXFLAGS="-g -O0 -Wall -Wextra"

# Build the SYCL test program
echo "Building test_sycl_device..."
dpcpp $SYCL_CXX_COMPILE_FLAGS $CXXFLAGS -o test_sycl_device test_sycl_device.cpp

if [ $? -eq 0 ]; then
    echo "✓ test_sycl_device built successfully"
else
    echo "✗ Failed to build test_sycl_device"
    exit 1
fi

echo

# Build PWDFT with SYCL support
echo "Building PWDFT with SYCL support..."
if [ -d "build_sycl" ]; then
    echo "Using existing build_sycl directory"
    cd build_sycl
else
    echo "Creating new build_sycl directory"
    mkdir build_sycl
    cd build_sycl
    
    # Configure with SYCL support
    cmake .. -DNWPW_SYCL=ON -DCMAKE_BUILD_TYPE=Debug
fi

# Build PWDFT
make -j4

if [ $? -eq 0 ]; then
    echo "✓ PWDFT built successfully"
else
    echo "✗ Failed to build PWDFT"
    exit 1
fi

cd ..

echo
echo "=== Build Complete ==="
echo
echo "To test the SYCL device selection:"
echo "  ./test_aurora_sycl.sh"
echo
echo "To test with Aurora GPU tiling:"
echo "  mpiexec -n 2 --ppn 2 --host localhost --depth=1 --cpu-bind depth \\"
echo "    --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \\"
echo "    ./gpu_tile_compact.sh ./build_sycl/pwdft test_input.nw" 