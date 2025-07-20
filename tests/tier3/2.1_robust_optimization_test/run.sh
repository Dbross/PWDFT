#!/bin/bash

# Test script for robust optimization implementations
# This test verifies that the robust line search and geodesic transport
# handle edge cases properly and provide better stability

set -e

echo "=========================================="
echo "Testing Robust Optimization Implementations"
echo "=========================================="

# Create test directory
mkdir -p test_outputs
cd test_outputs

# Test 1: Simple water molecule with robust line search
echo "Test 1: Water molecule with robust line search..."
cat > test_water_robust.nw << 'EOF'
title "Water molecule - Robust optimization test"

start water_robust

geometry
  O    0.00000000    0.00000000    0.00000000
  H    0.00000000    0.00000000    1.00000000
  H    0.00000000    1.00000000    0.00000000
end

basis
  * library 6-31G
end

dft
  xc pbe
  grid medium
  convergence energy 1e-6
  convergence density 1e-5
  convergence gradient 1e-4
end

driver
  clear
  maxiter 50
  gmax 0.0001
  grms 0.0001
  xrms 0.0001
  xmax 0.0001
end

task dft optimize
EOF

# Run the test
echo "Running water molecule optimization with robust line search..."
../../../../build/pwdft test_water_robust.nw > test_water_robust.out 2>&1

# Test 2: Silicon bulk with challenging convergence
echo "Test 2: Silicon bulk with challenging convergence..."
cat > test_si_bulk_robust.nw << 'EOF'
title "Silicon bulk - Robust optimization test"

start si_bulk_robust

geometry
  Si    0.00000000    0.00000000    0.00000000
  Si    1.35700000    1.35700000    1.35700000
  Si    2.71400000    0.00000000    2.71400000
  Si    4.07100000    1.35700000    4.07100000
  Si    0.00000000    2.71400000    2.71400000
  Si    1.35700000    4.07100000    4.07100000
  Si    2.71400000    2.71400000    0.00000000
  Si    4.07100000    4.07100000    1.35700000
end

basis
  * library 6-31G
end

dft
  xc pbe
  grid medium
  convergence energy 1e-6
  convergence density 1e-5
  convergence gradient 1e-4
end

driver
  clear
  maxiter 100
  gmax 0.0001
  grms 0.0001
  xrms 0.0001
  xmax 0.0001
end

task dft optimize
EOF

# Run the test
echo "Running silicon bulk optimization with robust line search..."
../../../../build/pwdft test_si_bulk_robust.nw > test_si_bulk_robust.out 2>&1

# Test 3: CO molecule (simplified)
echo "Test 3: CO molecule optimization..."
cat > test_co_robust.nw << 'EOF'
title "CO molecule - Robust optimization test"

start co_robust

geometry
  C     0.00000000    0.00000000    0.00000000
  O     0.00000000    0.00000000    1.13000000
end

basis
  * library 6-31G
end

dft
  xc pbe
  grid medium
  convergence energy 1e-6
  convergence density 1e-5
  convergence gradient 1e-4
end

driver
  clear
  maxiter 100
  gmax 0.0001
  grms 0.0001
  xrms 0.0001
  xmax 0.0001
end

task dft optimize
EOF

# Run the test
echo "Running CO molecule optimization with robust line search..."
../../../../build/pwdft test_co_robust.nw > test_co_robust.out 2>&1

# Test 4: Edge case - very small step sizes
echo "Test 4: Edge case with very small step sizes..."
cat > test_edge_case_robust.nw << 'EOF'
title "Edge case - Very small step sizes"

start edge_case_robust

geometry
  H    0.00000000    0.00000000    0.00000000
  H    0.00000000    0.00000000    0.74000000
end

basis
  * library 6-31G
end

dft
  xc pbe
  grid medium
  convergence energy 1e-8
  convergence density 1e-7
  convergence gradient 1e-6
end

driver
  clear
  maxiter 200
  gmax 0.00001
  grms 0.00001
  xrms 0.00001
  xmax 0.00001
end

task dft optimize
EOF

# Run the test
echo "Running edge case test with very small step sizes..."
../../../../build/pwdft test_edge_case_robust.nw > test_edge_case_robust.out 2>&1

echo "=========================================="
echo "All robust optimization tests completed!"
echo "=========================================="

cd .. 