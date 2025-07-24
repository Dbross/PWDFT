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

---

## 2025-07-24: NaN/Inf Root Cause Trace — H₂ BAND Test

### Findings from debug.log
- The **first NaN appears in `psi1` immediately after allocation** (before any Hamiltonian or SCF operation):
  - `[NAN/INF DETECTED] psi1 at step after psi1 alloc, index 0, value=nan`
- The NaN propagates to `rho1` after `genrho`, and then to `Hpsi` after the kinetic energy step.
- All subsequent Hamiltonian operations (nonlocal PSP, FFT, etc.) operate on and propagate the NaN values.
- No NaN/Inf is detected in other major arrays at allocation (dng1, dng2, hml, eig, lmbda, etc.).

### Interpretation
- The root cause is **not in the Hamiltonian or potential**, but in the **initialization of the wavefunction buffer (`psi1`)**.
- Since `psi1` is NaN immediately after allocation, this suggests:
  - The allocation routine may be using a debug allocator that fills new memory with NaN (for bug detection), or
  - There is a code path that writes NaN to `psi1` immediately after allocation, before any physical initialization.
- The NaN in `rho1` and `Hpsi` is a direct consequence of the NaN in `psi1`.

### Hypotheses for Failure
1. **Uninitialized or poisoned memory:**
   - If the allocator is set to fill new memory with NaN (for debug), and `psi1` is not explicitly initialized (e.g., to zero or a physical guess), NaN will appear immediately.
2. **Faulty initialization logic:**
   - If the code path that should initialize `psi1` is skipped or fails, the buffer remains NaN.
3. **Corrupt input or restart file:**
   - If a restart or input file is read into `psi1` and contains NaN, this will propagate instantly.

### Next Steps
- **Audit the allocation and initialization of `psi1`:**
  - Confirm whether it is always explicitly initialized after allocation.
  - Check for any debug allocator or build option that fills new allocations with NaN.
  - If using restarts, check the contents of the input file for NaN/Inf.
- **Instrument or print the initialization logic for `psi1`** to confirm the first write after allocation.

--- 

---

## 2025-07-24: C++ Debug Infrastructure and Compile Fixes

### Recent Progress
- Integrated robust debug macros for NaN/Inf detection and array state dumps in all main C++ BAND/SCF code paths.
- Resolved multiple compile errors due to missing or mismatched debug utilities:
  - Implemented `check_nan_inf` for double arrays, enabled by `ENABLE_NAN_INF_CHECKS`.
  - Added `array_to_string` overloads for `int*`, `float*`, and `double*` to support all debug print use cases (e.g., for `nfft3d`, `ispin`, and scalar state variables).
- All debug macros (`STATE_DUMP`, `TRACE_LOG`, etc.) now work for all main types and are guarded by appropriate preprocessor flags.
- The debug infrastructure now allows for rapid instrumentation and robust runtime checking of all major arrays for NaN/Inf propagation, including after allocation, normalization, and SCF steps.

### Next Steps
- Continue using the improved debug macros to trace any further NaN/Inf propagation or state anomalies.
- If new types or containers need support, extend the debug utilities as needed.

--- 