#include <cmath>
#include <algorithm>
#include <limits>

namespace pwdft {

/**
 * @brief Initialize robust line search (no-op for now)
 */
void util_linesearch_robust_init() {
    // No initialization needed for robust line search
}

/**
 * @brief Robust line search with enhanced numerical stability
 * 
 * This enhanced version includes:
 * - NaN/Inf detection and handling
 * - Bounded step size reduction
 * - Fallback strategies for failed interpolation
 * - Improved convergence criteria
 */
double util_linesearch_robust(const double t0, const double f0, const double df0,
                             double deltat, double (*FUNC1)(double),
                             double (*DFUNC1)(double), const double tolerance,
                             double *tmin_ret, double *deltaE_ret,
                             const int stoptype) {
    
    const double MIN_STEP = 1.0e-12;
    const double MAX_STEP = 1.0e+6;
    const int MAX_ITER = 50;
    const double SAFETY_FACTOR = 0.1;
    
    bool secant = true;
    bool notfinished;
    int iteration = 0;
    
    double tmin;
    double t[3], f[3], df[3];
    double t_last, f_last;
    double t_first, f_first, df_first;
    double up, down, fmin, dfmin, deltaf;
    
    // Initialize with robust bounds checking
    t[0] = t0;
    f[0] = f0;
    df[0] = df0;
    
    // Validate initial conditions
    if (std::isnan(f[0]) || std::isinf(f[0]) || 
        std::isnan(df[0]) || std::isinf(df[0])) {
        *tmin_ret = t0;
        *deltaE_ret = 0.0;
        return f[0];
    }
    
    t_last = t[0];
    f_last = f[0];
    t_first = t[0];
    f_first = f[0];
    df_first = df[0];
    
    // Robust initial step evaluation
    double initial_step = std::max(MIN_STEP, std::min(deltat, MAX_STEP));
    f[1] = FUNC1(t[0] + initial_step);
    df[1] = DFUNC1(t[0] + initial_step);
    
    // Check for NaN/Inf in function evaluation
    if (std::isnan(f[1]) || std::isinf(f[1]) || 
        std::isnan(df[1]) || std::isinf(df[1])) {
        // Fallback: use very small step
        initial_step = MIN_STEP;
        f[1] = FUNC1(t[0] + initial_step);
        df[1] = DFUNC1(t[0] + initial_step);
        
        if (std::isnan(f[1]) || std::isinf(f[1])) {
            *tmin_ret = t0;
            *deltaE_ret = 0.0;
            return f[0];
        }
    }
    
    iteration = 1;
    t[1] = t[0] + initial_step;
    t_last = t[1];
    f_last = f[1];
    
    // Robust step reduction with bounds
    while ((f[1] > f[0]) && (iteration <= MAX_ITER)) {
        deltat *= SAFETY_FACTOR;
        deltat = std::max(MIN_STEP, std::min(deltat, MAX_STEP));
        
        f[1] = FUNC1(t[0] + deltat);
        df[1] = DFUNC1(t[0] + deltat);
        
        // Check for NaN/Inf
        if (std::isnan(f[1]) || std::isinf(f[1])) {
            deltat *= SAFETY_FACTOR;
            continue;
        }
        
        t[1] = t[0] + deltat;
        t_last = t[1];
        f_last = f[1];
        ++iteration;
        
        // Prevent infinite loop
        if (deltat <= MIN_STEP) break;
    }
    
    // Robust secant method with bounds checking
    if (std::fabs(df[1] - df[0]) > 1.0e-12) {
        deltat = -df[0] * (t[1] - t[0]) / (df[1] - df[0]);
        deltat = std::max(MIN_STEP, std::min(deltat, MAX_STEP));
    } else {
        deltat = MIN_STEP;
    }
    
    t[2] = t[0] + deltat;
    f[2] = FUNC1(t[2]);
    df[2] = DFUNC1(t[2]);
    
    // Validate secant result
    if (std::isnan(f[2]) || std::isinf(f[2])) {
        t[2] = t[1];
        f[2] = f[1];
        df[2] = df[1];
    }
    
    ++iteration;
    t_last = t[2];
    f_last = f[2];
    
    // Sort function values with robust comparison
    int indx[3] = {0, 1, 2};
    if (f[1] < f[0]) std::swap(indx[0], indx[1]);
    if (f[2] < f[indx[0]]) {
        std::swap(indx[0], indx[2]);
        std::swap(indx[1], indx[2]);
    } else if (f[2] < f[indx[1]]) {
        std::swap(indx[1], indx[2]);
    }
    
    deltaf = f[indx[1]] - f[indx[0]];
    
    // Robust convergence criteria
    if (stoptype == 1)
        notfinished = (std::fabs(deltaf) > tolerance) && (iteration <= MAX_ITER);
    else
        notfinished = (std::fabs(df[indx[0]] / df_first) > tolerance) && (iteration <= MAX_ITER);
    
    while (notfinished) {
        // Robust secant interpolation
        if (secant) {
            if (std::fabs(df[indx[1]] - df[indx[0]]) > 1.0e-12) {
                deltat = -df[indx[0]] * (t[indx[1]] - t[indx[0]]) / (df[indx[1]] - df[indx[0]]);
                deltat = std::max(MIN_STEP, std::min(deltat, MAX_STEP));
            } else {
                deltat = MIN_STEP;
            }
            
            tmin = t[indx[0]] + deltat;
            fmin = FUNC1(tmin);
            dfmin = DFUNC1(tmin);
            
            // Validate interpolation result
            if (std::isnan(fmin) || std::isinf(fmin)) {
                secant = false;
                continue;
            }
            
            ++iteration;
            t_last = tmin;
            f_last = fmin;
            
            if (fmin >= f[indx[0]]) {
                secant = false;
                if (fmin < f[indx[2]]) {
                    t[indx[2]] = tmin;
                    f[indx[2]] = fmin;
                    df[indx[2]] = dfmin;
                    // Re-sort
                    if (f[1] < f[0]) std::swap(indx[0], indx[1]);
                    if (f[2] < f[indx[0]]) {
                        std::swap(indx[0], indx[2]);
                        std::swap(indx[1], indx[2]);
                    } else if (f[2] < f[indx[1]]) {
                        std::swap(indx[1], indx[2]);
                    }
                }
            }
        }
        
        // Robust quadratic interpolation
        if (!secant) {
            up = (t[1] * t[1] - t[2] * t[2]) * f[0] +
                 (t[2] * t[2] - t[0] * t[0]) * f[1] +
                 (t[0] * t[0] - t[1] * t[1]) * f[2];
            down = (t[1] - t[2]) * f[0] + (t[2] - t[0]) * f[1] + (t[0] - t[1]) * f[2];
            
            if (std::fabs(down) > tolerance * tolerance) {
                tmin = 0.50 * up / down;
                tmin = std::max(MIN_STEP, std::min(tmin, MAX_STEP));
                fmin = FUNC1(tmin);
                dfmin = DFUNC1(tmin);
                
                if (!std::isnan(fmin) && !std::isinf(fmin)) {
                    ++iteration;
                    t_last = tmin;
                    f_last = fmin;
                } else {
                    tmin = t[indx[2]];
                    fmin = f[indx[2]] + tolerance;
                    iteration = MAX_ITER + 1;
                }
            } else {
                tmin = t[indx[2]];
                fmin = f[indx[2]] + tolerance;
                iteration = MAX_ITER + 1;
            }
        }
        
        // Update with bounds checking
        if (fmin < f[indx[2]] && !std::isnan(fmin) && !std::isinf(fmin)) {
            t[indx[2]] = tmin;
            f[indx[2]] = fmin;
            df[indx[2]] = dfmin;
            
            // Re-sort
            if (f[1] < f[0]) std::swap(indx[0], indx[1]);
            if (f[2] < f[indx[0]]) {
                std::swap(indx[0], indx[2]);
                std::swap(indx[1], indx[2]);
            } else if (f[2] < f[indx[1]]) {
                std::swap(indx[1], indx[2]);
            }
            
            deltaf = f[indx[1]] - f[indx[0]];
        } else {
            deltaf = 0.0;
        }
        
        // Robust convergence check
        if (stoptype == 1)
            notfinished = (std::fabs(deltaf) > tolerance) && (iteration <= MAX_ITER);
        else
            notfinished = (std::fabs(df[indx[0]] / df_first) > tolerance) && (iteration <= MAX_ITER);
    }
    
    // Final validation and return
    tmin = t[indx[0]];
    fmin = f[indx[0]];
    
    if (tmin != t_last) {
        fmin = FUNC1(tmin);
        if (std::isnan(fmin) || std::isinf(fmin)) {
            tmin = t0;
            fmin = f0;
        }
    }
    
    *tmin_ret = tmin;
    *deltaE_ret = fmin - f0;
    
    return fmin;
}

} // namespace pwdft 