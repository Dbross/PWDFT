# Tier 3 Tests - Enhanced Features

This directory contains tests for enhanced features that have been added to PWDFT, particularly focusing on advanced SCF convergence improvements.

## Test Categories

### 1.1 Enhanced Local-TF Unit Test (`1.1_enhanced_local_tf_unit/`)
- **Purpose**: Verifies that the enhanced Local-TF preconditioning class compiles and functions correctly
- **What it tests**:
  - Enhanced Local-TF object creation with proper parameters
  - Application of enhanced Local-TF preconditioning algorithm
  - Correct handling of system size, FFT size, and spin channels
  - Successful completion of iterative refinement process
- **Expected outcome**: Unit test passes with all enhanced Local-TF functionality working

### 1.2 Enhanced Local-TF Integration Test (`1.2_enhanced_local_tf_integration/`)
- **Purpose**: Verifies that the enhanced Local-TF is properly integrated into the SCF mixing framework
- **What it tests**:
  - Enhanced Local-TF initialization when algorithm 4 is selected
  - Proper integration with existing SCF mixing infrastructure
  - Backward compatibility with existing Local-TF mixing
  - Debug output confirms enhanced Local-TF is available
- **Expected outcome**: Enhanced Local-TF is properly initialized and available for use

### 1.3 Enhanced Local-TF Compilation Test (`1.3_enhanced_local_tf_compilation/`)
- **Purpose**: Verifies that the enhanced Local-TF code compiles correctly with the main build system
- **What it tests**:
  - Enhanced Local-TF header file exists and is accessible
  - Header is properly included in SCF mixing framework
  - EnhancedLocalTF class is properly integrated
  - No compilation errors when including the enhanced Local-TF header
- **Expected outcome**: All compilation checks pass without errors

## Enhanced Local-TF Feature Overview

The enhanced Local-TF preconditioning is a sophisticated improvement to the standard Local-TF mixing algorithm that provides:

1. **Iterative Refinement**: Up to 12 iterations with adaptive convergence
2. **Matrix-Based Preconditioning**: Symmetric matrix construction and inversion
3. **Local Density Dependence**: Density-dependent screening parameters
4. **Quantum Espresso Compatibility**: Based on proven QE implementation
5. **Backward Compatibility**: Seamless integration with existing algorithm 4

## Running Tests Locally

To run these tests locally:

```bash
# Run all tier3 tests
./run_tests.sh --tier 3

# Run specific test
cd tests/tier3/1.1_enhanced_local_tf_unit
./run.sh
./validate.sh
```

## CI Integration

These tests are automatically run in the CI pipeline as part of the tier3 test suite, which executes on:
- Pull requests to main/master branches
- Direct pushes to main/master branches

The tests ensure that enhanced features are properly implemented and integrated before merging. 