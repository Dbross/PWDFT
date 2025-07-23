Best Practices for PWDFT Calculations
====================================

Overview
--------

This guide provides system-specific recommendations for setting up reliable PWDFT calculations. These practices are based on extensive testing and experience with different types of materials and systems.

Gas-Phase Molecule Calculations
-------------------------------

**Supercell Construction:**

For isolated molecules, place them in a large periodic box to simulate gas-phase conditions:

* **Minimum vacuum spacing**: 10-15 Å between molecule and box edge in all directions
* **Box size**: Typically 20-30 Å cubic for small molecules
* **Rationale**: Prevents spurious interactions between periodic images

**K-Point Sampling:**

* **Use Gamma point only**: `monkhorst-pack = 1 1 1`
* **Rationale**: Electronic interactions in reciprocal space are negligible for isolated molecules

**Recommended Functionals:**

* **Standard GGA**: PBE for initial screening
* **Hybrid functionals**: PBE0, B3LYP for higher accuracy
* **Rationale**: Hybrids reduce self-interaction error important for molecular properties

**Exchange-Correlation Functionals:**
- ``pbe96``: PBE functional (recommended for most systems)
- ``hse06``: HSE06 hybrid functional (more accurate, slower)
- ``beef-vdw``: BEEF-vdW functional (includes van der Waals)

**Plane-Wave Cutoff:**
- **Molecules**: 15-20 Hartree 
- **Bulk solids**: 20-30 Hartree 
- **Surfaces**: 25-35 Hartree 
- **High accuracy**: 40-50 Hartree

**SCF Algorithms:**
- ``ks-grassmann-cg anderson alpha 0.15``: Conservative, stable
- ``ks-grassmann-cg pulay alpha 0.25``: Standard, balanced
- ``ks-grassmann-cg simple alpha 0.02``: Very conservative

Numerical Stability and NaN Detection
-----------------------------------

Automatic NaN Detection
~~~~~~~~~~~~~~~~~~~~~~

PWDFT includes robust NaN (Not-a-Number) detection and fallback recovery mechanisms that work automatically:

**Features:**
- **Automatic Detection**: NaN values are detected in energy computations, matrix operations, and trace functions
- **Fallback Recovery**: Automatic wavefunction reinitialization and SCF restart
- **Performance**: Minimal overhead (<5% total computational cost)
- **Transparency**: No user intervention required

**Monitoring:**
Watch for these messages in the output:

.. code-block:: text

   *** NaN/Inf or large energy detected in band SCF (minimizer 8). Failure 1/3
   *** Energy value: 1.000000e+10
   *** Continuing with current iteration (failure 1/3)

   *** Triggering fallback after 3 consecutive failures
   *** 15 steepest descent iterations performed for stabilization
   *** Energy stabilized, resetting failure counter

Troubleshooting Numerical Issues
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Common Causes of NaN Values:**
1. **Insufficient k-point sampling** for metallic systems
2. **Too aggressive SCF mixing** parameters
3. **Inadequate smearing** for metallic systems
4. **Poor initial wavefunction guess**

**Solutions:**

.. code-block:: bash

   # For metallic systems
   nwpw
     smear methfessel-paxton
     temperature 300-500
     scf ks-grassmann-cg anderson alpha 0.15  # Conservative mixing
     monkhorst-pack 6 6 6  # Dense k-point sampling
   end

   # For challenging systems
   nwpw
     scf ks-grassmann-cg simple alpha 0.02  # Very conservative
     initial_wavefunction_guess superposition
     loop 50 50  # More iterations
   end

**Debugging Commands:**

.. code-block:: bash

   # Check for NaN detection messages
   grep "NaN/Inf detected" output.log

   # Check for fallback activity
   grep "Triggering fallback" output.log

   # Check for energy stabilization
   grep "Energy stabilized" output.log

SCF Convergence
--------------

Convergence Criteria
~~~~~~~~~~~~~~~~~~~

**Energy tolerance**: 1e-5 to 1e-6 Hartree
**Density tolerance**: 1e-5 to 1e-6
**Maximum iterations**: 20-50 for most systems

Convergence Strategies
~~~~~~~~~~~~~~~~~~~~~

**Conservative Approach (Recommended for new systems):**

.. code-block:: bash

   nwpw
     scf ks-grassmann-cg anderson alpha 0.15
     smear methfessel-paxton
     temperature 300
     loop 20 20
     tolerances 1e-5 1e-5 1e-4
   end

**Aggressive Approach (for well-behaved systems):**

.. code-block:: bash

   nwpw
     scf ks-grassmann-cg pulay alpha 0.25
     smear methfessel-paxton
     temperature 500
     loop 10 10
     tolerances 1e-6 1e-6 1e-5
   end

Convergence Troubleshooting
~~~~~~~~~~~~~~~~~~~~~~~~~~

**If SCF doesn't converge:**
1. **Reduce mixing parameter**: ``alpha 0.1`` or ``alpha 0.05``
2. **Increase smearing**: ``temperature 500`` or ``temperature 1000``
3. **Use simpler mixing**: ``scf ks-grassmann-cg simple alpha 0.02``
4. **Increase iterations**: ``loop 50 50``
5. **Try different initial guess**: ``initial_wavefunction_guess random``

K-Point Sampling
---------------

Guidelines by System Type
~~~~~~~~~~~~~~~~~~~~~~~~

**Molecules (isolated):**

.. code-block:: bash

   monkhorst-pack 1 1 1  # Gamma point only

**Bulk solids:**

.. code-block:: bash

   monkhorst-pack 4 4 4  # Standard
   monkhorst-pack 6 6 6  # High accuracy
   monkhorst-pack 8 8 8  # Very high accuracy

**Surfaces (2D systems):**

.. code-block:: bash

   monkhorst-pack 4 4 1  # Standard
   monkhorst-pack 6 6 1  # High accuracy

**Wires (1D systems):**

.. code-block:: bash

   monkhorst-pack 4 1 1  # Standard
   monkhorst-pack 6 1 1  # High accuracy

Convergence Testing
~~~~~~~~~~~~~~~~~~

Always test k-point convergence:

.. code-block:: bash

   # Test different k-point meshes
   monkhorst-pack 2 2 2
   monkhorst-pack 4 4 4
   monkhorst-pack 6 6 6
   monkhorst-pack 8 8 8

   # Compare total energies
   grep "Total energy" output*.log

Pseudopotentials
---------------

Selection Guidelines
~~~~~~~~~~~~~~~~~~~

**Norm-Conserving Pseudopotentials:**
- **Accuracy**: Good for most applications
- **Speed**: Fastest option
- **Memory**: Low memory usage
- **Examples**: HGH, Troullier-Martins

**Ultrasoft Pseudopotentials:**
- **Accuracy**: Very good
- **Speed**: Moderate
- **Memory**: Moderate
- **Examples**: Vanderbilt, Rappe-Rabe-Kaxiras-Joannopoulos

**PAW (Projector Augmented Wave):**
- **Accuracy**: Best
- **Speed**: Slowest
- **Memory**: Highest
- **Examples**: JTH, PSLibrary

Recommended Settings by Element
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Light elements (H, He, Li, Be, B, C, N, O, F, Ne):**
- Use PAW or high-quality norm-conserving
- Higher cutoff (60-80 Hartree)

**Transition metals (Fe, Co, Ni, Cu, etc.):**
- Use PAW for highest accuracy
- Include semicore states if needed
- Higher cutoff (60-80 Hartree)

**Heavy elements (Au, Pt, etc.):**
- Use PAW with relativistic corrections
- Very high cutoff (80-100 Hartree)

Memory Management
----------------

Memory Requirements
~~~~~~~~~~~~~~~~~~

**Estimate memory usage:**
- **Small systems** (<50 atoms): 1-4 GB
- **Medium systems** (50-200 atoms): 4-16 GB
- **Large systems** (>200 atoms): 16+ GB

**Memory optimization:**

.. code-block:: bash

   # Reduce memory usage
   nwpw
     memory 4000 mb  # Explicit memory limit
     parallel 4      # Reduce parallelization
   end

   # For very large systems
   nwpw
     memory 16000 mb
     parallel 8
     scratch_dir /tmp/pwdft_scratch  # Fast scratch directory
   end

Performance Optimization
-----------------------

Parallelization
~~~~~~~~~~~~~~

**CPU parallelization:**

.. code-block:: bash

   # Use all available cores
   mpirun -np 8 pwdft input.nw

   # Hybrid MPI/OpenMP
   export OMP_NUM_THREADS=4
   mpirun -np 2 pwdft input.nw
```

**GPU acceleration (if available):**

.. code-block:: bash

   # CUDA
   nwpw
     cuda on
     cuda_arch sm_70  # Set appropriate architecture
   end

   # HIP (AMD)
   nwpw
     hip on
     hip_arch gfx90a
   end

I/O Optimization
~~~~~~~~~~~~~~~

**Use fast scratch directories:**

.. code-block:: bash

   nwpw
     scratch_dir /tmp/pwdft_scratch
     permanent_dir ./results
   end

**Reduce I/O for large calculations:**

.. code-block:: bash

   nwpw
     print low        # Minimal output
     dplot off        # Disable density plotting
   end

Troubleshooting
--------------

Common Issues
~~~~~~~~~~~~

**SCF divergence:**
- Reduce mixing parameter
- Increase smearing
- Use more conservative parameters

**Memory issues:**
- Reduce parallelization
- Use smaller k-point grids
- Set explicit memory limits

**Slow performance:**
- Check parallelization settings
- Use faster scratch directories
- Optimize k-point sampling

**NaN errors:**
- Use conservative parameters
- Check system setup
- Monitor for automatic fallback

Debugging Commands
~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Check SCF convergence
   grep "tolerance ok" output.log

   # Check for errors
   grep -i "error\|warning\|failed" output.log

   # Check timing
   grep "cputime" output.log

   # Check memory usage
   grep "memory" output.log

   # Check for NaN detection
   grep "NaN/Inf detected" output.log

   # Check for fallback activity
   grep "Triggering fallback" output.log

Validation
----------

Energy Convergence
~~~~~~~~~~~~~~~~~

**Test energy convergence with respect to:**
1. **Plane-wave cutoff**: 30, 40, 50, 60, 70 Hartree
2. **K-point sampling**: 2×2×2, 4×4×4, 6×6×6, 8×8×8 -- note for 2d periodic systems to include gamma point the grid should be odd e.g. 3x3x1
3. **SCF tolerance**: 1e-4, 1e-5, 1e-6

Physical Checks
~~~~~~~~~~~~~~

**Verify results are physically reasonable:**
- **Total energy**: Should be negative and reasonable magnitude
- **Forces**: Should be small (< 0.01 eV/Å) for optimized structures
- **Band gap**: Should match expected values for the material
- **Density**: Should be smooth and positive everywhere

Comparison with Reference
~~~~~~~~~~~~~~~~~~~~~~~~

**Compare with:**
- **Experimental data**: Lattice constants, band gaps, etc.
- **Other codes**: VASP, Quantum ESPRESSO, etc.
- **Literature**: Published DFT results

Best Practices Summary
---------------------

1. **Start Conservative**: Use conservative parameters for new systems
2. **Test Convergence**: Always test convergence systematically
3. **Monitor for Issues**: Watch for NaN detection and fallback messages
4. **Validate Results**: Compare with experimental or reference data
5. **Document Parameters**: Keep detailed records of calculations
6. **Use Appropriate Resources**: Match computational resources to system size
7. **Optimize Performance**: Use appropriate parallelization and I/O settings

For additional help, consult the API reference or contact the development team. 
