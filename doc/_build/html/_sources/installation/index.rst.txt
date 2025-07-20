Installation Guide
=================

This section provides comprehensive installation instructions for PWDFT on various platforms and architectures.

Introduction
-----------

PWDFT can be installed on various platforms and architectures. The installation process involves compiling the source code and setting up the environment.

**Prerequisites**:
* C++ compiler (GCC, Clang, or Intel)
* MPI implementation (OpenMPI or MPICH)
* BLAS/LAPACK libraries
* FFTW library
* CMake build system

Basic Installation
-----------------

**Quick Installation**:
.. code-block:: bash

   git clone https://github.com/ebylaska/PWDFT.git
   cd PWDFT
   cmake -S Nwpw/ -B build
   cd build
   make

**Alternative Build Method**:
.. code-block:: bash

   mkdir build
   cd build
   cmake ../Nwpw
   make

Standard CMake Build Commands
----------------------------

.. code-block:: bash

   cmake [<options>] <path-to-source>
   $ mkdir build ; cd build
   $ cmake ../src

   cmake [<options>] -S <path-to-source> -B <path-to-build>
   $ cmake -S src -B build

   cmake [<options>] <path-to-existing-build>
   $ cd build
   $ cmake .

Platform-Specific Installation
-----------------------------

ALCF Aurora/Sunspot
~~~~~~~~~~~~~~~~~~

**Required Modules**:
.. code-block:: bash

   module restore
   module load cmake

**Getting the Code and Building**:
.. code-block:: bash

   export HTTP_PROXY=http://proxy.alcf.anl.gov:3128
   export HTTPS_PROXY=http://proxy.alcf.anl.gov:3128
   export http_proxy=http://proxy.alcf.anl.gov:3128
   export https_proxy=http://proxy.alcf.anl.gov:3128
   git config --global http.proxy http://proxy.alcf.anl.gov:3128
   git clone https://github.com/ebylaska/PWDFT.git

   cd PWDFT
   cmake -H. -Bbuild_sycl -DNWPW_SYCL=On -DCMAKE_CXX_COMPILER=icpx -DCMAKE_C_COMPILER=icx -DCMAKE_Fortran_COMPILER=ifx ./Nwpw

**Running**:
.. code-block:: bash

   qsub -l select=1 -l walltime=30:00 -A catalysis_aesp_CNDA -q lustre_scaling -I
   qsub -l select=4 -l walltime=30:00 -l filesystems=flare -A ExaCatChem -q debug-scaling -I

   export MPIR_CVAR_ENABLE_GPU=0
   export OMP_NUM_THREADS=1
   mpiexec -n 12 --ppn 12 --cpu-bind list:0-7:8-15:16-23:24-31:32-39:40-47:52-59:60-67:68-75:76-83:84-91:92-99 --mem-bind list:0:0:0:0:0:0:1:1:1:1:1:1 --env OMP_NUM_THREADS=1 gpu_tile_compact.sh ../../build_sycl/pwdft cco-cu_surf30.nw

OLCF Frontier
~~~~~~~~~~~~

**Login and Modules**:
.. code-block:: bash

   ssh frontier.olcf.ornl.gov
   module load amd-mixed

**CMake Build/Install**:
.. code-block:: bash

   cmake -Bbuild_hip -DNWPW_HIP=ON ./Nwpw -DGPU_TARGETS=gfx90a
   cd build_hip
   make -j

**Job Submission Script** (save as `job_submit.sbatch`):
.. code-block:: bash

   #!/bin/bash

   #SBATCH -A
   #SBATCH -J
   #SBATCH -o %x-%j.out
   #SBATCH -t 00:15:00
   #SBATCH -N 1
   #SBATCH -C nvme
   #SBATCH --mail-user=
   #SBATCH --mail-type=END

   module load amd-mixed
   module list

   export MPICH_GPU_SUPPORT_ENABLED=0
   export OMP_NUM_THREADS=1
   export CRAYPE_LINK_TYPE=dynamic

   date

   NNODES=1
   NRANKS_PER_NODE=8
   NTOTRANKS=$(( NNODES * NRANKS_PER_NODE ))

   PWDFT_EXE=
   PWDFT_INPUT=

   srun -N${NNODES} -n${NTOTRANKS} -c1 --ntasks-per-gpu=1 --gpus-per-node=8 --gpu-bind=closest ${PWDFT_EXE} ${PWDFT_INPUT}

NERSC Perlmutter
~~~~~~~~~~~~~~~

**CMake Build/Install**:
.. code-block:: bash

   cmake -H. -Bbuild_cuda -DNWPW_CUDA=ON ./Nwpw -DCMAKE_CUDA_ARCHITECTURES=80 -DCUDA_cublas_LIBRARY=${CRAY_CUDATOOLKIT_DIR}/../../math_libs/${CRAY_CUDATOOLKIT_VERSION#*_}/lib64/libcublas.so -DCUDA_cufft_LIBRARY=${CRAY_CUDATOOLKIT_DIR}/../../math_libs/${CRAY_CUDATOOLKIT_VERSION#*_}/lib64/libcufft.so -DCUDA_cusolver_LIBRARY=${CRAY_CUDATOOLKIT_DIR}/../../math_libs/${CRAY_CUDATOOLKIT_VERSION#*_}/lib64/libcusolver.so
   cd build_cuda
   make

NERSC Cori-Haswell
~~~~~~~~~~~~~~~~~~

**Required Modules**:
.. code-block:: bash

   module purge
   module load cmake/3.20.2
   module load PrgEnv-intel
   module load craype-haswell
   module load openmpi

**Build Instructions** (starting from PWDFT directory):
.. code-block:: bash

   mkdir build
   cd build
   cmake -DCMAKE_CXX_COMPILER=CC ../Nwpw/

**Running on Cori-Haswell**:
.. code-block:: bash

   salloc --nodes 1 --qos interactive --time 01:00:00 --constraint haswell
   srun -n <num_mpi_processes> -c <cpus_per_task> a.out
   srun -n <num_mpi_processes> -c <cpus_per_task> pwdft

**Example Run**:
.. code-block:: bash

   salloc --nodes 1 --qos interactive --time 01:00:00 --constraint haswell
   cd PWDFT/QA/C2_steepest_descent
   srun -n 24 ../../build/pwdft c2-sd.nw

ALCF Polaris
~~~~~~~~~~~

**Login and Modules**:
.. code-block:: bash

   ssh polaris.alcf.anl.gov
   module load PrgEnv-gnu cudatoolkit-standalone cmake

**CMake Build/Install**:
.. code-block:: bash

   cmake -Bbuild_cuda -DNWPW_CUDA=ON ./Nwpw -DCMAKE_CUDA_ARCHITECTURES=80
   cd build_cuda
   make -j

**GPU Affinity Script** (save as `gpu_bind_affinity.sh`):
.. code-block:: bash

   #!/bin/bash
   num_gpus=4
   # need to assign GPUs in reverse order due to topology
   # See Polaris Device Affinity Information https://www.alcf.anl.gov/support/user-guides/polaris/hardware-overview/machine-overview/index.html
   gpu=$((${num_gpus} - 1 - ${PMI_LOCAL_RANK} % ${num_gpus}))

   unset CUDA_VISIBLE_DEVICES
   if [ ${PMI_LOCAL_RANK} -ne 4 ]; then
      export CUDA_VISIBLE_DEVICES=$gpu
   fi
   #echo "RANK= ${PMI_RANK} LOCAL_RANK= ${PMI_LOCAL_RANK} gpu= ${CUDA_VISIBLE_DEVICES}"
   exec "$@"

**Job Submission Script** (save as `polaris_submit.pbs`):
.. code-block:: bash

   #!/bin/bash

   #PBS -N develop
   #PBS -l select=8:system=polaris
   #PBS -l place=scatter
   #PBS -l walltime=01:00:00
   #PBS -l filesystems=home:eagle
   #PBS -A
   #PBS -q workq

   module load PrgEnv-gnu cudatoolkit-standalone
   module list

   export MPICH_GPU_SUPPORT_ENABLED=0
   export CRAYPE_LINK_TYPE=dynamic
   env
   nvidia-smi topo -m

   cd ${PBS_O_WORKDIR}

   NNODES=`wc -l < $PBS_NODEFILE`
   NRANKS_PER_NODE=4
   NTHREADS=1

   NTOTRANKS=$(( NNODES * NRANKS_PER_NODE ))
   echo "NUM_OF_NODES= ${NNODES} TOTAL_NUM_RANKS= ${NTOTRANKS} RANKS_PER_NODE= ${NRANKS_PER_NODE} THREADS_PER_RANK= ${NTHREADS}"

   PWDFT_EXE=
   PWDFT_INPUT=

   mpiexec -n ${NTOTRANKS} --ppn ${NRANKS_PER_NODE} --mem-bind list:0:1:2:3 --cpu-bind list:0-7:8-15:16-23:24-31 --env OMP_NUM_THREADS=${NTHREADS} ./gpu_bind_affinity.sh ${PWDFT_EXE} ${PWDFT_INPUT}

NERSC Cori-CUDA
~~~~~~~~~~~~~~

**Required Modules**:
.. code-block:: bash

   module unload impi
   module load PrgEnv-intel
   module load cmake
   module load cudatoolkit

**Build Instructions** (starting from PWDFT directory):
.. code-block:: bash

   mkdir build_cuda
   cd build-cuda
   cmake -DNWPW_CUDA=ON ../Nwpw/

**Running on Cori-CUDA**:
.. code-block:: bash

   module load cgpu
   salloc -C gpu -t 60 -c 10 -G 1 -q interactive -A <account>
   salloc -C gpu -t 60 -c 10 -G 1 -q interactive -A mp119

Shared Library Generation
------------------------

To generate a shared library, clean the build directory and regenerate cmake with:

.. code-block:: bash

   cmake ../Nwpw -DMAKE_LIBRARY=true

On Linux, use:
.. code-block:: bash

   cmake ../Nwpw -DMAKE_LIBRARY=true -DCMAKE_POSITION_INDEPENDENT_CODE=ON

Compile and generate the shared library:
.. code-block:: bash

   make

The shared library (``libpwdft.dylib`` on macOS or ``libpwdft.so`` on Linux) should be generated in the build directory.

**Example Header for Function Calls**:
.. code-block:: cpp

   #include <string>
   #include "mpi.h"

   namespace pwdft {
   using namespace pwdft;

   extern char *util_date();
   extern void seconds(double *);
   extern int cpsd(MPI_Comm, std::string&);
   extern int cpmd(MPI_Comm, std::string&);
   extern int pspw_minimizer(MPI_Comm, std::string&);
   extern int pspw_geovib(MPI_Comm, std::string&);
   }

**Example Function Call**:
.. code-block:: cpp

   ierr += pwdft::pspw_geovib(MPI_COMM_WORLD,nwinput);

**Using Shared Library on macOS**:
.. code-block:: bash

   mpic++ test.cpp ../build-shared/libpwdft.dylib
   setenv DYLD_LIBRARY_PATH /Users/bylaska/Codes/PWDFT/build-shared
   a.out

**Using Shared Library on Linux**:
.. code-block:: bash

   mpic++ test.cpp ../build-shared/libpwdft.so
   setenv DYLD_LIBRARY_PATH /Users/bylaska/Codes/PWDFT/build-shared
   a.out

Environment Setup
----------------

* Add the PWDFT binary directory to your PATH
* Set up any required environment variables
* Configure your MPI environment

Troubleshooting
--------------

* Check that all dependencies are properly installed
* Verify MPI is working correctly
* Ensure sufficient disk space for compilation
* Check compiler compatibility
* For GPU builds, ensure CUDA/HIP/SYCL toolchains are properly configured

For more detailed information, see the source code and build configuration files. 