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