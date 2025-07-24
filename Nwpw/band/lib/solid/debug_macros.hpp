#pragma once
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

// Helper: format first N elements of an array for debug output
inline std::string array_to_string(const char* name, const double* arr, size_t n, size_t max_elems=10) {
    std::ostringstream oss;
    oss << name << ": ";
    size_t limit = (n < max_elems) ? n : max_elems;
    for (size_t i = 0; i < limit; ++i) {
        oss << std::setprecision(6) << arr[i] << " ";
    }
    if (n > max_elems) oss << "... (" << n << ")";
    return oss.str();
}
inline std::string array_to_string(const char* name, const int* arr, size_t n, size_t max_elems=10) {
    std::ostringstream oss;
    oss << name << ": ";
    size_t limit = (n < max_elems) ? n : max_elems;
    for (size_t i = 0; i < limit; ++i) {
        oss << arr[i] << " ";
    }
    if (n > max_elems) oss << "... (" << n << ")";
    return oss.str();
}
inline std::string array_to_string(const char* name, const float* arr, size_t n, size_t max_elems=10) {
    std::ostringstream oss;
    oss << name << ": ";
    size_t limit = (n < max_elems) ? n : max_elems;
    for (size_t i = 0; i < limit; ++i) {
        oss << std::setprecision(6) << arr[i] << " ";
    }
    if (n > max_elems) oss << "... (" << n << ")";
    return oss.str();
}

// NaN/Inf check utility for debug builds
inline void check_nan_inf(const char* name, const double* arr, size_t n, const char* context = "") {
#if defined(ENABLE_NAN_INF_CHECKS)
    for (size_t i = 0; i < n; ++i) {
        if (std::isnan(arr[i]) || std::isinf(arr[i])) {
            std::cerr << "[NAN/INF] " << name << "[" << i << "] = " << arr[i]
                      << " " << context << std::endl;
            break;
        }
    }
#endif
}

// All macros below take a single argument (string/stream expression)
// TRACE LOGGING
#if defined(ENABLE_TRACE_LOGGING)
  #define TRACE_LOG(msg) std::cerr << "[TRACE] " << msg << std::endl
#else
  #define TRACE_LOG(msg) do {} while(0)
#endif

// SCF DEBUG
#if defined(ENABLE_SCF_DEBUG)
  #define SCF_LOG(msg) std::cerr << "[SCF] " << msg << std::endl
#else
  #define SCF_LOG(msg) do {} while(0)
#endif

// HAMILTONIAN DEBUG
#if defined(ENABLE_HAMILTONIAN_DEBUG)
  #define HML_LOG(msg) std::cerr << "[HML] " << msg << std::endl
#else
  #define HML_LOG(msg) do {} while(0)
#endif

// WAVEFUNCTION DEBUG
#if defined(ENABLE_WAVEFUNC_DEBUG)
  #define WF_LOG(msg) std::cerr << "[WF] " << msg << std::endl
#else
  #define WF_LOG(msg) do {} while(0)
#endif

// NAN/INF CHECKS
#if defined(ENABLE_NAN_INF_CHECKS)
  #define NAN_INF_LOG(msg) std::cerr << "[NAN/INF] " << msg << std::endl
#else
  #define NAN_INF_LOG(msg) do {} while(0)
#endif

// PARALLEL DEBUG
#if defined(ENABLE_PARALLEL_DEBUG)
  #define PAR_LOG(msg) std::cerr << "[PAR] " << msg << std::endl
#else
  #define PAR_LOG(msg) do {} while(0)
#endif

// IO DEBUG
#if defined(ENABLE_IO_DEBUG)
  #define IO_LOG(msg) std::cerr << "[IO] " << msg << std::endl
#else
  #define IO_LOG(msg) do {} while(0)
#endif

// PERFORMANCE/TIMING
#if defined(ENABLE_PERF_TIMING)
  #define PERF_LOG(msg) std::cerr << "[PERF] " << msg << std::endl
#else
  #define PERF_LOG(msg) do {} while(0)
#endif

// STATE DUMP
#if defined(ENABLE_STATE_DUMP)
  #define STATE_DUMP(msg) std::cerr << "[STATE] " << msg << std::endl
#else
  #define STATE_DUMP(msg) do {} while(0)
#endif

// TEST HOOKS
#if defined(ENABLE_TEST_HOOKS)
  #define TEST_LOG(msg) std::cerr << "[TEST] " << msg << std::endl
#else
  #define TEST_LOG(msg) do {} while(0)
#endif

// MEMORY CHECKS
#if defined(ENABLE_MEMORY_CHECKS)
  #define MEM_LOG(msg) std::cerr << "[MEM] " << msg << std::endl
#else
  #define MEM_LOG(msg) do {} while(0)
#endif 