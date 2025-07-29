#ifndef VERIFICATION_UTILS_HPP
#define VERIFICATION_UTILS_HPP

#include <iostream>
#include <cmath>
#include <limits>
#include <cstring>
#include <sys/resource.h>
#include <unistd.h>

/**
 * Get current memory usage in bytes
 * @return Memory usage in bytes
 */
inline size_t get_memory_usage() {
    struct rusage r_usage;
    if (getrusage(RUSAGE_SELF, &r_usage) == 0) {
        return static_cast<size_t>(r_usage.ru_maxrss) * 1024;  // Convert KB to bytes
    }
    return 0;
}

/**
 * Check for NaN or Inf values in an array
 * @param name Array name for error reporting
 * @param data Pointer to data array
 * @param size Size of array
 * @return true if NaN or Inf found, false otherwise
 */
inline bool check_nan_inf(const char* name, const double* data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        if (std::isnan(data[i]) || std::isinf(data[i])) {
            std::cerr << "ERROR: NaN/Inf detected in " << name 
                      << " at index " << i << " value: " << data[i] << std::endl;
            return true;
        }
    }
    return false;
}

/**
 * Check for NaN or Inf values in a complex array (interleaved real/imaginary)
 * @param name Array name for error reporting
 * @param data Pointer to complex data array (real, imag, real, imag, ...)
 * @param size Size of complex array (number of complex numbers)
 * @return true if NaN or Inf found, false otherwise
 */
inline bool check_nan_inf_complex(const char* name, const double* data, size_t size) {
    for (size_t i = 0; i < 2*size; ++i) {
        if (std::isnan(data[i]) || std::isinf(data[i])) {
            std::cerr << "ERROR: NaN/Inf detected in " << name 
                      << " at index " << i << " value: " << data[i] << std::endl;
            return true;
        }
    }
    return false;
}

/**
 * Calculate RMS error between two arrays
 * @param a First array
 * @param b Second array
 * @param size Size of arrays
 * @return RMS error
 */
inline double calculate_rms_error(const double* a, const double* b, size_t size) {
    double sum = 0.0;
    for (size_t i = 0; i < size; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum / size);
}

/**
 * Calculate RMS error between two complex arrays
 * @param a First complex array (interleaved real/imaginary)
 * @param b Second complex array (interleaved real/imaginary)
 * @param size Size of complex arrays (number of complex numbers)
 * @return RMS error
 */
inline double calculate_rms_error_complex(const double* a, const double* b, size_t size) {
    double sum = 0.0;
    for (size_t i = 0; i < 2*size; ++i) {
        double diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum / (2*size));
}

/**
 * Calculate maximum absolute error between two arrays
 * @param a First array
 * @param b Second array
 * @param size Size of arrays
 * @return Maximum absolute error
 */
inline double calculate_max_error(const double* a, const double* b, size_t size) {
    double max_error = 0.0;
    for (size_t i = 0; i < size; ++i) {
        double error = std::abs(a[i] - b[i]);
        max_error = std::max(max_error, error);
    }
    return max_error;
}

/**
 * Calculate maximum absolute error between two complex arrays
 * @param a First complex array (interleaved real/imaginary)
 * @param b Second complex array (interleaved real/imaginary)
 * @param size Size of complex arrays (number of complex numbers)
 * @return Maximum absolute error
 */
inline double calculate_max_error_complex(const double* a, const double* b, size_t size) {
    double max_error = 0.0;
    for (size_t i = 0; i < 2*size; ++i) {
        double error = std::abs(a[i] - b[i]);
        max_error = std::max(max_error, error);
    }
    return max_error;
}

/**
 * Print array statistics (min, max, mean, std dev)
 * @param name Array name
 * @param data Pointer to data array
 * @param size Size of array
 */
inline void print_array_stats(const char* name, const double* data, size_t size) {
    if (size == 0) return;
    
    double min_val = data[0];
    double max_val = data[0];
    double sum = 0.0;
    double sum_sq = 0.0;
    
    for (size_t i = 0; i < size; ++i) {
        min_val = std::min(min_val, data[i]);
        max_val = std::max(max_val, data[i]);
        sum += data[i];
        sum_sq += data[i] * data[i];
    }
    
    double mean = sum / size;
    double variance = (sum_sq / size) - (mean * mean);
    double std_dev = std::sqrt(std::max(0.0, variance));
    
    std::cout << "  " << name << " stats:" << std::endl;
    std::cout << "    Min: " << min_val << std::endl;
    std::cout << "    Max: " << max_val << std::endl;
    std::cout << "    Mean: " << mean << std::endl;
    std::cout << "    Std Dev: " << std_dev << std::endl;
}

/**
 * Print complex array statistics
 * @param name Array name
 * @param data Pointer to complex data array (interleaved real/imaginary)
 * @param size Size of complex array (number of complex numbers)
 */
inline void print_complex_array_stats(const char* name, const double* data, size_t size) {
    if (size == 0) return;
    
    double min_real = data[0];
    double max_real = data[0];
    double min_imag = data[1];
    double max_imag = data[1];
    double sum_real = 0.0;
    double sum_imag = 0.0;
    double sum_mag = 0.0;
    
    for (size_t i = 0; i < size; ++i) {
        double real_part = data[2*i];
        double imag_part = data[2*i+1];
        double magnitude = std::sqrt(real_part*real_part + imag_part*imag_part);
        
        min_real = std::min(min_real, real_part);
        max_real = std::max(max_real, real_part);
        min_imag = std::min(min_imag, imag_part);
        max_imag = std::max(max_imag, imag_part);
        
        sum_real += real_part;
        sum_imag += imag_part;
        sum_mag += magnitude;
    }
    
    double mean_real = sum_real / size;
    double mean_imag = sum_imag / size;
    double mean_mag = sum_mag / size;
    
    std::cout << "  " << name << " complex stats:" << std::endl;
    std::cout << "    Real: [" << min_real << ", " << max_real << "] mean=" << mean_real << std::endl;
    std::cout << "    Imag: [" << min_imag << ", " << max_imag << "] mean=" << mean_imag << std::endl;
    std::cout << "    Magnitude mean: " << mean_mag << std::endl;
}

/**
 * Verify array bounds
 * @param name Array name
 * @param data Pointer to data array
 * @param size Size of array
 * @param min_bound Minimum allowed value
 * @param max_bound Maximum allowed value
 * @return true if all values are within bounds, false otherwise
 */
inline bool verify_array_bounds(const char* name, const double* data, size_t size, 
                               double min_bound, double max_bound) {
    for (size_t i = 0; i < size; ++i) {
        if (data[i] < min_bound || data[i] > max_bound) {
            std::cerr << "ERROR: Value out of bounds in " << name 
                      << " at index " << i << " value: " << data[i] 
                      << " bounds: [" << min_bound << ", " << max_bound << "]" << std::endl;
            return false;
        }
    }
    return true;
}

/**
 * Calculate parallel efficiency
 * @param serial_time Time for serial execution
 * @param parallel_time Time for parallel execution
 * @param nranks Number of MPI ranks
 * @return Parallel efficiency (0.0 to 1.0)
 */
inline double calculate_parallel_efficiency(double serial_time, double parallel_time, int nranks) {
    if (parallel_time <= 0.0 || nranks <= 0) return 0.0;
    double speedup = serial_time / parallel_time;
    return speedup / nranks;
}

/**
 * Print timing information
 * @param operation Operation name
 * @param time Time in seconds
 * @param nranks Number of MPI ranks (for parallel operations)
 */
inline void print_timing(const char* operation, double time, int nranks = 1) {
    std::cout << "  " << operation << " time: " << time << " seconds";
    if (nranks > 1) {
        std::cout << " (" << nranks << " ranks)";
    }
    std::cout << std::endl;
}

/**
 * Assert with custom message
 * @param condition Condition to check
 * @param message Error message if condition is false
 */
inline void assert_with_message(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "ASSERTION FAILED: " << message << std::endl;
        std::abort();
    }
}

/**
 * Check if two floating point numbers are approximately equal
 * @param a First number
 * @param b Second number
 * @param tolerance Relative tolerance
 * @return true if numbers are approximately equal
 */
inline bool approximately_equal(double a, double b, double tolerance = 1e-12) {
    if (std::abs(a - b) <= tolerance) return true;
    if (std::abs(a) <= tolerance && std::abs(b) <= tolerance) return true;
    return std::abs(a - b) <= tolerance * std::max(std::abs(a), std::abs(b));
}

#endif // VERIFICATION_UTILS_HPP 