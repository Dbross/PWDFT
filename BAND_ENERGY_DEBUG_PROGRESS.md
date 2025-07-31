# Band Module Energy Debug Progress

## Problem Summary
- **Target**: H₂ energy should be ~-1.17 Ry (PSPW result)
- **Current**: Band module produces ~-29.91 Ry (much improved!)
- **Root cause**: Double application of cell volume normalization in density integration + incorrect occupation number handling

## Fixes Applied

### 1. Density Integration Fix (cElectron.cpp:256)
**Before:**
```cpp
density_integral += dn[i] * mygrid->lattice->omega() / (mygrid->nx * mygrid->ny * mygrid->nz);
```
**After:**
```cpp
density_integral += dn[i];  // dn[i] already normalized per unit cell volume
```

### 2. Density Sum Fix (Solid.cpp:378, 385)
**Before:**
```cpp
double sum_rho_phys = sum_rho * dv;
```
**After:**
```cpp
double sum_rho_phys = sum_rho;  // rho1[i] already normalized per unit cell volume
```

### 3. Electron Count Fix (cElectron.cpp:1100)
**Before:**
```cpp
en[0] = dv * mygrid->r_dsum(dn);
```
**After:**
```cpp
en[0] = mygrid->r_dsum(dn);  // dn already normalized per unit cell volume
```

### 4. Occupation Number Fix (Solid.cpp:125-140) - CRITICAL FIX
**Before:**
```cpp
// Always allocated and set occupation numbers
int nocc = nbrillq * (ne[0] + ne[1]);
occ1 = new double[nocc];
occ1[0] = 2.0;  // This doubled the density!
```
**After:**
```cpp
// Only allocate when fractional=true (like PSPW)
occ1 = nullptr;
occ2 = nullptr;
```

### 5. Expected Electron Count Fix (Solid.cpp:356) - GENERALIZATION
**Before:**
```cpp
double expected_electrons = 2.0;  // Hardcoded for H₂ only
```
**After:**
```cpp
// Calculate expected electron count based on system parameters
double expected_electrons = nbrillq * ((ispin == 1) ? (ne[0] * 2 + ne[1] * 2) : (ne[0] + ne[1]));
```

## Results After All Fixes
- **Energy**: Improved from -70.35 Ry → -59.71 Ry → **-29.91 Ry**
- **Electron count**: Now correctly 2.0 electrons for H₂
- **Status**: Much closer to target! Still ~25x too large
- **Generalization**: Fix now works for any system, not just H₂

## Key Discovery
The **root cause** was that the Band module was incorrectly handling occupation numbers when `fractional=false`:
- **PSPW**: When `fractional=false`, `occ1` remains `nullptr` → uses `hr_aSumSqr` (no occupation weights)
- **Band**: When `fractional=false`, `occ1` was allocated with `occ1[0]=2.0` → used `hr_aSumSqr_occ` (doubled density!)

This caused the density to be **doubled**, which squared the energy discrepancy.

## Current Status
- **Energy**: -29.91 Ry (vs target -1.17 Ry)
- **Electron count**: Correctly 2.0 electrons
- **Density normalization**: Working correctly
- **Next issue**: Still need to identify why energy is ~25x too large

## Next Debugging Steps - Energy Component Analysis
**Goal**: Compare individual energy components between Band and PSPW to identify which term is scaled incorrectly.

### Step 1: Kinetic Energy Investigation
1. **Extract kinetic energy** from both Band and PSPW H₂ calculations
2. **Compare values** - identify if kinetic energy is the source of the 25x scaling
3. **Check kinetic operator implementation** - verify no volume normalization issues

### Step 2: Hartree Energy Investigation  
1. **Extract Hartree energy** from both calculations
2. **Compare values** - check if Coulomb interaction is scaled incorrectly
3. **Verify charge density normalization** - ensure ρ(r) is properly normalized

### Step 3: Exchange-Correlation Energy Investigation
1. **Extract XC energy** from both calculations  
2. **Compare values** - check if XC functional is scaled incorrectly
3. **Verify density input to XC** - ensure ρ(r) passed to XC is correct

### Step 4: Non-local Potential Energy Investigation
1. **Extract non-local energy** from both calculations
2. **Compare values** - check if pseudopotential terms are scaled incorrectly
3. **Verify wavefunction normalization** - ensure ψ is properly normalized

## Key Questions to Answer
- Which energy component is causing the ~25x scaling?
- Are there remaining volume normalization factors in energy calculations?
- Is there a difference in how Band vs PSPW handles the energy components?
- Are the wavefunctions properly normalized?

## Files to Investigate
- `PWDFT/Nwpw/band/lib/cElectron/cElectron.cpp` - energy component calculations
- `PWDFT/Nwpw/pspw/lib/electron/Electron.cpp` - PSPW comparison
- `PWDFT/tests/tier1/2.4.1_H2_band/h2_band.nw` - input parameters 

---

## 2025-07-31: Energy Component Extraction & Discrepancy Analysis

### Summary of Actions
- Extracted individual energy components (Kinetic, Hartree, XC, Non-local, etc.) from both Band and PSPW H₂ runs using output and JSON files.
- Tabulated all values in Ry and eV, computed ratios, and identified the largest discrepancies.
- **Key finding:** The Band module's Total Orbital and Kinetic energies are overestimated by ~25–40× compared to PSPW, while Ion-Ion energy matches exactly. Hartree, V_local, V_nl, and V_Coul are near zero in Band but significant in PSPW.
- All extraction and comparison scripts are in `energy_component_analysis.py`.

### Table: Side-by-Side Energy Component Comparison

| Component      | Band (Ry) | Band (eV) | PSPW (Ry) | PSPW (eV) | Ratio  |
| -------------- | --------- | --------- | --------- | --------- | ------ |
| Kinetic        | 30.095    | 409.47    | 1.062     | 14.44     | 28.35  |
| Hartree        | 0.000345  | 0.0047    | 0.7555    | 10.28     | 0.00   |
| Exchange-Corr. | -0.02916  | -0.397    | -0.6821   | -9.28     | 0.04   |
| Non-local      | -0.000364 | -0.00495  | -0.32196  | -4.38     | 0.00   |
| Total Orbital  | -30.064   | -409.04   | -0.7569   | -10.30    | 39.72  |
| Total Energy   | -29.906   | -406.89   | -1.167    | -15.88    | 25.62  |
| Ion-Ion        | 0.15091   | 2.05      | 0.15091   | 2.05      | 1.00   |
| V_local        | 0.000642  | 0.0087    | -2.1315   | -29.00    | 0.00   |
| V_Coul         | 0.000689  | 0.0094    | 1.5110    | 20.56     | 0.00   |
| V_xc           | -0.03715  | -0.505    | -0.8762   | -11.92    | 0.04   |

**Unit conversion:** 1 Ry = 13.6057 eV

#### Extraction commands (example):
```bash
grep -A10 "energy results" h2_band.out
grep -A10 "energy results" h2_energy.out
# Or parse h2-energy.json for precise values
```

### Discrepancy Analysis
- **Largest discrepancy:** Total Orbital Energy (39.7×), Kinetic Energy (28.4×), and Total Energy (25.6×).
- **Ion-Ion energy matches exactly** (1.0×), confirming the issue is not in the Ewald/ion-ion term.
- **Hartree, V_local, V_nl, V_Coul are near zero in Band** but significant in PSPW, suggesting a missing or mis-scaled contribution in Band.

### Next Steps
- **Primary culprit:** The orbital and kinetic energy terms in Band are overestimated by ~25–40×.
- **Immediate action:** Focus on the calculation and normalization of the kinetic and orbital energy in `cElectron.cpp` and the corresponding routines in PSPW.
- **Check for:** 
  - Volume normalization issues in the kinetic/orbital energy routines.
  - Differences in wavefunction normalization and occupation number handling.
  - Whether the potential energy terms are being computed and reported correctly in Band.

---

## Prompt for Codepath Audit (Template)

**Objective:**
Perform a detailed codepath audit of the Band and PSPW energy routines, focusing on the calculation and normalization of kinetic and orbital energy components. Use the previous module audit structure and the diagrams in `band_implemented.json` and `pspw_implemented.json` as a template for documenting the flow.

**Prompt:**

---

**Codepath Audit Prompt**

You are tasked with performing a codepath audit of the Band and PSPW energy routines for H₂, with the goal of pinpointing the source of the ~25–40× energy discrepancy in the Band module's kinetic and orbital energy terms. Your audit should:

1. **Trace the full computational flow** for energy evaluation in both Band and PSPW, from input parsing through to the final energy output, with special attention to:
   - Kinetic energy calculation
   - Orbital energy calculation
   - Density and wavefunction normalization
   - Volume and occupation number handling
2. **Document each major step** using a flowchart or structured list, referencing the relevant functions, files, and (if possible) MPI/data movement.
3. **Highlight all points where normalization, scaling, or summation occurs** (e.g., per-cell, per-electron, per-spin, per-k-point, per-volume).
4. **Compare the Band and PSPW flows side-by-side**, noting any deviations, missing steps, or extra scaling factors.
5. **Reference the previous module audits** and the diagrams in `band_implemented.json` and `pspw_implemented.json` for structure and completeness.
6. **Conclude with a prioritized list of code locations and hypotheses** most likely to explain the observed discrepancy.

**Resources:**
- `band_implemented.json` (Band module flowchart and node details)
- `pspw_implemented.json` (PSPW module flowchart and node details)
- Previous audit notes (see BAND_ENERGY_DEBUG_FINDINGS.md)
- Source files: `Nwpw/band/lib/cElectron/cElectron.cpp`, `Nwpw/pspw/lib/electron/Electron.cpp`, and related routines

**Deliverable:**
- A Markdown section or document containing:
  - A flowchart or structured list for each module
  - Annotations for all normalization/scaling points
  - A side-by-side comparison table or diagram
  - A short narrative summary of likely root causes and next investigation steps

---

**Use the above as your starting template for the next codepath audit.** 