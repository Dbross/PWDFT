#!/bin/bash

set -e

echo "=== Enhanced Local-TF Unit Test ==="

# Get the PWDFT source directory
PWDFT_SRC="$(cd ../../.. && pwd)"

# Compile the unit test
echo "Compiling enhanced Local-TF unit test..."
g++ -I"$PWDFT_SRC/Nwpw" -std=c++11 "$PWDFT_SRC/test_enhanced_local_tf_unit.cpp" -o test_enhanced_local_tf_unit

# Run the unit test
echo "Running enhanced Local-TF unit test..."
./test_enhanced_local_tf_unit > test_output.out 2>&1

echo "Unit test completed successfully!" 