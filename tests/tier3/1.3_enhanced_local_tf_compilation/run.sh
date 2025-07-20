#!/bin/bash

set -e

echo "=== Enhanced Local-TF Compilation Test ==="

# Get the PWDFT source directory
PWDFT_SRC="$(cd ../../.. && pwd)"

# Check if the enhanced Local-TF header file exists
if [ ! -f "$PWDFT_SRC/Nwpw/nwpwlib/utilities/nwpw_enhanced_local_tf.hpp" ]; then
    echo "ERROR: Enhanced Local-TF header file not found"
    exit 1
fi

# Check if the enhanced Local-TF is included in the SCF mixing header
if ! grep -q "nwpw_enhanced_local_tf.hpp" "$PWDFT_SRC/Nwpw/nwpwlib/utilities/nwpw_scf_mixing.hpp"; then
    echo "ERROR: Enhanced Local-TF header not included in SCF mixing"
    exit 1
fi

# Check if the enhanced Local-TF class is used in the SCF mixing
if ! grep -q "EnhancedLocalTF" "$PWDFT_SRC/Nwpw/nwpwlib/utilities/nwpw_scf_mixing.hpp"; then
    echo "ERROR: EnhancedLocalTF class not used in SCF mixing"
    exit 1
fi

# Try to compile a simple test that includes the enhanced Local-TF header
echo "Testing enhanced Local-TF header compilation..."
cat > test_compilation.cpp << 'EOF'
#include "Nwpw/nwpwlib/utilities/nwpw_enhanced_local_tf.hpp"
#include <iostream>

int main() {
    std::cout << "Enhanced Local-TF header compiles successfully!" << std::endl;
    return 0;
}
EOF

g++ -I"$PWDFT_SRC" -std=c++11 test_compilation.cpp -o test_compilation

# Run the compilation test
./test_compilation > test_output.out 2>&1

echo "Compilation test completed successfully!" 