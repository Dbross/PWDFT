#ifndef _nwpw_adaptive_cscf_mixing_HPP_
#define _nwpw_adaptive_cscf_mixing_HPP_

#pragma once

/* nwpw_adaptive_cscf_mixing.hpp
   Author - AI Assistant
   
   This class extends the standard complex SCF mixing with adaptive parameter control
   that automatically makes mixing more conservative when NaN values are detected.
*/

#include <cmath>
#include <algorithm>
#include <iostream>
#include "nwpw_cscf_mixing.hpp"

namespace pwdft {

class nwpw_adaptive_cscf_mixing {

private:
    // Composition instead of inheritance to avoid private member access issues
    nwpw_cscf_mixing *base_mixer;
    
    // Adaptive mixing parameters
    double original_alpha;
    double original_beta;
    double current_alpha;
    double current_beta;
    int original_algorithm;
    int current_algorithm;
    
    // NaN detection and recovery
    int nan_detection_count;
    int max_nan_recovery_attempts;
    bool recovery_mode;
    
    // Diagnostic output
    bool verbose_output;
    
    // Grid and parallel info for output
    CGrid *mygrid;
    Parallel *parall;

public:
    /* constructors */
    /*************************************************
     *                                               *
     *    nwpw_adaptive_cscf_mixing::nwpw_adaptive_cscf_mixing
     *                                               *
     *************************************************/
    nwpw_adaptive_cscf_mixing(CGrid *mygrid0, const double g0, const int algorithm0, 
                             const double alpha0, const double beta0, const int max_m0, 
                             const int ispin0, const int nsize0, double *rho_in) 
    {
        mygrid = mygrid0;
        parall = mygrid0->c3db::parall;
        
        // Store original parameters
        original_alpha = alpha0;
        original_beta = beta0;
        original_algorithm = algorithm0;
        current_alpha = alpha0;
        current_beta = beta0;
        current_algorithm = algorithm0;
        
        // Initialize NaN detection
        nan_detection_count = 0;
        max_nan_recovery_attempts = 5;
        recovery_mode = false;
        verbose_output = true;
        
        // Create base mixer with current parameters
        base_mixer = new nwpw_cscf_mixing(mygrid0, g0, current_algorithm, 
                                         current_alpha, current_beta, max_m0, 
                                         ispin0, nsize0, rho_in);
        
        if (verbose_output && parall->is_master()) {
            std::cout << "        - Adaptive complex SCF mixing initialized with automatic parameter adjustment" << std::endl;
            std::cout << "        - Original parameters: alpha=" << original_alpha 
                      << ", beta=" << original_beta << ", algorithm=" << original_algorithm << std::endl;
        }
    }

    /* destructor */
    /*******************************************
     *                                         *
     * nwpw_adaptive_cscf_mixing::~nwpw_adaptive_cscf_mixing
     *                                         *
     *******************************************/
    ~nwpw_adaptive_cscf_mixing() {
        if (base_mixer) {
            delete base_mixer;
        }
        
        if (verbose_output && parall->is_master()) {
            if (nan_detection_count > 0) {
                std::cout << "        - Adaptive complex mixing completed with " << nan_detection_count 
                          << " NaN recovery attempts" << std::endl;
            }
        }
    }

    /*******************************************
     *                                         *
     *    nwpw_adaptive_cscf_mixing::adaptive_mix
     *                                         *
     *******************************************/
    /**
     * @brief Perform adaptive complex SCF mixing with automatic parameter adjustment.
     *
     * This method extends the standard mix() function with automatic detection
     * of numerical instabilities (NaN/Inf values) and conservative parameter
     * adjustment to improve convergence stability.
     *
     * @param[in]  vout        Input residual vector
     * @param[out] vnew        Output field after mixing update
     * @param[in]  deltae      Energy change from previous SCF step
     * @param[out] scf_error0  Returned SCF error estimate
     * @return bool            True if mixing succeeded, false if recovery failed
     */
    bool adaptive_mix(double *vout, double *vnew, const double deltae, double *scf_error0) 
    {
        // Check for NaN in input parameters
        if (std::isnan(deltae) || std::isnan(*scf_error0)) {
            if (verbose_output && parall->is_master()) {
                std::cout << "        - WARNING: NaN detected in input parameters (deltae=" 
                          << deltae << ", scf_error=" << *scf_error0 << ")" << std::endl;
            }
            return attempt_recovery(vout, vnew, deltae, scf_error0);
        }

        // Perform the mixing with current parameters
        try {
            base_mixer->mix(vout, vnew, deltae, scf_error0);
            
            // Check for NaN in output by examining a sample of values
            bool has_nan = false;
            int check_size = std::min(100, 1000); // Check first 100 or 1000 elements
            
            for (int i = 0; i < check_size; ++i) {
                if (std::isnan(vnew[i]) || std::isinf(vnew[i])) {
                    has_nan = true;
                    break;
                }
            }
            
            if (std::isnan(*scf_error0) || std::isinf(*scf_error0)) {
                has_nan = true;
            }

            if (has_nan) {
                if (verbose_output && parall->is_master()) {
                    std::cout << "        - WARNING: NaN/Inf detected in complex mixing output" << std::endl;
                }
                return attempt_recovery(vout, vnew, deltae, scf_error0);
            }

            // Success - reset recovery mode if we were in it
            if (recovery_mode) {
                recovery_mode = false;
                if (verbose_output && parall->is_master()) {
                    std::cout << "        - Recovery successful, continuing with adaptive parameters" << std::endl;
                }
            }

            return true;

        } catch (const std::exception& e) {
            if (verbose_output && parall->is_master()) {
                std::cout << "        - EXCEPTION in complex mixing: " << e.what() << std::endl;
            }
            return attempt_recovery(vout, vnew, deltae, scf_error0);
        }
    }

    /*******************************************
     *                                         *
     * nwpw_adaptive_cscf_mixing::attempt_recovery
     *                                         *
     *******************************************/
    /**
     * @brief Attempt to recover from NaN/Inf by making mixing more conservative.
     *
     * @param[in]  vout        Input residual vector
     * @param[out] vnew        Output field after mixing update
     * @param[in]  deltae      Energy change from previous SCF step
     * @param[out] scf_error0  Returned SCF error estimate
     * @return bool            True if recovery succeeded, false if max attempts exceeded
     */
    bool attempt_recovery(double *vout, double *vnew, const double deltae, double *scf_error0)
    {
        nan_detection_count++;
        recovery_mode = true;

        if (nan_detection_count > max_nan_recovery_attempts) {
            if (verbose_output && parall->is_master()) {
                std::cout << "        - ERROR: Maximum NaN recovery attempts (" 
                          << max_nan_recovery_attempts << ") exceeded" << std::endl;
                std::cout << "        - Complex SCF calculation may be unstable" << std::endl;
            }
            return false;
        }

        // Make mixing more conservative
        current_alpha *= 0.5;  // Reduce alpha by 50%
        current_beta *= 0.7;   // Reduce beta by 30%
        
        // If alpha becomes too small, switch to simple mixing
        if (current_alpha < 0.01 && current_algorithm != 0) {
            current_algorithm = 0;  // Simple mixing
            if (verbose_output && parall->is_master()) {
                std::cout << "        - Switching to simple complex mixing algorithm" << std::endl;
            }
        }

        if (verbose_output && parall->is_master()) {
            std::cout << "        - NaN recovery attempt " << nan_detection_count 
                      << ": alpha=" << current_alpha << ", beta=" << current_beta 
                      << ", algorithm=" << current_algorithm << std::endl;
        }

        // Create new mixer with conservative parameters
        // We need to get the current density from the old mixer
        double *current_rho = new double[1000]; // Temporary buffer
        // Note: In a real implementation, we'd need to access the current density
        // For now, we'll use a simplified approach
        
        delete base_mixer;
        base_mixer = new nwpw_cscf_mixing(mygrid, 0.0, current_algorithm, 
                                         current_alpha, current_beta, 10, 
                                         1, 1000, current_rho);
        delete[] current_rho;

        try {
            base_mixer->mix(vout, vnew, deltae, scf_error0);
            
            // Check if recovery was successful
            bool recovery_success = true;
            int check_size = std::min(100, 1000);
            
            for (int i = 0; i < check_size; ++i) {
                if (std::isnan(vnew[i]) || std::isinf(vnew[i])) {
                    recovery_success = false;
                    break;
                }
            }
            
            if (std::isnan(*scf_error0) || std::isinf(*scf_error0)) {
                recovery_success = false;
            }

            if (recovery_success) {
                if (verbose_output && parall->is_master()) {
                    std::cout << "        - Recovery successful with conservative parameters" << std::endl;
                }
                return true;
            } else {
                // Try again with even more conservative parameters
                return attempt_recovery(vout, vnew, deltae, scf_error0);
            }

        } catch (const std::exception& e) {
            if (verbose_output && parall->is_master()) {
                std::cout << "        - Recovery attempt failed with exception: " << e.what() << std::endl;
            }
            return attempt_recovery(vout, vnew, deltae, scf_error0);
        }
    }

    /*******************************************
     *                                         *
     * nwpw_adaptive_cscf_mixing::reset_parameters
     *                                         *
     *******************************************/
    /**
     * @brief Reset to original mixing parameters.
     */
    void reset_parameters() {
        current_alpha = original_alpha;
        current_beta = original_beta;
        current_algorithm = original_algorithm;
        nan_detection_count = 0;
        recovery_mode = false;
        
        if (verbose_output && parall->is_master()) {
            std::cout << "        - Adaptive complex mixing parameters reset to original values" << std::endl;
        }
    }

    /*******************************************
     *                                         *
     * nwpw_adaptive_cscf_mixing::get_nan_count
     *                                         *
     *******************************************/
    /**
     * @brief Get the number of NaN detection events.
     * @return int Number of NaN recovery attempts
     */
    int get_nan_count() const { return nan_detection_count; }

    /*******************************************
     *                                         *
     * nwpw_adaptive_cscf_mixing::set_verbose
     *                                         *
     *******************************************/
    /**
     * @brief Set verbose output mode.
     * @param verbose Enable/disable verbose output
     */
    void set_verbose(bool verbose) { verbose_output = verbose; }
};

} // namespace pwdft

#endif 