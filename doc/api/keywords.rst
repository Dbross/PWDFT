Input Keywords Reference
========================

This section provides a comprehensive analysis of PWDFT input keywords, their functions, data types, defaults, and underlying physics.

Keyword Analysis Table
----------------------

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

   * - initial_wavefunction_guess
     - Initial wavefunction strategy
     - String: 'random', 'superposition', 'atomic'
     - 'superposition'. Physically reasonable
     - Wavefunction Initialization

   * - ispin
     - Number of spin components
     - Integer: 1, 2
     - 1. Non-spin-polarized default
     - Electron Spin, Magnetism

   * - mult
     - Multiplicity (spin state)
     - Integer: 1, 2, 3, 4
     - 1. Singlet default
     - Electron Spin, Magnetism

   * - memory
     - Memory allocation
     - String: '900 mb', '1900 mb'
     - '900 mb'. Default memory allocation
     - Memory Management

   * - mapping
     - Parallel mapping strategy
     - Integer: 1, 2, 3
     - 1. Default mapping
     - Parallel Computing

   * - np_dimensions
     - Parallel dimensions
     - Integer Array: [4, 2], [8, 4, 2]
     - [np, 1, 1]. Default parallel layout
     - Parallel Computing

   * - simulation_cell
     - Cell definition
     - Block: SC value or lattice_vectors
     - Auto-generated from ASE cell
     - Crystal Structure, Periodicity

   * - pseudopotentials
     - Pseudopotential specification
     - Block: Element library type
     - Auto-generated from ASE atoms
     - Pseudopotential Approximation

   * - vectors
     - Wavefunction file handling
     - String: 'input filename', 'output filename'
     - None. No file I/O by default
     - Wavefunction Storage

   * - steepest_descent
     - Optimization method
     - Block: loop, geometry_optimize, time_step
     - None. SCF only by default
     - Geometry Optimization

   * - car-parrinello
     - CP dynamics
     - Block: loop, time_step, fake_mass
     - None. SCF only by default
     - Molecular Dynamics

   * - 2d-hcurve
     - 2D Hilbert curve mapping
     - Logical: .true., .false.
     - .false. Default 3D mapping
     - Parallel Load Balancing

   * - lmbfgs
     - L-BFGS optimization
     - Logical: .true., .false.
     - .false. Default steepest descent
     - Geometry Optimization

   * - output_wavefunction_filename
     - Wavefunction output file
     - String: 'filename.wfn'
     - None. No output by default
     - Wavefunction Storage

Detailed Keyword Descriptions
----------------------------

.. _keyword-task:

task
~~~~
**Purpose**: Controls the type of calculation to be performed.

**Values**:
- ``pspw energy``: Single-point Self-Consistent Field calculation
- ``pspw steepest_descent``: Structural relaxation with steepest descent
- ``pspw car-parrinello``: Car-Parrinello molecular dynamics
- ``band energy``: Band structure calculation
- ``band structure``: Band structure with k-path

**Default**: ``pspw energy``

**Physics**: The task type determines which degrees of freedom are optimized during the calculation.

Electronic Structure Keywords
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _keyword-cutoff:

cutoff
~~~~~~

**Purpose**: Sets the kinetic energy cutoff for the plane-wave basis set.

**Range**: 5.0 - 9000.0 Rydberg

**Default**: 9000.0 Rydberg

**Physics**: The cutoff determines the maximum kinetic energy of plane waves used to expand electronic wavefunctions. Higher values increase accuracy but computational cost scales as :math:`E_{cut}^{3/2}`.

**Convergence**: Must be systematically converged for production calculations.

**Examples from code**:
- ``cutoff 35.00`` (DEMO/eric.nw)
- ``cutoff 5.0`` (DEMO2/ericmd0.nw)
- ``cutoff 60.0`` (cu_slab_init.nwxi)

.. _keyword-xc:

xc
~~

**Purpose**: Selects the exchange-correlation functional.

**Available Functionals**:

* **LDA**: ``slater``, ``vosko``
* **GGA**: ``pbe96``, ``pbesol``, ``revpbe``
* **Hybrid**: ``hse06``, ``pbe0``
* **vdW**: ``optb88-vdw``, ``beef-vdw``

**Default**: ``pbe96``

**Physics**: The XC functional approximates the complex many-body electron interactions. Different rungs of "Jacob's Ladder" provide increasing accuracy at higher computational cost.

SCF Convergence Keywords
^^^^^^^^^^^^^^^^^^^^^^^

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

K-Point Sampling Keywords
^^^^^^^^^^^^^^^^^^^^^^^^^

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

**Example**:

.. code-block:: text
=======
**Physics**: Smearing helps convergence in metallic systems by allowing fractional occupation of states near the Fermi level.
>>>>>>> parent of 3f31e17 (Implement comprehensive NaN detection and fallback recovery system)

.. _keyword-temperature:

temperature
~~~~~~~~~~~

**Purpose**: Sets electronic temperature for smearing.

**Range**: 100 - 10000 Kelvin

**Default**: 500 K

**Examples**:
- ``temperature 500``: Low temperature for semiconductors
- ``temperature 8000``: High temperature for metals

**Physics**: Electronic temperature controls the width of the smearing function.

**Example**:

.. code-block:: text
=======
Spin and Magnetism Keywords
^^^^^^^^^^^^^^^^^^^^^^^^^^
>>>>>>> parent of 3f31e17 (Implement comprehensive NaN detection and fallback recovery system)

.. _keyword-ispin:

ispin
~~~~~

**Purpose**: Controls spin polarization.

**Values**:
- ``1``: Non-spin-polarized calculation
- ``2``: Spin-polarized calculation

**Default**: ``1``

**Physics**: Spin polarization is essential for magnetic materials and open-shell systems.

.. _keyword-mult:

**Example**:

.. code-block:: text
=======
mult
~~~~
>>>>>>> parent of 3f31e17 (Implement comprehensive NaN detection and fallback recovery system)

**Purpose**: Sets spin multiplicity.

**Values**: 1, 2, 3, 4, ...

**Default**: ``1`` (singlet)

**Physics**: Multiplicity determines the total spin state of the system.

Memory and Performance Keywords
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. _keyword-memory:

memory
~~~~~~

**Purpose**: Sets memory allocation for the calculation.

**Format**: ``[size] [unit]``

**Examples**:
- ``memory 900 mb``
- ``memory 1900 mb``

**Default**: ``900 mb``

**Physics**: Memory allocation affects performance and maximum system size.

.. _keyword-mapping:

mapping
~~~~~~~

**Purpose**: Controls parallel mapping strategy.

**Values**: 1, 2, 3

**Default**: ``1``

**Physics**: Mapping affects load balancing in parallel calculations.

.. _keyword-2d-hcurve:

2d-hcurve
~~~~~~~~~

**Purpose**: Enables 2D Hilbert curve mapping for better load balancing.

**Values**: ``.true.``, ``.false.``

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
=======
**Default**: ``.false.``

**Physics**: 2D Hilbert curves provide better spatial locality for 2D systems.

System Definition Keywords
^^^^^^^^^^^^^^^^^^^^^^^^^

.. _keyword-simulation_cell:

simulation_cell
~~~~~~~~~~~~~~

**Purpose**: Defines the simulation cell parameters.

**Format**:
.. code-block:: text

   simulation_cell
     SC 20.0
>>>>>>> parent of 3f31e17 (Implement comprehensive NaN detection and fallback recovery system)
   end

**Alternative format for slabs**:
.. code-block:: text

   geometry units angstrom nocenter noautosym noautoz
     system surface
       lat_a 7.8743411152933946e+00
       lat_b 7.8743411152933938e+00
       lat_c 3.0715620996159458e+01
       alpha 9.0000000000000000e+01
       beta 9.0000000000000000e+01
       gamma 5.9999999999999993e+01
     end
   end

**Physics**: The simulation cell defines the periodic boundary conditions and system size.

.. _keyword-pseudopotentials:

pseudopotentials
~~~~~~~~~~~~~~~

**Purpose**: Specifies pseudopotential files.

**Format**:
.. code-block:: text

   pseudopotentials
   H library pspw_default
   C library pspw_default
   end

**Physics**: Pseudopotentials replace core electrons with effective potentials.

Wavefunction Keywords
^^^^^^^^^^^^^^^^^^^^

.. _keyword-vectors:

vectors
~~~~~~~

**Purpose**: Controls wavefunction file I/O.

**Format**: ``[input|output] [filename]``

**Examples**:
- ``vectors output test2.movecs``
- ``vectors input eric.movecs``

**Physics**: Wavefunction files allow restarting calculations and analysis.

.. _keyword-output_wavefunction_filename:

output_wavefunction_filename
~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Purpose**: Specifies output wavefunction file.

**Format**: ``[filename]``

**Example**: ``output_wavefunction_filename slab_init.wfn``

**Physics**: Saves final wavefunctions for restart or analysis.

.. _keyword-initial_wavefunction_guess:

initial_wavefunction_guess
~~~~~~~~~~~~~~~~~~~~~~~~~~

**Purpose**: Sets initial wavefunction strategy.

**Values**:
- ``superposition``: Atomic orbital superposition
- ``random``: Random initialization
- ``atomic``: Atomic orbital guess

**Default**: ``superposition``

**Physics**: Initial guess affects convergence speed and stability.

Optimization Keywords
^^^^^^^^^^^^^^^^^^^

.. _keyword-steepest_descent:

steepest_descent
~~~~~~~~~~~~~~~

**Purpose**: Controls steepest descent optimization.

**Format**:
.. code-block:: text

   steepest_descent
     loop 10 200
     geometry_optimize
   end

**Physics**: Steepest descent minimizes the total energy with respect to atomic positions.

.. _keyword-lmbfgs:

lmbfgs
~~~~~~

**Purpose**: Enables L-BFGS optimization.

**Values**: ``.true.``, ``.false.``

**Default**: ``.false.``

**Physics**: L-BFGS provides faster convergence than steepest descent for geometry optimization.

Best Practices
-------------

**Convergence Studies**:
1. **Cutoff**: Start with 35-50 Ry, converge to 1-2 mRy/atom
2. **k-points**: Use appropriate grid for system type
3. **SCF**: Monitor energy convergence to 1e-6 Hartree

**Performance Tips**:
1. **Memory**: Allocate sufficient memory for your system
2. **Parallelization**: Use appropriate mapping for your architecture
3. **Restart**: Use wavefunction files for long calculations

**System-Specific Settings**:
1. **Molecules**: Gamma point (1 1 1), no smearing
2. **Bulk**: Dense k-grid, smearing for metals
3. **Surfaces**: 2D k-grid, vacuum in z-direction 
