#ifndef _NWPW_SCF_ADAPTIVE_THRESHOLD_HPP_
#define _NWPW_SCF_ADAPTIVE_THRESHOLD_HPP_

#pragma once

/* nwpw_scf_adaptive_threshold.hpp
   Author - AI Assistant (based on Quantum Espresso implementation)
   
   This module implements adaptive diagonalization threshold adjustment
   for SCF convergence, similar to Quantum Espresso's ethr adjustment logic.
*/

#include <cmath>
#include <algorithm>

namespace pwdft {

/**
 * @brief Adjust diagonalization threshold based on SCF progress
 * 
 * This function implements the Quantum Espresso logic for adaptive
 * diagonalization threshold adjustment during SCF iterations.
 * 
 * QE Logic: ethr = MIN(ethr, 0.1*dr2/nelec)
 * 
 * @param iteration     Current SCF iteration number (1-based)
 * @param dr2           Current density residual squared norm
 * @param nelec         Number of electrons in the system
 * @param current_ethr  Current diagonalization threshold
 * @param initial_ethr  Initial threshold for first iteration
 * @param min_ethr      Minimum allowed threshold
 * @param ethr_factor   Factor for threshold adjustment (default 0.1)
 * 
 * @return Adjusted diagonalization threshold
 */
inline double adjust_scf_diagonalization_threshold(
    const int iteration,
    const double dr2,
    const double nelec,
    const double current_ethr,
    const double initial_ethr,
    const double min_ethr,
    const double ethr_factor = 0.1)
{
    // First iteration: use initial threshold
    if (iteration == 1) {
        return initial_ethr;
    }
    
    // QE logic: ethr = MIN(ethr, factor*dr2/nelec)
    double new_threshold = std::min(current_ethr, ethr_factor * dr2 / std::max(1.0, nelec));
    
    // Prevent instability: ensure threshold >= min_ethr
    new_threshold = std::max(new_threshold, min_ethr);
    
    return new_threshold;
}

/**
 * @brief Check if adaptive threshold adjustment should be applied
 * 
 * @param adaptive_enabled  Whether adaptive threshold is enabled
 * @param iteration         Current SCF iteration
 * @param dr2               Current density residual
 * 
 * @return true if adaptive adjustment should be applied
 */
inline bool should_apply_adaptive_threshold(
    const bool adaptive_enabled,
    const int iteration,
    const double dr2)
{
    // Only apply if enabled and not first iteration
    if (!adaptive_enabled || iteration <= 1) {
        return false;
    }
    
    // Only apply if we have a meaningful density residual
    if (dr2 <= 0.0 || std::isnan(dr2) || std::isinf(dr2)) {
        return false;
    }
    
    return true;
}

/**
 * @brief Get recommended diagonalization threshold for current SCF state
 * 
 * This is the main interface function that combines all the logic
 * for adaptive threshold adjustment.
 * 
 * @param adaptive_enabled  Whether adaptive threshold is enabled
 * @param iteration         Current SCF iteration number
 * @param dr2               Current density residual squared norm
 * @param nelec             Number of electrons
 * @param current_ethr      Current diagonalization threshold
 * @param initial_ethr      Initial threshold
 * @param min_ethr          Minimum threshold
 * @param ethr_factor       Adjustment factor
 * 
 * @return Recommended diagonalization threshold
 */
inline double get_scf_diagonalization_threshold(
    const bool adaptive_enabled,
    const int iteration,
    const double dr2,
    const double nelec,
    const double current_ethr,
    const double initial_ethr,
    const double min_ethr,
    const double ethr_factor = 0.1)
{
    if (should_apply_adaptive_threshold(adaptive_enabled, iteration, dr2)) {
        return adjust_scf_diagonalization_threshold(
            iteration, dr2, nelec, current_ethr, 
            initial_ethr, min_ethr, ethr_factor);
    } else {
        // Return current threshold unchanged
        return current_ethr;
    }
}

} // namespace pwdft

#endif 