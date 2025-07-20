#!/bin/bash

# Check Aurora Files - Run this to see what files are ready for Aurora

echo "=== Aurora File Check ==="
echo "Date: $(date)"
echo ""

# Core test files
echo "Core Test Files:"
files=(
    "aurora_test_runner.sh"
    "build_aurora_tests.sh"
    "gpu_tile_compact.sh"
    "test_sycl_device.cpp"
    "aurora_config.sh"
    "test_simple.nw"
    "AURORA_TEST_INSTRUCTIONS.md"
)

for file in "${files[@]}"; do
    if [ -f "$file" ]; then
        echo "✓ $file"
    else
        echo "✗ $file (MISSING)"
    fi
done

echo ""
echo "Build Files:"
if [ -f "build/pwdft" ]; then
    echo "✓ build/pwdft"
else
    echo "✗ build/pwdft (MISSING - will need to build on Aurora)"
fi

echo ""
echo "Pseudopotential Files:"
pseudos=("H.psp" "O.psp" "C.psp")
for psp in "${pseudos[@]}"; do
    if [ -f "$psp" ]; then
        echo "✓ $psp"
    else
        echo "✗ $psp (MISSING)"
    fi
done

echo ""
echo "=== Summary ==="
echo "Files ready for Aurora:"
ls -la aurora_test_runner.sh build_aurora_tests.sh gpu_tile_compact.sh test_sycl_device.cpp aurora_config.sh test_simple.nw AURORA_TEST_INSTRUCTIONS.md 2>/dev/null | wc -l | tr -d ' '
echo "out of 7 core files"

echo ""
echo "=== Next Steps ==="
echo "1. Push the files marked with ✓ to Aurora"
echo "2. Follow instructions in AURORA_TEST_INSTRUCTIONS.md"
echo "3. Run: ./build_aurora_tests.sh"
echo "4. Run: ./aurora_test_runner.sh"
echo "5. Report results back" 