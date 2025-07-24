# PWDFT BAND H₂ Debug Log (as of 2025-07-23)

## Summary of Progress

### 1. **Initialization and Allocation Fixes**
- Moved all critical allocations (`psi1`, `psi2`, `rho1`, etc.) into the `Solid` constructor, ensuring all pointers are valid and correctly sized.
- Fixed allocation size for `psi1` and related arrays to match the sum over all k-points: `sum_k 2*(ne[0]+ne[1])*npack(k)`.
- Added debug prints for all allocations, pointer values, and sizes.

### 2. **Occupation and Density Normalization**
- Ensured `occ1` is always allocated and set to 1.0 for all bands, regardless of spin.
- Density normalization now checks the integrated density (sum * dv) and compares to the expected electron count.
- Added iterative normalization of the initial wavefunction (`psi1`) to match the expected electron count, using the correct allocated size.
- Normalization is skipped if a `movecs` file is present (restart), and a warning is printed.
- If the density is already correct, normalization is skipped.

### 3. **NaN/Inf and SCF Failures**
- Despite correct allocation and normalization, the code still produces NaN/Inf in the electron count and energies after the first SCF step.
- The code retries with a new wavefunction, but the problem persists.
- Added debug prints for NaN/Inf detection and propagation.

### 4. **Debug Output and Diagnostics**
- All debug output is now flushed to `debug.out`.
- The debug log includes pointer values, allocation sizes, normalization steps, and density checks.
- The code now aborts with a clear error if the density normalization or allocation fails.

## Current State
- All allocation and normalization logic is correct and robust against buffer overruns.
- The code still fails with NaN/Inf in the SCF loop, indicating a deeper issue (possibly in the initial guess, potential, or Hamiltonian application).
- The debug log is large but contains all relevant information for resuming debugging.

## Next Steps for Tomorrow
1. **Audit NaN/Inf propagation:**
   - Add checks and debug prints for NaN/Inf in `psi1`, `rho1`, and all major arrays after normalization and after the first SCF step.
2. **Audit initial guess logic:**
   - Confirm whether the default is superposition or random, and ensure normalization is applied in all cases.
   - If superposition, print and check the initial guess.
3. **Check for uninitialized or zeroed arrays:**
   - Ensure all arrays are properly initialized before use.
4. **Continue systematic debugging:**
   - Trace the first appearance of NaN/Inf and identify the root cause.

---

**Ready to resume from this point tomorrow.** 