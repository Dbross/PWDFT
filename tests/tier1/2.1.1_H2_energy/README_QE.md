# Quantum ESPRESSO H2 Test

This directory contains an equivalent H2 molecular test for Quantum ESPRESSO (QE) that matches the PWDFT test parameters.

## Files

- `h2_energy.qe.in` - QE input file
- `run_qe.sh` - Script to run the QE calculation
- `validate_qe.sh` - Script to validate QE results
- `README_QE.md` - This file

## Parameter Mapping

| PWDFT Parameter | QE Parameter | Value | Notes |
|----------------|--------------|-------|-------|
| `cutoff 40.0` | `ecutwfc = 40.0` | 40.0 Ry | Wavefunction cutoff |
| `xc pbe` | `input_dft = 'PBE'` | PBE | Exchange-correlation functional |
| `scf_alpha 0.25` | `mixing_beta = 0.25` | 0.25 | Mixing parameter |
| `scf_beta 0.1` | N/A | - | QE uses single mixing parameter |
| `diis_histories 10` | `mixing_ndim = 10` | 10 | DIIS history length |
| `loop 10 100` | `electron_maxstep = 100` | 100 | Maximum SCF iterations |
| `tolerances 1.0e-7 1.0e-7 1.0e-4` | `conv_thr = 1.0d-7` | 1e-7 | Energy convergence |
| | `conv_thr_ion = 1.0d-4` | 1e-4 | Force convergence |
| `simulation_cell SC 10.0` | `celldm(1) = 18.8973` | 10.0 Å | Supercell size (converted to Bohr) |

## Geometry

Both calculations use identical H2 geometry:
- Bond length: 0.74 Å (0.3705 Å from center)
- Atoms: H at (0, 0, ±0.3705) Å

## Running the Test

```bash
# Run QE calculation
./run_qe.sh

# Validate results
./validate_qe.sh
```

## Requirements

- Quantum ESPRESSO installed with `pw.x` executable in PATH
- H pseudopotential file: `H.pbe-rrkjus_psl.1.0.0.UPF`
- Pseudopotential directory: `/global/homes/b/brossdh/espresso/pseudo`

## Expected Results

- Final energy: ~-1.17 Ry
- Tolerance: ±0.0001 Ry (1e-4)
- Both PWDFT and QE should give similar results within tolerance

## Notes

- QE uses Gaussian smearing (degauss = 0.01 Ry) for numerical stability
- QE uses `assume_isolated = 'molecule'` for molecular calculations
- The QE input uses the same convergence criteria as PWDFT 