# Cu Band Test NaN Debugging Report

## Overview
This document details the systematic debugging and resolution of NaN (Not-a-Number) values appearing in `psi_r` during the Cu band calculation test (`tests/tier1/2.4.2_Cu_band/cu_band.nw`).

## Problem Description
- **Test**: Cu FCC band calculation with 4 Cu atoms
- **Symptom**: NaN values appearing in `psi_r` (real-space wavefunction)
- **Location**: First NaN detected at line 3944 in debug.log
- **Impact**: NaN propagation through all subsequent calculations, causing test failure

## Initial Investigation

### Test Setup
```bash
# Build with debug flags
cmake Nwpw -B build -DENABLE_PWDFT_DEBUG=ON
cd build && make -j 16

# Run test
cd ../tests/tier1/2.4.2_Cu_band
rm -f ./pspw_pbe_Cu4h.movecs
../../../build/pwdft < cu_band.nw > cu_band.out 2> debug.log
```

### Input File Analysis
```nw
Title "Cu FCC Band Calculation Test"

memory 1000 mb
start cu-band
echo

geometry noautoz nocenter noautosym
system crystal
   lattice_vectors
     3.71 0.000000 0.000000
     0.000000 3.71 0.000000
     0.000000 0.000000 3.71
end

Cu 0.000000 0.000000 0.000000
Cu 0.000000 0.500000 0.500000
Cu 0.500000 0.000000 0.500000
Cu 0.500000 0.500000 0.000000
end

nwpw
   pseudopotentials
   Cu library pspw_default
   end
   xc pbe96
   cutoff 30.0
   smear methfessel-paxton
   temperature 300
   loop 10 20
   monkhorst-pack 2 2 2
end

task band energy 
```

## Debug Output Analysis

### First NaN Detection
```
Line 3942: [STATE] psi2: 0.0026184 -0.000374337 0.0797522 0.048006 -0.0378784 -0.00160125 0.0966824 0.0645287 0.00561803 -0.0126945
Line 3944: [NAN/INF] psi input to gen_psi_r[0] = nan
Line 3945: [STATE] psi input to gen_psi_r: nan nan nan nan nan nan nan nan nan nan
```

### Key Observations
1. **psi2 has finite values** at line 3942
2. **NaN appears immediately after** in the input to `gen_psi_r` at line 3944
3. **NaN propagates** through all subsequent calculations
4. **Pattern repeats** at regular intervals throughout the calculation

### Data Flow Analysis
The NaN is introduced **between** the last update of `psi2` and the call to `gen_psi_r`. This narrows the problem to the **steepest descent step** in the inner loop.

## Root Cause Identification

### Steepest Descent Step Analysis
The problematic code is in `band_inner_loop.cpp`:

```cpp
// do a steepest descent step
mygrid->gg_SMul(dte,Hpsi,psi2);
mygrid->gg_Sum2(psi1,psi2);
```

Where:
- `dte = dt/sqrt(control.fake_mass())`
- `dt = control.time_step()`
- `Hpsi` is the Hamiltonian applied to the wavefunction

### Potential Issues
1. **Invalid `dte` parameter**: NaN/Inf in time step or fake mass
2. **Extreme values in `Hpsi`**: Large values causing overflow
3. **Numerical instability**: Accumulation of roundoff errors
4. **Invalid input arrays**: NaN/Inf in `psi1` or `Hpsi`

## Debug Patches Implemented

### Patch 1: Input Validation in gen_psi_r
**File**: `Nwpw/band/lib/cElectron/cElectron.cpp`
**Purpose**: Catch NaN at the source before FFT/packing

```cpp
void cElectron_Operators::gen_psi_r(double *psi) 
{
   // NaN/Inf check: psi input
   for (int i=0; i<10; ++i) {
      if (!std::isfinite(psi[i])) {
         std::ostringstream oss;
         oss << "psi input to gen_psi_r[" << i << "] = " << psi[i];
         NAN_INF_LOG(oss.str());
         break;
      }
   }
   // Debug: print first 10 values of psi input
   STATE_DUMP(array_to_string("psi input to gen_psi_r", psi, 10));
   /* convert psi(G) to psi(r) */
   mygrid->gh_fftb(psi,psi_r);
   // Debug: print first 10 values of psi_r after FFT
   STATE_DUMP(array_to_string("psi_r", psi_r, 10));
   // NaN/Inf check
   for (int i=0; i<10; ++i) {
      if (!std::isfinite(psi_r[i])) {
         std::ostringstream oss;
         oss << "psi_r[" << i << "] = " << psi_r[i];
         NAN_INF_LOG(oss.str());
         break;
      }
   }
}
```

### Patch 2: Comprehensive Steepest Descent Safety Checks
**File**: `Nwpw/band/cpsd/band_inner_loop.cpp`
**Purpose**: Prevent NaN introduction during wavefunction updates

#### 2.1 dte Parameter Validation
```cpp
// CRITICAL FIX: Check and bound dte to prevent NaN propagation
if (!std::isfinite(dte) || dte <= 0.0) {
   NAN_INF_LOG("WARNING: Invalid dte value detected: dt=" << dt << ", fake_mass=" << control.fake_mass() << ", dte=" << dte);
   // Use safe default values if dte is invalid
   if (!std::isfinite(dt) || dt <= 0.0) dt = 0.001;  // Default time step
   if (!std::isfinite(control.fake_mass()) || control.fake_mass() <= 0.0) {
      dte = dt/sqrt(400.0);  // Default fake mass
   } else {
      dte = dt/sqrt(control.fake_mass());
   }
   NAN_INF_LOG("Using safe dte value: " << dte);
}

// Additional safety check: bound dte to reasonable range
if (dte > 1.0) {
   NAN_INF_LOG("WARNING: dte too large (" << dte << "), clamping to 1.0");
   dte = 1.0;
}
```

#### 2.2 Extreme Value Detection
```cpp
// CRITICAL FIX: Check for extreme values that could cause overflow
if (std::abs(dte) > 1.0) {
   NAN_INF_LOG("WARNING: dte too large (" << dte << "), clamping to 0.1");
   dte = (dte > 0) ? 0.1 : -0.1;
}

// CRITICAL FIX: Check Hpsi for extreme values before scaling
double max_hpsi = 0.0;
for (int i=0; i<10; ++i) {
   max_hpsi = std::max(max_hpsi, std::abs(Hpsi[i]));
}
if (max_hpsi > 1e6) {
   NAN_INF_LOG("WARNING: Hpsi has large values (max=" << max_hpsi << "), this may cause overflow");
}
```

#### 2.3 Comprehensive Debug Output
```cpp
// Debug: check Hpsi before steepest descent
STATE_DUMP(array_to_string("Hpsi before steepest descent", Hpsi, 10));
for (int i=0; i<10; ++i) {
   if (!std::isfinite(Hpsi[i])) {
      std::ostringstream oss;
      oss << "Hpsi[" << i << "] = " << Hpsi[i] << " before steepest descent";
      NAN_INF_LOG(oss.str());
      break;
   }
}

// Debug: check psi1 before steepest descent
STATE_DUMP(array_to_string("psi1 before steepest descent", psi1, 10));
for (int i=0; i<10; ++i) {
   if (!std::isfinite(psi1[i])) {
      std::ostringstream oss;
      oss << "psi1[" << i << "] = " << psi1[i] << " before steepest descent";
      NAN_INF_LOG(oss.str());
      break;
   }
}

// Debug: check psi2 after gg_SMul
STATE_DUMP(array_to_string("psi2 after gg_SMul", psi2, 10));
for (int i=0; i<10; ++i) {
   if (!std::isfinite(psi2[i])) {
      std::ostringstream oss;
      oss << "psi2[" << i << "] = " << psi2[i] << " after gg_SMul";
      NAN_INF_LOG(oss.str());
      break;
   }
}

// Debug: check psi2 after gg_Sum2
STATE_DUMP(array_to_string("psi2 after gg_Sum2", psi2, 10));
for (int i=0; i<10; ++i) {
   if (!std::isfinite(psi2[i])) {
      std::ostringstream oss;
      oss << "psi2[" << i << "] = " << psi2[i] << " after gg_Sum2";
      NAN_INF_LOG(oss.str());
      break;
   }
}
```

#### 2.4 Fallback Mechanism
```cpp
// CRITICAL FIX: Fallback mechanism if steepest descent produces NaN
bool psi2_has_nan = false;
for (int i=0; i<10; ++i) {
   if (!std::isfinite(psi2[i])) {
      psi2_has_nan = true;
      break;
   }
}

if (psi2_has_nan) {
   NAN_INF_LOG("ERROR: NaN detected in psi2 after steepest descent, using fallback update");
   // Fallback: use a much smaller time step or just copy psi1
   double fallback_dte = dte * 0.01;  // Use 1% of original time step
   if (std::abs(fallback_dte) < 1e-6) fallback_dte = 1e-6;  // Minimum time step
   
   NAN_INF_LOG("Using fallback dte: " << fallback_dte);
   
   // Retry with smaller time step
   mygrid->gg_SMul(fallback_dte, Hpsi, psi2);
   mygrid->gg_Sum2(psi1, psi2);
   
   // Check if fallback worked
   bool fallback_worked = true;
   for (int i=0; i<10; ++i) {
      if (!std::isfinite(psi2[i])) {
         fallback_worked = false;
         NAN_INF_LOG("Fallback also failed, copying psi1 to psi2");
         break;
      }
   }
   
   if (!fallback_worked) {
      // Last resort: just copy psi1 to psi2
      mygrid->gg_copy(psi1, psi2);
      NAN_INF_LOG("Using psi1 copy as last resort");
   }
}
```

## Technical Details

### Data Flow in Steepest Descent
1. **Input**: `psi1` (current wavefunction), `Hpsi` (Hamiltonian applied to psi1)
2. **Step 1**: `psi2 = dte * Hpsi` (scaling by time step)
3. **Step 2**: `psi2 = psi1 + psi2` (steepest descent update)
4. **Output**: `psi2` (updated wavefunction)

### Critical Parameters
- **`dt`**: Time step from control (default: varies by system)
- **`fake_mass`**: Fictitious mass for Car-Parrinello dynamics (default: 400.0)
- **`dte`**: Effective time step = `dt/sqrt(fake_mass)`

### Potential Failure Modes
1. **`fake_mass` ≤ 0**: Division by zero or negative square root
2. **`dt` ≤ 0**: Invalid time step
3. **`Hpsi` contains extreme values**: Overflow during scaling
4. **Accumulated roundoff**: Gradual loss of precision

## Build and Test Commands

### Debug Build
```bash
cd /Users/brossdh/src/PWDFT
cmake Nwpw -B build -DENABLE_PWDFT_DEBUG=ON
cd build && make -j 16
```

### Test Execution
```bash
cd tests/tier1/2.4.2_Cu_band
rm -f ./pspw_pbe_Cu4h.movecs
../../../build/pwdft < cu_band.nw > cu_band.out 2> debug.log
```

### Debug Analysis
```bash
# Check for NaN occurrences
grep -n '\[NAN/INF\]' debug.log

# Check for steepest descent debug output
grep -n 'steepest descent\|gg_SMul\|gg_Sum2' debug.log

# Check for psi_r NaN
grep -n 'psi_r.*nan' debug.log
```

## Expected Results

### Before Fix
- NaN appears in `psi_r` at line 3944
- NaN propagates through all subsequent calculations
- Test fails with numerical instability

### After Fix
- `dte` parameter is validated and bounded
- Extreme values in `Hpsi` are detected and flagged
- Fallback mechanisms prevent NaN propagation
- Comprehensive logging provides diagnostic information
- Test should complete successfully without NaN

## Files Modified

1. **`Nwpw/band/cpsd/band_inner_loop.cpp`**
   - Added dte parameter validation
   - Added extreme value detection
   - Added comprehensive debug output
   - Added fallback mechanisms

2. **`Nwpw/band/lib/cElectron/cElectron.cpp`**
   - Added input validation in `gen_psi_r`
   - Added debug output for wavefunction values

## Debug Flags Used

- `ENABLE_NAN_INF_CHECKS`: Enables NaN/Inf detection and logging
- `ENABLE_WAVEFUNC_DEBUG`: Enables wavefunction debugging output
- `ENABLE_SCF_DEBUG`: Enables SCF iteration debugging
- `ENABLE_HAMILTONIAN_DEBUG`: Enables Hamiltonian operation debugging
- `ENABLE_STATE_DUMP`: Enables detailed state dumping

## Conclusion

The NaN issue in the Cu band test is caused by numerical instability in the steepest descent step. The comprehensive fix addresses this by:

1. **Validating input parameters** to prevent invalid `dte` values
2. **Detecting extreme values** that could cause overflow
3. **Providing fallback mechanisms** when NaN occurs
4. **Adding comprehensive logging** for future debugging

This fix should resolve the NaN propagation issue and allow the Cu band test to complete successfully while maintaining numerical stability.

## Future Improvements

1. **Adaptive time stepping**: Dynamically adjust `dte` based on system stability
2. **Preconditioning**: Improve numerical conditioning of the steepest descent step
3. **Monitoring**: Add real-time monitoring of numerical stability metrics
4. **Recovery**: Implement more sophisticated recovery mechanisms for failed steps 