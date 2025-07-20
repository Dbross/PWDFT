#ifndef UTIL_LINESEARCH_ROBUST_HPP
#define UTIL_LINESEARCH_ROBUST_HPP

namespace pwdft {

/**
 * @brief Initialize robust line search (no-op for now)
 */
void util_linesearch_robust_init();

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
                             const int stoptype);

} // namespace pwdft

#endif 