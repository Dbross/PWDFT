System Classification
====================

PWDFT features intelligent automatic system classification that analyzes the geometric and electronic properties of the input system to apply optimal convergence parameters. This ensures robust and efficient SCF convergence across a wide range of system types.

Overview
--------

The system classification algorithm automatically detects the type of system being simulated and applies appropriate SCF parameters, convergence thresholds, and algorithm choices. This eliminates the need for manual parameter tuning in most cases.

**Supported System Types**:

* **Molecules**: Small, isolated systems (e.g., H₂O, CO₂, organic molecules)
* **Surfaces/Slabs**: 2D extended systems (e.g., metal surfaces, oxide slabs)
* **Bulk Metals**: 3D metallic crystals (e.g., Cu, Al, Fe)
* **Bulk Insulators**: 3D insulating crystals (e.g., SiO₂, diamond)
* **Interfaces**: Heterogeneous systems (e.g., metal-oxide interfaces)

Classification Algorithm
-----------------------

The classification process involves several steps:

1. **Geometry Analysis**: Analyze cell vectors, aspect ratios, and dimensionality
2. **Atom Counting**: Determine number of atoms and their distribution
3. **Crystallinity Detection**: Check if the system is crystalline
4. **Electronic Properties**: Estimate metallic vs. insulating character
5. **Parameter Application**: Apply system-specific defaults

**Classification Criteria**:

.. code-block:: text

   System Type          | Atoms | Dimensionality | Cell Type | SCF Algorithm
   -------------------- | ----- | -------------- | --------- | -------------
   Molecules            | < 10  | 3D             | Any       | DIIS (2)
   Surfaces/Slabs       | ≥ 8   | 2D-like        | Non-cubic | Local-TF (4)
   Bulk Metals          | ≥ 8   | 3D             | Cubic     | DIIS (2)
   Bulk Insulators      | ≥ 8   | 3D             | Cubic     | DIIS (2)
   Interfaces           | ≥ 8   | 3D             | Any       | Local-TF (4)

Detailed Classification Logic
----------------------------

**1. Molecule Detection**:

Molecules are identified by:
* Small number of atoms (< 10)
* Isolated structure (no periodic boundary conditions or large supercells)
* 3D dimensionality

**Example Molecule Input**:
.. code-block:: text

   geometry noautosym noautoz center
   O 0.0 0.0 0.0
   H 0.957 0.0 0.0
   H -0.24 0.927 0.0
   end

**Applied Parameters**:
* SCF Algorithm: DIIS (2)
* Mixing Parameter: 0.25
* Convergence: Standard thresholds
* Loop limits: 20-50 iterations
* Minimizer: L-BFGS (3) - Fast convergence for molecular systems

**2. Surface/Slab Detection**:

Surfaces are identified by:
* Sufficient atoms (≥ 8)
* Non-cubic cell structure
* 2D-like dimensionality or large aspect ratios
* Non-orthogonal cell vectors (FCC-like)

**Example Surface Input**:
.. code-block:: text

   geometry noautosym noautoz center
   # Cu surface - 2x2 supercell with 2 layers
   Cu 0.0 0.0 0.0
   Cu 2.55 0.0 0.0
   Cu 0.0 2.55 0.0
   Cu 2.55 2.55 0.0
   Cu 1.275 1.275 2.08
   Cu 3.825 1.275 2.08
   Cu 1.275 3.825 2.08
   Cu 3.825 3.825 2.08
   end
   
   nwpw
     simulation_cell
       FCC 30.0
     end
   end

**Applied Parameters**:
* SCF Algorithm: Local-TF (4)
* Mixing Parameter: 0.25
* Convergence: Relaxed thresholds for inhomogeneous systems
* Loop limits: 25-60 iterations
* Minimizer: Adaptive (5) - Handles complex energy landscapes in surfaces

**3. Bulk Metal Detection**:

Bulk metals are identified by:
* Many atoms (≥ 8, typically 16+)
* 3D dimensionality
* Orthogonal cubic cell structure
* Metallic character (estimated from band structure)

**Example Bulk Metal Input**:
.. code-block:: text

   geometry noautosym noautoz center
   # Cu bulk - 2x2x2 supercell (16 atoms)
   Cu 0.0 0.0 0.0
   Cu 1.8075 1.8075 0.0
   Cu 0.0 3.615 0.0
   Cu 1.8075 5.4225 0.0
   # ... more atoms
   end
   
   nwpw
     simulation_cell
       FCC 20.0
     end
   end

**Applied Parameters**:
* SCF Algorithm: DIIS (2)
* Mixing Parameter: 0.25
* Convergence: Standard thresholds
* Loop limits: 20-50 iterations
* Minimizer: L-BFGS (3) - Efficient for periodic bulk systems

**4. Bulk Insulator Detection**:

Bulk insulators are identified by:
* Many atoms (≥ 8)
* 3D dimensionality
* Orthogonal cubic cell structure
* Insulating character (estimated from band structure)

**Example Bulk Insulator Input**:
.. code-block:: text

   geometry noautosym noautoz center
   # C diamond - simple cubic structure (8 atoms)
   C 0.0 0.0 0.0
   C 3.57 0.0 0.0
   C 0.0 3.57 0.0
   C 3.57 3.57 0.0
   C 0.0 0.0 3.57
   C 3.57 0.0 3.57
   C 0.0 3.57 3.57
   C 3.57 3.57 3.57
   end
   
   nwpw
     simulation_cell
       SC 20.0
     end
   end

**Applied Parameters**:
* SCF Algorithm: DIIS (2)
* Mixing Parameter: 0.25
* Convergence: Standard thresholds
* Loop limits: 20-50 iterations
* Minimizer: L-BFGS (3) - Efficient for periodic bulk systems

**5. Interface Detection**:

Interfaces are identified by:
* Multiple element types
* Heterogeneous structure
* 3D dimensionality
* Complex electronic structure

**Example Interface Input**:
.. code-block:: text

   geometry noautosym noautoz center
   # Cu/Al interface - layered structure
   Cu 0.0 0.0 0.0
   Cu 2.55 0.0 0.0
   Cu 0.0 2.55 0.0
   Cu 2.55 2.55 0.0
   Al 0.0 0.0 2.55
   Al 2.55 0.0 2.55
   Al 0.0 2.55 2.55
   Al 2.55 2.55 2.55
   end
   
   nwpw
     simulation_cell
       SC 20.0
     end
   end

**Applied Parameters**:
* SCF Algorithm: Local-TF (4)
* Mixing Parameter: 0.25
* Convergence: Relaxed thresholds for heterogeneous systems
* Loop limits: 25-60 iterations
* Minimizer: Adaptive (5) - Handles complex energy landscapes in interfaces

Geometric Analysis
-----------------

**Cell Vector Analysis**:

The classification algorithm analyzes the simulation cell vectors:

.. math::

   \mathbf{a}_1 = [a_{1x}, a_{1y}, a_{1z}]
   \mathbf{a}_2 = [a_{2x}, a_{2y}, a_{2z}]
   \mathbf{a}_3 = [a_{3x}, a_{3y}, a_{3z}]

**Vector Lengths**:
* :math:`a_1 = |\mathbf{a}_1|`
* :math:`a_2 = |\mathbf{a}_2|`
* :math:`a_3 = |\mathbf{a}_3|`

**Aspect Ratio**:
* :math:`\text{aspect} = \max(a_1, a_2, a_3) / \min(a_1, a_2, a_3)`

**Dimensionality**:
* Calculated from the eigenvalues of the metric tensor
* 3D: All eigenvalues significant
* 2D: One eigenvalue small
* 1D: Two eigenvalues small

**Orthogonality Check**:
* Checks if off-diagonal elements of the cell matrix are zero
* Orthogonal cells suggest bulk systems
* Non-orthogonal cells suggest surfaces or complex systems

**Cubic Detection**:
* Checks if all cell vectors have similar lengths
* :math:`|a_1 - a_2| < \epsilon`, :math:`|a_2 - a_3| < \epsilon`, :math:`|a_1 - a_3| < \epsilon`
* Cubic cells suggest bulk systems

Parameter Application
--------------------

**Automatic Parameter Selection**:

Based on system classification, PWDFT automatically applies:

.. code-block:: text

   System Type          | SCF Algorithm | Alpha | Beta | Histories | Minimizer | Loop Limits
   -------------------- | ------------- | ----- | ---- | --------- | --------- | -----------
   Molecules            | 2 (DIIS)      | 0.25  | 0.1  | 8         | 3 (L-BFGS)| 20-50
   Surfaces/Slabs       | 4 (Local-TF)  | 0.25  | 0.1  | 8         | 5 (Adaptive)| 25-60
   Bulk Metals          | 2 (DIIS)      | 0.25  | 0.1  | 8         | 3 (L-BFGS)| 20-50
   Bulk Insulators      | 2 (DIIS)      | 0.25  | 0.1  | 8         | 3 (L-BFGS)| 20-50
   Interfaces           | 4 (Local-TF)  | 0.25  | 0.1  | 8         | 5 (Adaptive)| 25-60

**Convergence Thresholds**:

.. code-block:: text

   System Type          | Initial E_thr | Min E_thr | E_thr Factor
   -------------------- | ------------- | --------- | ------------
   Molecules            | 1.0e-2        | 1.0e-13   | 0.1
   Surfaces/Slabs       | 1.0e-2        | 1.0e-12   | 0.1
   Bulk Metals          | 1.0e-2        | 1.0e-13   | 0.1
   Bulk Insulators      | 1.0e-2        | 1.0e-13   | 0.1
   Interfaces           | 1.0e-2        | 1.0e-12   | 0.1

Debugging and Monitoring
-----------------------

**Classification Output**:

PWDFT provides detailed classification information in the output:

.. code-block:: text

   === System Classification Debug Info ===
    Number of atoms: 8
    Is crystal (input): false
    Cell volume: 8000
    Cell aspect ratio: 1
    Dimensionality: 3
    Cell vectors: [20, 0, 0]
                   [0, 20, 0]
                   [0, 0, 20]
   =======================================
   
   System Classification: Surface/Slab
   Applied SCF Algorithm: 4
   Applied SCF Alpha: 0.25
   Applied SCF Beta: 0.1
   Applied DIIS Histories: 8
   Applied Minimizer Type: 5

**Manual Override**:

You can override automatic classification by explicitly setting parameters:

.. code-block:: text

   nwpw
     # Override automatic classification
     scf_algorithm 2
     scf_alpha 0.3
     scf_beta 0.15
     diis_histories 12
     minimizer 3
     loop 30 80
   end

**Best Practices**:

1. **Trust the classification**: The automatic system detection is well-tested
2. **Monitor the output**: Check the classification debug information
3. **Override when needed**: Use manual parameters for special cases
4. **Report issues**: If classification fails, report the system geometry

**Troubleshooting**:

**Wrong Classification**:
* Check the debug output for classification details
* Verify the input geometry is correct
* Consider if the system is ambiguous (e.g., small surface vs. large molecule)

**Poor Convergence**:
* Check if the classification is appropriate
* Try manual parameter override
* Consider system-specific issues

**Example Complete Workflow**:

.. code-block:: text

   memory 900 mb
   start calculation
   echo
   
   # Let PWDFT classify and optimize automatically
   geometry noautosym noautoz center
   O 0.0 0.0 0.0
   H 0.957 0.0 0.0
   H -0.24 0.927 0.0
   end
   
   nwpw
     simulation_cell
       SC 15.0
     end
     cutoff 60.0
     xc pbe96
     # Automatic classification will choose optimal parameters
     smear methfessel-paxton
     temperature 500
     loop 20 50
     monkhorst-pack 1 1 1
     initial_wavefunction_guess superposition
     mult 1
   end
   
   task pspw energy 