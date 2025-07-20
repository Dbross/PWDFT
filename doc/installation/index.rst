Installation Guide
=================

This section provides installation instructions for PWDFT.

.. toctree::
   :maxdepth: 2
   :caption: Installation Contents:

   requirements
   compilation
   testing

Introduction
-----------

PWDFT can be installed on various platforms and architectures. The installation process involves compiling the source code and setting up the environment.

**Prerequisites**:
* C++ compiler (GCC, Clang, or Intel)
* MPI implementation (OpenMPI or MPICH)
* BLAS/LAPACK libraries
* FFTW library
* CMake build system

**Quick Installation**:
.. code-block:: bash

   git clone https://github.com/pwdft/pwdft.git
   cd pwdft
   mkdir build && cd build
   cmake ..
   make -j4
   make install

For detailed installation instructions, see the sections below. 