# PWDFT Band Module Energy Discrepancy - Debug Findings

**Date**: July 30, 2025  
**Issue**: Band module γ-point calculation produces incorrect positive energy while PSPW module produces correct negative energy  
**Status**: ❌ **UNRESOLVED** - Core issue identified but not fixed

## Executive Summary

The Band module's γ-point calculation for H₂ produces a **positive energy** (`1.8223036336e-01` Ry) while the PSPW module produces the **correct negative energy** (`-1.1674552938e+00` Ry). This represents a massive discrepancy of ~1.35 Ry, far exceeding the required tolerance of 1×10⁻⁶ Ry.

## Problem Analysis

### Energy Comparison

| Component | PSPW (Correct) | Band (Incorrect) | Difference |
|-----------|----------------|------------------|------------|
| **Total Energy** | `-1.1674552938e+00` Ry | `1.8223036336e-01` Ry | **1.35 Ry** |
| Kinetic | `1.0616519704e+00` Ry | `5.1356363414e-03` Ry | **1.06 Ry** |
| V_local | `-2.1314882222e+00` Ry | `7.3414576870e-04` Ry | **2.13 Ry** |
| V_nl | `-3.2196248326e-01` Ry | `-7.4099978233e-03` Ry | **0.31 Ry** |
| V_Coul | `1.5110402188e+00` Ry | `5.3760915223e-05` Ry | **1.51 Ry** |
| V_xc | `-8.7617983818e-01` Ry | `-1.7571306068e-01` Ry | **0.70 Ry** |
| Ion-ion | `1.5090652314e-01` Ry | `1.5090652314e-01` Ry | **0.00 Ry** |

### Key Observations

1. **Ion-ion energy is identical** - This confirms the geometry and pseudopotentials are the same
2. **All electronic energy components are dramatically different** - This indicates the issue is in wavefunction handling, not geometry
3. **Band calculation completes successfully** - No hanging or crashes, just incorrect energy values
4. **FFT implementation is not the root cause** - Complex FFT should work correctly for γ-point

## Technical Investigation

### FFT Implementation Analysis

**Attempted Fix**: Modified `batch_cfftx_tmpx_band` to use real FFTs for γ-point calculations
**Result**: ❌ **Failed** - Caused hanging during superposition generation
**Conclusion**: The FFT implementation is not the core issue

**Reverted to**: Complex FFT implementation (original behavior)
**Result**: ✅ **Band calculation completes** but produces incorrect energy

### Code Path Analysis

1. **PSPW Module**: Uses `task pspw energy` → `Pneb::gh_fftb` → Complex FFT
2. **Band Module**: Uses `task band energy` → `Cneb::gh_fftb` → Complex FFT

Both modules use similar FFT paths but produce dramatically different results.

### Wavefunction Analysis

**PSPW Output**:
```
number of electrons: spin up= 0.99962 down= 0.99962 (real space)
orbital energies: -3.7846918e-01 (-10.299eV)
```

**Band Output**:
```
number of electrons: spin up= 0.46720 down= 0.46720 (real space)
orbital energies: -4.4220812e-03 (-0.120eV)
```

**Critical Differences**:
- **Electron count**: PSPW shows ~1.0 electrons, Band shows ~0.47 electrons
- **Orbital energy**: PSPW shows -10.3 eV, Band shows -0.12 eV
- **Energy scale**: Band energies are ~100x smaller than PSPW

## Root Cause Hypothesis

The issue appears to be in **wavefunction normalization and energy scaling** rather than FFT implementation:

1. **Electron count discrepancy** suggests wavefunction normalization is incorrect
2. **Energy scale difference** suggests energy calculation scaling is wrong
3. **All electronic components affected** suggests systematic issue in energy evaluation

## Next Steps for Debug

### Priority 1: Wavefunction Analysis
- Compare wavefunction normalization between PSPW and Band modules
- Check if Band module properly handles γ-point wavefunction constraints
- Verify electron count calculation in Band module

### Priority 2: Energy Calculation Audit
- Trace energy calculation paths in both modules
- Compare scaling factors and normalization constants
- Check for missing energy terms in Band module

### Priority 3: γ-Point Specific Handling
- Investigate if Band module has γ-point detection logic
- Check if wavefunction constraints are properly applied for γ-point
- Verify that complex wavefunctions are handled correctly

## Files to Investigate

### Core Energy Calculation
- `Nwpw/pspw/lib/electron/Electron.cpp` - PSPW energy calculation
- `Nwpw/band/lib/cElectron/cElectron.cpp` - Band energy calculation
- `Nwpw/pspw/lib/electron/psi_H.cpp` - PSPW Hamiltonian application
- `Nwpw/band/lib/cElectron/cpsi_H.cpp` - Band Hamiltonian application

### Wavefunction Handling
- `Nwpw/pspw/lib/psi/psi.cpp` - PSPW wavefunction management
- `Nwpw/band/lib/cpsi/cpsi.cpp` - Band wavefunction management
- `Nwpw/nwpwlib/C3dB/Cneb.cpp` - Band wavefunction operations

### γ-Point Detection
- `Nwpw/nwpwlib/brillouin/Brillouin.cpp` - k-point handling
- Check for γ-point specific logic in Band module

## Test Cases

### Current Status
- ✅ PSPW H₂ energy test: `-1.1674552938e+00` Ry (correct)
- ❌ Band H₂ energy test: `1.8223036336e-01` Ry (incorrect)
- ✅ Both tests complete without hanging

### Required Fix
- Band H₂ energy must match PSPW within 1×10⁻⁶ Ry
- Energy should be negative (stable H₂ molecule)
- All energy components should be consistent

## Conclusion

The Band module's γ-point energy calculation has a fundamental issue in wavefunction handling and energy evaluation. The problem is **not** in the FFT implementation but rather in how the Band module processes γ-point calculations compared to the PSPW module. A systematic audit of wavefunction normalization and energy calculation algorithms is required to identify and fix the root cause. 