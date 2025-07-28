#include "Parallel.hpp"
#include <cmath>
#include <mpi.h>

namespace pwdft {

/**
 * @brief Compute global norm of a wavefunction array using MPI_Allreduce
 * 
 * This function computes the L2 norm of a wavefunction array across all MPI processes
 * using MPI_Allreduce to ensure the result is consistent across all ranks.
 * 
 * @param parall Pointer to the Parallel object containing MPI communicator
 * @param array Pointer to the wavefunction array
 * @param size Local size of the array on this process
 * @return Global L2 norm of the array
 */
double global_norm(Parallel *parall, const double *array, int size) {
    double local_sum = 0.0;
    
    // Compute local sum of squares
    for (int i = 0; i < size; ++i) {
        local_sum += array[i] * array[i];
    }
    
    // Global reduction using Parallel::SumAll
    double global_sum = parall->SumAll(0, local_sum);
    
    return std::sqrt(global_sum);
}

/**
 * @brief Compute global dot product of two wavefunction arrays using MPI_Allreduce
 * 
 * This function computes the dot product of two wavefunction arrays across all MPI processes
 * using MPI_Allreduce to ensure the result is consistent across all ranks.
 * 
 * @param parall Pointer to the Parallel object containing MPI communicator
 * @param array1 Pointer to the first wavefunction array
 * @param array2 Pointer to the second wavefunction array
 * @param size Local size of the arrays on this process
 * @return Global dot product of the arrays
 */
double global_dot_product(Parallel *parall, const double *array1, const double *array2, int size) {
    double local_sum = 0.0;
    
    // Compute local dot product
    for (int i = 0; i < size; ++i) {
        local_sum += array1[i] * array2[i];
    }
    
    // Global reduction using Parallel::SumAll
    double global_sum = parall->SumAll(0, local_sum);
    
    return global_sum;
}

/**
 * @brief Normalize a wavefunction array using global norm
 * 
 * This function normalizes a wavefunction array in place using the global norm
 * computed across all MPI processes.
 * 
 * @param parall Pointer to the Parallel object containing MPI communicator
 * @param array Pointer to the wavefunction array to normalize
 * @param size Local size of the array on this process
 * @return The global norm before normalization
 */
double global_normalize(Parallel *parall, double *array, int size) {
    double norm = global_norm(parall, array, size);
    
    // Avoid division by zero
    if (norm > 1.0e-12) {
        double scale = 1.0 / norm;
        for (int i = 0; i < size; ++i) {
            array[i] *= scale;
        }
    }
    
    return norm;
}

/**
 * @brief Compute global trace of wavefunction overlap matrix
 * 
 * This function computes the trace of the overlap matrix ⟨ψ|ψ⟩ across all MPI processes
 * using MPI_Allreduce to ensure the result is consistent across all ranks.
 * 
 * @param parall Pointer to the Parallel object containing MPI communicator
 * @param psi1 Pointer to the first wavefunction array
 * @param psi2 Pointer to the second wavefunction array (can be same as psi1)
 * @param size Local size of the arrays on this process
 * @return Global trace of the overlap matrix
 */
double global_trace_overlap(Parallel *parall, const double *psi1, const double *psi2, int size) {
    return global_dot_product(parall, psi1, psi2, size);
}

} // namespace pwdft 