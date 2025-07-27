#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <complex.h>

// FFTPACK function declarations
extern void dcffti_(const int *, double *);
extern void dcfftf_(const int *, double *, const double *);
extern void dcfftb_(const int *, double *, const double *);
extern void drffti_(const int *, double *);
extern void drfftf_(const int *, double *, const double *);
extern void drfftb_(const int *, double *, const double *);

// FFTW3 declarations (if available)
#ifdef USE_FFTW
#include <fftw3.h>
#endif

#define PI 3.14159265358979323846
#define TOLERANCE 1e-10

// Test real FFT with guard bands and correctness check
int test_real_fft() {
    printf("=== Testing Real FFT ===\n");
    
    int N = 32;
    int workspace_size = 2*N + 15;
    int guard = 4;
    
    // Allocate memory with guard bands
    double *buf = malloc((workspace_size + 2*guard) * sizeof(double));
    double *data = malloc(N * sizeof(double));
    double *data_backup = malloc(N * sizeof(double));
    
    if (!buf || !data || !data_backup) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    // Initialize guard bands
    for(int i = 0; i < guard; i++) {
        buf[i] = buf[workspace_size + guard + i] = 0xDEADBEEFul;
    }
    
    double *workspace = buf + guard;
    
    // Initialize test data (simple sine wave)
    for(int i = 0; i < N; i++) {
        data[i] = sin(2.0 * PI * i / N);
        data_backup[i] = data[i];
    }
    
    // Initialize FFT
    drffti_(&N, workspace);
    
    // Forward transform
    drfftf_(&N, data, workspace);
    
    // Check for buffer overruns
    for(int i = 0; i < guard; i++) {
        if(buf[i] != 0xDEADBEEFul || buf[workspace_size + guard + i] != 0xDEADBEEFul) {
            printf("drfftf_ overrun guard at %d\n", i);
            free(buf); free(data); free(data_backup);
            return 1;
        }
    }
    
    // Inverse transform
    drfftb_(&N, data, workspace);
    
    // Check for buffer overruns again
    for(int i = 0; i < guard; i++) {
        if(buf[i] != 0xDEADBEEFul || buf[workspace_size + guard + i] != 0xDEADBEEFul) {
            printf("drfftb_ overrun guard at %d\n", i);
            free(buf); free(data); free(data_backup);
            return 1;
        }
    }
    
    // Check correctness (data should be restored to original * N)
    double max_error = 0.0;
    for(int i = 0; i < N; i++) {
        double expected = data_backup[i] * N;
        double error = fabs(data[i] - expected);
        if(error > max_error) max_error = error;
    }
    
    printf("Real FFT: workspace=%d, max_error=%.2e\n", workspace_size, max_error);
    
    if(max_error > TOLERANCE) {
        printf("Real FFT accuracy test FAILED\n");
        free(buf); free(data); free(data_backup);
        return 1;
    }
    
    printf("Real FFT PASS\n");
    free(buf); free(data); free(data_backup);
    return 0;
}

// Test complex FFT with guard bands and correctness check
int test_complex_fft() {
    printf("=== Testing Complex FFT ===\n");
    
    int N = 32;
    int workspace_size = 4*N + 15;
    int guard = 4;
    
    // Allocate memory with guard bands
    double *buf = malloc((workspace_size + 2*guard) * sizeof(double));
    double *data = malloc(2*N * sizeof(double));  // Complex data
    double *data_backup = malloc(2*N * sizeof(double));
    
    if (!buf || !data || !data_backup) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    // Initialize guard bands
    for(int i = 0; i < guard; i++) {
        buf[i] = buf[workspace_size + guard + i] = 0xDEADBEEFul;
    }
    
    double *workspace = buf + guard;
    
    // Initialize test data (complex exponential)
    for(int i = 0; i < N; i++) {
        double phase = 2.0 * PI * i / N;
        data[2*i] = cos(phase);     // Real part
        data[2*i+1] = sin(phase);   // Imaginary part
        data_backup[2*i] = data[2*i];
        data_backup[2*i+1] = data[2*i+1];
    }
    
    // Initialize FFT
    dcffti_(&N, workspace);
    
    // Forward transform
    dcfftf_(&N, data, workspace);
    
    // Check for buffer overruns
    for(int i = 0; i < guard; i++) {
        if(buf[i] != 0xDEADBEEFul || buf[workspace_size + guard + i] != 0xDEADBEEFul) {
            printf("dcfftf_ overrun guard at %d\n", i);
            free(buf); free(data); free(data_backup);
            return 1;
        }
    }
    
    // Inverse transform
    dcfftb_(&N, data, workspace);
    
    // Check for buffer overruns again
    for(int i = 0; i < guard; i++) {
        if(buf[i] != 0xDEADBEEFul || buf[workspace_size + guard + i] != 0xDEADBEEFul) {
            printf("dcfftb_ overrun guard at %d\n", i);
            free(buf); free(data); free(data_backup);
            return 1;
        }
    }
    
    // Check correctness (data should be restored to original * N)
    double max_error = 0.0;
    for(int i = 0; i < 2*N; i++) {
        double expected = data_backup[i] * N;
        double error = fabs(data[i] - expected);
        if(error > max_error) max_error = error;
    }
    
    printf("Complex FFT: workspace=%d, max_error=%.2e\n", workspace_size, max_error);
    
    if(max_error > TOLERANCE) {
        printf("Complex FFT accuracy test FAILED\n");
        free(buf); free(data); free(data_backup);
        return 1;
    }
    
    printf("Complex FFT PASS\n");
    free(buf); free(data); free(data_backup);
    return 0;
}

// Test edge cases with different sizes
int test_edge_cases() {
    printf("=== Testing Edge Cases ===\n");
    
    int sizes[] = {1, 2, 4, 8, 16, 64, 128};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    int failures = 0;
    
    for(int s = 0; s < num_sizes; s++) {
        int N = sizes[s];
        int real_workspace = 2*N + 15;
        int complex_workspace = 4*N + 15;
        int guard = 4;
        
        // Test real FFT
        double *real_buf = malloc((real_workspace + 2*guard) * sizeof(double));
        if (real_buf) {
            for(int i = 0; i < guard; i++) {
                real_buf[i] = real_buf[real_workspace + guard + i] = 0xDEADBEEFul;
            }
            double *real_ws = real_buf + guard;
            
            drffti_(&N, real_ws);
            
            // Check for overruns
            int overrun = 0;
            for(int i = 0; i < guard; i++) {
                if(real_buf[i] != 0xDEADBEEFul || real_buf[real_workspace + guard + i] != 0xDEADBEEFul) {
                    overrun = 1;
                    break;
                }
            }
            
            if(overrun) {
                printf("Real FFT overrun at N=%d\n", N);
                failures++;
            } else {
                printf("Real FFT N=%d PASS\n", N);
            }
            free(real_buf);
        }
        
        // Test complex FFT
        double *complex_buf = malloc((complex_workspace + 2*guard) * sizeof(double));
        if (complex_buf) {
            for(int i = 0; i < guard; i++) {
                complex_buf[i] = complex_buf[complex_workspace + guard + i] = 0xDEADBEEFul;
            }
            double *complex_ws = complex_buf + guard;
            
            dcffti_(&N, complex_ws);
            
            // Check for overruns
            int overrun = 0;
            for(int i = 0; i < guard; i++) {
                if(complex_buf[i] != 0xDEADBEEFul || complex_buf[complex_workspace + guard + i] != 0xDEADBEEFul) {
                    overrun = 1;
                    break;
                }
            }
            
            if(overrun) {
                printf("Complex FFT overrun at N=%d\n", N);
                failures++;
            } else {
                printf("Complex FFT N=%d PASS\n", N);
            }
            free(complex_buf);
        }
    }
    
    return failures;
}

int main() {
    int failures = 0;
    
    printf("=== Comprehensive FFT Correctness Test ===\n");
    
    failures += test_real_fft();
    failures += test_complex_fft();
    failures += test_edge_cases();
    
    printf("=== Test Summary ===\n");
    printf("Total failures: %d\n", failures);
    
    if(failures == 0) {
        printf("ALL TESTS PASSED - FFT routines are working correctly!\n");
    } else {
        printf("SOME TESTS FAILED - FFT routines have issues!\n");
    }
    
    return failures;
} 