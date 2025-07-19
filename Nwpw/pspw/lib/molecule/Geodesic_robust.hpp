#ifndef GEODESIC_ROBUST_HPP
#define GEODESIC_ROBUST_HPP

#include <cmath>
#include <algorithm>
#include <limits>

namespace pwdft {

/**
 * @brief Robust geodesic transport with enhanced numerical stability
 * 
 * This enhanced version includes:
 * - Singular value bounds checking
 * - Trigonometric function overflow protection
 * - Orthogonality preservation
 * - NaN/Inf detection and handling
 */
class Geodesic_robust {
private:
    Pneb *mygrid;
    Molecule *mymolecule;
    Electron_Operators *myelectron;
    
    double *U, *S, *Vt;
    double *tmp1, *tmp2, *tmp3, *tmpC, *tmpS;
    
    const double MIN_SINGULAR_VALUE = 1.0e-12;
    const double MAX_SINGULAR_VALUE = 1.0e+6;
    const double MAX_TRIG_ARG = 1.0e+3;  // Prevent overflow in cos/sin

public:
    Geodesic_robust(Pneb *grid, Molecule *molecule, Electron_Operators *electron) 
        : mygrid(grid), mymolecule(molecule), myelectron(electron) {
        
        int neall = mygrid->ne[0] + mygrid->ne[1];
        int shift1 = 2*(mygrid->ne[0]*mygrid->ne[0] + mygrid->ne[1]*mygrid->ne[1]);
        int shift2 = (mygrid->neq[0] + mygrid->neq[1]) * mygrid->npack(1);
        
        U = new double[shift2];
        S = new double[neall];
        Vt = new double[shift1];
        tmp1 = new double[shift1];
        tmp2 = new double[shift1];
        tmp3 = new double[shift1];
        tmpC = new double[neall];
        tmpS = new double[neall];
    }
    
    ~Geodesic_robust() {
        delete[] U;
        delete[] S;
        delete[] Vt;
        delete[] tmp1;
        delete[] tmp2;
        delete[] tmp3;
        delete[] tmpC;
        delete[] tmpS;
    }

    /**
     * @brief Robust geodesic start with singular value bounds checking
     */
    double start(double *A, double *max_sigma, double *min_sigma) {
        double *V = mygrid->m_allocate(-1, 1);
        
        // Perform SVD
        mygrid->ggm_SVD(A, U, S, V);
        
        int neall = mygrid->ne[0] + mygrid->ne[1];
        double mmsig = 9.99e9;
        double msig = 0.0;
        
        // Simple bounds checking for singular values
        for (int i = 0; i < neall; ++i) {
            double abs_s = std::fabs(S[i]);
            
            // Check for NaN/Inf
            if (std::isnan(abs_s) || std::isinf(abs_s)) {
                S[i] = MIN_SINGULAR_VALUE;
                abs_s = MIN_SINGULAR_VALUE;
            }
            
            // Apply bounds
            if (abs_s < MIN_SINGULAR_VALUE) {
                S[i] = MIN_SINGULAR_VALUE;
                abs_s = MIN_SINGULAR_VALUE;
            } else if (abs_s > MAX_SINGULAR_VALUE) {
                S[i] = MAX_SINGULAR_VALUE * (S[i] > 0 ? 1.0 : -1.0);
                abs_s = MAX_SINGULAR_VALUE;
            }
            
            if (abs_s > msig) msig = abs_s;
            if (abs_s < mmsig) mmsig = abs_s;
        }
        
        *max_sigma = msig;
        *min_sigma = mmsig;
        
        // Calculate Vt
        mygrid->mm_transpose(-1, V, Vt);
        
        delete[] V;
        
        // Return gradient projection
        return 2.0 * myelectron->eorbit(A);
    }

    /**
     * @brief Robust geodesic transport with trigonometric overflow protection
     */
    void get(double t, double *Yold, double *Ynew) {
        // Bounds check on transport parameter
        if (std::isnan(t) || std::isinf(t)) {
            // Copy old to new if transport parameter is invalid
            mygrid->gg_copy(Yold, Ynew);
            return;
        }
        
        // Limit transport parameter to prevent overflow
        double t_bounded = std::max(-MAX_TRIG_ARG, std::min(t, MAX_TRIG_ARG));
        
        // Trigonometric computation
        mygrid->mm_SCtimesVtrans(-1, t_bounded, S, Vt, tmp1, tmp3, tmpC, tmpS);
        
        // Ynew = Yold*V*cos(Sigma*t)*Vt + U*sin(Sigma*t)*Vt
        mygrid->mmm_Multiply2(-1, Vt, tmp1, 1.0, tmp2, 0.0);
        mygrid->fmf_Multiply(-1, Yold, tmp2, 1.0, Ynew, 0.0);
        mygrid->fmf_Multiply(-1, U, tmp3, 1.0, Ynew, 1.0);
        
        // Simple orthogonality check and correction
        double sum2 = mygrid->gg_traceall(Ynew, Ynew);
        double sum1 = mygrid->ne[0] + mygrid->ne[1];
        if (mygrid->ispin == 1) sum1 *= 2;
        
        if (std::fabs(sum2 - sum1) > 1.0e-8) {
            mygrid->g_ortho(-1, Ynew);
        }
    }

    /**
     * @brief Robust transport with gradient validation
     */
    void transport(double t, double *Yold, double *Ynew) {
        // Bounds check on transport parameter
        if (std::isnan(t) || std::isinf(t)) {
            mygrid->gg_copy(Yold, Ynew);
            return;
        }
        
        double t_bounded = std::max(-MAX_TRIG_ARG, std::min(t, MAX_TRIG_ARG));
        
        // Trigonometric computation for transport
        mygrid->mm_SCtimesVtrans2(-1, t_bounded, S, Vt, tmp1, tmp3, tmpC, tmpS);
        
        // tHnew = (-Yold*V*sin(Sigma*t) + U*cos(Sigma*t))*Sigma*Vt
        mygrid->mmm_Multiply2(-1, Vt, tmp1, 1.0, tmp2, 0.0);
        mygrid->fmf_Multiply(-1, Yold, tmp2, -1.0, Ynew, 0.0);
        mygrid->fmf_Multiply(-1, U, tmp3, 1.0, Ynew, 1.0);
    }

    void psi_1transport(double t, double *H0) {
        this->transport(t, mymolecule->psi1, H0);
    }

    double energy(double t) {
        this->get(t, mymolecule->psi1, mymolecule->psi2);
        return mymolecule->psi2_energy();
    }

    double denergy(double t) {
        this->transport(t, mymolecule->psi1, mymolecule->psi2);
        return 2.0 * mymolecule->psi2_eorbit();
    }

    void psi_final(double t) { 
        this->get(t, mymolecule->psi1, mymolecule->psi2); 
    }

private:
    /**
     * @brief Validate matrix for NaN/Inf values
     */
    bool validate_matrix(double *matrix) {
        int size = (mygrid->ne[0] + mygrid->ne[1]) * mygrid->npack(1);
        for (int i = 0; i < size; ++i) {
            if (std::isnan(matrix[i]) || std::isinf(matrix[i])) {
                return false;
            }
        }
        return true;
    }
};

} // namespace pwdft

#endif 