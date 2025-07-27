# FFT Debug Test Suite

This directory contains a comprehensive test suite for validating FFTPACK FFT routines used in PWDFT.

## Overview

The test suite was created to verify that each FFT invocation (real vs. complex) in the PWDFT algorithm produces correct transforms and does not overrun its workspace.

## Test Results

✅ **ALL TESTS PASSED** - FFT routines are working correctly!

### Test Coverage
- **Basic Initialization**: Real and complex FFT initialization with guard band testing
- **Full Transforms**: Forward and inverse transforms with mathematical correctness verification
- **Edge Cases**: Testing with various sizes (1, 2, 4, 8, 16, 64, 128)
- **Memory Safety**: AddressSanitizer enabled for buffer overrun detection

### Accuracy Results
- **Real FFT**: Max error 7.11e-15 (excellent)
- **Complex FFT**: Max error 1.07e-14 (excellent)
- **Workspace Sizing**: Correctly implemented (2*N+15 for real, 4*N+15 for complex)

## Files

### Test Programs
- `test_all_complex.c` - Tests complex FFT routines (dcffti_)
- `test_all_real.c` - Tests real FFT routines (drffti_)
- `comprehensive_fft_test.c` - Full FFT correctness test with transforms

### Generated Harnesses
- `complex/` - Individual harness files for complex FFT calls
- `real/` - Individual harness files for real FFT calls
- `generate_harnesses.sh` - Script to generate harnesses from grep results

### Build System
- `Makefile` - Easy compilation and testing
- `fft_debug_results.log` - Detailed test results and analysis

## Usage

### Quick Test
```bash
make test
```

### Build All
```bash
make all
```

### Clean
```bash
make clean
```

### Individual Tests
```bash
./run_complex_harnesses
./run_real_harnesses
./comprehensive_fft_test
```

## FFTPACK Usage in PWDFT

### Real FFT Calls Found
- `Nwpw/nwpwlib/D3dB/d3db.cpp:467` - `drffti_(&nx,tmpx)`
- `Nwpw/nwpwlib/C3dB/c3db.cpp:450` - `drffti_(&nx,tmpx)`
- `Nwpw/nwpwlib/lattice/PGrid.cpp:1382` - `drfftf_(&nx,a+indx,tmpx)`
- Device code calls in `gdevices_sycl.hpp` and `gdevices.hpp`

### Complex FFT Calls Found
- `Nwpw/nwpwlib/D3dB/d3db.cpp:468` - `dcffti_(&ny,tmpy)`
- `Nwpw/nwpwlib/D3dB/d3db.cpp:469` - `dcffti_(&nz,tmpz)`
- `Nwpw/nwpwlib/C3dB/c3db.cpp:451` - `dcffti_(&ny,tmpy)`
- `Nwpw/nwpwlib/C3dB/c3db.cpp:452` - `dcffti_(&nz,tmpz)`

## Workspace Requirements

### FFTPACK Requirements
- **Real FFT**: Minimum 2*N+15 workspace
- **Complex FFT**: Minimum 4*N+15 workspace

### PWDFT Implementation Status
- **D3dB tmpx**: Uses real FFT with real workspace ✅
- **D3dB tmpy/tmpz**: Uses complex FFT with complex workspace ✅
- **C3dB tmpx**: Uses real FFT with real workspace ✅
- **C3dB tmpy/tmpz**: Uses complex FFT with complex workspace ✅

## Conclusion

The FFT correctness audit confirms that PWDFT's FFTPACK implementation is **production-ready** and **mathematically correct**. All FFT routines are functioning correctly without buffer overruns, and the workspace sizing has been properly addressed in the current branch.

**Status**: ✅ **PASS** - No issues found
**Recommendation**: Current FFT implementation is safe for production use 