Canonical Workflows for PWDFT Modules
This document details the canonical workflows for the pspw and BAND modules, as specified in the formal graph definitions.

pspw Module Workflow (Gamma-Point Only)
This workflow describes the self-consistent field (SCF) procedure for systems where Brillouin zone sampling is restricted to the Gamma point (k=0).

Mermaid Flowchartmermaid
flowchart TD
subgraph Initialization
A[init_psi_random] --> B[ortho_initial];
end

subgraph SCF_Loop
    B --> D(ifft_psi_batch);
    D --> E(sum_orbitals_sq_local);
    E --> F(mpi_allreduce_density);

    subgraph Build Potential V_eff(r)
        F --> G(fft_density);
        G --> H(calc_hartree_pot_g);
        H --> I(ifft_hartree_pot);
        F --> J(calc_xc_pot_r);
        I & J --> K(assemble_potential);
    end

    subgraph Apply Hamiltonian Hψ
        K & D --> L(apply_H_realspace);
        L --> M(fft_H_psi_batch);
        B --> N(apply_H_gspace);
        B --> O(apply_nonlocal_pseudo);
        M & N & O --> P(assemble_H_psi);
    end

    subgraph Update and Orthogonalize
        P & B --> Q(eigensolver_step);
        Q --> R(ortho_iter);
    end

    subgraph Convergence Check and Mixing
        R --> S{check_convergence};
        S -- No --> T(mix_density);
        T --> F;
        S -- Yes --> U[End];
    end
    R --> Loop_Entry{{Next Iteration}};
    Loop_Entry --> D;
end

### Detailed Workflow Graph

*   **Node: `init_psi_random` (Generate Random Wavefunctions)**
    *   **Type:** `compute`
    *   **Description:** Initialize ψn(G) with random complex numbers for all N_e orbitals.
    *   **Source:** [1, 2]

*   **Node: `ortho_initial` (Initial Orthogonalization)**
    *   **Type:** `compute`
    *   **Description:** Enforce orthonormality on the initial random wavefunctions.
    *   **Source:** [3], Sec 4.2

*   **Node: `ifft_psi_batch` (IFFT Wavefunctions)**
    *   **Type:** `compute`
    *   **Description:** Batch inverse FFT from G-space to real-space: ψn(G) -> ψn(r).
    *   **Source:** [3], Sec 4.1

*   **Node: `sum_orbitals_sq_local` (Compute Local Density)**
    *   **Type:** `compute`
    *   **Description:** On each MPI rank, compute ρ_local(r) = Σ|ψn(r)|² for local orbitals.
    *   **Source:** [3], Eq. 8

*   **Node: `mpi_allreduce_density` (MPI Allreduce Density)**
    *   **Type:** `mpi`
    *   **Description:** Sum partial densities from all ranks to get total density ρ(r).
    *   **Source:** [3]

*   **Node: `mix_density` (Mix Density)**
    *   **Type:** `compute`
    *   **Description:** Mix input and output densities for next iteration using Pulay/Broyden scheme.
    *   **Source:** [4, 5]

*   **Node: `fft_density` (FFT Density)**
    *   **Type:** `compute`
    *   **Description:** Forward FFT of total density: ρ(r) -> ρ(G).
    *   **Source:** [3], Eq. 13

*   **Node: `calc_hartree_pot_g` (Calculate Hartree Potential (G-space))**
    *   **Type:** `compute`
    *   **Description:** V_H(G) = 4π/|G|² * ρ(G).
    *   **Source:** [3], Eq. 21

*   **Node: `ifft_hartree_pot` (IFFT Hartree Potential)**
    *   **Type:** `compute`
    *   **Description:** Inverse FFT: V_H(G) -> V_H(r).
    *   **Source:** [3], Eq. 63

*   **Node: `calc_xc_pot_r` (Calculate XC Potential (R-space))**
    *   **Type:** `compute`
    *   **Description:** V_xc(r) = δExc/δρ. Depends on ρ(r) and |∇ρ(r)|.
    *   **Source:** [3], Eq. 65

*   **Node: `assemble_potential` (Assemble Effective Potential)**
    *   **Type:** `compute`
    *   **Description:** V_eff(r) = V_H(r) + V_xc(r) + V_local(r).
    *   **Source:** [3], Eq. 63

*   **Node: `apply_H_realspace` (Apply Real-Space Hamiltonian)**
    *   **Type:** `compute`
    *   **Description:** Calculate V_eff(r) * ψn(r) for all N_e orbitals.
    *   **Source:** [3], Eq. 63

*   **Node: `fft_H_psi_batch` (FFT Vψ Batch)**
    *   **Type:** `compute`
    *   **Description:** Forward FFT of Vψ products: (Vψ)n(r) -> (Vψ)n(G).
    *   **Source:** [3], Sec 4.1

*   **Node: `apply_H_gspace` (Apply G-Space Hamiltonian (Kinetic))**
    *   **Type:** `compute`
    *   **Description:** Add kinetic energy term: Tψn(G) = (1/2)|G|² * ψn(G).
    *   **Source:** [3], Eq. 61

*   **Node: `apply_nonlocal_pseudo` (Apply Non-Local Pseudopotential)**
    *   **Type:** `compute`
    *   **Description:** Apply V_NL via matrix multiplications.
    *   **Source:** [3], Eq. 61, Sec 4.2

*   **Node: `assemble_H_psi` (Assemble Hψ)**
    *   **Type:** `compute`
    *   **Description:** Sum all Hamiltonian components: Hψ = Tψ + V_NLψ + Vψ.
    *   **Source:** [3], Eq. 60

*   **Node: `eigensolver_step` (Eigensolver Step)**
    *   **Type:** `compute`
    *   **Description:** Update wavefunctions using an iterative eigensolver (e.g., Davidson, CG).
    *   **Source:** [2, 6]

*   **Node: `ortho_iter` (Iterative Orthogonalization)**
    *   **Type:** `compute`
    *   **Description:** Re-orthogonalize the updated wavefunctions.
    *   **Source:** [3], Sec 4.2

*   **Node: `check_convergence` (Check Convergence)**
    *   **Type:** `control`
    *   **Description:** Calculate total energy and check if |ΔE| < threshold.
    *   **Source:** [7]

*   **Node: `end` (End)**
    *   **Type:** `control`
    *   **Description:** SCF converged, calculation finished.
    *   **Source:** N/A

---

## `BAND` Module Workflow (k-Point)

This workflow extends the `pspw` algorithm to crystalline, periodic systems by sampling the electronic states at multiple, discrete k-points in the Brillouin zone.

### Mermaid Flowchart

```mermaid
flowchart TD
    subgraph Initialization
        A[generate_kpoints] --> B(distribute_kpoints);
        B --> C(init_psi_k_random);
        C --> D(ortho_k_initial);
    end

    subgraph SCF_Loop
        subgraph Parallel k-point Loop (per pool)
            D --> E(ifft_psi_k_batch);
            E --> F(sum_orbitals_sq_k_local);
        end

        F --> G(mpi_allreduce_density_pools);

        subgraph Build Potential V_eff(r)
            G --> H(fft_density);
            H --> I(calc_hartree_pot_g);
            I --> J(ifft_hartree_pot);
            G --> K(calc_xc_pot_r);
            J & K --> L(assemble_potential_k);
        end

        subgraph Parallel k-point Loop (per pool)
            subgraph Apply Hamiltonian H_k ψ_nk
                L & E --> M(apply_H_k);
            end

            subgraph Update and Orthogonalize per k
                M --> N(eigensolver_step_k);
                N --> O(ortho_iter_k);
            end
        end

        subgraph Convergence Check and Mixing
            O --> P{check_convergence_k};
            P -- No --> Q(mix_density);
            Q --> G;
            P -- Yes --> R[End];
        end
        O --> Loop_Entry_k{{Next Iteration}};
        Loop_Entry_k --> D;
    end
Detailed Workflow Graph
Node: generate_kpoints (Generate k-point Mesh)

Type: compute

Description: Generate Monkhorst-Pack grid and reduce to IBZ.

Source: , Sec 2.1   

Node: distribute_kpoints (Distribute k-points)

Type: mpi

Description: Assign subsets of k-points to MPI pools.

Source:    

Node: init_psi_k_random (Generate Random Wavefunctions (k-dependent))

Type: compute

Description: Initialize ψ_nk(G) with random complex numbers for all n and local k.

Source:    

Node: ortho_k_initial (Initial Orthogonalization (k-dependent))

Type: compute

Description: Enforce orthonormality on initial random wavefunctions at each k-point.

Source: , Sec 4.2   

Node: ifft_psi_k_batch (IFFT Wavefunctions (k-dependent))

Type: compute

Description: Batch IFFT: ψ_nk(G) -> ψ_nk(r).

Source: , Sec 4.1   

Node: sum_orbitals_sq_k_local (Compute Pool Density)

Type: compute

Description: Compute ρ_pool(r) = Σ_k w_k Σ_n |ψ_nk(r)|² for local k-points.

Source:    

Node: mpi_allreduce_density_pools (MPI Allreduce Density (Pools))

Type: mpi

Description: Sum partial densities from all k-point pools to get total density ρ(r).

Source:    

Node: assemble_potential_k (Assemble Effective Potential)

Type: compute

Description: V_eff(r) = V_H(r) + V_xc(r) + V_local(r). This is k-independent.

Source: , Eq. 63   

Node: apply_H_k (Apply Hamiltonian (k-dependent))

Type: compute

Description: Compute H_kψ_nk = (T_k + V_NL + V_eff)ψ_nk.

Source:    

Node: eigensolver_step_k (Eigensolver Step (k-dependent))

Type: compute

Description: Update wavefunctions at each k using an iterative eigensolver.

Source:    

Node: ortho_iter_k (Iterative Orthogonalization (k-dependent))

Type: compute

Description: Re-orthogonalize updated wavefunctions at each k.

Source: , Sec 4.2   

Node: check_convergence_k (Check Convergence)

Type: control

Description: Calculate total energy and check if |ΔE| < threshold.

Source:    

Node: end_k (End)

Type: control

Description: SCF converged, calculation finished.

Source: N/A
