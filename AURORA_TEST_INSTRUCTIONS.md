# Aurora PWDFT Test Instructions

This document provides step-by-step instructions for running the Aurora PWDFT test suite on the Aurora supercomputer.

## Files to Push to Aurora

Push these files to your Aurora workspace:

### Core Test Files
- `aurora_test_runner.sh` - Main test runner script
- `build_aurora_tests.sh` - SYCL test build script
- `gpu_tile_compact.sh` - GPU tile assignment script
- `test_sycl_device.cpp` - SYCL device test program
- `aurora_config.sh` - Aurora configuration
- `test_simple.nw` - Simple PWDFT input file

### Build Files (if needed)
- `build/pwdft` - PWDFT executable (or build it on Aurora)
- Pseudopotential files (H.psp, O.psp, etc.)

## Step-by-Step Instructions

### 1. Prepare Aurora Environment

```bash
# Load required modules (adjust based on your Aurora setup)
module load intel-oneapi
module load intel-mpi
module load intel-mkl

# Or if using different module names:
# module load oneapi
# module load intel/2023.0.0
```

### 2. Build SYCL Test Program

```bash
# Make scripts executable
chmod +x build_aurora_tests.sh
chmod +x aurora_test_runner.sh
chmod +x gpu_tile_compact.sh

# Build the SYCL test program
./build_aurora_tests.sh
```

**Expected Output:**
```
=== Building Aurora SYCL Test Programs ===
Found Intel oneAPI at: /opt/intel/oneapi
Setting up Intel oneAPI environment...
✓ Found dpcpp compiler: /opt/intel/oneapi/compiler/latest/linux/bin/dpcpp

Building test_sycl_device...
✓ test_sycl_device built successfully

Testing built program...
Testing SYCL Device Selection...
✓ GPU available: Intel(R) Data Center GPU Max 1550
✓ Selected GPU: Intel(R) Data Center GPU Max 1550
✓ Queue created successfully
✓ Kernel executed successfully, result: 42
✓ test_sycl_device runs successfully

=== Build completed successfully ===
```

### 3. Verify PWDFT Build

```bash
# Check if PWDFT is built
ls -la build/pwdft

# If not built, build it:
mkdir -p build
cd build
cmake .. -DNWPW_SYCL=ON -DCMAKE_BUILD_TYPE=Debug
make -j4
cd ..
```

### 4. Run Aurora Test Suite

```bash
# Run the comprehensive test suite
./aurora_test_runner.sh
```

**Expected Output:**
```
==========================================
Aurora PWDFT Test Suite Runner
==========================================
Date: Sun Jul 20 10:00:00 CDT 2025
Commit: 441860ceb6729e41c64d4b81cfaac3a53c408d00

Environment check completed: environment_check.txt

Running: Basic PWDFT Single Rank
Command: ../build/pwdft ../test_simple.nw
Output: test_1_basic_single.out
---
✓ PASS: Basic PWDFT Single Rank

Running: PWDFT MPI 2 Ranks
Command: mpirun -np 2 ../build/pwdft ../test_simple.nw
Output: test_2_mpi_2ranks.out
---
✓ PASS: PWDFT MPI 2 Ranks

[... more tests ...]

==========================================
Test Summary
==========================================
Passed: 7
Failed: 0
Total: 7

All tests passed! Aurora integration is working correctly.
```

## Generated Output Files

After running the tests, you'll find these files in the `aurora_test_outputs/` directory:

### Test Results
- `aurora_test_results.txt` - Summary of all test results
- `environment_check.txt` - System environment information
- `performance_comparison.txt` - Performance timing data

### Individual Test Outputs
- `test_1_basic_single.out` - Single rank PWDFT test
- `test_2_mpi_2ranks.out` - MPI 2-rank test
- `test_3_gpu_tiling_2ranks.out` - GPU tiling 2-rank test
- `test_4_gpu_tiling_4ranks.out` - GPU tiling 4-rank test
- `test_5_gpu_tiling_6ranks.out` - GPU tiling 6-rank test
- `test_6_stress_4ranks.out` - Stress test
- `test_7_long_2ranks.out` - Long-running test

### Performance Data
- `cpu_only.out` - CPU-only performance test
- `gpu_test.out` - GPU performance test

## Troubleshooting

### Common Issues

1. **Intel oneAPI not found**
   ```bash
   # Try loading modules
   module load intel-oneapi
   module load oneapi
   
   # Or set environment manually
   export ONEAPI_ROOT="/opt/intel/oneapi"
   source $ONEAPI_ROOT/setvars.sh
   ```

2. **MPI issues**
   ```bash
   # Check MPI installation
   mpirun --version
   
   # Try different MPI implementations
   # OpenMPI: mpirun -np 2
   # Intel MPI: mpiexec -n 2
   ```

3. **GPU not detected**
   ```bash
   # Check GPU availability
   sycl-ls
   
   # Check environment variables
   env | grep -E "(ZE_|ONEAPI_|SYCL_)"
   ```

4. **PWDFT build issues**
   ```bash
   # Check dependencies
   cmake .. -DNWPW_SYCL=ON -DCMAKE_BUILD_TYPE=Debug
   
   # Check for missing libraries
   ldd build/pwdft
   ```

### Debug Mode

To run with more verbose output:

```bash
# Set debug environment variables
export DEBUG_MODE=true
export VERBOSE_OUTPUT=true

# Run tests with debug output
./aurora_test_runner.sh
```

## Expected Results

### Successful Test Run
- All 7 tests should pass
- GPU tiling should work correctly
- Performance should be reasonable
- No segmentation faults

### Key Success Indicators
1. **Device Selection**: GPU should be detected and selected
2. **MPI Communication**: Multi-rank tests should complete
3. **GPU Tiling**: Each rank should get different GPU tiles
4. **SCF Convergence**: PWDFT calculations should converge
5. **Performance**: GPU runs should be at least as fast as CPU runs

## Reporting Results

After running the tests, provide:

1. **Test Summary**: Copy the contents of `aurora_test_results.txt`
2. **Environment Info**: Copy the contents of `environment_check.txt`
3. **Performance Data**: Copy the contents of `performance_comparison.txt`
4. **Any Error Logs**: If tests failed, provide the relevant `.out` files

## Example Successful Output

```
=== Aurora Test Summary ===
Date: Sun Jul 20 10:00:00 CDT 2025
Commit: 441860ceb6729e41c64d4b81cfaac3a53c408d00
Hostname: aurora-node-001

Test Results:
1. Basic Single Rank: [PASS]
2. MPI 2 Ranks: [PASS]
3. GPU Tiling 2 Ranks: [PASS]
4. GPU Tiling 4 Ranks: [PASS]
5. GPU Tiling 6 Ranks: [PASS]
6. Stress Test: [PASS]
7. Long-Running Test: [PASS]

Summary:
Passed: 7
Failed: 0
Total: 7
```

This indicates that the Aurora integration is working correctly and the SYCL device selection fix is functioning as expected. 