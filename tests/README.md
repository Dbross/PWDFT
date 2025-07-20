# PWDFT Tests

This directory contains all test files for the PWDFT project, organized by tier and functionality.

## Directory Structure

### `root_tests/`
Contains test files that were previously scattered in the root directory:
- **Input files**: `.nw` files for NWChem input
- **Scripts**: `.py`, `.sh` files for test automation
- **Output files**: `.out`, `.log` files for validation
- **Pseudopotentials**: `.psp` files needed for specific tests
- **Test data**: Various test artifacts and movecs files

### `tier1/`
Basic unit tests and simple functionality tests.

### `tier2/`
Integration tests and more complex scenarios.

### `tier3/`
Advanced tests including:
- Enhanced local TF integration tests
- Robust optimization tests
- System classification tests
- Performance benchmarks

## Test Organization

Tests are organized by functionality and complexity:
- **Unit tests**: Test individual components
- **Integration tests**: Test component interactions
- **System tests**: Test complete workflows
- **Performance tests**: Benchmark performance characteristics

## Running Tests

Most tests can be run using the scripts in `root_tests/`:
- `run_tests.sh`: Run the main test suite
- `run_aurora_tests.sh`: Run Aurora-specific tests
- `build_aurora_tests.sh`: Build Aurora test environment

## Validation

Test outputs are compared against reference results to ensure:
- Correct energy calculations
- Proper convergence behavior
- Expected system classifications
- Performance within acceptable bounds

## Adding New Tests

When adding new tests:
1. Place them in the appropriate tier directory
2. Include necessary pseudopotentials in the test directory
3. Document the test purpose and expected results
4. Update this README if adding new test categories 