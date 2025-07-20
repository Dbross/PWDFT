#!/bin/bash

set -e

echo "=== Enhanced Local-TF Comparison Test ==="

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

# Create test system: Water molecule (inhomogeneous system that benefits from enhanced Local-TF)
cat > water_test.nw << 'EOF'
title "Enhanced Local-TF Comparison - Water Molecule"

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
  loop 5 20
  tolerances 1.0e-4 1.0e-4 1.0e-4
  
  # Conservative parameters for stability
  mixing_alpha 0.25
  mixing_beta 0.10
  mixing_ndim 4
end

task pspw energy
EOF

# Test 1: Original Local-TF (algorithm 4 without enhanced features)
echo "Running test with original Local-TF..."
cat > test_original_local_tf.nw << 'EOF'
title "Original Local-TF Test - Water Molecule"

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
  loop 5 20
  tolerances 1.0e-4 1.0e-4 1.0e-4
  
  # Original Local-TF mixing
  mixing_algorithm 4
  mixing_alpha 0.25
  mixing_beta 0.10
  mixing_ndim 4
end

task pspw energy
EOF

$PWDFT_BIN < test_original_local_tf.nw > original_local_tf.out 2>&1

# Test 2: Enhanced Local-TF (algorithm 4 with enhanced features)
echo "Running test with enhanced Local-TF..."
cat > test_enhanced_local_tf.nw << 'EOF'
title "Enhanced Local-TF Test - Water Molecule"

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
  loop 5 20
  tolerances 1.0e-4 1.0e-4 1.0e-4
  
  # Enhanced Local-TF mixing (same algorithm 4, but with enhanced features)
  scf_algorithm 4  # Force Local-TF
  mixing_algorithm 4
  mixing_alpha 0.25
  mixing_beta 0.10
  mixing_ndim 4
end

task pspw energy
EOF

$PWDFT_BIN < test_enhanced_local_tf.nw > enhanced_local_tf.out 2>&1

echo "Comparison test completed!" 