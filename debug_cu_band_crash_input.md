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

## Update (2025-07-24)

### 5. **Robust NaN/Inf Detection and Fallbacks Implemented**
- Integrated comprehensive NaN/Inf detection in all major energy, matrix, and trace computations (see `NaN_DETECTION_FIXES_SUMMARY.md`).
- Fallback logic now triggers after 3 consecutive NaN/Inf/large energy events: reinitializes wavefunction, performs 15 steepest descent steps, and restarts SCF loop.
- All fallback and recovery activity is logged, with clear diagnostic messages for each event.
- Adaptive SCF mixing now reduces alpha/beta and switches to simple mixing if repeated NaNs are detected.
- All minimizer and geodesic transport routines now use robust, bounded algorithms with overflow/underflow protection.

### 6. **Current State**
- NaN/Inf events are now reliably detected and handled without crashing or infinite loops.
- Automatic recovery and stabilization work as designed, but NaN/Inf can still recur in challenging systems.
- Broader SCF convergence issues persist, especially during wavefunction optimization (not just mixing or preconditioning).
- Enhanced Local-TF and robust optimization are not the root cause; the problem is systemic to SCF convergence for some systems.
- All debug output and fallback events are flushed to logs for traceability.

### 7. **Next Steps**
1. Continue tracing the first appearance of NaN/Inf in the SCF loop, focusing on wavefunction initialization and optimization.
2. Investigate system-aware default overrides and their effect on user-specified SCF parameters.
3. Explore further improvements to initial guess and fallback strategies for problematic systems.
4. Document all findings and recovery events for future reference.

---

**Ready to resume from this point.** 

## Call Site Audit and Allocation Guidance Request (2025-07-24)

I have fixed all Fortran FFTPACK routines to use assumed-size arrays and ensured all workspace arrays are allocated with sufficient size at the top level. However, I am still seeing out-of-bounds warnings in routines like dradf2.f and dradf3.f (e.g., wa1(i-2) in a loop starting at i=3). Please:

- Audit all call sites to these FFTPACK routines (e.g., dradf2, dradf3, etc.) in my codebase. For each call:
  - Show how the workspace arrays (wa1, etc.) are allocated and passed.
  - Confirm that they are always allocated with at least ido elements and that the offset is correct (if passing subarrays).
  - If any call site is unsafe, provide explicit guidance or code changes to fix the allocation or offset.

My codebase is a mix of Fortran and C/C++. Please be thorough and check both languages. Summarize your findings and provide actionable next steps. 

---

## FFTPACK Workspace Audit: Completion Update (2025-07-24)

- All FFT workspace allocations (e.g., tmpx, tmpy, tmpz) in both main and N2PW code paths have been audited and patched to guarantee at least 2*nx+15 elements for FFTPACK safety.
- All direct FFTPACK workspace uses are now protected by runtime size checks, which are only active if ENABLE_FFT_SIZE_CHECKS is enabled in CMake.
- No missed or unsafe FFT workspace allocations were found in the codebase.
- Step 2 (runtime checks) is now fully complete.

**Next step:** Resume systematic debugging of the persistent NaN/Inf issue in BAND/SCF, now with confidence that FFT workspace overruns are not the cause. 

## 2025-07-24: Macro-Guarded Debug Print Refactor

- All major debug prints (C_PACK, CC_PACK_COPY, HR_A_SUM_SQR, CPSI_H, NAN/INF, PSI ALLOC) are now wrapped in appropriate macro guards:
    - ENABLE_WAVEFUNC_DEBUG: wavefunction/packing/alloc debug
    - ENABLE_HAMILTONIAN_DEBUG: Hamiltonian application debug
    - ENABLE_NAN_INF_CHECKS: NaN/Inf detection and propagation
- Only enabled debug macros now produce output; unconditional prints are removed.
- This commit enables focused, category-specific debugging and reduces log noise.
- Next: Resume debugging NaN/Inf propagation in the H2 band test, starting from the first appearance in the Hamiltonian application.

---

**To continue debugging:**
Start a new chat and say:
> Resume NaN/Inf debugging in the H2 band test. Start from the first NaN in Hpsi and trace backwards through the Hamiltonian and input buffers. 