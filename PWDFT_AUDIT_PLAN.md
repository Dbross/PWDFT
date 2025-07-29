# PWDFT Codebase Audit Plan of Action

## Executive Summary

This document outlines a comprehensive four-phase audit plan to systematically compare the actual implemented workflow in the PWDFT codebase against the canonical workflow prescribed by the reference book chapter. The audit will focus on both the **pspw** (Gamma-point) and **BAND** (k-point) modules, producing detailed implementation maps that highlight deviations and propose verification protocols.

## Phase 1: Code Path Tracing & Analysis

### 1.1 Primary Execution Path Identification

**Objective**: Trace the complete SCF execution path for both pspw and BAND modules.

**Methodology**:
- **Entry Point Analysis**: 
  - `Nwpw/nwpw.cpp` - Main entry point with Fortran interfaces
  - `Nwpw/pspw/cpsd/cpsd.cpp` - PSPW SCF driver
  - `Nwpw/band/cpsd/band_cpsd.cpp` - BAND SCF driver

- **SCF Loop Structure**:
  - **PSPW**: `inner_loop()` in `Nwpw/pspw/cpsd/inner_loop.cpp`
  - **BAND**: `band_inner_loop()` in `Nwpw/band/cpsd/band_inner_loop.cpp`

### 1.2 Key Algorithmic Stage Mapping

#### Initialization Phase
- **Wavefunction Guess**: 
  - File: `Nwpw/pspw/lib/psi/psi.cpp` (PSPW)
  - File: `Nwpw/band/lib/cpsi/cpsi.cpp` (BAND)
  - Function: Random initialization with orthogonalization

- **K-point Generation** (BAND only):
  - File: `Nwpw/band/lib/solid/Solid.cpp`
  - Function: Monkhorst-Pack grid generation and IBZ reduction

#### Electron Density Construction
- **FFT Operations**:
  - File: `Nwpw/nwpwlib/` (FFT library components)
  - Functions: `gh_fftb()` (G→r), `rc_pfft3f()` (r→G)
  - MPI: Parallel FFT with domain decomposition

- **Orbital Summation**:
  - PSPW: `hr_aSumSqr()` in `inner_loop.cpp:130`
  - BAND: `hr_aSumSqr()` in `band_inner_loop.cpp:130`
  - MPI: Local density computation followed by `MPI_Allreduce`

#### Effective Potential Construction
- **Hartree Potential**:
  - File: `Nwpw/pspw/lib/coulomb/` (PSPW)
  - File: `Nwpw/band/lib/cCoulomb/` (BAND)
  - Function: `vcoulomb()` with FFT-based Poisson solver

- **Exchange-Correlation**:
  - File: `Nwpw/pspw/lib/exchange-correlation/` (PSPW)
  - File: `Nwpw/band/lib/cExchange-Correlation/` (BAND)
  - Function: `v_exc_all()` for real-space XC potential

- **Local Pseudopotential**:
  - File: `Nwpw/pspw/lib/psp/` (PSPW)
  - File: `Nwpw/band/lib/cpsp/` (BAND)
  - Function: `v_local()` for local potential contribution

#### Hamiltonian Application
- **Kinetic Energy**:
  - PSPW: `Kinetic_Operator::ke()` in `Nwpw/pspw/lib/kinetic/`
  - BAND: `cKinetic_Operator::ke()` in `Nwpw/band/lib/cKinetic/`
  - Implementation: G-space kinetic energy operator

- **Non-local Pseudopotential**:
  - Function: `v_nonlocal_fion()` for projector-based non-local terms
  - MPI: Parallel matrix operations with load balancing

- **Real-space Potential**:
  - Function: `psi_H()` and `psi_Hv4()` for real-space Hamiltonian application
  - FFT: Real-space multiplication followed by FFT to G-space

#### Wavefunction Update
- **Eigensolver**:
  - PSPW: Steepest descent with Lagrange multipliers
  - BAND: Conjugate gradient with geodesic line search
  - Functions: `ggm_lambda()` for orthogonalization

- **Orthogonalization**:
  - PSPW: `g_ortho()` for Gram-Schmidt orthogonalization
  - BAND: `g_project_out_filled()` and `g_project_out_virtual()`

#### Convergence Check
- **Energy Calculation**:
  - File: `inner_loop.cpp:250-300` (PSPW)
  - File: `band_inner_loop.cpp:400-450` (BAND)
  - Metrics: `deltae`, `deltac`, `deltar`

- **Density Mixing**:
  - File: `Nwpw/nwpwlib/utilities/nwpw_scf_mixing.hpp`
  - Algorithms: Simple, Broyden, DIIS, Local-TF

### 1.3 MPI Communication Points

**Critical MPI Operations**:
- `MPI_Allreduce` for density summation across ranks
- `MPI_Bcast` for k-point distribution in BAND
- `MPI_Allgather` for wavefunction synchronization
- Domain decomposition for FFT operations

## Phase 2: Mapping Implementation to Canonical Workflow

### 2.1 PSPW Implementation Map

**Canonical Steps → Implementation Mapping**:

1. **init_psi_random**:
   - File: `Nwpw/pspw/lib/psi/psi.cpp`
   - Function: `psi_randomize()`
   - MPI: None (local operation)

2. **orthogonalize_psi**:
   - File: `Nwpw/pspw/lib/psi/psi.cpp`
   - Function: `g_ortho()`
   - MPI: `MPI_Allreduce` for overlap matrix

3. **ifft_psi_batch**:
   - File: `Nwpw/pspw/cpsd/inner_loop.cpp:115`
   - Function: `gh_fftb()`
   - Library: FFTW or internal FFT

4. **sum_orbitals_sq_local**:
   - File: `Nwpw/pspw/cpsd/inner_loop.cpp:130`
   - Function: `hr_aSumSqr()`
   - MPI: Local computation

5. **mpi_allreduce_density**:
   - File: `Nwpw/pspw/cpsd/inner_loop.cpp:135`
   - Function: `rrr_Sum()` followed by `MPI_Allreduce`
   - MPI: `MPI_Allreduce` for global density

6. **fft_density**:
   - File: `Nwpw/pspw/cpsd/inner_loop.cpp:140`
   - Function: `rc_pfft3f()`
   - Library: FFTW

7. **calc_hartree_pot_g**:
   - File: `Nwpw/pspw/lib/coulomb/`
   - Function: `vcoulomb()`
   - Implementation: `4π/|G|² * ρ(G)`

8. **ifft_hartree_pot**:
   - File: `Nwpw/pspw/lib/coulomb/`
   - Function: `vcoulomb()` (includes IFFT)
   - Library: FFTW

9. **calc_xc_pot_r**:
   - File: `Nwpw/pspw/lib/exchange-correlation/`
   - Function: `v_exc_all()`
   - Implementation: Real-space XC functional evaluation

10. **assemble_potential**:
    - File: `Nwpw/pspw/cpsd/inner_loop.cpp:200`
    - Function: `psi_H()` or `psi_Hv4()`
    - Implementation: `V_eff = V_H + V_xc + V_local`

11. **apply_H_realspace**:
    - File: `Nwpw/pspw/cpsd/inner_loop.cpp:205`
    - Function: `psi_H()` (real-space multiplication)
    - Implementation: `V_eff(r) * ψ(r)`

12. **fft_H_psi_batch**:
    - File: `Nwpw/pspw/cpsd/inner_loop.cpp:205`
    - Function: `psi_H()` (includes FFT)
    - Library: FFTW

13. **apply_H_gspace**:
    - File: `Nwpw/pspw/lib/kinetic/`
    - Function: `ke()`
    - Implementation: `(1/2)|G|² * ψ(G)`

14. **apply_nonlocal_pseudo**:
    - File: `Nwpw/pspw/lib/psp/`
    - Function: `v_nonlocal_fion()`
    - Implementation: Projector-based non-local potential

15. **assemble_H_psi**:
    - File: `Nwpw/pspw/cpsd/inner_loop.cpp:205`
    - Function: `psi_H()`
    - Implementation: `Hψ = Tψ + V_NLψ + Vψ`

16. **eigensolver_step**:
    - File: `Nwpw/pspw/cpsd/inner_loop.cpp:215`
    - Function: Steepest descent with `gg_SMul()` and `gg_Sum2()`
    - Implementation: `ψ_new = ψ_old + α * Hψ`

17. **orthogonalize_psi**:
    - File: `Nwpw/pspw/cpsd/inner_loop.cpp:225`
    - Function: `ggm_lambda()`
    - Implementation: Lagrange multiplier orthogonalization

18. **calc_energy_and_check_convergence**:
    - File: `Nwpw/pspw/cpsd/inner_loop.cpp:250-300`
    - Function: Energy calculation and convergence check
    - Metrics: `deltae`, `deltac`, `deltar`

19. **mix_density**:
    - File: `Nwpw/nwpwlib/utilities/nwpw_scf_mixing.hpp`
    - Function: `mix()`
    - Algorithms: Simple, Broyden, DIIS, Local-TF

### 2.2 BAND Implementation Map

**Canonical Steps → Implementation Mapping**:

1. **generate_kpoints**:
   - File: `Nwpw/band/lib/solid/Solid.cpp`
   - Function: Monkhorst-Pack grid generation
   - Implementation: IBZ reduction with symmetry

2. **distribute_kpoints**:
   - File: `Nwpw/band/lib/solid/Solid.cpp`
   - Function: K-point distribution across MPI pools
   - MPI: `MPI_Bcast` and `MPI_Scatter`

3. **init_psi_k_random**:
   - File: `Nwpw/band/lib/cpsi/cpsi.cpp`
   - Function: `psi_randomize()` (k-dependent)
   - MPI: Local operation per k-point

4. **ortho_k_initial**:
   - File: `Nwpw/band/lib/cpsi/cpsi.cpp`
   - Function: `g_ortho()` (k-dependent)
   - MPI: `MPI_Allreduce` per k-point

5. **ifft_psi_k_batch**:
   - File: `Nwpw/band/cpsd/band_inner_loop.cpp:115`
   - Function: `gh_fftb()` (k-dependent)
   - Library: FFTW

6. **sum_orbitals_sq_k_local**:
   - File: `Nwpw/band/cpsd/band_inner_loop.cpp:130`
   - Function: `hr_aSumSqr()` (k-weighted)
   - Implementation: `ρ_pool = Σ_k w_k Σ_n |ψ_nk|²`

7. **mpi_allreduce_density_pools**:
   - File: `Nwpw/band/cpsd/band_inner_loop.cpp:135`
   - Function: `MPI_Allreduce` across k-point pools
   - MPI: Pool-level reduction

8. **assemble_potential_k**:
   - File: `Nwpw/band/cpsd/band_inner_loop.cpp:200`
   - Function: `psi_H()` (k-independent potential)
   - Implementation: Same as PSPW (k-independent)

9. **apply_H_k**:
   - File: `Nwpw/band/cpsd/band_inner_loop.cpp:205`
   - Function: `psi_H()` (k-dependent kinetic energy)
   - Implementation: `H_k = T_k + V_NL + V_eff`

10. **eigensolver_step_k**:
    - File: `Nwpw/band/minimizer/band_cgsd_cgminimize.cpp`
    - Function: Conjugate gradient with geodesic line search
    - Implementation: Advanced optimization algorithm

11. **ortho_iter_k**:
    - File: `Nwpw/band/lib/cpsi/cpsi.cpp`
    - Function: `g_project_out_filled()` and `g_project_out_virtual()`
    - Implementation: Projection-based orthogonalization

12. **check_convergence_k**:
    - File: `Nwpw/band/cpsd/band_inner_loop.cpp:400-450`
    - Function: Energy calculation and convergence check
    - Metrics: Same as PSPW

## Phase 3: Deviation Identification and Analysis

### 3.1 Expected Deviations

#### Numerical Algorithm Deviations

1. **Advanced Density Mixing**:
   - **Canonical**: Simple mixing `ρ_n+1 = α ρ_in + (1-α) ρ_n`
   - **Implemented**: Multiple algorithms (DIIS, Broyden, Local-TF)
   - **Rationale**: Improved convergence for difficult systems
   - **Files**: `Nwpw/nwpwlib/utilities/nwpw_scf_mixing.hpp`

2. **Eigensolver Complexity**:
   - **Canonical**: Basic steepest descent
   - **Implemented**: 
     - PSPW: Steepest descent with Lagrange multipliers
     - BAND: Conjugate gradient with geodesic line search
   - **Rationale**: Better convergence properties for different system types

3. **Orthogonalization Methods**:
   - **Canonical**: Gram-Schmidt orthogonalization
   - **Implemented**: 
     - PSPW: Lagrange multiplier method
     - BAND: Projection-based orthogonalization
   - **Rationale**: Improved numerical stability

#### Parallelization Strategy Deviations

1. **MPI Density Reduction**:
   - **Canonical**: Implicit in serial equations
   - **Implemented**: Explicit `MPI_Allreduce` operations
   - **Rationale**: Parallel scalability requirement
   - **Files**: `inner_loop.cpp:135`, `band_inner_loop.cpp:135`

2. **K-point Parallelization** (BAND):
   - **Canonical**: Serial k-point loop
   - **Implemented**: MPI pool-based k-point distribution
   - **Rationale**: Parallel efficiency for large k-point meshes
   - **Files**: `Nwpw/band/lib/solid/Solid.cpp`

3. **Domain Decomposition**:
   - **Canonical**: Single domain
   - **Implemented**: FFT domain decomposition
   - **Rationale**: Memory and computational efficiency
   - **Files**: `Nwpw/nwpwlib/` (FFT components)

#### Performance Optimization Deviations

1. **External Library Usage**:
   - **Canonical**: Bespoke implementations
   - **Implemented**: FFTW, BLAS, ScaLAPACK
   - **Rationale**: Optimized, tested numerical libraries
   - **Files**: Various library interfaces

2. **Memory Management**:
   - **Canonical**: Simple array operations
   - **Implemented**: Sophisticated memory pools and caching
   - **Rationale**: Performance optimization for large systems
   - **Files**: `Nwpw/nwpwlib/` (memory management)

3. **Adaptive Algorithms**:
   - **Canonical**: Fixed parameters
   - **Implemented**: Adaptive thresholds and mixing
   - **Rationale**: Robust convergence across system types
   - **Files**: `Nwpw/nwpwlib/utilities/nwpw_scf_mixing.hpp`

### 3.2 Critical Deviation Analysis

#### Statefulness Bug Fixes
- **Issue**: Stateful objects not reset between iterations
- **Fix**: Explicit state reset in outer loop iterations
- **Files**: `cpsd.cpp:530`, `band_cpsd.cpp:470`
- **Impact**: Prevents convergence issues in multi-iteration runs

#### NaN/Inf Propagation Prevention
- **Issue**: Numerical instabilities in steepest descent
- **Fix**: Bounds checking and fallback mechanisms
- **Files**: `band_inner_loop.cpp:60-80`
- **Impact**: Robust numerical stability

## Phase 4: Verification Protocols

### 4.1 Physics-Based Smoke Tests

#### Density Conservation Test
**Purpose**: Verify MPI density reduction correctness
**Test**: `∫ ρ(r) d³r = N_electrons`
**Implementation**:
```cpp
double total_electrons = mygrid->rr_sum(dn) * dv;
assert(std::abs(total_electrons - nelec) < 1e-10);
```

#### Energy Conservation Test
**Purpose**: Verify energy calculation consistency
**Test**: `E_total = E_kinetic + E_Hartree + E_XC + E_ion`
**Implementation**:
```cpp
double E_calculated = E[1] + E[2] + E[3] + E[4];
assert(std::abs(E[0] - E_calculated) < 1e-12);
```

#### FFT Round-Trip Test
**Purpose**: Verify FFT library correctness
**Test**: `FFT(IFFT(ψ(G))) = ψ(G)`
**Implementation**:
```cpp
mygrid->gh_fftb(psi1, psi_r);  // G -> r
mygrid->gh_fftf(psi_r, psi2);  // r -> G
double error = mygrid->gg_dot(psi1, psi2);
assert(error < 1e-14);
```

#### Orthogonality Test
**Purpose**: Verify wavefunction orthogonality
**Test**: `⟨ψ_i|ψ_j⟩ = δ_ij`
**Implementation**:
```cpp
mygrid->ggm_sym_Multiply(psi1, psi1, overlap);
for(int i=0; i<neall; i++) {
    assert(std::abs(overlap[i*neall+i] - 1.0) < 1e-12);
}
```

### 4.2 Convergence Verification Tests

#### SCF Convergence Test
**Purpose**: Verify SCF loop convergence
**Test**: Monitor `deltae`, `deltac`, `deltar`
**Implementation**:
```cpp
bool converged = (std::abs(deltae) < tol_e) && 
                 (deltac < tol_c) && 
                 (deltar < tol_r);
```

#### K-point Convergence Test (BAND)
**Purpose**: Verify k-point sampling convergence
**Test**: Energy convergence with k-point mesh density
**Implementation**:
```cpp
// Compare energies for different k-point meshes
double E_coarse = calculate_energy(kmesh_coarse);
double E_fine = calculate_energy(kmesh_fine);
assert(std::abs(E_fine - E_coarse) < convergence_threshold);
```

### 4.3 Performance Verification Tests

#### MPI Scalability Test
**Purpose**: Verify parallel efficiency
**Test**: Speedup vs. number of MPI ranks
**Implementation**:
```cpp
double speedup = time_serial / time_parallel;
double efficiency = speedup / nranks;
assert(efficiency > 0.7);  // 70% parallel efficiency
```

#### Memory Usage Test
**Purpose**: Verify memory efficiency
**Test**: Memory usage vs. system size
**Implementation**:
```cpp
size_t memory_usage = get_memory_usage();
double memory_per_atom = memory_usage / natoms;
assert(memory_per_atom < max_memory_per_atom);
```

### 4.4 Numerical Stability Tests

#### NaN/Inf Detection Test
**Purpose**: Verify numerical stability
**Test**: Check for NaN/Inf in key arrays
**Implementation**:
```cpp
bool has_nan = check_nan_inf("psi1", psi1, size);
assert(!has_nan);
```

#### Energy Monotonicity Test
**Purpose**: Verify energy convergence behavior
**Test**: Energy should decrease or converge
**Implementation**:
```cpp
if (iteration > 1) {
    assert(E_current <= E_previous + energy_tolerance);
}
```

## Deliverables

### 4.1 Implementation Maps
- `pspw_implemented.json`: Complete PSPW implementation mapping
- `band_implemented.json`: Complete BAND implementation mapping

### 4.2 Deviation Documentation
- Detailed analysis of algorithmic deviations
- Rationale for each deviation
- Impact assessment on accuracy and performance

### 4.3 Verification Suite
- Automated test suite for all verification protocols
- Integration with existing PWDFT test framework
- Performance benchmarking scripts

### 4.4 Recommendations
- Optimization opportunities
- Potential improvements to canonical workflow
- Best practices for future development

## Timeline

- **Phase 1**: 2-3 weeks (Code path tracing)
- **Phase 2**: 2-3 weeks (Implementation mapping)
- **Phase 3**: 1-2 weeks (Deviation analysis)
- **Phase 4**: 1-2 weeks (Verification protocols)

**Total Estimated Duration**: 6-10 weeks

## Success Criteria

1. **Complete Mapping**: All canonical workflow steps mapped to implementation
2. **Deviation Documentation**: All significant deviations identified and rationalized
3. **Verification Coverage**: Test suite covers all critical functionality
4. **Performance Validation**: Parallel efficiency and numerical stability verified
5. **Documentation Quality**: Clear, maintainable documentation for future reference

This audit plan provides a systematic approach to understanding the relationship between the canonical PWDFT workflow and its actual implementation, ensuring both correctness and optimal performance. 