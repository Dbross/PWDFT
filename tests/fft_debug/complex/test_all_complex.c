#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// FFTPACK function declarations
extern void dcffti_(const int *, double *);
extern void dcfftf_(const int *, double *, const double *);
extern void dcfftb_(const int *, double *, const double *);

// Test function for a single FFT routine
int test_fft_routine(const char* routine_name, void (*fft_func)(const int*, double*), int workspace_size) {
    int N = 28;    // adjust for vector sizes
    int req = workspace_size;
    int guard = 4;
    double *buf = malloc((req + 2*guard) * sizeof(double));
    
    if (!buf) {
        printf("Memory allocation failed for %s\n", routine_name);
        return 1;
    }
    
    // Initialize guard bands
    for(int i = 0; i < guard; i++) {
        buf[i] = buf[req + guard + i] = 0xDEADBEEFul;
    }
    
    double *ws = buf + guard;
    
    // Initialize FFT
    fft_func(&N, ws);
    
    // Check for buffer overruns
    for(int i = 0; i < guard; i++) {
        if(buf[i] != 0xDEADBEEFul || buf[req + guard + i] != 0xDEADBEEFul) {
            printf("%s overrun guard at %d\n", routine_name, i);
            free(buf);
            return 1;
        }
    }
    
    printf("%s PASS N=%d (workspace=%d)\n", routine_name, N, req);
    free(buf);
    return 0;
}

int main() {
    int failures = 0;
    
    printf("=== Testing Complex FFT Routines ===\n");
    
    // Test dcffti_ with ny dimension (complex FFT)
    failures += test_fft_routine("dcffti_ny", dcffti_, 4*28+15);
    
    // Test dcffti_ with nz dimension (complex FFT)  
    failures += test_fft_routine("dcffti_nz", dcffti_, 4*28+15);
    
    printf("=== Complex FFT Test Summary ===\n");
    printf("Failures: %d\n", failures);
    
    return failures;
} 