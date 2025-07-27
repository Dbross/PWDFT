#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// FFTPACK function declarations
extern void drffti_(const int *, double *);
extern void dcfftf_(const int *, double *, const double *);
extern void dcfftb_(const int *, double *, const double *);
extern void drfftf_(const int *, double *, const double *);
extern void drfftb_(const int *, double *, const double *);

int main() {
    int N = 28;    // adjust for vector sizes
    int req = 2*N+15;
    int guard = 4;
    double *buf = malloc((req + 2*guard) * sizeof(double));
    
    if (!buf) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    // Initialize guard bands
    for(int i = 0; i < guard; i++) {
        buf[i] = buf[req + guard + i] = 0xDEADBEEFul;
    }
    
    double *ws = buf + guard;
    
    // Initialize FFT
    drffti_(&N, ws);
    
    // Check for buffer overruns
    for(int i = 0; i < guard; i++) {
        if(buf[i] != 0xDEADBEEFul || buf[req + guard + i] != 0xDEADBEEFul) {
            printf("%s overrun guard at %d\n", "drffti_", i);
            free(buf);
            return 1;
        }
    }
    
    printf("%s PASS N=%d (workspace=%d)\n", "drffti_", N, req);
    free(buf);
    return 0;
}
