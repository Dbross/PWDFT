# Band Module Density Integration & Normalization: Findings and Next Steps

## Findings

### 1. Density Generation (`gen_density`)
- Computes real-space electron density from wavefunctions (`psi_r`).
- Uses occupation numbers if provided: `mygrid->hr_aSumSqr_occ(scal2, occ, psi_r, dn);` with `scal2 = 1.0 / omega`.
- Integrates density as: `sum(dn[i] * omega / (nx*ny*nz))` (i.e., sum over grid, times grid spacing).

### 2. Normalization
- After initial density calculation, checks if integrated density matches expected electron count.
- If not, rescales wavefunction by `sqrt(expected_electrons / rho_check)` and recomputes density (up to 10 iterations).
- Comment: "Density is already calculated with occupation numbers, so no need to multiply by 2.0".

### 3. Potential Issues
- Output shows integrated electron count ~0.27 (should be 2.0 for H₂), and kinetic energy ~60x too large.
- Suggests normalization or occupation error remains, or normalization loop is not being triggered/converging.

### 4. Comparison to PSPW
- PSPW uses similar logic, but produces correct electron count and energies.
- Band's normalization or occupation handling may differ in subtle ways for ispin=1 (singlet) cases.

---

## Next Actions (for next prompt)

1. **Add debug print** in `gen_density` to always output integrated electron density.
2. **Check normalization loop** in `Solid.cpp` to ensure it triggers and converges to correct electron count.
3. **Compare occupation and normalization logic** between Band and PSPW, especially for ispin=1.
4. **If normalization fails,** force a single normalization step and print before/after integrated density and scaling factor.
5. **If density is still wrong,** check `hr_aSumSqr_occ` for indexing/scaling errors.

---

*Prepared by: PWDFT Band/PSPW Debug Agent, 2025-07-30* 