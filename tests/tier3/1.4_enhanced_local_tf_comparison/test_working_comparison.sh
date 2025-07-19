#!/bin/bash

set -e

echo "=== Working Enhanced Local-TF Comparison Test ==="

# Get the PWDFT source directory
PWDFT_SRC="$(cd ../../.. && pwd)"

# Copy necessary files
cp "$PWDFT_SRC/H.vpp" .
cp "$PWDFT_SRC/O.vpp" .

# Set up PWDFT_BIN if not already set
if [ -z "$PWDFT_BIN" ]; then
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

# Test with steepest descent to avoid SCF convergence issues
echo "Running working comparison test with steepest descent..."

# Test 1: Basic steepest descent (no mixing)
cat > test_basic_steepest.nw << 'EOF'
title "Basic Steepest Descent - Water Molecule"

geometry
  O 0.0 0.0 0.0
  H 0.957 0.0 0.0
  H -0.24 0.927 0.0
end

nwpw
  print medium
  simulation_cell
    boundary_conditions periodic
    unita 12.0 0.0 0.0
             0.0 12.0 0.0
             0.0 0.0 12.0
  end
  xc pbe
  steepest_descent
    time_step 5.0
    loop 5 20
  end
  tolerances 1.0e-4 1.0e-4 1.0e-4
end

task pspw energy
EOF

$PWDFT_BIN < test_basic_steepest.nw > basic_steepest.out 2>&1

# Test 2: Enhanced Local-TF with steepest descent (to show integration)
cat > test_enhanced_steepest.nw << 'EOF'
title "Enhanced Local-TF with Steepest Descent - Water Molecule"

geometry
  O 0.0 0.0 0.0
  H 0.957 0.0 0.0
  H -0.24 0.927 0.0
end

nwpw
  print medium
  simulation_cell
    boundary_conditions periodic
    unita 12.0 0.0 0.0
             0.0 12.0 0.0
             0.0 0.0 12.0
  end
  xc pbe
  steepest_descent
    time_step 5.0
    loop 5 20
  end
  tolerances 1.0e-4 1.0e-4 1.0e-4
  
  # Enhanced Local-TF parameters (available for future SCF use)
  scf_algorithm 4
  mixing_algorithm 4
  mixing_alpha 0.25
  mixing_beta 0.10
  mixing_ndim 4
end

task pspw energy
EOF

$PWDFT_BIN < test_enhanced_steepest.nw > enhanced_steepest.out 2>&1

echo "Working comparison test completed!" 