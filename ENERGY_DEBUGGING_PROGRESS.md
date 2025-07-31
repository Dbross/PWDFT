# Energy Sign & Magnitude Debugging Progress Report

## Executive Summary

**Problem:** Band total energy was +39.95 Ry vs PSPW -1.17 Ry (34.2× magnitude difference, wrong sign)

**Progress:** 
- ✅ **Sign Issue FIXED:** Applied missing sign flip in Band energy calculation
- ✅ **Magnitude Improved:** Reduced from +39.95 Ry to +0.18 Ry (222× improvement)
- ❌ **Density Issue Found:** Band shows 12.63 electrons vs PSPW 1.0 electrons per spin
- ❌ **Hartree Energy Wrong:** Near zero due to density normalization problem

**Current Status:** Band energy +0.18 Ry vs PSPW -1.17 Ry (still needs density fix)

## Detailed Analysis

### 1. Initial Problem Identification

**Original Energy Values:**
- **Band Total Energy:** +39.9488 Ry (positive, wrong sign)
- **PSPW Total Energy:** -1.1675 Ry (negative, correct sign)
- **Ratio:** 34.2× magnitude difference with wrong sign

### 2. Root Cause Analysis

#### 2.1 Sign Issue - FIXED ✅

**Problem:** Missing sign flip in Band energy calculation

**PSPW (`inner_loop.cpp` line 254):**
```cpp
mygrid->ggm_sym_Multiply(psi1, Hpsi, hml);
mygrid->m_scal(-1.0, hml);  // ← ACTIVE SIGN FLIP
eorbit = mygrid->m_trace(hml);
```

**Band (`band_inner_loop.cpp` line 401):**
```cpp
mygrid->ggw_sym_Multiply(psi1, Hpsi, hml);
// mygrid->w_scal(-1.0, hml);  // ← COMMENTED OUT SIGN FLIP
eorbit = mygrid->w_trace(hml);
```

**Fix Applied:**
```cpp
// UNCOMMENTED the sign flip
mygrid->w_scal(-1.0, hml);  // Apply sign flip to match PSPW behavior
```

**Result:** Orbital energy now negative (correct sign)

#### 2.2 Matrix Multiplication Differences

**Key Differences Between Band and PSPW:**

| Aspect | Band (`ggw_sym_Multiply`) | PSPW (`ggm_sym_Multiply`) |
|--------|---------------------------|---------------------------|
| **Scaling Factor** | `rone[2] = {1.0,0.0}` | `rtwo = 2.0` |
| **Matrix Type** | Complex | Real |
| **Indexing** | `2*(i+i*ne[ms])` | `i+i*ne[ms]` |
| **Symmetrization** | Complex conjugate | Real symmetry |

**Impact:** Contributes to magnitude differences but not the primary issue.

#### 2.3 Density Normalization Issue - CURRENT PROBLEM ❌

**Critical Finding:** Band density calculation is wrong

**Electron Count Comparison:**
- **PSPW (Correct):** 0.99962 electrons per spin (≈1.0)
- **Band (Wrong):** 12.63 electrons per spin (12.6× too high!)

**Energy Component Impact:**
- **Hartree Energy:** Band +2.73e-5 Ry vs PSPW +0.7556 Ry
- **XC Energy:** Band -0.1358 Ry vs PSPW -0.6821 Ry
- **Total Energy:** Band +0.18 Ry vs PSPW -1.17 Ry

### 3. Code Changes Made

#### 3.1 Sign Flip Fix
**File:** `PWDFT/Nwpw/band/cpsd/band_inner_loop.cpp`
**Line:** 402
**Change:** Uncommented `mygrid->w_scal(-1.0, hml);`

```diff
- // mygrid->w_scal(-1.0, hml);  // Comment out to match PSPW behavior
+ mygrid->w_scal(-1.0, hml);  // Apply sign flip to match PSPW behavior
```

### 4. Test Results

#### 4.1 Before Fix
```
Band Total Energy: +39.9488 Ry
Band Orbital Energy: +39.7957 Ry (wrong sign)
Band Hartree Energy: +1.26e-5 Ry
```

#### 4.2 After Sign Flip Fix
```
Band Total Energy: +0.1785 Ry (222× improvement!)
Band Orbital Energy: -0.0127 Ry (correct sign!)
Band Hartree Energy: +2.73e-5 Ry (still wrong)
```

### 5. Remaining Issues

#### 5.1 Density Normalization Problem
**Symptoms:**
- Electron count 12.6× too high
- Hartree energy near zero
- XC energy wrong magnitude

**Likely Causes:**
- FFT normalization differences
- Density scaling factors
- Volume normalization issues

#### 5.2 Energy Assembly Differences
**Components to Investigate:**
- Hartree energy calculation
- XC energy calculation  
- Density-dependent terms

### 6. Next Steps

#### 6.1 Immediate Actions
1. **Investigate density calculation** in Band vs PSPW
2. **Check FFT normalization** in density computation
3. **Verify volume factors** in energy calculations
4. **Compare density assembly** between implementations

#### 6.2 Code Areas to Examine
- `cElectron.cpp` vs `Electron.cpp` density generation
- `CGrid.cpp` vs `PGrid.cpp` FFT normalization
- Density scaling factors in energy calculations

### 7. Technical Details

#### 7.1 Matrix Operation Analysis
**Band (`w_trace`):**
```cpp
int idx = 2*(i+i*ne[ms]) + mshift + mshift0;
double contribution = hml[idx]*weight;
```

**PSPW (`m_trace`):**
```cpp
sum += hml[i + i * ne[ms] + mshift];
```

#### 7.2 Energy Assembly Formula
**Both implementations use:**
```cpp
total_energy = eorbit0 + exc0 - ehartr0 - pxc0;
```

**Difference is in component values, not assembly.**

### 8. Files Modified
- `PWDFT/Nwpw/band/cpsd/band_inner_loop.cpp` - Sign flip fix

### 9. Files Analyzed
- `PWDFT/Nwpw/band/lib/cElectron/cElectron.cpp`
- `PWDFT/Nwpw/pspw/lib/electron/Electron.cpp`
- `PWDFT/Nwpw/nwpwlib/C3dB/Cneb.cpp`
- `PWDFT/Nwpw/nwpwlib/D3dB/Pneb.cpp`
- `PWDFT/Nwpw/band/cpsd/band_inner_loop.cpp`
- `PWDFT/Nwpw/pspw/cpsd/inner_loop.cpp`

### 10. Test Commands Used
```bash
# Build
cd /Users/brossdh/src/PWDFT/build && make band pspw

# Band H₂ test
cd /Users/brossdh/src/PWDFT/tests/tier1/2.4.1_H2_band
rm -f h2-energy.movecs
../../../build/pwdft < h2_band.nw > h2_band.out 2> debug.log

# PSPW H₂ test  
cd /Users/brossdh/src/PWDFT/tests/tier1/2.1.1_H2_energy
../../../build/pwdft < h2_energy.nw > h2_energy.out 2> debug.log
```

## Conclusion

**Major Progress:** Sign flip fix resolved the primary sign issue and dramatically improved energy magnitude.

**Current Blocking Issue:** Density normalization problem causing wrong electron count and Hartree energy.

**Next Priority:** Fix density calculation to get correct electron count and energy components.

**Status:** WIP - Sign issue resolved, density issue identified and needs fixing. 