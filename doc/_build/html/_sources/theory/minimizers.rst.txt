Energy Minimizers
================

PWDFT implements various energy minimization algorithms for optimizing atomic positions and cell parameters. The choice of minimizer significantly impacts the efficiency and reliability of structural optimization.

Overview
--------

Energy minimization in PWDFT involves finding the minimum of the potential energy surface with respect to atomic coordinates and optionally cell parameters. Different minimizers are optimized for different optimization scenarios:

* **Steepest Descent**: Basic gradient-based optimization
* **Conjugate Gradient**: Improved gradient-based optimization
* **L-BFGS**: Quasi-Newton method with limited memory
* **FIRE**: Fast Inertial Relaxation Engine
* **Adaptive**: Intelligent algorithm selection

Minimizer Selection
------------------

PWDFT automatically selects the optimal minimizer based on system characteristics:

.. code-block:: text

   System Type          | Recommended Minimizer | Use Case
   -------------------- | --------------------- | --------
   Molecules            | L-BFGS               | Small systems, fast convergence
   Surfaces/Slabs       | Adaptive             | Complex energy landscapes
   Bulk Metals          | L-BFGS               | Periodic systems
   Bulk Insulators      | L-BFGS               | Crystalline systems
   Interfaces           | Adaptive             | Heterogeneous systems

Steepest Descent
---------------

**Type**: Basic gradient-based optimization
**Use Case**: Simple systems, educational purposes, debugging

**Description**:
Steepest descent follows the negative gradient of the energy:

.. math::

   \mathbf{r}_{n+1} = \mathbf{r}_n - \alpha \nabla E(\mathbf{r}_n)

where :math:`\alpha` is the step size.

**Parameters**:
* ``minimizer_step_size``: Step size (default: 0.01)
* ``minimizer_max_iterations``: Maximum iterations (default: 1000)
* ``minimizer_tolerance``: Energy tolerance (default: 1.0e-6)

**Advantages**:
* Simple and robust
* Easy to understand and debug
* Works for most systems

**Disadvantages**:
* Slow convergence
* May require many iterations
* Poor performance for complex energy landscapes

**Example Input**:
.. code-block:: text

   nwpw
     minimizer steepest_descent
     minimizer_step_size 0.01
     minimizer_max_iterations 1000
     minimizer_tolerance 1.0e-6
   end

Conjugate Gradient
-----------------

**Type**: Improved gradient-based optimization
**Use Case**: Small to medium systems, good convergence

**Description**:
Conjugate gradient uses conjugate directions to improve convergence:

.. math::

   \mathbf{d}_{n+1} = -\nabla E(\mathbf{r}_{n+1}) + \beta_n \mathbf{d}_n

where :math:`\beta_n` is the conjugate gradient parameter.

**Parameters**:
* ``minimizer_step_size``: Step size (default: 0.01)
* ``minimizer_max_iterations``: Maximum iterations (default: 1000)
* ``minimizer_tolerance``: Energy tolerance (default: 1.0e-6)

**Advantages**:
* Better convergence than steepest descent
* Memory efficient
* Good for small systems

**Disadvantages**:
* May struggle with complex energy landscapes
* Requires careful step size tuning

**Example Input**:
.. code-block:: text

   nwpw
     minimizer conjugate_gradient
     minimizer_step_size 0.01
     minimizer_max_iterations 1000
     minimizer_tolerance 1.0e-6
   end

L-BFGS (Limited Memory Broyden-Fletcher-Goldfarb-Shanno)
-------------------------------------------------------

**Type**: Quasi-Newton method with limited memory
**Use Case**: **Recommended for most systems** (molecules, bulk)

**Description**:
L-BFGS approximates the inverse Hessian using a limited number of vectors, making it memory efficient while providing fast convergence.

**Mathematical Foundation**:
L-BFGS updates the inverse Hessian approximation:

.. math::

   H_{n+1} = (I - \rho_n s_n y_n^T) H_n (I - \rho_n y_n s_n^T) + \rho_n s_n s_n^T

where :math:`s_n = \mathbf{r}_{n+1} - \mathbf{r}_n` and :math:`y_n = \nabla E_{n+1} - \nabla E_n`.

**Parameters**:
* ``minimizer_step_size``: Initial step size (default: 0.01)
* ``minimizer_max_iterations``: Maximum iterations (default: 1000)
* ``minimizer_tolerance``: Energy tolerance (default: 1.0e-6)
* ``lbfgs_histories``: Number of vectors to store (default: 8)

**Advantages**:
* Fast convergence for most systems
* Memory efficient
* Robust and well-tested
* Good for molecules and bulk systems

**Disadvantages**:
* May struggle with very complex energy landscapes
* Requires sufficient iteration history

**Example Input**:
.. code-block:: text

   nwpw
     minimizer lbfgs
     minimizer_step_size 0.01
     minimizer_max_iterations 1000
     minimizer_tolerance 1.0e-6
     lbfgs_histories 8
   end

**Recommended Settings by System Type**:

.. code-block:: text

   # Molecules
   minimizer lbfgs
   minimizer_step_size 0.01
   minimizer_max_iterations 500
   minimizer_tolerance 1.0e-6
   lbfgs_histories 8

   # Bulk Metals
   minimizer lbfgs
   minimizer_step_size 0.01
   minimizer_max_iterations 1000
   minimizer_tolerance 1.0e-6
   lbfgs_histories 8

   # Bulk Insulators
   minimizer lbfgs
   minimizer_step_size 0.01
   minimizer_max_iterations 1000
   minimizer_tolerance 1.0e-6
   lbfgs_histories 8

FIRE (Fast Inertial Relaxation Engine)
-------------------------------------

**Type**: Inertial dynamics-based optimization
**Use Case**: Complex energy landscapes, difficult convergence

**Description**:
FIRE uses inertial dynamics to escape local minima and find global minima more effectively.

**Mathematical Foundation**:
FIRE combines velocity and force updates:

.. math::

   \mathbf{v}_{n+1} = (1-\alpha) \mathbf{v}_n + \alpha \hat{\mathbf{F}}_n |\mathbf{v}_n|
   \mathbf{r}_{n+1} = \mathbf{r}_n + \Delta t \mathbf{v}_{n+1}

where :math:`\alpha` is the FIRE parameter and :math:`\hat{\mathbf{F}}_n` is the normalized force.

**Parameters**:
* ``minimizer_step_size``: Time step (default: 0.01)
* ``minimizer_max_iterations``: Maximum iterations (default: 1000)
* ``minimizer_tolerance``: Energy tolerance (default: 1.0e-6)
* ``fire_alpha``: FIRE parameter (default: 0.1)
* ``fire_finc``: FIRE increment factor (default: 1.1)
* ``fire_fdec``: FIRE decrement factor (default: 0.5)

**Advantages**:
* Can escape local minima
* Good for complex energy landscapes
* Effective for difficult systems

**Disadvantages**:
* More complex than L-BFGS
* May be less stable
* Requires careful parameter tuning

**Example Input**:
.. code-block:: text

   nwpw
     minimizer fire
     minimizer_step_size 0.01
     minimizer_max_iterations 1000
     minimizer_tolerance 1.0e-6
     fire_alpha 0.1
     fire_finc 1.1
     fire_fdec 0.5
   end

Adaptive Minimizer
-----------------

**Type**: Intelligent algorithm selection
**Use Case**: **Recommended for complex systems** (surfaces, interfaces)

**Description**:
The adaptive minimizer automatically selects the best optimization algorithm based on system characteristics and convergence behavior.

**Algorithm Selection Logic**:
1. **Initial Assessment**: Analyze system size, complexity, and energy landscape
2. **Algorithm Choice**: Select appropriate minimizer (L-BFGS, FIRE, etc.)
3. **Dynamic Switching**: Switch algorithms if convergence stalls
4. **Parameter Tuning**: Adjust parameters based on performance

**Parameters**:
* ``minimizer_step_size``: Initial step size (default: 0.01)
* ``minimizer_max_iterations``: Maximum iterations (default: 1000)
* ``minimizer_tolerance``: Energy tolerance (default: 1.0e-6)
* ``adaptive_switch_threshold``: Iterations before switching (default: 50)
* ``adaptive_energy_threshold``: Energy change threshold (default: 1.0e-5)

**Advantages**:
* Automatic algorithm selection
* Robust for complex systems
* Handles difficult convergence cases
* Optimized for surfaces and interfaces

**Disadvantages**:
* More complex than single algorithms
* May use more computational resources
* Requires understanding of multiple algorithms

**Example Input**:
.. code-block:: text

   nwpw
     minimizer adaptive
     minimizer_step_size 0.01
     minimizer_max_iterations 1000
     minimizer_tolerance 1.0e-6
     adaptive_switch_threshold 50
     adaptive_energy_threshold 1.0e-5
   end

**Recommended Settings by System Type**:

.. code-block:: text

   # Surfaces/Slabs
   minimizer adaptive
   minimizer_step_size 0.01
   minimizer_max_iterations 1000
   minimizer_tolerance 1.0e-6
   adaptive_switch_threshold 50
   adaptive_energy_threshold 1.0e-5

   # Interfaces
   minimizer adaptive
   minimizer_step_size 0.01
   minimizer_max_iterations 1000
   minimizer_tolerance 1.0e-6
   adaptive_switch_threshold 50
   adaptive_energy_threshold 1.0e-5

   # Complex systems
   minimizer adaptive
   minimizer_step_size 0.01
   minimizer_max_iterations 1500
   minimizer_tolerance 1.0e-6
   adaptive_switch_threshold 50
   adaptive_energy_threshold 1.0e-5

Parameter Guidelines
-------------------

**Step Size**:

* ``minimizer_step_size``: Controls the size of optimization steps
  * **Conservative**: 0.005-0.01 (slow but stable)
  * **Standard**: 0.01 (good balance)
  * **Aggressive**: 0.02-0.05 (fast but may be unstable)

**Tolerance**:

* ``minimizer_tolerance``: Energy convergence threshold
  * **Loose**: 1.0e-4 (quick optimization)
  * **Standard**: 1.0e-6 (good balance)
  * **Tight**: 1.0e-8 (high precision)

**Maximum Iterations**:

* ``minimizer_max_iterations``: Maximum optimization steps
  * **Small systems**: 200-500 iterations
  * **Medium systems**: 500-1000 iterations
  * **Large systems**: 1000-2000 iterations

**L-BFGS Parameters**:

* ``lbfgs_histories``: Number of vectors to store
  * **Minimum**: 4 (reduces memory, may be less stable)
  * **Standard**: 8 (good balance)
  * **Maximum**: 12-16 (more stable, uses more memory)

**FIRE Parameters**:

* ``fire_alpha``: FIRE mixing parameter
  * **Conservative**: 0.05-0.1 (stable)
  * **Standard**: 0.1 (good balance)
  * **Aggressive**: 0.15-0.2 (fast but may oscillate)

* ``fire_finc``: FIRE increment factor
  * **Standard**: 1.1 (good balance)
  * **Aggressive**: 1.2 (faster adaptation)

* ``fire_fdec``: FIRE decrement factor
  * **Standard**: 0.5 (good balance)
  * **Conservative**: 0.3 (more stable)

Convergence Monitoring
---------------------

**Energy Convergence**:
Monitor the total energy change between iterations:

.. code-block:: text

   Iteration    Energy (Ha)    Delta E (Ha)    Max Force (Ha/Bohr)
   ---------    -----------    ------------    -------------------
   1            -1234.5678     0.0000          0.1234
   2            -1234.5689     0.0011          0.0987
   3            -1234.5695     0.0006          0.0765
   ...
   15           -1234.5701     0.0000          0.0001

**Force Convergence**:
Check the maximum force on any atom:

.. code-block:: text

   Convergence criteria:
   - Energy change: 1.0e-6 Ha
   - Maximum force: 1.0e-4 Ha/Bohr
   - RMS force: 1.0e-5 Ha/Bohr

**Structural Convergence**:
Monitor atomic position changes:

.. code-block:: text

   Iteration    Max Displacement (Bohr)    RMS Displacement (Bohr)
   ---------    -----------------------    ----------------------
   1            0.1234                     0.0456
   2            0.0987                     0.0321
   3            0.0765                     0.0234
   ...
   15           0.0001                     0.0000

Troubleshooting
--------------

**Slow Convergence**:
* Try a larger step size (increase ``minimizer_step_size``)
* Use more L-BFGS histories (increase ``lbfgs_histories``)
* Consider switching to FIRE or adaptive minimizer
* Check if the system is near a local minimum

**Oscillations**:
* Reduce step size (decrease ``minimizer_step_size``)
* Use more conservative FIRE parameters
* Increase L-BFGS histories
* Consider switching to conjugate gradient

**No Convergence**:
* Check system classification
* Verify forces are reasonable
* Try different initial geometry
* Consider system-specific issues (charge transfer, etc.)

**Best Practices**:

1. **Start with system classification**: Let PWDFT choose the minimizer automatically
2. **Use recommended parameters**: The automatic defaults are well-tested
3. **Monitor convergence**: Watch energy and force convergence
4. **Adjust gradually**: Make small parameter changes
5. **Consider system type**: Different systems need different approaches

**Example Complete Input**:
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
     # Let system classification choose optimal minimizer
     # minimizer lbfgs  # Uncomment to override
     # minimizer_step_size 0.01  # Uncomment to override
     # minimizer_max_iterations 500  # Uncomment to override
     # minimizer_tolerance 1.0e-6  # Uncomment to override
     smear methfessel-paxton
     temperature 500
     loop 20 50
     monkhorst-pack 1 1 1
     initial_wavefunction_guess superposition
     mult 1
   end
   
   task pspw optimize 