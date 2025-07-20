#!/bin/bash

# Aurora Test Runner for SYCL Device Selection Fix
# This script runs the most critical tests to validate the fix

set -e

echo "=========================================="
echo "Aurora SYCL Device Selection Test Suite"
echo "=========================================="
echo "Date: $(date)"
echo "Commit: $(git rev-parse HEAD)"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
PASSED=0
FAILED=0

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

# Test 1: Basic SYCL device test
run_test "Basic SYCL Device Selection" \
    "./test_sycl_device" \
    "✓ Selected"

# Test 2: Single rank without GPU tiling
run_test "Single Rank SYCL Test" \
    "mpiexec -n 1 --ppn 1 --host localhost --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close ./test_sycl_device" \
    "✓ Selected"

# Test 3: Single rank with Aurora GPU tiling
run_test "Single Rank with Aurora GPU Tiling" \
    "mpiexec -n 1 --ppn 1 --host localhost --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close ./gpu_tile_compact.sh ./test_sycl_device" \
    "✓ Selected"

# Test 4: Multiple ranks with GPU tiling
run_test "Multiple Ranks with Aurora GPU Tiling" \
    "mpiexec -n 4 --ppn 4 --host localhost --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close ./gpu_tile_compact.sh ./test_sycl_device" \
    "✓ Selected"

# Test 5: Simple PWDFT single rank
run_test "Simple PWDFT Single Rank" \
    "./build_sycl/pwdft test_simple.nw" \
    "task completed"

# Test 6: PWDFT with Aurora GPU tiling (small)
run_test "PWDFT with Aurora GPU Tiling (Small)" \
    "mpiexec -n 2 --ppn 2 --host localhost --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close ./gpu_tile_compact.sh ./build_sycl/pwdft test_simple.nw" \
    "task completed"

# Test 7: Error handling - no GPU fallback
run_test "No GPU Fallback Test" \
    "unset ZE_AFFINITY_MASK; unset ONEAPI_DEVICE_SELECTOR; ./build_sycl/pwdft test_simple.nw" \
    "task completed"

# Test 8: Memory stress test
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
    "mpiexec -n 4 --ppn 4 --host localhost --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close ./gpu_tile_compact.sh ./build_sycl/pwdft test_stress.nw" \
    "task completed"

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