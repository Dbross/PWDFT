# Aurora Testing Plan for SYCL Device Selection Fix

## Pre-Testing Setup

### 1. Environment Verification
```bash
# Check Aurora environment
echo "=== Aurora Environment Check ==="
env | grep -E "(ZE_|ONEAPI_|SYCL_|MPI_)" | sort

# Check GPU availability
echo "=== GPU Detection ==="
/usr/bin/udevadm info /sys/module/i915/drivers/pci:i915/* |& grep -v Unknown | grep -c "P: /devices"

# Check SYCL devices
echo "=== SYCL Device List ==="
sycl-ls

# Check Intel oneAPI installation
echo "=== Intel oneAPI Check ==="
which dpcpp
dpcpp --version
```

## Test Suite 1: Basic SYCL Functionality

### Test 1.1: Simple SYCL Device Test
```bash
echo "=== Test 1.1: Basic SYCL Device Selection ==="
./test_sycl_device
```
**Expected Output**: Should show available devices and successful device selection

### Test 1.2: Single Rank SYCL Test
```bash
echo "=== Test 1.2: Single Rank SYCL Test ==="
mpiexec -n 1 --ppn 1 --host localhost --depth=1 --cpu-bind depth \
  --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
  ./test_sycl_device
```
**Expected Output**: Should work without GPU tiling, fallback to CPU/default

## Test Suite 2: Aurora GPU Tiling Integration

### Test 2.1: Single Rank with GPU Tiling
```bash
echo "=== Test 2.1: Single Rank with Aurora GPU Tiling ==="
mpiexec -n 1 --ppn 1 --host localhost --depth=1 --cpu-bind depth \
  --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
  ./gpu_tile_compact.sh ./test_sycl_device
```
**Expected Output**: Should show Aurora GPU tile assignment and successful device selection

### Test 2.2: Multiple Ranks with GPU Tiling
```bash
echo "=== Test 2.2: Multiple Ranks with Aurora GPU Tiling ==="
mpiexec -n 4 --ppn 4 --host localhost --depth=1 --cpu-bind depth \
  --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
  ./gpu_tile_compact.sh ./test_sycl_device
```
**Expected Output**: Each rank should get different GPU tiles (0.0, 0.1, 1.0, 1.1)

### Test 2.3: Full Node GPU Tiling
```bash
echo "=== Test 2.3: Full Node GPU Tiling ==="
mpiexec -n 12 --ppn 12 --host localhost --depth=1 --cpu-bind depth \
  --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
  ./gpu_tile_compact.sh ./test_sycl_device
```
**Expected Output**: All 12 ranks should get GPU tiles in round-robin fashion

## Test Suite 3: PWDFT Integration Tests

### Test 3.1: Simple PWDFT Single Rank
```bash
echo "=== Test 3.1: Simple PWDFT Single Rank ==="
./build_sycl/pwdft test_simple.nw
```
**Expected Output**: Should complete without segmentation fault, show device selection info

### Test 3.2: PWDFT with Aurora GPU Tiling (Small)
```bash
echo "=== Test 3.2: PWDFT with Aurora GPU Tiling (Small) ==="
mpiexec -n 2 --ppn 2 --host localhost --depth=1 --cpu-bind depth \
  --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
  ./gpu_tile_compact.sh ./build_sycl/pwdft test_simple.nw
```
**Expected Output**: Should complete SCF cycles without segmentation fault

### Test 3.3: PWDFT with Aurora GPU Tiling (Medium)
```bash
echo "=== Test 3.3: PWDFT with Aurora GPU Tiling (Medium) ==="
mpiexec -n 6 --ppn 6 --host localhost --depth=1 --cpu-bind depth \
  --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
  ./gpu_tile_compact.sh ./build_sycl/pwdft test_simple.nw
```
**Expected Output**: Should complete successfully with multiple ranks

## Test Suite 4: Stress Tests

### Test 4.1: Memory Stress Test
```bash
echo "=== Test 4.1: Memory Stress Test ==="
# Create larger test case
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

mpiexec -n 4 --ppn 4 --host localhost --depth=1 --cpu-bind depth \
  --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
  ./gpu_tile_compact.sh ./build_sycl/pwdft test_stress.nw
```

### Test 4.2: Long-Running Test
```bash
echo "=== Test 4.2: Long-Running Test ==="
# Create test with more iterations
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

mpiexec -n 2 --ppn 2 --host localhost --depth=1 --cpu-bind depth \
  --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
  ./gpu_tile_compact.sh ./build_sycl/pwdft test_long.nw
```

## Test Suite 5: Error Handling Tests

### Test 5.1: No GPU Available Test
```bash
echo "=== Test 5.1: No GPU Available Test ==="
# Temporarily unset GPU environment
unset ZE_AFFINITY_MASK
unset ONEAPI_DEVICE_SELECTOR
./build_sycl/pwdft test_simple.nw
```
**Expected Output**: Should fallback to CPU and complete successfully

### Test 5.2: Invalid GPU Assignment Test
```bash
echo "=== Test 5.2: Invalid GPU Assignment Test ==="
# Set invalid GPU assignment
export ZE_AFFINITY_MASK="99.99"
mpiexec -n 1 --ppn 1 --host localhost --depth=1 --cpu-bind depth \
  --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
  ./gpu_tile_compact.sh ./build_sycl/pwdft test_simple.nw
```
**Expected Output**: Should handle gracefully and fallback to CPU

## Test Suite 6: Performance Validation

### Test 6.1: Performance Comparison
```bash
echo "=== Test 6.1: Performance Comparison ==="
# Test with CPU only
unset ZE_AFFINITY_MASK
unset ONEAPI_DEVICE_SELECTOR
time ./build_sycl/pwdft test_simple.nw > cpu_output.log 2>&1

# Test with GPU
export ZE_AFFINITY_MASK="0.0"
time mpiexec -n 1 --ppn 1 --host localhost --depth=1 --cpu-bind depth \
  --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
  ./gpu_tile_compact.sh ./build_sycl/pwdft test_simple.nw > gpu_output.log 2>&1

echo "CPU time:"
grep "real" cpu_output.log
echo "GPU time:"
grep "real" gpu_output.log
```

## Success Criteria

### ✅ **Test Passes If:**
1. **No Segmentation Faults**: All tests complete without crashes
2. **Device Selection Works**: Proper device selection messages appear
3. **Aurora Integration**: GPU tiling works correctly with multiple ranks
4. **Graceful Fallback**: CPU fallback works when GPU unavailable
5. **SCF Convergence**: PWDFT calculations complete successfully
6. **Performance**: GPU runs are at least as fast as CPU runs

### ❌ **Test Fails If:**
1. **Segmentation Fault**: Any test crashes with segfault
2. **Device Selection Error**: "No device of requested type" errors
3. **MPI Issues**: Ranks fail to communicate or hang
4. **Memory Issues**: Out of memory or allocation failures
5. **SCF Divergence**: PWDFT calculations fail to converge

## Debugging Commands

### If Tests Fail:
```bash
# Check device availability
sycl-ls

# Check environment variables
env | grep -E "(ZE_|ONEAPI_|SYCL_)"

# Check GPU status
nvidia-smi  # if available
lspci | grep -i vga

# Check MPI setup
mpirun --version
which mpirun

# Check build configuration
./build_sycl/pwdft --help
```

### Memory Debugging:
```bash
# Run with memory debugging
export SYCL_DEVICE_FILTER="level_zero:gpu"
export ZE_ENABLE_PCI_ID_DEVICE_ORDER=1
export SYCL_PI_LEVEL_ZERO_USE_IMMEDIATE_COMMANDLISTS=1

# Check for memory leaks
valgrind --tool=memcheck --leak-check=full ./build_sycl/pwdft test_simple.nw
```

## Test Execution Order

1. **Start with Test Suite 1** (Basic functionality)
2. **Proceed to Test Suite 2** (Aurora integration)
3. **Run Test Suite 3** (PWDFT integration)
4. **Execute Test Suite 4** (Stress tests)
5. **Validate with Test Suite 5** (Error handling)
6. **Finish with Test Suite 6** (Performance)

## Reporting

After running all tests, create a summary report:

```bash
echo "=== Aurora Test Summary ===" > aurora_test_results.txt
echo "Date: $(date)" >> aurora_test_results.txt
echo "Commit: $(git rev-parse HEAD)" >> aurora_test_results.txt
echo "" >> aurora_test_results.txt

# Add test results
echo "Test Results:" >> aurora_test_results.txt
echo "1.1 Basic SYCL: [PASS/FAIL]" >> aurora_test_results.txt
echo "1.2 Single Rank: [PASS/FAIL]" >> aurora_test_results.txt
echo "2.1 GPU Tiling Single: [PASS/FAIL]" >> aurora_test_results.txt
echo "2.2 GPU Tiling Multi: [PASS/FAIL]" >> aurora_test_results.txt
echo "3.1 PWDFT Single: [PASS/FAIL]" >> aurora_test_results.txt
echo "3.2 PWDFT GPU Small: [PASS/FAIL]" >> aurora_test_results.txt
echo "3.3 PWDFT GPU Medium: [PASS/FAIL]" >> aurora_test_results.txt
echo "4.1 Memory Stress: [PASS/FAIL]" >> aurora_test_results.txt
echo "4.2 Long Running: [PASS/FAIL]" >> aurora_test_results.txt
echo "5.1 No GPU Fallback: [PASS/FAIL]" >> aurora_test_results.txt
echo "5.2 Invalid GPU: [PASS/FAIL]" >> aurora_test_results.txt
echo "6.1 Performance: [PASS/FAIL]" >> aurora_test_results.txt
```

This comprehensive test plan will validate that the SYCL device selection fix works correctly on Aurora and handles all edge cases properly. 