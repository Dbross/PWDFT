#!/bin/bash

# System Classification Unit Test
# Tests all system types with early termination after 1 iteration

echo "=========================================="
echo "System Classification Unit Test"
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
    
    echo "  Detected: $detected_type"
    echo "  Atoms: $n_atoms, Crystal: $is_crystal, Aspect: $cell_aspect_ratio, Dim: $dimensionality"
    
    if [ "$detected_type" = "$expected_type" ]; then
        echo "  ✅ PASS"
    else
        echo "  ❌ FAIL - Expected $expected_type, got $detected_type"
    fi
    
    echo ""
}

# Create test directory
mkdir -p test_results
cd test_results

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

# Test 2: Surface/Slab (Pt(111) with proper cell)
cat > test_surface.nw << 'EOF'
print medium
title "Pt(111) Surface Test"

geometry noautoz nocenter noautosym
# Pt(111) surface - 3x3 supercell with 3 layers (27 Pt atoms)
# Bottom layer
Pt    0.00000    0.00000    0.00000
Pt    2.77500    0.00000    0.00000
Pt    5.55000    0.00000    0.00000
Pt    1.38750    2.40250    0.00000
Pt    4.16250    2.40250    0.00000
Pt    6.93750    2.40250    0.00000
Pt    2.77500    4.80500    0.00000
Pt    5.55000    4.80500    0.00000
Pt    8.32500    4.80500    0.00000
# Middle layer
Pt    1.38750    0.80167    2.31500
Pt    4.16250    0.80167    2.31500
Pt    6.93750    0.80167    2.31500
Pt    2.77500    3.20417    2.31500
Pt    5.55000    3.20417    2.31500
Pt    8.32500    3.20417    2.31500
Pt    0.00000    5.60667    2.31500
Pt    2.77500    5.60667    2.31500
Pt    5.55000    5.60667    2.31500
# Top layer
Pt    0.00000    1.60333    4.63000
Pt    2.77500    1.60333    4.63000
Pt    5.55000    1.60333    4.63000
Pt    1.38750    4.00583    4.63000
Pt    4.16250    4.00583    4.63000
Pt    6.93750    4.00583    4.63000
Pt    2.77500    6.40833    4.63000
Pt    5.55000    6.40833    4.63000
Pt    8.32500    6.40833    4.63000
end

nwpw
   pseudopotentials
   Pt library pspw_default
   end
   simulation_cell
     FCC 40.0
   end
   mapping 2
   cutoff 50.0
   xc pbe
   loop 1 1  # Early termination
end

task pspw energy
EOF

# Test 3: Bulk Metal (Cu bulk)
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
   cutoff 30.0
   xc pbe
   loop 1 1  # Early termination
end

task pspw energy
EOF

# Test 4: Bulk Insulator (Si bulk)
cat > test_bulk_insulator.nw << 'EOF'
print medium
title "Si Bulk Insulator Test"

geometry noautoz nocenter noautosym
# Si bulk - diamond structure
Si 0.0 0.0 0.0
Si 2.715 2.715 0.0
Si 0.0 5.43 0.0
Si 2.715 8.145 0.0
Si 5.43 0.0 0.0
Si 8.145 2.715 0.0
Si 5.43 5.43 0.0
Si 8.145 8.145 0.0
Si 1.3575 1.3575 1.3575
Si 4.0725 4.0725 1.3575
Si 1.3575 6.7875 1.3575
Si 4.0725 9.5025 1.3575
Si 6.7875 1.3575 1.3575
Si 9.5025 4.0725 1.3575
Si 6.7875 6.7875 1.3575
Si 9.5025 9.5025 1.3575
# ... (simplified for brevity)
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

# Test 5: Interface (Cu/O interface)
cat > test_interface.nw << 'EOF'
print medium
title "Cu/O Interface Test"

geometry noautoz nocenter noautosym
# Cu/O interface - layered structure
# Cu layer
Cu 0.0 0.0 0.0
Cu 2.55 0.0 0.0
Cu 0.0 2.55 0.0
Cu 2.55 2.55 0.0
Cu 5.1 0.0 0.0
Cu 7.65 2.55 0.0
Cu 5.1 5.1 0.0
Cu 7.65 7.65 0.0
# O layer
O 0.0 0.0 2.55
O 2.55 2.55 2.55
O 0.0 5.1 2.55
O 2.55 7.65 2.55
O 5.1 0.0 2.55
O 7.65 2.55 2.55
O 5.1 5.1 2.55
O 7.65 7.65 2.55
end

nwpw
   pseudopotentials
   Cu library pspw_default
   O library pspw_default
   end
   simulation_cell
     SC 20.0
   end
   mapping 1
   cutoff 30.0
   xc pbe
   loop 1 1  # Early termination
end

task pspw energy
EOF

echo "Running classification tests..."

# Run all tests
run_classification_test "Molecule" "test_molecule.nw" "Molecule"
run_classification_test "Surface" "test_surface.nw" "Surface/Slab"
run_classification_test "Bulk_Metal" "test_bulk_metal.nw" "Bulk Metal"
run_classification_test "Bulk_Insulator" "test_bulk_insulator.nw" "Bulk Insulator"
run_classification_test "Interface" "test_interface.nw" "Interface"

echo "=========================================="
echo "Test Summary"
echo "=========================================="

# Generate summary
echo "System Classification Test Results:" > summary.txt
echo "=================================" >> summary.txt

for test_file in test_*.nw; do
    test_name=$(basename "$test_file" .nw)
    if [ -f "${test_name}.out" ]; then
        detected_type=$(grep "System Classification:" "${test_name}.out" | head -1 | awk '{print $3}')
        echo "$test_name: $detected_type" >> summary.txt
    fi
done

cat summary.txt

echo ""
echo "Detailed results saved in test_results/ directory"
echo "Check individual .out files for debug information" 