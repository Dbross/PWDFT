Convergence Strategies
=====================

PWDFT implements sophisticated convergence strategies to ensure reliable and efficient SCF convergence across diverse system types. Understanding these strategies is crucial for successful calculations.

Overview
--------

Convergence in PWDFT involves multiple criteria that must be satisfied simultaneously:

* **Energy Convergence**: Total energy change between iterations
* **Density Convergence**: Electron density change
* **Force Convergence**: Forces on atoms (for optimization)
* **Wavefunction Convergence**: Eigenvalue convergence

The convergence strategy adapts these criteria based on system classification and convergence behavior.

Convergence Criteria
-------------------

**Primary Convergence Criteria**:

.. code-block:: text

   Criterion              | Symbol | Default Value | Description
   --------------------- | ------ | ------------- | -----------
   Energy Threshold      | E_thr  | 1.0e-13      | Total energy change
   Density Threshold     | ρ_thr  | 1.0e-8       | Electron density change
   Force Threshold       | F_thr  | 1.0e-4       | Maximum force (Ha/Bohr)
   Wavefunction Threshold| ψ_thr  | 1.0e-6       | Eigenvalue change

**System-Specific Thresholds**:

.. code-block:: text

   System Type          | E_thr    | ρ_thr   | F_thr   | ψ_thr
   -------------------- | -------- | ------- | ------- | ------
   Molecules            | 1.0e-13  | 1.0e-8  | 1.0e-4  | 1.0e-6
   Surfaces/Slabs       | 1.0e-12  | 1.0e-7  | 1.0e-4  | 1.0e-6
   Bulk Metals          | 1.0e-13  | 1.0e-8  | 1.0e-4  | 1.0e-6
   Bulk Insulators      | 1.0e-13  | 1.0e-8  | 1.0e-4  | 1.0e-6
   Interfaces           | 1.0e-12  | 1.0e-7  | 1.0e-4  | 1.0e-6

Adaptive Convergence
-------------------

**Initial Threshold Strategy**:

PWDFT uses adaptive convergence thresholds that start loose and tighten as convergence progresses:

.. math::

   E_{thr}^{(n)} = \max(E_{min}, E_{thr}^{(0)} \times f^n)

where:
* :math:`E_{thr}^{(0)}` is the initial threshold (typically 1.0e-2)
* :math:`E_{min}` is the minimum threshold (typically 1.0e-13)
* :math:`f` is the reduction factor (typically 0.1)
* :math:`n` is the iteration number

**Threshold Reduction Schedule**:

.. code-block:: text

   Iteration Range | Energy Threshold | Density Threshold | Description
   --------------- | ---------------- | ----------------- | -----------
   1-5             | 1.0e-2           | 1.0e-3           | Initial relaxation
   6-10            | 1.0e-3           | 1.0e-4           | Coarse convergence
   11-15           | 1.0e-4           | 1.0e-5           | Fine convergence
   16-20           | 1.0e-5           | 1.0e-6           | Precise convergence
   21+             | 1.0e-6           | 1.0e-7           | Final convergence

**Adaptive Parameters**:

* ``scf_initial_ethr``: Initial energy threshold (default: 1.0e-2)
* ``scf_min_ethr``: Minimum energy threshold (default: 1.0e-13)
* ``scf_ethr_factor``: Energy threshold reduction factor (default: 0.1)
* ``scf_ethr_reduction_interval``: Iterations between reductions (default: 5)

Convergence Monitoring
---------------------

**Energy Convergence**:

Monitor the total energy change between iterations:

.. code-block:: text

   Iteration    Energy (Ha)    Delta E (Ha)    Threshold    Converged
   ---------    -----------    ------------    ----------   ----------
   1            -1234.5678     0.0000          1.0e-2       No
   2            -1234.5689     0.0011          1.0e-2       No
   3            -1234.5695     0.0006          1.0e-2       No
   4            -1234.5698     0.0003          1.0e-2       No
   5            -1234.5699     0.0001          1.0e-2       No
   6            -1234.5700     0.0001          1.0e-3       No
   7            -1234.5701     0.0001          1.0e-3       No
   ...
   15           -1234.5701     0.0000          1.0e-6       Yes

**Density Convergence**:

Monitor the electron density change:

.. code-block:: text

   Iteration    Density Change    Threshold    Converged
   ---------    --------------    ----------   ----------
   1            0.1234            1.0e-3       No
   2            0.0987            1.0e-3       No
   3            0.0765            1.0e-3       No
   4            0.0543            1.0e-3       No
   5            0.0321            1.0e-3       No
   6            0.0234            1.0e-4       No
   7            0.0156            1.0e-4       No
   ...
   15           0.0000            1.0e-7       Yes

**Force Convergence** (for optimization):

Monitor forces on atoms:

.. code-block:: text

   Iteration    Max Force (Ha/Bohr)    RMS Force (Ha/Bohr)    Converged
   ---------    --------------------    -------------------    ----------
   1            0.1234                 0.0456                 No
   2            0.0987                 0.0321                 No
   3            0.0765                 0.0234                 No
   4            0.0543                 0.0156                 No
   5            0.0321                 0.0098                 No
   ...
   15           0.0001                 0.0000                 Yes

**Wavefunction Convergence**:

Monitor eigenvalue changes:

.. code-block:: text

   Iteration    Max Eigenvalue Change    RMS Eigenvalue Change    Converged
   ---------    ---------------------    ----------------------    ----------
   1            0.1234                  0.0456                    No
   2            0.0987                  0.0321                    No
   3            0.0765                  0.0234                    No
   4            0.0543                  0.0156                    No
   5            0.0321                  0.0098                    No
   ...
   15           0.0001                  0.0000                    Yes

Convergence Acceleration
-----------------------

**DIIS Acceleration**:

DIIS (Direct Inversion in Iterative Subspace) accelerates convergence by minimizing the error in the iterative subspace:

.. math::

   \min_{\{c_i\}} \left\| \sum_{i=1}^{N} c_i \mathbf{e}_i \right\|^2

subject to :math:`\sum_{i=1}^{N} c_i = 1`, where :math:`\mathbf{e}_i` are the error vectors.

**Parameters**:
* ``diis_histories``: Number of previous iterations to use (default: 8)
* ``diis_start_iteration``: Iteration to start DIIS (default: 5)
* ``diis_error_threshold``: Error threshold for DIIS (default: 1.0e-3)

**Mixing Acceleration**:

Density mixing accelerates convergence by combining current and previous densities:

.. math::

   \rho_{n+1} = \alpha \rho_{in} + (1-\alpha) \rho_n

**Parameters**:
* ``scf_alpha``: Mixing parameter (default: 0.25)
* ``scf_beta``: Acceleration parameter (default: 0.1)

**Local-TF Preconditioning**:

Local-TF mixing applies a physical preconditioner for inhomogeneous systems:

.. math::

   \rho_{n+1}(\mathbf{r}) = \rho_n(\mathbf{r}) + \alpha \frac{q^2}{q^2 + q_0^2(\mathbf{r})} \Delta \rho(\mathbf{r})

where :math:`q_0^2(\mathbf{r}) = 4\pi \rho(\mathbf{r})` is the local Thomas-Fermi screening parameter.

Convergence Troubleshooting
--------------------------

**Slow Convergence**:

**Symptoms**:
* Energy changes slowly or oscillates
* Many iterations required
* Convergence stalls

**Solutions**:
1. **Check system classification**: Ensure appropriate algorithm is selected
2. **Adjust mixing parameters**: Try different ``scf_alpha`` values
3. **Increase DIIS histories**: Use more previous iterations
4. **Switch algorithms**: Try Local-TF for inhomogeneous systems
5. **Check initial guess**: Use better initial wavefunction guess

**Example adjustments**:
.. code-block:: text

   nwpw
     # For slow convergence
     scf_alpha 0.3        # More aggressive mixing
     scf_beta 0.15        # More aggressive acceleration
     diis_histories 12    # More DIIS histories
     loop 50 100          # More iterations
   end

**Oscillations**:

**Symptoms**:
* Energy oscillates between iterations
* Convergence criteria not met
* Unstable behavior

**Solutions**:
1. **Reduce mixing parameter**: Use smaller ``scf_alpha``
2. **Use more conservative acceleration**: Reduce ``scf_beta``
3. **Increase DIIS histories**: Use more stable DIIS
4. **Switch to simple mixing**: Use algorithm 1 temporarily
5. **Check system stability**: Verify pseudopotentials and geometry

**Example adjustments**:
.. code-block:: text

   nwpw
     # For oscillations
     scf_algorithm 1      # Simple mixing
     scf_alpha 0.1        # Conservative mixing
     scf_beta 0.05        # Conservative acceleration
     loop 100 200         # More iterations
   end

**No Convergence**:

**Symptoms**:
* Energy diverges
* Maximum iterations reached
* Calculation fails

**Solutions**:
1. **Check system classification**: Verify automatic detection
2. **Verify pseudopotentials**: Ensure appropriate potentials
3. **Check geometry**: Verify atomic positions and cell
4. **Try different initial guess**: Use atomic or random guess
5. **Reduce cutoff**: Use lower plane-wave cutoff initially

**Example adjustments**:
.. code-block:: text

   nwpw
     # For no convergence
     initial_wavefunction_guess atomic  # Different initial guess
     cutoff 30.0                       # Lower cutoff initially
     scf_algorithm 1                   # Simple mixing
     scf_alpha 0.1                     # Very conservative
     loop 200 500                      # Many iterations
   end

**System-Specific Issues**:

**Molecules**:
* Usually converge well with DIIS
* May need careful initial geometry
* Check for charge transfer issues

**Surfaces**:
* Use Local-TF mixing
* May need more iterations
* Check vacuum region size

**Bulk Systems**:
* Usually converge well with DIIS
* Check k-point sampling
* Verify cell parameters

**Interfaces**:
* Use Local-TF mixing
* May need careful geometry
* Check for charge transfer

Best Practices
-------------

**1. Start with System Classification**:
Let PWDFT automatically select optimal parameters:

.. code-block:: text

   nwpw
     # Let system classification choose parameters
     cutoff 60.0
     xc pbe96
     # Automatic algorithm and parameter selection
   end

**2. Monitor Convergence**:
Watch the output for convergence behavior:

.. code-block:: text

   # Look for these patterns in output:
   # - Energy decreasing monotonically
   # - Density changes decreasing
   # - Forces decreasing (for optimization)
   # - No oscillations
   # - Reasonable iteration count
```

**3. Use Appropriate Tolerances**:
Choose convergence criteria based on accuracy needs:

.. code-block:: text

   # High accuracy
   scf_min_ethr 1.0e-13
   
   # Standard accuracy
   scf_min_ethr 1.0e-12
   
   # Quick calculations
   scf_min_ethr 1.0e-10
```

**4. Adjust Gradually**:
Make small parameter changes:

.. code-block:: text

   # Start with defaults
   nwpw
     scf_alpha 0.25
     scf_beta 0.1
   end
   
   # If slow convergence, increase slightly
   nwpw
     scf_alpha 0.3
     scf_beta 0.15
   end
   
   # If oscillations, decrease slightly
   nwpw
     scf_alpha 0.2
     scf_beta 0.05
   end
```

**5. Consider System Type**:
Different systems need different approaches:

.. code-block:: text

   # Molecules - usually work well with defaults
   # Surfaces - may need Local-TF mixing
   # Bulk - usually work well with DIIS
   # Interfaces - may need careful tuning
```

**Example Complete Workflow**:

.. code-block:: text

   memory 900 mb
   start calculation
   echo
   
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
     # Let system classification choose optimal parameters
     # scf_algorithm 2  # Uncomment to override
     # scf_alpha 0.25   # Uncomment to override
     # scf_beta 0.1     # Uncomment to override
     # diis_histories 8 # Uncomment to override
     smear methfessel-paxton
     temperature 500
     loop 20 50
     monkhorst-pack 1 1 1
     initial_wavefunction_guess superposition
     mult 1
   end
   
   task pspw energy
```

**Convergence Checklist**:

Before starting a calculation:
- [ ] System classification is appropriate
- [ ] Pseudopotentials are suitable
- [ ] Geometry is reasonable
- [ ] Cutoff is appropriate
- [ ] k-point sampling is adequate

During calculation:
- [ ] Energy is decreasing
- [ ] No oscillations
- [ ] Convergence criteria are reasonable
- [ ] Iteration count is reasonable

After calculation:
- [ ] All convergence criteria met
- [ ] Energy is reasonable
- [ ] Forces are small (for optimization)
- [ ] Results are physically meaningful 