# PWDFT Test Suite

This directory contains a comprehensive, tiered test suite for the PWDFT computational chemistry research code.

## Test Philosophy

For HPC applications like PWDFT, traditional unit testing of individual functions is often impractical. Therefore, this CI suite redefines a "unit test" as a fast-running, feature-oriented system test that validates a specific input keyword or a minimal, well-understood physical behavior.

## Test Structure

The test suite is organized into two tiers:

### Tier 1: "Unit-like" Tests
- **Purpose**: Run on every commit for rapid feedback
- **Runtime**: Must be extremely fast (<60 seconds)
- **Scope**: Check for crashes, correct input parsing, and basic physical soundness
- **Location**: `tests/tier1/`

### Tier 2: Integration/Regression Tests
- **Purpose**: Run on pull requests or nightly builds
- **Runtime**: Slower (1-15 minutes)
- **Scope**: Compute physical observables and compare to benchmark values to detect scientific regressions
- **Location**: `tests/tier2/`

## Test Organization

Each test is contained in its own directory with the following structure:

```
test_name/
├── input.nw          # PWDFT input file
├── run.sh           # Script to execute the test
└── validate.sh      # Script to validate results
```

## Running Tests

### Using the Master Test Runner

```bash
# Run all Tier 1 tests
./run_tests.sh --tier 1

# Run all Tier 2 tests
./run_tests.sh --tier 2

# Run Tier 1 tests (default)
./run_tests.sh
```

### Running Individual Tests

```bash
cd tests/tier1/2.1.1_H2_energy
./run.sh
./validate.sh
```

## Test Descriptions

### Tier 1 Tests

#### 2.1.1_H2_energy
- **Purpose**: Basic "hello, world" test
- **System**: H2 molecule in a large box
- **Validation**: Total energy convergence and final energy value
- **Expected Runtime**: <30 seconds

#### 2.3.1_Si_energy
- **Purpose**: Periodic semiconductor calculation
- **System**: Bulk Si with FCC structure
- **Validation**: Energy per atom and SCF convergence
- **Expected Runtime**: <45 seconds

#### 2.3.2_Al_smearing
- **Purpose**: Metallic system with electronic smearing
- **System**: Bulk Al with Fermi-Dirac smearing
- **Validation**: Metallic behavior (zero band gap)
- **Expected Runtime**: <60 seconds

### Tier 2 Tests

#### 3.1.1_Si_vcrelax_bandgap
- **Purpose**: Variable-cell relaxation and band structure
- **System**: Bulk Si with lattice optimization
- **Validation**: Final lattice constant and band gap
- **Expected Runtime**: 5-10 minutes

#### 3.2.3_CO_on_Pt111
- **Purpose**: Surface adsorption calculation
- **System**: CO molecule on Pt(111) surface
- **Validation**: Adsorption energy calculation
- **Expected Runtime**: 10-15 minutes

## Prerequisites

### Pseudopotentials

The tests require pseudopotential files to be available. In the CI environment, these are set up automatically. For local testing, ensure the following files are available:

- `H.pbe-gth.upf`
- `Si.lda.upf`
- `Al.lda.upf`
- `Pt.pbe-rrkjus.UPF`
- `C.pbe-rrkjus.UPF`
- `O.pbe-rrkjus.UPF`

### Environment Variables

- `PWDFT_BIN`: Path to the pwdft executable (auto-detected if not set)

## Adding New Tests

To add a new test:

1. Create a new directory in the appropriate tier: `tests/tier1/` or `tests/tier2/`
2. Create the PWDFT input file (`.nw`)
3. Create `run.sh` to execute the calculation
4. Create `validate.sh` to check results
5. Make all scripts executable: `chmod +x run.sh validate.sh`

### Validation Guidelines

- Always check for "JOB DONE" in the output
- Extract and validate physical observables (energies, forces, etc.)
- Use appropriate tolerances for numerical comparisons
- Provide clear error messages for debugging

## CI Integration

The test suite is integrated with GitHub Actions via `.github/workflows/ci.yml`. The workflow:

1. Builds the PWDFT executable
2. Runs Tier 1 tests on every commit
3. Runs Tier 2 tests on pull requests and main branch pushes
4. Uploads failed test artifacts for debugging

## Troubleshooting

### Common Issues

1. **Pseudopotential not found**: Ensure pseudopotential files are in the PATH or current directory
2. **MPI errors**: Check that OpenMPI is properly installed and configured
3. **Memory issues**: Reduce `ecutwfc` or system size for faster tests
4. **Convergence failures**: Adjust `conv_thr` or `mixing_beta` parameters

### Debugging Failed Tests

Failed tests create compressed archives (`*_failed.tar.gz`) containing:
- Input files
- Output files
- Any intermediate files

These can be downloaded from the GitHub Actions artifacts for local analysis.

## Benchmark Values

The benchmark values used in validation are based on:
- Literature values where available
- Previous PWDFT calculations
- Physical expectations (e.g., metals should have zero band gap)

**Note**: These values may need adjustment based on the specific pseudopotentials and calculation parameters used. 