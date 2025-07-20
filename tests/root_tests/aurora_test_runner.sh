#!/bin/bash

# Aurora Test Runner - Run this on Aurora to generate test outputs
# This script runs comprehensive Aurora tests and generates detailed outputs

set -e

echo "=========================================="
echo "Aurora PWDFT Test Suite Runner"
echo "=========================================="
echo "Date: $(date)"
echo "Commit: $(git rev-parse HEAD 2>/dev/null || echo 'unknown')"
echo ""

# Create output directory
mkdir -p aurora_test_outputs
cd aurora_test_outputs

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
PASSED=0
FAILED=0

# Function to run test and save output
run_test() {
    local test_name="$1"
    local test_command="$2"
    local output_file="$3"
    
    echo -e "${YELLOW}Running: $test_name${NC}"
    echo "Command: $test_command"
    echo "Output: $output_file"
    echo "---"
    
    if eval "$test_command" > "$output_file" 2>&1; then
        echo -e "${GREEN}✓ PASS: $test_name${NC}"
        ((PASSED++))
    else
        echo -e "${RED}✗ FAIL: $test_name${NC}"
        ((FAILED++))
    fi
    echo ""
}

# Environment check
echo "=== Environment Check ===" > environment_check.txt
echo "Date: $(date)" >> environment_check.txt
echo "Hostname: $(hostname)" >> environment_check.txt
echo "User: $(whoami)" >> environment_check.txt
echo "" >> environment_check.txt

echo "=== GPU Detection ===" >> environment_check.txt
if command -v nvidia-smi >/dev/null 2>&1; then
    nvidia-smi >> environment_check.txt 2>&1 || echo "nvidia-smi failed" >> environment_check.txt
else
    echo "nvidia-smi not available" >> environment_check.txt
fi

echo "=== Intel GPU Detection ===" >> environment_check.txt
if [ -f /usr/bin/udevadm ]; then
    /usr/bin/udevadm info /sys/module/i915/drivers/pci:i915/* 2>&1 | grep -c "P: /devices" >> environment_check.txt || echo "0" >> environment_check.txt
else
    echo "udevadm not available" >> environment_check.txt
fi

echo "=== SYCL Devices ===" >> environment_check.txt
if command -v sycl-ls >/dev/null 2>&1; then
    sycl-ls >> environment_check.txt 2>&1 || echo "sycl-ls failed" >> environment_check.txt
else
    echo "sycl-ls not available" >> environment_check.txt
fi

echo "=== MPI Version ===" >> environment_check.txt
if command -v mpirun >/dev/null 2>&1; then
    mpirun --version >> environment_check.txt 2>&1 || echo "mpirun --version failed" >> environment_check.txt
fi

echo "=== Environment Variables ===" >> environment_check.txt
env | grep -E "(ZE_|ONEAPI_|SYCL_|MPI_|OMP_)" | sort >> environment_check.txt 2>&1 || echo "No relevant environment variables found" >> environment_check.txt

echo "Environment check completed: environment_check.txt"
echo ""

# Test 1: Basic PWDFT single rank
run_test "Basic PWDFT Single Rank" \
    "../build/pwdft ../test_simple.nw" \
    "test_1_basic_single.out"

# Test 2: PWDFT with basic MPI (2 ranks)
run_test "PWDFT MPI 2 Ranks" \
    "mpirun -np 2 ../build/pwdft ../test_simple.nw" \
    "test_2_mpi_2ranks.out"

# Test 3: PWDFT with GPU tiling (2 ranks)
run_test "PWDFT with GPU Tiling (2 ranks)" \
    "mpirun -np 2 ../gpu_tile_compact.sh ../build/pwdft ../test_simple.nw" \
    "test_3_gpu_tiling_2ranks.out"

# Test 4: PWDFT with GPU tiling (4 ranks)
run_test "PWDFT with GPU Tiling (4 ranks)" \
    "mpirun -np 4 ../gpu_tile_compact.sh ../build/pwdft ../test_simple.nw" \
    "test_4_gpu_tiling_4ranks.out"

# Test 5: PWDFT with GPU tiling (6 ranks)
run_test "PWDFT with GPU Tiling (6 ranks)" \
    "mpirun -np 6 ../gpu_tile_compact.sh ../build/pwdft ../test_simple.nw" \
    "test_5_gpu_tiling_6ranks.out"

# Test 6: Stress test with larger system
echo "Creating stress test input..."
cat > ../test_stress.nw << EOF
title "Aurora Stress Test"

geometry
  H 0.0 0.0 0.0
  H 0.74 0.0 0.0
  O 0.0 1.0 0.0
  H 0.0 1.5 0.0
  H 2.0 0.0 0.0
  H 2.74 0.0 0.0
end

nwpw
  simulation_cell
    boundary_conditions periodic
    unita 20.0 0.0 0.0
             0.0 20.0 0.0
             0.0 0.0 20.0
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

run_test "Stress Test (4 ranks)" \
    "mpirun -np 4 ../gpu_tile_compact.sh ../build/pwdft ../test_stress.nw" \
    "test_6_stress_4ranks.out"

# Test 7: Long-running test
echo "Creating long-running test input..."
cat > ../test_long.nw << EOF
title "Aurora Long-Running Test"

geometry
  H 0.0 0.0 0.0
  H 0.74 0.0 0.0
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
  lmbfgs_size 3
  loop 10 50
  tolerances 1.0e-8 1.0e-8 1.0e-6
  print medium
end

task pspw energy
EOF

run_test "Long-Running Test (2 ranks)" \
    "mpirun -np 2 ../gpu_tile_compact.sh ../build/pwdft ../test_long.nw" \
    "test_7_long_2ranks.out"

# Test 8: Performance comparison
echo "=== Performance Comparison ===" > performance_comparison.txt
echo "Date: $(date)" >> performance_comparison.txt
echo "" >> performance_comparison.txt

echo "CPU-only test..." >> performance_comparison.txt
time (../build/pwdft ../test_simple.nw > cpu_only.out 2>&1) 2>> performance_comparison.txt

echo "GPU test..." >> performance_comparison.txt
time (mpirun -np 2 ../gpu_tile_compact.sh ../build/pwdft ../test_simple.nw > gpu_test.out 2>&1) 2>> performance_comparison.txt

echo "Performance comparison completed: performance_comparison.txt"
echo ""

# Generate summary report
echo "=== Aurora Test Summary ===" > aurora_test_results.txt
echo "Date: $(date)" >> aurora_test_results.txt
echo "Commit: $(git rev-parse HEAD 2>/dev/null || echo 'unknown')" >> aurora_test_results.txt
echo "Hostname: $(hostname)" >> aurora_test_results.txt
echo "" >> aurora_test_results.txt

echo "Test Results:" >> aurora_test_results.txt
echo "1. Basic Single Rank: [$(grep -q "task completed" test_1_basic_single.out && echo "PASS" || echo "FAIL")]" >> aurora_test_results.txt
echo "2. MPI 2 Ranks: [$(grep -q "task completed" test_2_mpi_2ranks.out && echo "PASS" || echo "FAIL")]" >> aurora_test_results.txt
echo "3. GPU Tiling 2 Ranks: [$(grep -q "task completed" test_3_gpu_tiling_2ranks.out && echo "PASS" || echo "FAIL")]" >> aurora_test_results.txt
echo "4. GPU Tiling 4 Ranks: [$(grep -q "task completed" test_4_gpu_tiling_4ranks.out && echo "PASS" || echo "FAIL")]" >> aurora_test_results.txt
echo "5. GPU Tiling 6 Ranks: [$(grep -q "task completed" test_5_gpu_tiling_6ranks.out && echo "PASS" || echo "FAIL")]" >> aurora_test_results.txt
echo "6. Stress Test: [$(grep -q "task completed" test_6_stress_4ranks.out && echo "PASS" || echo "FAIL")]" >> aurora_test_results.txt
echo "7. Long-Running Test: [$(grep -q "task completed" test_7_long_2ranks.out && echo "PASS" || echo "FAIL")]" >> aurora_test_results.txt
echo "" >> aurora_test_results.txt

echo "Summary:" >> aurora_test_results.txt
echo "Passed: $PASSED" >> aurora_test_results.txt
echo "Failed: $FAILED" >> aurora_test_results.txt
echo "Total: $((PASSED + FAILED))" >> aurora_test_results.txt

# Final summary
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo "Passed: $PASSED"
echo "Failed: $FAILED"
echo "Total: $((PASSED + FAILED))"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed! Aurora integration is working correctly.${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed. Check the output files for details.${NC}"
    exit 1
fi 