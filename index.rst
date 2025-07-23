PWDFT Documentation
==================

**Plane-Wave Density Functional Theory for Materials Science**

.. toctree::
   :maxdepth: 2
   :caption: Contents:

   introduction/index
   installation/index
   tutorials/index
   theory/index
   api/index
   faq/index
   developers/index

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

Welcome to PWDFT
----------------

PWDFT is a high-performance, massively parallel implementation of Plane-Wave Density Functional Theory (PW-DFT) designed for materials science applications. This code solves the Kohn-Sham equations via a Self-Consistent Field (SCF) procedure, employing iterative eigensolvers to diagonalize the Hamiltonian.

**Key Features:**

* **High Performance**: Optimized for HPC systems with advanced parallelization
* **Comprehensive Functionals**: Support for LDA, GGA, hybrid, and vdW functionals
* **Multiple Pseudopotentials**: Norm-conserving, ultrasoft, and PAW potentials
* **Advanced Features**: DFT+U, spin-orbit coupling, and more
* **ASE Integration**: Seamless integration with Atomic Simulation Environment

**Quick Start:**

.. code-block:: bash

   # Basic SCF calculation
   pwdft < input.nw > output.out

   # With ASE Python interface
   from ase import Atoms
   from ase.calculators.pwdft import PWDFT
   
   atoms = Atoms('H2O', positions=[[0, 0, 0], [0.957, 0, 0], [0.24, 0.927, 0]])
   calc = PWDFT(xc='pbe96', cutoff=60.0)
   atoms.calc = calc
   energy = atoms.get_potential_energy()

**Example Input File:**

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
     scf ks-grassmann-cg anderson alpha 0.15
     smear methfessel-paxton
     temperature 500
     loop 20 20
     monkhorst-pack 3 3 1
     initial_wavefunction_guess superposition
     mult 1
   end
   
   task pspw energy

**Getting Help:**

* :doc:`tutorials/index` - Step-by-step tutorials
* :doc:`api/index` - Complete API reference
* :doc:`faq/index` - Common questions and troubleshooting

**Citing PWDFT:**

If you use PWDFT in your research, please cite:

.. code-block:: text

   @software{pwdft2024,
     title={PWDFT: Plane-Wave Density Functional Theory Code},
     author={PWDFT Development Team},
     year={2024},
     url={https://github.com/pwdft/pwdft}
   } 