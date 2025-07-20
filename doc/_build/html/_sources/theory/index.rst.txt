Theory and Algorithms
====================

This section provides comprehensive documentation of the theoretical foundations and algorithms used in PWDFT.

.. toctree::
   :maxdepth: 2
   :caption: Theory Contents:

   scf_algorithms
   system_classification
   minimizers
   convergence_strategies

Introduction
-----------

PWDFT implements state-of-the-art algorithms for solving the Kohn-Sham equations in plane-wave basis sets. The code features intelligent system classification and adaptive parameter selection to optimize convergence for different types of systems.

Key Theoretical Components:

* **Self-Consistent Field (SCF) Algorithms**: Multiple algorithms for solving the Kohn-Sham equations
* **System Classification**: Automatic detection of system type (molecule, surface, bulk, etc.)
* **Minimizers**: Various energy minimization strategies
* **Convergence Strategies**: Adaptive thresholds and intelligent defaults

.. _scf-theory:

SCF Theory
----------

The Self-Consistent Field (SCF) procedure is the core algorithm in PWDFT. It solves the Kohn-Sham equations:

.. math::

   \left[-\frac{1}{2}\nabla^2 + V_{ext}(\mathbf{r}) + V_H(\mathbf{r}) + V_{xc}(\mathbf{r})\right]\psi_i(\mathbf{r}) = \epsilon_i\psi_i(\mathbf{r})

where:
* :math:`V_{ext}` is the external potential (nuclei)
* :math:`V_H` is the Hartree potential
* :math:`V_{xc}` is the exchange-correlation potential

The SCF procedure iteratively:
1. Constructs the Kohn-Sham Hamiltonian
2. Diagonalizes to find eigenvalues and eigenvectors
3. Builds the electron density
4. Updates the potentials
5. Repeats until convergence

.. _system-classification:

System Classification
--------------------

PWDFT automatically classifies systems based on their geometric and electronic properties to apply optimal convergence parameters. The classification considers:

* **Number of atoms**: Small systems vs. extended systems
* **Cell geometry**: Aspect ratios and dimensionality
* **Crystallinity**: Whether the system is crystalline
* **Electronic properties**: Metallic vs. insulating character

This enables intelligent default parameter selection for optimal convergence. 