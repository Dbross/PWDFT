#!/bin/bash

# Simple System Classification Test
# Tests with only available pseudopotentials

echo "=========================================="
echo "Simple System Classification Test"
echo "=========================================="

PWDFT_EXECUTABLE=${PWDFT_BIN:-../../../../build/pwdft}

# Function to run test and extract classification
run_classification_test() {
    local test_name="$1"
    local input_file="$2"
    local expected_type="$3"
    
    echo "Testing: $test_name"
    echo "Expected: $expected_type"
    
    # Run calculation with early termination
    $PWDFT_EXECUTABLE < "$input_file" > "${test_name}.out" 2>&1
    
    # Extract system classification
    local detected_type=$(grep "System Classification:" "${test_name}.out" | head -1 | awk '{print $3}')
    local cell_aspect_ratio=$(grep "Cell aspect ratio:" "${test_name}.out" | head -1 | awk '{print $4}')
    local dimensionality=$(grep "Dimensionality:" "${test_name}.out" | head -1 | awk '{print $3}')
    local n_atoms=$(grep "Number of atoms:" "${test_name}.out" | head -1 | awk '{print $4}')
    local is_crystal=$(grep "Is crystal (input):" "${test_name}.out" | head -1 | awk '{print $5}')
    local scf_algorithm=$(grep "Applied SCF Algorithm:" "${test_name}.out" | head -1 | awk '{print $4}')
    
    echo "  Detected: $detected_type"
    echo "  Atoms: $n_atoms, Crystal: $is_crystal, Aspect: $cell_aspect_ratio, Dim: $dimensionality"
    echo "  SCF Algorithm: $scf_algorithm"
    
    if [ "$detected_type" = "$expected_type" ]; then
        echo "  ✅ PASS"
        return 0
    else
        echo "  ❌ FAIL - Expected $expected_type, got $detected_type"
        return 1
    fi
}

# Create test directory
mkdir -p simple_test_results
cd simple_test_results

echo "Creating test systems..."

# Test 1: Molecule (H2O)
cat > test_molecule.nw << 'EOF'
print medium
title "Water Molecule Test"

geometry noautoz nocenter noautosym
O 0.0 0.0 0.0
H 0.957 0.0 0.0
H -0.24 0.927 0.0
end

nwpw
   pseudopotentials
   H library pspw_default
   O library pspw_default
   end
   simulation_cell
     SC 20.0
   end
   mapping 1
   cutoff 30.0 120.0
   xc pbe
   loop 1 1  # Early termination
end

task pspw energy
EOF

# Test 2: Surface/Slab (Cu surface)
cat > test_surface.nw << 'EOF'
print medium
title "Cu Surface Test"

geometry noautoz nocenter noautosym
# Simple Cu surface - 2x2 supercell with 2 layers (8 Cu atoms)
# Bottom layer
Cu 0.0 0.0 0.0
Cu 2.55 0.0 0.0
Cu 0.0 2.55 0.0
Cu 2.55 2.55 0.0
# Top layer
Cu 1.275 1.275 2.08
Cu 3.825 1.275 2.08
Cu 1.275 3.825 2.08
Cu 3.825 3.825 2.08
end

nwpw
   pseudopotentials
   Cu library pspw_default
   end
   simulation_cell
     FCC 30.0
   end
   mapping 2
   cutoff 40.0
   xc pbe
   loop 1 1  # Early termination
end

task pspw energy
EOF

# Test 3: Bulk Metal (Cu bulk - simplified)
cat > test_bulk_metal.nw << 'EOF'
print medium
title "Cu Bulk Metal Test"

geometry noautoz nocenter noautosym
# Cu bulk - 2x2x2 supercell (16 atoms)
Cu 0.0 0.0 0.0
Cu 1.8075 1.8075 0.0
Cu 0.0 3.615 0.0
Cu 1.8075 5.4225 0.0
Cu 3.615 0.0 0.0
Cu 5.4225 1.8075 0.0
Cu 3.615 3.615 0.0
Cu 5.4225 5.4225 0.0
Cu 0.0 0.0 1.8075
Cu 1.8075 1.8075 1.8075
Cu 0.0 3.615 1.8075
Cu 1.8075 5.4225 1.8075
Cu 3.615 0.0 1.8075
Cu 5.4225 1.8075 1.8075
Cu 3.615 3.615 1.8075
Cu 5.4225 5.4225 1.8075
end

nwpw
   pseudopotentials
   Cu library pspw_default
   end
   simulation_cell
     FCC 20.0
   end
   mapping 2
   cutoff 40.0
   xc pbe
   loop 1 1  # Early termination
end

task pspw energy
EOF

# Test 4: Bulk Insulator (Si bulk - simplified)
cat > test_bulk_insulator.nw << 'EOF'
print medium
title "Si Bulk Insulator Test"

geometry noautoz nocenter noautosym
# Si bulk - diamond structure (8 atoms)
Si 0.0 0.0 0.0
Si 2.715 2.715 0.0
Si 0.0 5.43 0.0
Si 2.715 8.145 0.0
Si 1.3575 1.3575 1.3575
Si 4.0725 4.0725 1.3575
Si 1.3575 6.7875 1.3575
Si 4.0725 9.5025 1.3575
end

nwpw
   pseudopotentials
   Si library pspw_default
   end
   simulation_cell
     SC 25.0
   end
   mapping 1
   cutoff 35.0
   xc pbe
   loop 1 1  # Early termination
end

task pspw energy
EOF

# Test 5: Interface (Si/Si interface - using only Si)
cat > test_interface.nw << 'EOF'
print medium
title "Si/Si Interface Test"

geometry noautoz nocenter noautosym
# Si/Si interface - layered structure (simplified)
# Bottom layer
Si 0.0 0.0 0.0
Si 2.715 2.715 0.0
Si 0.0 5.43 0.0
Si 2.715 8.145 0.0
# Top layer
Si 0.0 0.0 2.715
Si 2.715 2.715 2.715
Si 0.0 5.43 2.715
Si 2.715 8.145 2.715
end

nwpw
   pseudopotentials
   Si library pspw_default
   end
   simulation_cell
     SC 25.0
   end
   mapping 1
   cutoff 35.0
   xc pbe
   loop 1 1  # Early termination
end

task pspw energy
EOF

echo "Running simple classification tests..."

# Run all tests and track results
total_tests=0
passed_tests=0

run_classification_test "Molecule" "test_molecule.nw" "Molecule"
if [ $? -eq 0 ]; then passed_tests=$((passed_tests + 1)); fi
total_tests=$((total_tests + 1))

run_classification_test "Surface" "test_surface.nw" "Surface/Slab"
if [ $? -eq 0 ]; then passed_tests=$((passed_tests + 1)); fi
total_tests=$((total_tests + 1))

run_classification_test "Bulk_Metal" "test_bulk_metal.nw" "Bulk"
if [ $? -eq 0 ]; then passed_tests=$((passed_tests + 1)); fi
total_tests=$((total_tests + 1))

run_classification_test "Bulk_Insulator" "test_bulk_insulator.nw" "Bulk Insulator"
if [ $? -eq 0 ]; then passed_tests=$((passed_tests + 1)); fi
total_tests=$((total_tests + 1))

run_classification_test "Interface" "test_interface.nw" "Interface"
if [ $? -eq 0 ]; then passed_tests=$((passed_tests + 1)); fi
total_tests=$((total_tests + 1))

echo ""
echo "=========================================="
echo "Test Summary"
echo "=========================================="
echo "Tests passed: $passed_tests/$total_tests"

# Generate detailed summary
echo "System Classification Test Results:" > summary.txt
echo "=================================" >> summary.txt
echo "Tests passed: $passed_tests/$total_tests" >> summary.txt
echo "" >> summary.txt

for test_file in test_*.nw; do
    test_name=$(basename "$test_file" .nw)
    if [ -f "${test_name}.out" ]; then
        detected_type=$(grep "System Classification:" "${test_name}.out" | head -1 | awk '{print $3}')
        scf_algorithm=$(grep "Applied SCF Algorithm:" "${test_name}.out" | head -1 | awk '{print $4}')
        echo "$test_name: $detected_type (SCF: $scf_algorithm)" >> summary.txt
    fi
done

cat summary.txt

echo ""
echo "Detailed results saved in simple_test_results/ directory"
echo "Check individual .out files for debug information"

# Exit with appropriate code
if [ $passed_tests -eq $total_tests ]; then
    echo "🎉 All tests passed!"
    exit 0
else
    echo "❌ Some tests failed!"
    exit 1
fi 