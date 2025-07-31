# Codepath Audit Prompt: Band vs PSPW Energy Discrepancy

## Context & Objective

You are tasked with performing a detailed codepath audit of the Band and PSPW energy routines for H₂, with the goal of pinpointing the source of the ~25–40× energy discrepancy in the Band module's kinetic and orbital energy terms.

**Key Finding from Energy Component Analysis:**
- **Total Orbital Energy:** Band = -30.064 Ry, PSPW = -0.757 Ry (39.7× discrepancy)
- **Kinetic Energy:** Band = 30.095 Ry, PSPW = 1.062 Ry (28.4× discrepancy)  
- **Total Energy:** Band = -29.906 Ry, PSPW = -1.167 Ry (25.6× discrepancy)
- **Ion-Ion Energy:** Identical in both modules (1.0× ratio) - confirms issue is NOT in Ewald/ion-ion terms
- **Hartree, V_local, V_nl, V_Coul:** Near zero in Band but significant in PSPW

## Audit Requirements

### 1. **Trace the Full Computational Flow**
For both Band and PSPW modules, trace the energy evaluation from input parsing through final output, with special attention to:

- **Kinetic energy calculation** (`ke_ave` functions)
- **Orbital energy calculation** (`eorbit` functions) 
- **Density and wavefunction normalization**
- **Volume and occupation number handling**
- **Energy component assembly** (`gen_energies_en` functions)

### 2. **Document Each Major Step**
Using the structure from `band_implemented.json` and `pspw_implemented.json` as templates:

- Create a flowchart or structured list for each module
- Reference relevant functions, files, and MPI/data movement
- Include function signatures and key parameters
- Note any differences in function names between modules

### 3. **Highlight Normalization/Scaling Points**
Identify and document ALL points where normalization, scaling, or summation occurs:

- **Per-cell normalization** (volume factors)
- **Per-electron scaling** (occupation numbers)
- **Per-spin handling** (ispin loops)
- **Per-k-point operations** (k-point weights)
- **MPI reductions** (global sums)
- **FFT scaling factors**

### 4. **Side-by-Side Comparison**
Create a detailed comparison table showing:

| Step | Band Function | PSPW Function | Key Differences | Scaling Factors |
|------|---------------|---------------|-----------------|-----------------|
| Kinetic Energy | `cElectron::ke_ave()` | `Electron::eke()` | ... | ... |
| Orbital Energy | `cElectron::eorbit()` | `Electron::energy()` | ... | ... |
| ... | ... | ... | ... | ... |

### 5. **Reference Previous Audits**
Use the diagrams and node details from:
- `band_implemented.json` - Band module flowchart and implementation details
- `pspw_implemented.json` - PSPW module flowchart and implementation details

These provide the overall computational flow and can help identify where energy calculations fit into the broader SCF loop.

## Key Files to Investigate

### Band Module:
- `Nwpw/band/lib/cElectron/cElectron.cpp` - Main energy calculation routines
- `Nwpw/band/cpsd/band_inner_loop.cpp` - Energy calculation in inner loop
- `Nwpw/band/minimizer/band_cgsd_energy.cpp` - Energy calculation in minimizer
- `Nwpw/band/lib/solid/Solid.cpp` - Energy wrapper functions

### PSPW Module:
- `Nwpw/pspw/lib/electron/Electron.cpp` - Main energy calculation routines
- `Nwpw/pspw/cpsd/inner_loop.cpp` - Energy calculation in inner loop
- `Nwpw/pspw/lib/molecule/Molecule.cpp` - Energy wrapper functions

### Common Libraries:
- `Nwpw/nwpwlib/kinetic/` - Kinetic energy operators
- `Nwpw/nwpwlib/coulomb/` - Hartree energy calculations
- `Nwpw/nwpwlib/exchange-correlation/` - XC energy calculations

## Specific Investigation Points

### 1. **Kinetic Energy Discrepancy (28.4×)**
- Compare `cElectron::ke_ave()` vs `Electron::eke()`
- Check for differences in kinetic operator application
- Verify wavefunction normalization in kinetic calculations
- Look for volume normalization factors

### 2. **Orbital Energy Discrepancy (39.7×)**
- Compare `cElectron::eorbit()` vs `Electron::energy()`
- Check Hamiltonian matrix construction (`ggw_sym_Multiply` vs `ggm_sym_Multiply`)
- Verify trace operations (`w_trace` vs `m_trace`)
- Look for scaling factors in matrix operations

### 3. **Near-Zero Potential Terms in Band**
- Investigate why Hartree, V_local, V_nl, V_Coul are ~0.001 Ry in Band
- Compare potential calculation routines between modules
- Check if potentials are being calculated but incorrectly scaled
- Verify density input to potential calculations

### 4. **Occupation Number Handling**
- Check how `occ` parameters are passed and used
- Verify occupation number normalization
- Look for differences in `w_trace_occ` vs `m_trace_occ`

## Deliverable Format

Create a Markdown document containing:

### 1. **Flowcharts**
- One flowchart per module showing energy calculation path
- Annotate with function names, file paths, and key parameters
- Highlight normalization/scaling points with special markers

### 2. **Comparison Table**
- Side-by-side comparison of equivalent functions
- Key differences in implementation
- Scaling factors and normalization steps

### 3. **Code Snippets**
- Critical code sections showing differences
- Function signatures and key parameters
- Comments explaining scaling factors

### 4. **Hypothesis Summary**
- Prioritized list of likely root causes
- Specific code locations to investigate
- Recommended next steps for verification

## Success Criteria

The audit should identify:
1. **Exact location** of the scaling discrepancy
2. **Specific code differences** causing the 25–40× energy overestimation
3. **Clear path forward** for implementing the fix
4. **Verification strategy** to confirm the fix works

## Resources

- **Previous Analysis:** `BAND_ENERGY_DEBUG_PROGRESS.md`
- **Energy Components:** `energy_component_analysis.py`
- **Module Diagrams:** `band_implemented.json`, `pspw_implemented.json`
- **Test Results:** `tests/tier1/2.4.1_H2_band/`, `tests/tier1/2.1.1_H2_energy/`

---

**Start with the kinetic and orbital energy functions, as these show the largest discrepancies and are most likely to contain the root cause of the scaling issue.** 