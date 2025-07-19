#!/bin/bash

set -e

echo "=== Validating Enhanced Local-TF Compilation Test ==="

# Check if the test output file exists
if [ ! -f "test_output.out" ]; then
    echo "ERROR: Test output file not found"
    exit 1
fi

# Check for successful compilation
if ! grep -q "Enhanced Local-TF header compiles successfully" test_output.out; then
    echo "ERROR: Enhanced Local-TF header compilation failed"
    exit 1
fi

# Check for any compilation errors
if grep -q "error:" test_output.out || grep -q "Error:" test_output.out; then
    echo "ERROR: Compilation errors detected"
    exit 1
fi

echo "✅ Enhanced Local-TF compilation test validation passed!"
exit 0 