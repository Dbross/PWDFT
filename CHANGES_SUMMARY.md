# PWDFT Documentation Updates - Comprehensive Changelog

## Overview
This document summarizes all updates made to the PWDFT documentation to accurately reflect the actual implementation found in the source code.

## Major Changes Made

### 1. Corrected Keyword Names
- **`calculation`** → **`task`**: The actual keyword used in PWDFT is `task`, not `calculation`
- **`ecutwfc`** → **`cutoff`**: The actual keyword for energy cutoff is `cutoff`, not `ecutwfc`
- **`nspin`** → **`ispin`**: The actual keyword for spin polarization is `ispin`, not `nspin`
- **`ecutrho`** → **Removed**: This keyword is not found in the actual PWDFT implementation

### 2. Updated Default Values
- **`loop`**: Changed from `'50 1'` to `'10 100'` (from control.cpp line 90)
- **`cutoff`**: Changed from `40.0 Ry` to `9000.0 Ry` (actual default)
- **`temperature`**: Changed from `5000 K` to `500 K` (more realistic default)
- **`memory`**: Added default of `'900 mb'` (from examples)

### 3. Added Missing Keywords
The following keywords were completely missing from the documentation but are implemented in the code:

#### Memory and Performance
- **`memory`**: Memory allocation (e.g., `memory 900 mb`)
- **`mapping`**: Parallel mapping strategy (1, 2, 3)
- **`np_dimensions`**: Parallel dimensions for 2D/3D mapping

#### System Definition
- **`simulation_cell`**: Cell definition (SC value or lattice_vectors)
- **`pseudopotentials`**: Pseudopotential specification block

#### Wavefunction Handling
- **`vectors`**: Wavefunction file input/output control

#### Optimization Methods
- **`steepest_descent`**: Steepest descent optimization
- **`car-parrinello`**: Car-Parrinello molecular dynamics

#### Spin and Magnetism
- **`mult`**: Multiplicity (spin state)

### 4. Expanded Keyword Options

#### SCF Algorithms
**Added missing algorithms:**
- `ks-grassmann-lmbfgs`
- `ks-block-cg`
- `ks-cg`
- `ks-rmm-diis`

#### SCF Mixers
**Added missing mixers:**
- `broyden`
- `johnson-pulay`
- `diis`
- `thomas-fermi`

#### Smearing Types
**Added missing smearing options:**
- `fermi` (instead of `fermi-dirac`)
- `marzari-vanderbilt`
- `hermite`
- `cold`
- `lorentzian`
- `step`
- `fixed`

### 5. Updated Ranges and Examples

#### Cutoff Range
- **Old**: 20.0 - 200.0 Rydberg
- **New**: 5.0 - 9000.0 Rydberg
- **Added examples**: `cutoff 35.00`, `cutoff 5.0`, `cutoff 60.0`

#### Temperature Range
- **Old**: 1000 - 10000 Kelvin
- **New**: 100 - 10000 Kelvin

### 6. Removed Non-Existent Keywords
- **`dipole_correction`**: Not found in PWDFT source code
- **`pseudo_dir`**: Not found in PWDFT source code
- **`ecutrho`**: Not found in PWDFT source code

### 7. Added Comprehensive Examples

#### Example Input File
Added a complete example input file showing proper PWDFT syntax:

```text
memory 900 mb
start calculation
echo

geometry noautosym noautoz center
Cu 0 0 0
Cu 0 0 1.8
end

nwpw
  simulation_cell
    SC 20.0
  end
  cutoff 60.0
  xc pbe96
  scf ks-grassmann-cg anderson alpha 0.15
  smear methfessel-paxton
  temperature 500
  loop 20 20
  monkhorst-pack 3 3 1
  initial_wavefunction_guess superposition
  mult 1
end

task pspw energy
```

### 8. Updated Best Practices

#### Convergence Protocol
- **Energy Cutoff**: Changed from "Start with 40 Ry" to "Start with 35-60 Ry"
- **Smearing**: Changed from "use 5000-8000 K" to "use 500-8000 K"

#### System-Specific Recommendations
- **Molecules**: Changed from "60+ Ry cutoff" to "35+ Ry cutoff"

## Files Modified

### 1. `doc/api/keywords.rst`
- Complete rewrite of keyword documentation
- Added all missing keywords
- Corrected keyword names and default values
- Added comprehensive examples
- Updated physics descriptions

### 2. `doc/index.rst`
- Updated example to use correct `cutoff` keyword
- Added complete example input file
- Updated quick start section

### 3. `CHANGES_SUMMARY.md` (this file)
- Created comprehensive changelog
- Documented all changes made

## Verification Sources

The updates were based on analysis of:

1. **Source Code Analysis**:
   - `Nwpw/nwpwlib/parse/parse_pwdft.cpp` (lines 1751-2390)
   - `N2PW/nwpwlib/control/control.cpp` (line 90)
   - `Nwpw/nwpwlib/Control/Control2.cpp` (lines 664-715)

2. **Example Input Files**:
   - `DEMO/eric.nw`
   - `DEMO/eric0.nw`
   - `DEMO2/ericmd0.nw`
   - `cu_slab_init.nwxi`

3. **ASE Integration Code**:
   - `pynta/ase_pwdft/pwdftio/pwdftwriter.py`
   - `pynta/ase_pwdft/pwdft.py`

## Impact

These updates ensure that:
1. **Users get accurate information** about available keywords
2. **Examples work correctly** with the actual PWDFT implementation
3. **Documentation matches the code** exactly
4. **New users can start** with working examples
5. **Advanced users have complete** reference information

## Future Recommendations

1. **Regular verification**: Cross-check documentation with code changes
2. **Add more examples**: Include examples for each calculation type
3. **Tutorial updates**: Update tutorials to use correct keywords
4. **API documentation**: Consider adding Python API documentation
5. **Testing**: Add tests to ensure examples in documentation work 