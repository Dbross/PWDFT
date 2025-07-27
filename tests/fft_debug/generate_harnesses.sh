#!/bin/bash

# Generate FFT test harnesses from grep results
# Usage: ./generate_harnesses.sh

set -e

# Function to generate a single harness
generate_harness() {
    local file="$1"
    local routine="$2"
    local dimvar="$3"
    local bufvar="$4"
    local type="$5"
    
    local output_dir="tests/fft_debug/$type"
    local filename="harness_${routine}_${dimvar}_${bufvar}.c"
    
    # Determine workspace size based on FFT type
    local workspace_size
    if [[ "$type" == "real" ]]; then
        workspace_size="2*N+15"
    else
        workspace_size="4*N+15"
    fi
    
    cat > "$output_dir/$filename" << EOF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// FFTPACK function declarations
extern void ${routine}(const int *, double *);
extern void dcfftf_(const int *, double *, const double *);
extern void dcfftb_(const int *, double *, const double *);
extern void drfftf_(const int *, double *, const double *);
extern void drfftb_(const int *, double *, const double *);

int main() {
    int N = 28;    // adjust for vector sizes
    int req = ${workspace_size};
    int guard = 4;
    double *buf = malloc((req + 2*guard) * sizeof(double));
    
    if (!buf) {
        printf("Memory allocation failed\\n");
        return 1;
    }
    
    // Initialize guard bands
    for(int i = 0; i < guard; i++) {
        buf[i] = buf[req + guard + i] = 0xDEADBEEFul;
    }
    
    double *ws = buf + guard;
    
    // Initialize FFT
    ${routine}(&N, ws);
    
    // Check for buffer overruns
    for(int i = 0; i < guard; i++) {
        if(buf[i] != 0xDEADBEEFul || buf[req + guard + i] != 0xDEADBEEFul) {
            printf("%s overrun guard at %d\\n", "${routine}", i);
            free(buf);
            return 1;
        }
    }
    
    printf("%s PASS N=%d (workspace=%d)\\n", "${routine}", N, req);
    free(buf);
    return 0;
}
EOF
}

# Process complex FFT calls
echo "Processing complex FFT calls..."
while IFS= read -r line; do
    if [[ -z "$line" ]]; then continue; fi
    
    # Extract routine name, dimension variable, and buffer variable
    if [[ "$line" =~ dcffti_ ]]; then
        routine="dcffti_"
        # Extract dimension variable (after &)
        dimvar=$(echo "$line" | grep -o '&[a-zA-Z_][a-zA-Z0-9_]*' | head -1 | tr -d '&')
        # Extract buffer variable (after comma)
        bufvar=$(echo "$line" | grep -o ',[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*' | head -1 | sed 's/, *//')
        
        if [[ -n "$dimvar" && -n "$bufvar" ]]; then
            echo "Generating complex harness: ${routine}_${dimvar}_${bufvar}"
            generate_harness "$line" "$routine" "$dimvar" "$bufvar" "complex"
        fi
    fi
done < fft_complex_calls.txt

# Process real FFT calls
echo "Processing real FFT calls..."
while IFS= read -r line; do
    if [[ -z "$line" ]]; then continue; fi
    
    # Extract routine name, dimension variable, and buffer variable
    if [[ "$line" =~ drffti_ ]]; then
        routine="drffti_"
        # Extract dimension variable (after &)
        dimvar=$(echo "$line" | grep -o '&[a-zA-Z_][a-zA-Z0-9_]*' | head -1 | tr -d '&')
        # Extract buffer variable (after comma)
        bufvar=$(echo "$line" | grep -o ',[[:space:]]*[a-zA-Z_][a-zA-Z0-9_]*' | head -1 | sed 's/, *//')
        
        if [[ -n "$dimvar" && -n "$bufvar" ]]; then
            echo "Generating real harness: ${routine}_${dimvar}_${bufvar}"
            generate_harness "$line" "$routine" "$dimvar" "$bufvar" "real"
        fi
    fi
done < fft_real_calls.txt

echo "Harness generation complete!" 