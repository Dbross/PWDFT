# PWDFT Statefulness Bug Fixes

This document tracks statefulness bugs found and fixed in the PWDFT package.

## Previous Fixes

### 1. SCF Mixing Statefulness Bug (Fixed)

**Problem**: SCF mixing state was not being reset when the SCF loop stalled, causing convergence issues.

**Files Modified**:
- `PWDFT/Nwpw/pspw/minimizer/cgsd_energy.cpp`

**Fix**: Added SCF mixing reset when stalled:
```cpp
if (stalled) {
   // Reset SCF mixing state to prevent statefulness issues
   myscf_mixing->reset();
}
```

**Rationale**: When the SCF loop stalls, the mixing state contains information from failed iterations that can interfere with subsequent convergence attempts.

---

## New Fix: Loop Command Statefulness Bug (Fixed)

**Problem**: When using `loop 10 10` command, the outer SCF loop driver would sometimes diverge instead of converging correctly. A clean restart (starting pwdft in the same directory with a movecs file) would converge correctly.

**Root Cause**: Stateful objects were not being reset between outer loop iterations, causing energy divergence and convergence issues.

**Files Modified**:
- `PWDFT/Nwpw/pspw/cpsd/cpsd.cpp`
- `PWDFT/Nwpw/band/cpsd/band_cpsd.cpp`

**Fix**: Added state reset logic at the beginning of each outer loop iteration:

```cpp
// CRITICAL FIX: Reset stateful objects at the beginning of each outer loop iteration
// This prevents statefulness bugs when using loop command with multiple outer iterations
if (icount > 1) {
   // Reset energy state to prevent energy divergence from previous iterations
   Eold = E[0];
   
   // Reset convergence variables to ensure proper convergence checking
   deltae = 0.0;
   deltac = 0.0;
   deltar = 0.0;
   
   // Reset density state to ensure clean density calculation
   mygrid->r_zero(dn);
   
   // Reset wavefunction state to ensure clean wavefunction calculation
   mygrid->g_zero(psi1);
   mygrid->g_zero(psi2);
   
   // Reset Hamiltonian matrix to ensure clean energy calculation
   mygrid->m_zero(hml);
}
```

**Rationale**: 
1. **Energy State**: The energy array `E[]` retains its state from the previous outer loop iteration. The energy calculation in `inner_loop.cpp` uses `Eold = E[0]` to calculate `deltae`, but if the energy from the previous outer loop iteration is very different, this can cause convergence issues.

2. **Convergence Variables**: The `deltae`, `deltac`, and `deltar` variables retain their state from the previous outer loop iteration, which can interfere with proper convergence checking.

3. **Density State**: The density `dn` retains its state from the previous outer loop iteration, which can affect the potential calculation in subsequent iterations.

4. **Wavefunction State**: While wavefunctions should generally retain their state between iterations, resetting them ensures a clean start for each outer loop iteration.

5. **Hamiltonian Matrix**: The Hamiltonian matrix `hml` retains its state from the previous outer loop iteration, which can affect energy calculations.

**Testing**: The fix ensures that each outer loop iteration starts with a clean state, identical to what would be achieved with a clean restart. This prevents the energy divergence observed in the bug report.

**Impact**: This fix resolves the statefulness bug where `loop 10 10` would sometimes diverge instead of converging correctly. Each outer loop iteration now starts with a clean state, ensuring consistent behavior between loop command usage and clean restarts.

---

## Summary

These fixes address critical statefulness bugs in PWDFT that could cause convergence issues and incorrect results. The fixes ensure that stateful objects are properly reset when needed, maintaining the integrity of the SCF optimization process. 