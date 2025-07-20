SCF Algorithms
=============

PWDFT implements multiple Self-Consistent Field (SCF) algorithms to solve the Kohn-Sham equations efficiently for different system types. The choice of algorithm significantly impacts convergence behavior and computational performance.

Overview
--------

The SCF procedure in PWDFT iteratively solves the Kohn-Sham equations until the electron density converges. Different algorithms are optimized for different system characteristics:

* **Algorithm 1**: Simple mixing (basic)
* **Algorithm 2**: DIIS (Direct Inversion in Iterative Subspace) - **Recommended for most systems**
* **Algorithm 3**: Broyden mixing
* **Algorithm 4**: Local-TF (Local Thomas-Fermi) mixing - **Recommended for surfaces and interfaces**

Algorithm Selection
------------------

PWDFT automatically selects the optimal SCF algorithm based on system classification:

.. code-block:: text

   System Type          | Recommended Algorithm | Use Case
   -------------------- | --------------------- | --------
   Molecules            | Algorithm 2 (DIIS)   | Small, isolated systems
   Surfaces/Slabs       | Algorithm 4 (Local-TF)| Inhomogeneous systems
   Bulk Metals          | Algorithm 2 (DIIS)   | Metallic systems
   Bulk Insulators      | Algorithm 2 (DIIS)   | Insulating crystals
   Interfaces           | Algorithm 4 (Local-TF)| Heterogeneous systems

Algorithm 1: Simple Mixing
-------------------------

**Type**: Basic density mixing
**Use Case**: Simple systems, debugging, educational purposes

**Description**:
Simple mixing applies a linear combination of the current and previous electron densities:

.. math::

   \rho_{n+1} = \alpha \rho_{in} + (1-\alpha) \rho_n

where :math:`\alpha` is the mixing parameter.

**Parameters**:
* ``scf_alpha``: Mixing parameter (default: 0.25)
* ``scf_beta``: Not used in simple mixing

**Advantages**:
* Simple and robust
* Easy to understand and debug
* Works for most systems

**Disadvantages**:
* Slow convergence for difficult systems
* No acceleration techniques
* May require many iterations

**Example Input**:
.. code-block:: text

   nwpw
     scf_algorithm 1
     scf_alpha 0.25
     loop 50 100
   end

Algorithm 2: DIIS (Direct Inversion in Iterative Subspace)
--------------------------------------------------------

**Type**: Accelerated mixing with error minimization
**Use Case**: **Recommended for most systems** (molecules, bulk metals, bulk insulators)

**Description**:
DIIS (Direct Inversion in Iterative Subspace) is an accelerated mixing scheme that minimizes the error in the iterative subspace. It constructs the optimal linear combination of previous iterations to minimize the residual error.

**Mathematical Foundation**:
DIIS minimizes the error function:

.. math::

   \min_{\{c_i\}} \left\| \sum_{i=1}^{N} c_i \mathbf{e}_i \right\|^2

subject to :math:`\sum_{i=1}^{N} c_i = 1`, where :math:`\mathbf{e}_i` are the error vectors from previous iterations.

**Parameters**:
* ``scf_alpha``: Mixing parameter (default: 0.25)
* ``scf_beta``: DIIS mixing parameter (default: 0.1)
* ``diis_histories``: Number of previous iterations to use (default: 8)

**Advantages**:
* Fast convergence for most systems
* Robust and well-tested
* Automatic acceleration
* Works well for molecules and bulk systems

**Disadvantages**:
* May struggle with inhomogeneous systems (surfaces, interfaces)
* Can become unstable for difficult systems
* Requires sufficient iteration history

**Example Input**:
.. code-block:: text

   nwpw
     scf_algorithm 2
     scf_alpha 0.25
     scf_beta 0.1
     diis_histories 8
     loop 20 50
   end

**Recommended Settings by System Type**:

.. code-block:: text

   # Molecules
   scf_algorithm 2
   scf_alpha 0.25
   scf_beta 0.1
   diis_histories 8

   # Bulk Metals
   scf_algorithm 2
   scf_alpha 0.25
   scf_beta 0.1
   diis_histories 8

   # Bulk Insulators
   scf_algorithm 2
   scf_alpha 0.25
   scf_beta 0.1
   diis_histories 8

Algorithm 3: Broyden Mixing
--------------------------

**Type**: Quasi-Newton method for density mixing
**Use Case**: Systems with strong charge transfer, difficult convergence

**Description**:
Broyden mixing is a quasi-Newton method that approximates the inverse Jacobian of the SCF mapping. It updates the mixing matrix based on the difference between input and output densities.

**Mathematical Foundation**:
Broyden updates the mixing matrix :math:`A_n`:

.. math::

   A_{n+1} = A_n + \frac{(\Delta \rho_{out} - A_n \Delta \rho_{in}) \Delta \rho_{in}^T}{\Delta \rho_{in}^T \Delta \rho_{in}}

where :math:`\Delta \rho_{in}` and :math:`\Delta \rho_{out}` are the differences in input and output densities.

**Parameters**:
* ``scf_alpha``: Initial mixing parameter (default: 0.25)
* ``scf_beta``: Broyden mixing parameter (default: 0.1)
* ``diis_histories``: Number of iterations to store (default: 8)

**Advantages**:
* Can handle strong charge transfer
* Adaptive mixing matrix
* Good for difficult systems

**Disadvantages**:
* More complex than DIIS
* May be less stable
* Requires careful parameter tuning

**Example Input**:
.. code-block:: text

   nwpw
     scf_algorithm 3
     scf_alpha 0.25
     scf_beta 0.1
     diis_histories 8
     loop 30 80
   end

Algorithm 4: Local-TF (Local Thomas-Fermi) Mixing
-----------------------------------------------

**Type**: Preconditioned mixing for inhomogeneous systems
**Use Case**: **Recommended for surfaces, interfaces, and inhomogeneous systems**

**Description**:
Local-TF mixing applies a local Thomas-Fermi preconditioner to the density mixing. This is particularly effective for systems with strong spatial inhomogeneities, such as surfaces and interfaces.

**Mathematical Foundation**:
Local-TF mixing applies a screening function:

.. math::

   \rho_{n+1}(\mathbf{r}) = \rho_n(\mathbf{r}) + \alpha \frac{q^2}{q^2 + q_0^2(\mathbf{r})} \Delta \rho(\mathbf{r})

where :math:`q_0^2(\mathbf{r}) = 4\pi \rho(\mathbf{r})` is the local Thomas-Fermi screening parameter.

**Parameters**:
* ``scf_alpha``: Mixing parameter (default: 0.25)
* ``scf_beta``: Local-TF mixing parameter (default: 0.1)
* ``diis_histories``: Number of iterations to store (default: 8)

**Advantages**:
* Excellent for surfaces and interfaces
* Handles spatial inhomogeneities
* Robust convergence for difficult systems
* Based on physical screening

**Disadvantages**:
* More computationally expensive
* May be overkill for simple systems
* Requires more iterations for simple systems

**Example Input**:
.. code-block:: text

   nwpw
     scf_algorithm 4
     scf_alpha 0.25
     scf_beta 0.1
     diis_histories 8
     loop 25 60
   end

**Recommended Settings by System Type**:

.. code-block:: text

   # Surfaces/Slabs
   scf_algorithm 4
   scf_alpha 0.25
   scf_beta 0.1
   diis_histories 8

   # Interfaces
   scf_algorithm 4
   scf_alpha 0.25
   scf_beta 0.1
   diis_histories 8

   # Heterogeneous systems
   scf_algorithm 4
   scf_alpha 0.25
   scf_beta 0.1
   diis_histories 8

Parameter Guidelines
-------------------

**Mixing Parameters**:

* ``scf_alpha``: Controls the weight of new density in mixing
  * **Conservative**: 0.1-0.2 (slow but stable)
  * **Standard**: 0.25 (good balance)
  * **Aggressive**: 0.3-0.5 (fast but may be unstable)

* ``scf_beta``: Controls acceleration in DIIS/Broyden
  * **Conservative**: 0.05-0.1 (stable)
  * **Standard**: 0.1 (good balance)
  * **Aggressive**: 0.15-0.2 (fast but may oscillate)

* ``diis_histories``: Number of previous iterations to use
  * **Minimum**: 4 (reduces memory, may be less stable)
  * **Standard**: 8 (good balance)
  * **Maximum**: 12-16 (more stable, uses more memory)

**Convergence Thresholds**:

* ``scf_initial_ethr``: Initial energy threshold (default: 1.0e-2)
* ``scf_min_ethr``: Minimum energy threshold (default: 1.0e-13)
* ``scf_ethr_factor``: Energy threshold reduction factor (default: 0.1)

Troubleshooting
--------------

**Slow Convergence**:
* Try a more aggressive mixing parameter (increase ``scf_alpha``)
* Use more DIIS histories (increase ``diis_histories``)
* Consider switching to Local-TF mixing for inhomogeneous systems

**Oscillations**:
* Reduce mixing parameter (decrease ``scf_alpha``)
* Use more conservative acceleration (decrease ``scf_beta``)
* Increase DIIS histories

**No Convergence**:
* Check system classification
* Verify pseudopotentials
* Try different initial wavefunction guess
* Consider system-specific issues (charge transfer, etc.)

**Best Practices**:

1. **Start with system classification**: Let PWDFT choose the algorithm automatically
2. **Use recommended parameters**: The automatic defaults are well-tested
3. **Monitor convergence**: Watch for oscillations or slow convergence
4. **Adjust gradually**: Make small parameter changes
5. **Consider system type**: Different systems need different approaches

**Example Complete Input**:
.. code-block:: text

   memory 900 mb
   start calculation
   echo
   
   geometry noautosym noautoz center
   Cu 0 0 0
   Cu 0 0 1.8
   end
   
   nwpw
     simulation_cell
       SC 20.0
     end
     cutoff 60.0
     xc pbe96
     # Let system classification choose optimal algorithm
     # scf_algorithm 2  # Uncomment to override
     # scf_alpha 0.25   # Uncomment to override
     # scf_beta 0.1     # Uncomment to override
     # diis_histories 8 # Uncomment to override
     smear methfessel-paxton
     temperature 500
     loop 20 50
     monkhorst-pack 3 3 1
     initial_wavefunction_guess superposition
     mult 1
   end
   
   task pspw energy 