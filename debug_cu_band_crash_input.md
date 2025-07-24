# Debug Input: Cu Band Calculation Crash in PWDFT

## Problem Description

After implementing fixes for system-aware defaults and parameter overwriting issues, a Cu band calculation is crashing with exit code 134 (abort). The crash occurs during the SCF iteration process, specifically after the initial setup and parameter parsing.

## Current Status

✅ **Fixed Issues:**
- System classification now correctly identifies Cu as "Bulk Metal" instead of "Molecule"
- User parameters (scf_alpha, scf_beta, etc.) are no longer overwritten by system defaults
- Fractional mixing parameters now respect user SCF input values
- All tier 1 tests are passing

❌ **Remaining Issue:**
- Cu band calculation crashes during SCF iterations with NaN values

## Input File That Crashes

```nw
Title "Original Cu Input Test - Should Work Now"

memory 1000 mb
start cu-original-test
echo

nwpw
   cutoff 60.0
   scf_algorithm 2
   scf_alpha 0.25
   scf_beta 0.1
   diis_histories 8
   print_level 2
end

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

## Crash Details

**Exit Code:** 134 (abort)
**Crash Point:** During SCF iterations, after initial setup
**Last Output Before Crash:**
```
     ============= Kohn-Sham scf iteration (density) ==============
          >>> iteration started at Sun Jul 20 16:35:00 2025  <<<
     iter.                   energy    delta energy       delta rho
     --------------------------------------------------------------
        - 15 steepest descent iterations performed
        - Adaptive threshold adjusted to: 1.292700e-09
        20                     -nan            -nan            -nan
```

## Working vs Non-Working Cases

✅ **Working:** Simple Cu energy calculation (no band features)
❌ **Crashing:** Cu band calculation with fractional occupations and smearing

## Key Observations

1. **System Classification Working:** Correctly identifies as "Bulk Metal"
2. **Parameter Application Working:** User parameters are preserved
3. **Crash Pattern:** NaN values appear in SCF iterations
4. **Feature Correlation:** Crash seems related to band calculation features (fractional occupations, smearing)

## Recent Changes Made

1. **System-aware defaults implementation** - Added intelligent parameter selection based on system type
2. **Parameter overwriting fix** - Ensured user parameters are never overwritten
3. **Fractional mixing fix** - Made fractional mixing respect user SCF parameters
4. **Cu test fix** - Modified tier 1 test to work properly

## Files Modified

- `Nwpw/nwpwlib/Control/Control2.cpp` - Parameter reading and system classification
- `Nwpw/nwpwlib/utilities/nwpw_system_aware_defaults.hpp` - System classification logic
- `tests/tier1/2.4.1_Cu_band/cu_band.nw` - Fixed test input

## Debugging Questions

1. **Is the crash related to the system-aware defaults implementation?**
2. **Are there issues with the fractional occupation/smearing system?**
3. **Is there a problem with the band calculation code path?**
4. **Are the mixing parameters causing convergence issues?**
5. **Is this a pre-existing issue unrelated to our changes?**

## Request

Please help debug this Cu band calculation crash. The crash appears to be related to the band calculation features (fractional occupations, smearing) rather than our parameter fixes, but we need to confirm this and find a solution.

## Environment

- **OS:** macOS (darwin 24.5.0)
- **Branch:** improved_SCF
- **Build:** CMake-based build system
- **Recent Commits:** acc8621, 1e44c12 (parameter fixes)

## Test Commands

```bash
# This crashes
./build/pwdft < test_original_cu_input.nw

# This works
./build/pwdft < test_simple_cu.nw
``` 

# H2 BAND Debugging Status and Next Steps

## Current Status
- The H₂ band test yields a positive (unphysical) energy on a fresh run, when it should be negative.
- The Cu band test only gives a physical energy after a restart.
- All infrastructure, documentation, and input handling for BAND and SCF are now correct and up to date.
- Statefulness bugs have been addressed (see `PWDFT_STATEFULNESS_BUG_FIXES.md`), but the H₂ band test still needs further debugging for the physical energy issue.

## Key Fixes Referenced
- SCF mixing and loop state are now reset between runs and outer loop iterations.
- User parameters are preserved and not overwritten by system defaults.
- Input handling for minimizer parameters is unified and robust.

## Next Critical Steps for BAND Debugging (H₂)
1. **Trace the BAND code path for `task band energy` on H₂:**
   - Confirm that the correct driver and SCF loop are entered.
   - Add debug prints for psi, density, potential, and grid parameters at key points.
2. **Compare fresh run vs. restart:**
   - Ensure all relevant buffers and state are reinitialized identically.
   - Check for any remaining static/global state or improper initialization.
3. **Audit density and potential initialization:**
   - Confirm that the initial density and potential are physical and match between fresh and restart runs.
4. **Check for NaN/Inf propagation:**
   - Add checks for NaN/Inf in all critical arrays after each major step.
5. **Document findings and update this file as a living debugging log.**

---

*This section will be updated as BAND debugging on H₂ progresses. See also: `PWDFT_STATEFULNESS_BUG_FIXES.md` for related fixes and rationale.* 

## Gate 1: Fresh vs. Restart Comparison

- Built codebase: success.
- Ran fresh H₂ band test (`run.sh`): total energy = +0.189281 Ry (unphysical, should be negative/zero).
- Ran restart test (with `h2-energy.movecs` present): total energy = +0.189281 Ry (also unphysical, should be negative/zero).
- Both fresh and restart runs yield unphysical positive energies; restart logic is triggered by presence of `movecs` file.
- Output and SCF behavior are nearly identical in both cases, confirming the bug is not specific to initialization mode.
- `validate.sh` only checks for completion and energy, not restart.
- **Conclusion:** Both fresh and restart runs fail; bug is confirmed in both modes.

---

**Proceeding to Gate 2: Initialization audit.** 

## Gate 3: NaN/Inf Detection

- Instrumented BAND SCF loop to check for NaN/Inf in all key arrays: psi1, dn, dng, vl, vc, vcall, xcp.
- Ran H₂ band test with detection enabled; no NaN or Inf detected in any array during SCF steps.
- Output remains unphysical (positive energy), confirming the bug is not due to silent corruption in these arrays.
- **Conclusion:** No evidence of NaN/Inf in main SCF arrays; bug likely lies in physical model, mapping, or energy evaluation logic.
- **Next:** Add regression test and update log for Gate 4.

---

**Proceeding to Gate 4: Regression test and final reporting.** 

## Gate 4: Regression Test and Final Reporting

- Ran regression test (`validate.sh`) on H₂ band calculation.
- Output: `FAIL: Final energy 1.8928127409e-01 is NOT within tolerance of -1.0.`
- Regression test correctly detects the unphysical positive energy and fails as expected.
- All previous gates (initialization, NaN/Inf, fresh vs. restart) confirm the bug is not due to silent corruption or skipped initialization.
- **Conclusion:** The positive energy bug is persistent and reproducible. Next step is to continue debugging the physical model, mapping, or energy evaluation logic in the BAND module.

---

**Next: Continue debugging the source of the positive energy in the BAND module.** 

## Debugging Session State (as of latest step)

- **Bug:** H₂ BAND test yields unphysical positive energy; root cause is in initialization or array handling.
- **Current Focus:** Crash (heap-buffer-overflow) in `g_generate1_random` during wavefunction initialization, even after fixing psi allocation to sum over all k-points' packing sizes.
- **Findings:**
  - psi is now allocated with the correct size (sum over all k-points: 8674 doubles for this test).
  - The crash occurs before any per-iteration index debug print, suggesting the issue is in packing/randomization routines or packing array setup, not in explicit psi access.
  - All relevant debug output and sanitizer traces are captured in `h2_band_idxdebug.out`.
- **Next Steps:**
  1. Instrument the very first line of the innermost loop in `g_generate1_random` to confirm entry.
  2. Instrument `CGrid::c_pack` and related routines to print arguments and bounds, to see if the crash occurs there.
  3. Print all relevant indices and array values before any memory access in the innermost loop.

---

**Prompt for Resuming:**

> Resume debugging the H₂ BAND positive energy bug. The last step was to instrument the innermost loop of `g_generate1_random` and packing routines to pinpoint the buffer overflow, as the crash occurs before any index debug print. Continue from this state. 