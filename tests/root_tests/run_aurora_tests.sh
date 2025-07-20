#!/bin/bash

# Aurora Test Runner for SYCL Device Selection Fix
# This script runs the most critical tests to validate the fix

set -e

# Source Aurora configuration
if [ -f "./aurora_config.sh" ]; then
    source ./aurora_config.sh
    echo "✓ Loaded Aurora configuration"
else
    echo "Warning: aurora_config.sh not found, using defaults"
    # Set defaults
    HOST="localhost"
    PWDFT_PATH="./build/pwdft"
    GPU_TILE_SCRIPT="./gpu_tile_compact.sh"
    MPI_NUM_RANKS_SMALL=2
    MPI_NUM_RANKS_MEDIUM=6
    MPI_NUM_RANKS_LARGE=12
fi

echo "=========================================="
echo "Aurora SYCL Device Selection Test Suite"
echo "=========================================="
echo "Date: $(date)"
echo "Commit: $(git rev-parse HEAD)"
echo ""

# Print configuration
if command -v print_config >/dev/null 2>&1; then
    print_config
fi

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
PASSED=0
FAILED=0

# Function to check if file exists
check_file() {
    local file="$1"
    if [ ! -f "$file" ]; then
        echo -e "${RED}Error: $file not found${NC}"
        return 1
    fi
    return 0
}

# Function to run test and check result
run_test() {
    local test_name="$1"
    local test_command="$2"
    local expected_pattern="$3"
    
    echo -e "${YELLOW}Running: $test_name${NC}"
    echo "Command: $test_command"
    echo "---"
    
    if eval "$test_command" 2>&1 | tee /tmp/test_output.log; then
        if [ -n "$expected_pattern" ]; then
            if grep -q "$expected_pattern" /tmp/test_output.log; then
                echo -e "${GREEN}✓ PASS: $test_name${NC}"
                ((PASSED++))
            else
                echo -e "${RED}✗ FAIL: $test_name (expected pattern not found)${NC}"
                ((FAILED++))
            fi
        else
            echo -e "${GREEN}✓ PASS: $test_name${NC}"
            ((PASSED++))
        fi
    else
        echo -e "${RED}✗ FAIL: $test_name (command failed)${NC}"
        ((FAILED++))
    fi
    echo ""
}

# Pre-test environment check
echo "=== Environment Check ==="
echo "GPU Count: $(/usr/bin/udevadm info /sys/module/i915/drivers/pci:i915/* |& grep -v Unknown | grep -c "P: /devices" || echo "0")"
echo "SYCL Devices:"
sycl-ls || echo "sycl-ls not available"
echo ""

# Build test_sycl_device if it doesn't exist
echo "=== Building Test Programs ==="
if [ ! -f "./test_sycl_device" ]; then
    echo "Building test_sycl_device..."
    if [ -f "./test_sycl_device.cpp" ]; then
        # Try to find dpcpp compiler
        if command -v dpcpp >/dev/null 2>&1; then
            dpcpp -fsycl -fsycl-device-code-split=per_kernel -sycl-std=2020 -g -O0 -Wall -Wextra -o test_sycl_device test_sycl_device.cpp
            echo "✓ test_sycl_device built successfully"
        else
            echo -e "${RED}Error: dpcpp compiler not found. Please set up Intel oneAPI environment.${NC}"
            exit 1
        fi
    else
        echo -e "${RED}Error: test_sycl_device.cpp not found${NC}"
        exit 1
    fi
else
    echo "✓ test_sycl_device already exists"
fi

# Check if PWDFT is built
if [ ! -f "$PWDFT_PATH" ]; then
    echo -e "${YELLOW}Warning: PWDFT not found at $PWDFT_PATH${NC}"
    echo "To build PWDFT, run:"
    echo "  mkdir build && cd build"
    echo "  cmake .. -DNWPW_SYCL=ON -DCMAKE_BUILD_TYPE=Debug"
    echo "  make -j4"
    echo ""
    # Try alternative paths
    if [ -f "./build_sycl/pwdft" ]; then
        PWDFT_PATH="./build_sycl/pwdft"
        echo "Found PWDFT at: $PWDFT_PATH"
    else
        echo -e "${YELLOW}Warning: PWDFT executable not found. PWDFT tests will be skipped.${NC}"
    fi
fi

# Test 1: Basic SYCL device test
if check_file "./test_sycl_device"; then
    run_test "Basic SYCL Device Selection" \
        "./test_sycl_device" \
        "✓ Selected"
fi

# Test 2: Single rank without GPU tiling
if check_file "./test_sycl_device"; then
    run_test "Single Rank SYCL Test" \
        "mpiexec -n 1 --ppn 1 --host $HOST --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close ./test_sycl_device" \
        "✓ Selected"
fi

# Test 3: Single rank with Aurora GPU tiling
if check_file "./test_sycl_device" && check_file "$GPU_TILE_SCRIPT"; then
    run_test "Single Rank with Aurora GPU Tiling" \
        "mpiexec -n 1 --ppn 1 --host $HOST --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close $GPU_TILE_SCRIPT ./test_sycl_device" \
        "✓ Selected"
fi

# Test 4: Multiple ranks with GPU tiling
if check_file "./test_sycl_device" && check_file "$GPU_TILE_SCRIPT"; then
    run_test "Multiple Ranks with Aurora GPU Tiling" \
        "mpiexec -n 4 --ppn 4 --host $HOST --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close $GPU_TILE_SCRIPT ./test_sycl_device" \
        "✓ Selected"
fi

# Test 5: Simple PWDFT single rank
if [ -n "$PWDFT_PATH" ] && check_file "test_simple.nw"; then
    run_test "Simple PWDFT Single Rank" \
        "$PWDFT_PATH test_simple.nw" \
        "task completed"
fi

# Test 6: PWDFT with Aurora GPU tiling (small)
if [ -n "$PWDFT_PATH" ] && check_file "test_simple.nw" && check_file "$GPU_TILE_SCRIPT"; then
    run_test "PWDFT with Aurora GPU Tiling (Small)" \
        "mpiexec -n $MPI_NUM_RANKS_SMALL --ppn $MPI_NUM_RANKS_SMALL --host $HOST --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close $GPU_TILE_SCRIPT $PWDFT_PATH test_simple.nw" \
        "task completed"
fi

# Test 7: PWDFT with Aurora GPU tiling (medium)
if [ -n "$PWDFT_PATH" ] && check_file "test_simple.nw" && check_file "$GPU_TILE_SCRIPT"; then
    run_test "PWDFT with Aurora GPU Tiling (Medium)" \
        "mpiexec -n $MPI_NUM_RANKS_MEDIUM --ppn $MPI_NUM_RANKS_MEDIUM --host $HOST --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close $GPU_TILE_SCRIPT $PWDFT_PATH test_simple.nw" \
        "task completed"
fi

# Test 8: Error handling - no GPU fallback
if [ -n "$PWDFT_PATH" ] && check_file "test_simple.nw"; then
    run_test "No GPU Fallback Test" \
        "unset ZE_AFFINITY_MASK; unset ONEAPI_DEVICE_SELECTOR; $PWDFT_PATH test_simple.nw" \
        "task completed"
fi

# Test 9: Memory stress test (conditional)
if [ "$RUN_STRESS_TESTS" = "true" ] && [ -n "$PWDFT_PATH" ] && check_file "$GPU_TILE_SCRIPT"; then
    echo "Creating stress test input..."
    cat > test_stress.nw << EOF
title "Memory Stress Test"

geometry
  H 0.0 0.0 0.0
  H 0.74 0.0 0.0
  O 0.0 1.0 0.0
  H 0.0 1.5 0.0
end

nwpw
  simulation_cell
    boundary_conditions periodic
    unita 15.0 0.0 0.0
             0.0 15.0 0.0
             0.0 0.0 15.0
  end
  xc pbe
  minimizer 2
  scf_algorithm broyden
  scf_alpha 0.25
  scf_beta 0.5
  lmbfgs_size 5
  print medium
end

task pspw energy
EOF

    run_test "Memory Stress Test" \
        "mpiexec -n 4 --ppn 4 --host $HOST --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close $GPU_TILE_SCRIPT $PWDFT_PATH test_stress.nw" \
        "task completed"
fi

# Test 10: Long-running test (conditional)
if [ "$RUN_LONG_TESTS" = "true" ] && [ -n "$PWDFT_PATH" ] && check_file "$GPU_TILE_SCRIPT"; then
    echo "Creating long-running test input..."
    cat > test_long.nw << EOF
title "Long-Running Test"

geometry
  H 0.0 0.0 0.0
  H 0.74 0.0 0.0
end

nwpw
  simulation_cell
    boundary_conditions periodic
    unita 10.0 0.0 0.0
             0.0 10.0 0.0
             0.0 0.0 10.0
  end
  xc pbe
  minimizer 2
  scf_algorithm broyden
  scf_alpha 0.25
  scf_beta 0.5
  lmbfgs_size 3
  loop 10 50
  tolerances 1.0e-8 1.0e-8 1.0e-6
  print medium
end

task pspw energy
EOF

    run_test "Long-Running Test" \
        "mpiexec -n 2 --ppn 2 --host $HOST --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close $GPU_TILE_SCRIPT $PWDFT_PATH test_long.nw" \
        "task completed"
fi

# Summary
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo "Passed: $PASSED"
echo "Failed: $FAILED"
echo "Total: $((PASSED + FAILED))"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed! SYCL device selection fix is working correctly.${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed. Please check the output above for details.${NC}"
    exit 1
fi 