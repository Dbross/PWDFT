#!/bin/bash

set -e

echo "=== Validating Enhanced Local-TF Unit Test ==="

# Check if the test output file exists
if [ ! -f "test_output.out" ]; then
    echo "ERROR: Test output file not found"
    exit 1
fi

# Check for successful execution indicators
if ! grep -q "Enhanced Local-TF object created successfully" test_output.out; then
    echo "ERROR: Enhanced Local-TF object creation failed"
    exit 1
fi

if ! grep -q "=== Enhanced Local-TF Preconditioning Applied ===" test_output.out; then
    echo "ERROR: Enhanced Local-TF preconditioning was not applied"
    exit 1
fi

if ! grep -q "=== Enhanced Local-TF Preconditioning Completed ===" test_output.out; then
    echo "ERROR: Enhanced Local-TF preconditioning did not complete"
    exit 1
fi

if ! grep -q "Test completed successfully" test_output.out; then
    echo "ERROR: Unit test did not complete successfully"
    exit 1
fi

# Check for expected system parameters
if ! grep -q "System size: 1000" test_output.out; then
    echo "ERROR: Incorrect system size"
    exit 1
fi

if ! grep -q "Spin channels: 1" test_output.out; then
    echo "ERROR: Incorrect spin channels"
    exit 1
fi

echo "✅ Enhanced Local-TF unit test validation passed!"
exit 0 