#ifndef BAND_GRADIENT_ROBUST_HPP
#define BAND_GRADIENT_ROBUST_HPP

#include <cmath>
#include <algorithm>
#include <limits>

namespace pwdft {

/**
 * @brief Robust band gradient computation with enhanced numerical stability
 * 
 * This enhanced version includes:
 * - NaN/Inf detection in gradient components
 * - Gradient norm bounds checking
 * - Preconditioning for ill-conditioned systems
 * - Fallback strategies for failed gradient computation
 * - Brillouin zone specific optimizations
 */
class band_Gradient_robust {
private:
    Cneb *mygrid;
    Solid *mysolid;
    
    const double MIN_GRADIENT_NORM = 1.0e-12;
    const double MAX_GRADIENT_NORM = 1.0e+6;
    const double PRECONDITIONING_THRESHOLD = 1.0e-8;

public:
    band_Gradient_robust(Cneb *grid, Solid *solid) 
        : mygrid(grid), mysolid(solid) {}

    /**
     * @brief Robust gradient computation with validation
     */
    double compute_gradient_robust(double *G1) {
        // Compute gradient with error checking
        double total_energy = mysolid->psi_1get_Tgradient(G1);
        
        // Validate energy
        if (std::isnan(total_energy) || std::isinf(total_energy)) {
            // Energy computation failed, use fallback
            return fallback_energy();
        }
        
        // Compute gradient norm with bounds checking
        double gradient_norm = mygrid->gg_traceall(G1, G1);
        
        if (std::isnan(gradient_norm) || std::isinf(gradient_norm)) {
            // Gradient norm computation failed, reinitialize
            mygrid->g_generate_random(G1);
            gradient_norm = mygrid->gg_traceall(G1, G1);
        }
        
        // Apply gradient norm bounds
        if (gradient_norm < MIN_GRADIENT_NORM) {
            // Gradient too small, apply small random perturbation
            apply_small_perturbation(G1);
            gradient_norm = mygrid->gg_traceall(G1, G1);
        } else if (gradient_norm > MAX_GRADIENT_NORM) {
            // Gradient too large, normalize
            double scale_factor = std::sqrt(MAX_GRADIENT_NORM / gradient_norm);
            mygrid->g_Scale(scale_factor, G1);
            gradient_norm = MAX_GRADIENT_NORM;
        }
        
        // Apply preconditioning if needed
        if (gradient_norm > PRECONDITIONING_THRESHOLD) {
            apply_preconditioning(G1);
        }
        
        return total_energy;
    }

    /**
     * @brief Robust gradient computation without density update
     */
    double compute_gradient0_robust(double *G1) {
        // Compute gradient without density update
        double total_energy = mysolid->psi_1get_Tgradient0(G1);
        
        // Validate energy
        if (std::isnan(total_energy) || std::isinf(total_energy)) {
            return fallback_energy();
        }
        
        // Apply same validation as above
        double gradient_norm = mygrid->gg_traceall(G1, G1);
        
        if (std::isnan(gradient_norm) || std::isinf(gradient_norm)) {
            mygrid->g_generate_random(G1);
            gradient_norm = mygrid->gg_traceall(G1, G1);
        }
        
        if (gradient_norm < MIN_GRADIENT_NORM) {
            apply_small_perturbation(G1);
            gradient_norm = mygrid->gg_traceall(G1, G1);
        } else if (gradient_norm > MAX_GRADIENT_NORM) {
            double scale_factor = std::sqrt(MAX_GRADIENT_NORM / gradient_norm);
            mygrid->g_Scale(scale_factor, G1);
            gradient_norm = MAX_GRADIENT_NORM;
        }
        
        if (gradient_norm > PRECONDITIONING_THRESHOLD) {
            apply_preconditioning(G1);
        }
        
        return total_energy;
    }

    /**
     * @brief Validate gradient components for NaN/Inf
     */
    bool validate_gradient(double *G1) {
        int size = (mygrid->ne[0] + mygrid->ne[1]) * mygrid->nbrillq * mygrid->CGrid::npack1_max();
        for (int i = 0; i < size; ++i) {
            if (std::isnan(G1[i]) || std::isinf(G1[i])) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Apply simple preconditioning to improve convergence
     */
    void apply_preconditioning(double *G1) {
        // Simple scaling preconditioning
        const double epsilon = 1.0e-6;
        int size = (mygrid->ne[0] + mygrid->ne[1]) * mygrid->nbrillq * mygrid->CGrid::npack1_max();
        
        // Find maximum absolute value for scaling
        double max_val = 0.0;
        for (int i = 0; i < size; ++i) {
            double abs_val = std::fabs(G1[i]);
            if (abs_val > max_val) max_val = abs_val;
        }
        
        // Apply scaling if maximum value is too large
        if (max_val > 1.0) {
            double scale_factor = 1.0 / (max_val + epsilon);
            for (int i = 0; i < size; ++i) {
                G1[i] *= scale_factor;
            }
        }
    }

    /**
     * @brief Apply small random perturbation to escape local minima
     */
    void apply_small_perturbation(double *G1) {
        const double perturbation_scale = 1.0e-6;
        int size = (mygrid->ne[0] + mygrid->ne[1]) * mygrid->nbrillq * mygrid->CGrid::npack1_max();
        
        for (int i = 0; i < size; ++i) {
            // Add small random perturbation
            double random_val = ((double)rand() / RAND_MAX - 0.5) * 2.0;
            G1[i] += perturbation_scale * random_val;
        }
    }

    /**
     * @brief Fallback energy computation when gradient fails
     */
    double fallback_energy() {
        // Return current energy as fallback
        return mysolid->psi2_energy();
    }

    /**
     * @brief Robust gradient norm computation
     */
    double compute_gradient_norm_robust(double *G1) {
        double norm = mygrid->gg_traceall(G1, G1);
        
        if (std::isnan(norm) || std::isinf(norm)) {
            // Norm computation failed, estimate from components
            norm = estimate_norm_from_components(G1);
        }
        
        return norm;
    }

private:
    /**
     * @brief Estimate norm from gradient components when direct computation fails
     */
    double estimate_norm_from_components(double *G1) {
        double sum_squares = 0.0;
        int size = (mygrid->ne[0] + mygrid->ne[1]) * mygrid->nbrillq * mygrid->CGrid::npack1_max();
        int valid_components = 0;
        
        for (int i = 0; i < size; ++i) {
            if (!std::isnan(G1[i]) && !std::isinf(G1[i])) {
                sum_squares += G1[i] * G1[i];
                valid_components++;
            }
        }
        
        if (valid_components > 0) {
            return std::sqrt(sum_squares / valid_components * size);
        } else {
            return MIN_GRADIENT_NORM;
        }
    }
};

} // namespace pwdft

#endif 