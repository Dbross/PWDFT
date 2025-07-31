# PWDFT Band Module Energy Discrepancy - Debug Findings

**Date**: July 30, 2025  
**Issue**: Band module γ-point calculation produces incorrect positive energy while PSPW module produces correct negative energy  
**Status**: 🔄 **IN PROGRESS** - Density normalization fixed, energy scaling issue remains

## Executive Summary

The Band module's γ-point calculation for H₂ produces an **incorrect energy** (`-70.35125315449193` Ry) while the PSPW module produces the **correct negative energy** (`-1.1674552938e+00` Ry). This represents a massive discrepancy of ~60x scaling, far exceeding the required tolerance of 1×10⁻⁶ Ry.

### Recent Progress (July 30, 2025)
✅ **Density normalization fixed**: Integrated density now correctly shows 2.0 electrons for H₂  
✅ **Occupation scaling fixed**: Occupation values now correctly set to 2.0 for singlet state  
✅ **Normalization loop working**: Wavefunction normalization converges properly  
❌ **Energy scaling issue remains**: Energy is still ~60x larger than expected

## Problem Analysis

### Energy Comparison

| Component | PSPW (Correct) | Band (Incorrect) | Difference |
|-----------|----------------|------------------|------------|
| **Total Energy** | `-1.1674552938e+00` Ry | `-70.35125315449193` Ry | **~60x scaling** |
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
number of electrons: spin up= 1.00000 down= 1.00000 (real space) [FIXED]
orbital energies: -15.586 Ry (-212.0 eV) [STILL WRONG]
```

**Critical Differences**:
- **Electron count**: ✅ **FIXED** - Both PSPW and Band now show ~1.0 electrons
- **Orbital energy**: ❌ **STILL WRONG** - Band shows -212 eV vs PSPW -10.3 eV
- **Energy scale**: ❌ **STILL WRONG** - Band energies are ~60x larger than PSPW

## Root Cause Hypothesis

The issue appears to be in **energy calculation scaling** rather than FFT implementation or wavefunction normalization:

1. ✅ **Electron count discrepancy FIXED** - Wavefunction normalization is now correct
2. ❌ **Energy scale difference REMAINS** - Energy calculation scaling is still wrong
3. **All electronic components affected** suggests systematic issue in energy evaluation

### Current Focus
The energy scaling issue appears to be in the **Hamiltonian application or energy calculation** rather than wavefunction handling. The ~60x scaling factor suggests a systematic error in how energies are computed or scaled in the Band module compared to PSPW.

## Next Steps for Debug

### Priority 1: Energy Calculation Audit ✅ **COMPLETED**
- ✅ Compare energy calculation paths in both modules
- ✅ Compare scaling factors and normalization constants
- ✅ Check for missing energy terms in Band module

### Priority 2: Hamiltonian Application Analysis 🔄 **IN PROGRESS**
- Compare Hamiltonian application between PSPW and Band modules
- Check for differences in `ggw_sym_Multiply` vs `ggm_sym_Multiply`
- Verify energy scaling in `w_trace_occ` vs `m_trace_occ`

### Priority 3: γ-Point Specific Handling
- Investigate if Band module has γ-point detection logic
- Check if wavefunction constraints are properly applied for γ-point
- Verify that complex wavefunctions are handled correctly

### Priority 4: Systematic Energy Scaling Investigation
- The ~60x scaling factor suggests a systematic error
- Check for differences in energy units or scaling constants
- Compare the `dv` (volume element) scaling between modules

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

## 2025-07-31: Residual Energy Scaling Error — Root Cause & Next Steps

### Summary of Findings

- **Symptom:** Band module’s γ-point H₂ test yields total energy ≈ –70.35 Ry (should be ≈ –1.167455 Ry, matching PSPW).
- **Electron count:** Output electron count is ≈ 0.27 per spin (should be 1.0 per spin for H₂).
- **Energy components:** Kinetic and orbital energies are ≈ 60× too large in magnitude.
- **Hamiltonian and kinetic routines:** Confirmed sign and volume scaling match PSPW after recent fixes.
- **Density normalization:**  
  - The density is generated with a `scal2 = 1/omega` factor (i.e., already normalized per unit cell volume).
  - The integrated electron count is computed as  
    `sum += dn[i] * omega / (nx * ny * nz)`  
    which **reapplies the cell volume normalization**, leading to a ~60× undercount of electrons and overestimation of energy.
- **Conclusion:** The Band module is double-normalizing by the cell volume in its density integration, causing the energy scaling error.

### Physically Correct Fix

- **Integration of density:**  
  - The electron count should be computed as a simple sum over the grid:  
    `electron_count = sum_i dn[i]`  
    (since `dn[i]` is already in units of electrons per cell, per grid point).
  - Remove the extra `* omega / (nx * ny * nz)` factor from all density integrations and normalization checks.
- **Normalization loop:**  
  - Ensure the normalization in `Solid.cpp` and related routines is based on the correct integrated density, without redundant volume factors.

### Next Steps

- Apply the normalization fix in the most physically intuitive and least computationally obstructive way:  
  - Integrate the density as a sum over grid points (no extra volume factors).
  - Normalize wavefunctions so that the integrated density matches the expected electron count.
- Rebuild and rerun the Band H₂ test to confirm energy and electron count match PSPW within tolerance. 