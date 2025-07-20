#!/bin/bash

set -e

echo "=== Enhanced Local-TF Integration Test ==="

# Get the PWDFT source directory
PWDFT_SRC="$(cd ../../.. && pwd)"

# Copy necessary files
cp "$PWDFT_SRC/H.vpp" .
cp "$PWDFT_SRC/O.vpp" .

# Create a simple test input that forces algorithm 4
cat > test_enhanced_local_tf_integration.nw << 'EOF'
title "Enhanced Local-TF Integration Test"

geometry
  H 0.0 0.0 0.0
end

nwpw
  print medium
  simulation_cell
    boundary_conditions periodic
    unita 10.0 0.0 0.0
             0.0 10.0 0.0
             0.0 0.0 10.0
  end
  xc pbe
  loop 2 5
  tolerances 1.0e-2 1.0e-2 1.0e-2
  
  # Force Local-TF mixing to test enhanced implementation
  scf_algorithm 4  # Force Local-TF mixing
  mixing_algorithm 4  # Local-TF mixing with enhanced preconditioning
  mixing_alpha 0.25
  mixing_beta 0.10
  mixing_ndim 4
end

task pspw energy
EOF

# Set up PWDFT_BIN if not already set
if [ -z "$PWDFT_BIN" ]; then
    # Look for pwdft in common locations
    if [ -f "../../../build/pwdft" ]; then
        export PWDFT_BIN="$(cd ../../../build && pwd)/pwdft"
    elif [ -f "../../../pwdft" ]; then
        export PWDFT_BIN="$(cd ../../.. && pwd)/pwdft"
    else
        echo "ERROR: PWDFT executable not found. Please set PWDFT_BIN environment variable."
        exit 1
    fi
fi

echo "Using PWDFT executable: $PWDFT_BIN"

# Run the test
echo "Running enhanced Local-TF integration test..."
$PWDFT_BIN < test_enhanced_local_tf_integration.nw > test_output.out 2>&1

echo "Integration test completed!" 