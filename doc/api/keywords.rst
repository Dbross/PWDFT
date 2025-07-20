Input Keywords Reference
========================

This section provides a comprehensive analysis of PWDFT input keywords, their functions, data types, defaults, and underlying physics.

Keyword Analysis Table
---------------------

.. list-table:: PWDFT Input Keywords
   :widths: 20 25 15 15 25
   :header-rows: 1

   * - Keyword/Concept
     - Function/Purpose
     - Data Type & Example
     - Default Value & Rationale
     - Associated Physics

   * - task
     - Specifies calculation type
     - String: 'pspw energy', 'band energy', 'pspw steepest_descent'
     - 'pspw energy'. Single-point SCF is fundamental
     - Kohn-Sham DFT, Structural Optimization

   * - cutoff
     - Kinetic energy cutoff for wavefunctions
     - Float: 35.0, 60.0 (Rydberg)
     - 9000.0 Ry. Balance of cost and accuracy
     - Plane-Wave Basis Set, Fourier Expansion

   * - xc
     - Exchange-correlation functional
     - String: 'pbe96', 'hse06', 'beef-vdw'
     - 'pbe96'. Robust GGA for solids
     - Exchange-Correlation Energy, Jacob's Ladder of DFT

   * - monkhorst-pack
     - K-point grid dimensions
     - Integer Array: [1,1,1], [8,8,8]
     - [1,1,1]. Gamma point for molecules
     - k-point Sampling, Brillouin Zone Integration

   * - smear
     - Occupation number smearing
     - String: 'methfessel-paxton', 'gaussian', 'fermi'
     - None. Only needed for metals
     - Fermi-Dirac Statistics, Electronic Temperature

   * - temperature
     - Smearing width (electronic temperature)
     - Float: 500, 8000 (Kelvin)
     - 500 K. Stabilize convergence in metals
     - Fermi-Dirac Distribution

   * - scf
     - SCF algorithm and mixing method
     - String: 'ks-grassmann-cg simple alpha 0.02'
     - 'ks-grassmann-cg pulay alpha 0.1'
     - Self-Consistent Field (SCF) Convergence

   * - loop
     - Maximum SCF iterations
     - String: '10 10', '20 20'
     - '10 100'. Allow sufficient iterations
     - SCF Convergence Criteria

Numerical Stability and NaN Detection
------------------------------------

PWDFT includes automatic NaN (Not-a-Number) detection and fallback recovery mechanisms to handle numerical instabilities during SCF calculations.

**Automatic Features:**
- **NaN Detection**: Automatically detects NaN/Inf values in energy computations, matrix operations, and trace functions
- **Fallback Recovery**: Triggers wavefunction reinitialization and SCF restart when numerical issues are detected
- **Performance**: Minimal overhead (<5% total computational cost)
- **Transparency**: No user intervention required

**Monitoring Messages:**
The system provides clear feedback when numerical issues are detected:

.. code-block:: text

   *** NaN/Inf or large energy detected in band SCF (minimizer 8). Failure 1/3
   *** Energy value: 1.000000e+10
   *** Continuing with current iteration (failure 1/3)

   *** Triggering fallback after 3 consecutive failures
   *** 15 steepest descent iterations performed for stabilization
   *** Energy stabilized, resetting failure counter

**Debugging Commands:**
.. code-block:: bash

   # Check for NaN detection messages
   grep "NaN/Inf detected" output.log

   # Check for fallback activity
   grep "Triggering fallback" output.log

   # Check for energy stabilization
   grep "Energy stabilized" output.log

**Common Causes and Solutions:**

1. **Insufficient k-point sampling for metallic systems**
   - **Solution**: Increase k-point mesh density (e.g., 6x6x6 instead of 4x4x4)

2. **Too aggressive SCF mixing parameters**
   - **Solution**: Use conservative mixing (e.g., `alpha 0.15` instead of `alpha 0.25`)

3. **Inadequate smearing for metallic systems**
   - **Solution**: Use Methfessel-Paxton smearing with appropriate temperature

4. **Poor initial wavefunction guess**
   - **Solution**: Try different initial guesses (superposition, random, atomic)

**Recommended Settings for Challenging Systems:**
.. code-block:: text

   nwpw
     scf ks-grassmann-cg anderson alpha 0.15
     smear methfessel-paxton
     temperature 300
     loop 20 20
     monkhorst-pack 6 6 6
     initial_wavefunction_guess superposition
   end

Core Keywords
------------

.. _keyword-task:

task
~~~~

**Purpose**: Specifies the type of calculation to perform.

**Format**: ``[module] [operation]``

**Available Modules**:
- ``pspw``: Plane-wave self-consistent field
- ``band``: Band structure calculations
- ``neb``: Nudged elastic band
- ``md``: Molecular dynamics

**Available Operations**:
- ``energy``: Single-point energy calculation
- ``gradient``: Energy and forces calculation
- ``optimize``: Geometry optimization
- ``steepest_descent``: Steepest descent minimization

**Examples**:
- ``task pspw energy``: Single-point SCF calculation
- ``task band gradient``: Band calculation with forces
- ``task pspw optimize``: Geometry optimization

**Default**: ``pspw energy``

**Physics**: Determines the computational approach and output properties.

.. _keyword-cutoff:

cutoff
~~~~~~

**Purpose**: Sets the kinetic energy cutoff for plane-wave basis set.

**Format**: ``[value]`` (Rydberg)

**Range**: 20-100 Rydberg (typical)

**Examples**:
- ``cutoff 30.0``: Low accuracy, fast
- ``cutoff 60.0``: Standard accuracy
- ``cutoff 80.0``: High accuracy

**Default**: 9000.0 Ry (very high)

**Physics**: Higher cutoff = more plane waves = better accuracy but higher cost.

**Convergence Guidelines**:
- **Molecules**: 30-40 Ry
- **Bulk solids**: 40-60 Ry
- **Surfaces**: 50-70 Ry
- **High accuracy**: 80-100 Ry

.. _keyword-xc:

xc
~~

**Purpose**: Specifies the exchange-correlation functional.

**Available Functionals**:
- ``pbe96``: PBE functional (recommended)
- ``hse06``: HSE06 hybrid functional
- ``beef-vdw``: BEEF-vdW functional
- ``pbe0``: PBE0 hybrid functional
- ``b3lyp``: B3LYP hybrid functional

**Examples**:
- ``xc pbe96``: Standard GGA functional
- ``xc hse06``: Hybrid functional for accurate band gaps
- ``xc beef-vdw``: Includes van der Waals interactions

**Default**: ``pbe96``

**Physics**: Determines the treatment of electron exchange and correlation.

**Selection Guidelines**:
- **General purpose**: PBE96
- **Accurate band gaps**: HSE06
- **Layered materials**: BEEF-vdW
- **Molecular systems**: PBE0 or B3LYP

.. _keyword-monkhorst-pack:

monkhorst-pack
~~~~~~~~~~~~~

**Purpose**: Defines k-point grid for Brillouin zone sampling.

**Format**: ``[nx] [ny] [nz]``

**Examples**:
- ``monkhorst-pack 1 1 1``: Gamma point only
- ``monkhorst-pack 3 3 1``: 3×3×1 grid for 2D systems
- ``monkhorst-pack 8 8 8``: Dense grid for bulk

**Default**: ``1 1 1``

**Physics**: k-point sampling is crucial for accurate electronic structure calculations in periodic systems.

**Guidelines by System Type**:
- **Molecules**: 1×1×1 (Gamma point only)
- **Bulk solids**: 4×4×4 to 8×8×8
- **Surfaces**: 4×4×1 to 8×8×1
- **Wires**: 4×1×1 to 6×1×1

Smearing Keywords
^^^^^^^^^^^^^^^^

.. _keyword-smear:

smear
~~~~~

**Purpose**: Specifies occupation number smearing method.

**Available Methods**:
- ``methfessel-paxton``: Methfessel-Paxton smearing
- ``gaussian``: Gaussian smearing
- ``fermi``: Fermi-Dirac smearing

**Default**: None (integer occupation)

**Physics**: Smearing helps convergence in metallic systems by allowing fractional occupation of states near the Fermi level.

**Selection Guidelines**:
- **Metals**: Methfessel-Paxton (recommended)
- **Semiconductors**: None or Gaussian
- **Insulators**: None

.. _keyword-temperature:

temperature
~~~~~~~~~~~

**Purpose**: Sets electronic temperature for smearing.

**Range**: 100 - 10000 Kelvin

**Default**: 500 K

**Examples**:
- ``temperature 300``: Conservative, stable
- ``temperature 1000``: More aggressive
- ``temperature 5000``: Very aggressive

**Physics**: Higher temperature = broader smearing = better convergence but less accurate.

**Guidelines**:
- **Conservative**: 300-500 K
- **Standard**: 500-1000 K
- **Aggressive**: 1000-5000 K

SCF Keywords
^^^^^^^^^^^

.. _keyword-scf:

scf
~~~

**Purpose**: Controls the SCF convergence algorithm and mixing parameters.

**Format**: ``ks-grassmann-cg [method] [alpha] [value]``

**Methods**:
- ``simple``: Simple mixing
- ``pulay``: Pulay mixing (default)
- ``anderson``: Anderson mixing

**Examples**:
- ``scf ks-grassmann-cg simple alpha 0.02``
- ``scf ks-grassmann-cg pulay alpha 0.1``

**Default**: ``ks-grassmann-cg pulay alpha 0.1``

**Physics**: The SCF procedure iteratively solves the Kohn-Sham equations until self-consistency is achieved.

**Selection Guidelines**:
- **Conservative**: Simple mixing with low alpha
- **Standard**: Pulay mixing
- **Aggressive**: Anderson mixing

.. _keyword-loop:

loop
~~~~

**Purpose**: Sets maximum number of SCF iterations.

**Format**: ``[outer] [inner]``

**Examples**:
- ``loop 10 10``: 10 outer, 10 inner iterations
- ``loop 50 1``: 50 outer, 1 inner iteration

**Default**: ``10 100``

**Physics**: Controls convergence of the self-consistent field procedure.

**Guidelines**:
- **Simple systems**: 10-20 iterations
- **Complex systems**: 20-50 iterations
- **Challenging systems**: 50-100 iterations

Advanced Keywords
^^^^^^^^^^^^^^^^

.. _keyword-adaptive-mixing:

scf_adaptive_mixing
~~~~~~~~~~~~~~~~~~

**Purpose**: Enables adaptive SCF mixing for challenging systems.

**Type**: Boolean

**Default**: False

**Example**:
.. code-block:: text

   nwpw
     scf_adaptive_mixing true
     scf_alpha 0.25
     scf_beta 0.1
   end

**Physics**: Automatically adjusts mixing parameters based on convergence behavior.

.. _keyword-adaptive-threshold:

scf_adaptive_threshold
~~~~~~~~~~~~~~~~~~~~~

**Purpose**: Enables adaptive diagonalization thresholds.

**Type**: Boolean

**Default**: False

**Example**:
.. code-block:: text

   nwpw
     scf_adaptive_threshold true
     scf_initial_ethr 1.0e-2
     scf_min_ethr 1.0e-13
     scf_ethr_factor 0.1
   end

**Physics**: Dynamically adjusts diagonalization accuracy based on convergence progress.

.. _keyword-fractional:

fractional
~~~~~~~~~

**Purpose**: Enables fractional occupation for metallic systems.

**Type**: Boolean

**Default**: False

**Example**:
.. code-block:: text

   nwpw
     fractional true
     fractional_kT 0.001
     fractional_orbitals 4
   end

**Physics**: Allows fractional occupation of electronic states near the Fermi level.

Performance Keywords
^^^^^^^^^^^^^^^^^^^

.. _keyword-memory:

memory
~~~~~~

**Purpose**: Sets memory allocation for the calculation.

**Format**: ``[amount] [unit]``

**Examples**:
- ``memory 1000 mb``: 1 GB memory
- ``memory 4 gb``: 4 GB memory
- ``memory 16 gb``: 16 GB memory

**Default**: System-dependent

**Guidelines**:
- **Small systems**: 1-2 GB
- **Medium systems**: 4-8 GB
- **Large systems**: 16+ GB

.. _keyword-parallel:

parallel
~~~~~~~~

**Purpose**: Controls parallel execution parameters.

**Examples**:
- ``parallel 4``: Use 4 processes
- ``parallel 16``: Use 16 processes

**Default**: 1 (serial execution)

**Physics**: Parallelization can significantly speed up calculations for large systems.

I/O Keywords
^^^^^^^^^^^

.. _keyword-scratch:

scratch_dir
~~~~~~~~~~

**Purpose**: Sets directory for temporary files.

**Example**:
.. code-block:: text

   scratch_dir /tmp/pwdft_scratch
   permanent_dir ./results

**Default**: Current directory

**Physics**: Using a fast scratch directory can improve I/O performance.

.. _keyword-print:

print
~~~~~

**Purpose**: Controls output verbosity.

**Options**:
- ``low``: Minimal output
- ``medium``: Standard output
- ``high``: Verbose output

**Default**: ``medium``

**Example**:
.. code-block:: text

   nwpw
     print low
   end

**Physics**: Reduces I/O overhead for large calculations.

Validation and Testing
---------------------

### Energy Convergence

**Test energy convergence with respect to:**
1. **Plane-wave cutoff**: 30, 40, 50, 60, 70 Ry
2. **K-point sampling**: 2×2×2, 4×4×4, 6×6×6, 8×8×8
3. **SCF tolerance**: 1e-4, 1e-5, 1e-6

### Physical Checks

**Verify results are physically reasonable:**
- **Total energy**: Should be negative and reasonable magnitude
- **Forces**: Should be small (< 0.01 eV/Å) for optimized structures
- **Band gap**: Should match expected values for the material
- **Density**: Should be smooth and positive everywhere

### Comparison with Reference

**Compare with:**
- **Experimental data**: Lattice constants, band gaps, etc.
- **Other codes**: VASP, Quantum ESPRESSO, etc.
- **Literature**: Published DFT results

Troubleshooting
--------------

### Common Issues

1. **SCF divergence**
   - **Symptom**: Energy oscillates or diverges
   - **Solution**: Reduce mixing parameter, increase smearing

2. **NaN errors**
   - **Symptom**: "NaN/Inf detected" messages
   - **Solution**: Use more conservative parameters, check system setup

3. **Memory issues**
   - **Symptom**: Out of memory errors
   - **Solution**: Reduce parallelization, use smaller k-point grids

4. **Slow convergence**
   - **Symptom**: Many iterations required
   - **Solution**: Use better initial guess, adjust mixing parameters

### Debugging Commands

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

Best Practices
-------------

### 1. Start Conservative

**For new systems, use conservative parameters:**
.. code-block:: text

   nwpw
     scf ks-grassmann-cg anderson alpha 0.15
     smear methfessel-paxton
     temperature 300
     loop 20 20
     monkhorst-pack 4 4 4
   end

### 2. Perform Convergence Studies

**Always test convergence systematically:**
1. **K-point convergence**: Test different mesh densities
2. **Cutoff convergence**: Test different energy cutoffs
3. **SCF convergence**: Test different tolerances

### 3. Monitor for Numerical Issues

**Watch for NaN detection messages and use fallback features:**
- Monitor output for "NaN/Inf detected" messages
- Use conservative parameters for challenging systems
- Let the automatic fallback mechanisms work

### 4. Validate Results

**Compare with reference data:**
- Experimental lattice constants
- Published band gaps
- Known structural properties

### 5. Document Parameters

**Keep detailed records of:**
- Input parameters used
- Convergence criteria met
- Results obtained
- Any issues encountered

Conclusion
---------

This keyword reference provides comprehensive guidance for setting up PWDFT calculations. Key points:

1. **Start with conservative parameters** and optimize systematically
2. **Use NaN detection features** for robust calculations
3. **Perform convergence studies** for new system types
4. **Monitor for numerical issues** and use fallback mechanisms
5. **Validate results** against experimental or reference data

For additional help, consult the best practices guide or contact the development team. 