#ifndef _MPIUTILS_H_
#define _MPIUTILS_H_

#include "Parallel.hpp"

namespace pwdft {

/**
 * @brief Compute global norm of a wavefunction array using MPI_Allreduce
 * 
 * @param parall Pointer to the Parallel object containing MPI communicator
 * @param array Pointer to the wavefunction array
 * @param size Local size of the array on this process
 * @return Global L2 norm of the array
 */
double global_norm(Parallel *parall, const double *array, int size);

/**
 * @brief Compute global dot product of two wavefunction arrays using MPI_Allreduce
 * 
 * @param parall Pointer to the Parallel object containing MPI communicator
 * @param array1 Pointer to the first wavefunction array
 * @param array2 Pointer to the second wavefunction array
 * @param size Local size of the arrays on this process
 * @return Global dot product of the arrays
 */
double global_dot_product(Parallel *parall, const double *array1, const double *array2, int size);

/**
 * @brief Normalize a wavefunction array using global norm
 * 
 * @param parall Pointer to the Parallel object containing MPI communicator
 * @param array Pointer to the wavefunction array to normalize
 * @param size Local size of the array on this process
 * @return The global norm before normalization
 */
double global_normalize(Parallel *parall, double *array, int size);

/**
 * @brief Compute global trace of wavefunction overlap matrix
 * 
 * @param parall Pointer to the Parallel object containing MPI communicator
 * @param psi1 Pointer to the first wavefunction array
 * @param psi2 Pointer to the second wavefunction array (can be same as psi1)
 * @param size Local size of the arrays on this process
 * @return Global trace of the overlap matrix
 */
double global_trace_overlap(Parallel *parall, const double *psi1, const double *psi2, int size);

} // namespace pwdft

#endif // _MPIUTILS_H_ 