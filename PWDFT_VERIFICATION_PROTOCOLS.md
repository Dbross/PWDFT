# PWDFT Verification Protocols

## Overview

This document provides comprehensive verification protocols for the PWDFT codebase, ensuring that the implemented deviations from the canonical workflow are physically correct and numerically robust. These protocols serve as "smoke tests" to validate the implementation's correctness.

## 1. Physics-Based Smoke Tests

### 1.1 Density Conservation Test

**Purpose**: Verify that MPI density reduction operations preserve the total number of electrons.

**Physical Principle**: The integral of the electron density over the unit cell must equal the total number of valence electrons.

**Test Implementation**:
```cpp
// Test for PSPW
double total_electrons = mygrid->rr_sum(dn) * dv;
double expected_electrons = mygrid->ne[0] + mygrid->ne[1];
if (mygrid->ispin == 1) expected_electrons *= 2;

double density_error = std::abs(total_electrons - expected_electrons);
assert(density_error < 1e-10);

// Test for BAND (k-point weighted)
double total_electrons_band = 0.0;
for (int k = 0; k < nkpoints; k++) {
    total_electrons_band += kweights[k] * mygrid->rr_sum(dn_k[k]) * dv;
}
assert(std::abs(total_electrons_band - expected_electrons) < 1e-10);
```

**Expected Result**: Density conservation should be maintained to machine precision.

**Failure Indication**: MPI communication errors or numerical precision issues.

### 1.2 Energy Conservation Test

**Purpose**: Verify that the total energy calculation is consistent and all components sum correctly.

**Physical Principle**: The total energy should equal the sum of its components: kinetic, Hartree, exchange-correlation, and ion-ion energies.

**Test Implementation**:
```cpp
// Verify energy components sum to total
double E_calculated = E[1] + E[2] + E[3] + E[4];  // eorbit + ehartr + exc + eion
double energy_error = std::abs(E[0] - E_calculated);
assert(energy_error < 1e-12);

// Verify individual energy components are reasonable
assert(E[1] > 0.0);  // Kinetic energy should be positive
assert(E[2] > 0.0);  // Hartree energy should be positive
assert(E[4] > 0.0);  // Ion-ion energy should be positive
```

**Expected Result**: Energy components should sum to the total energy within machine precision.

**Failure Indication**: Numerical errors in energy calculation or missing energy terms.

### 1.3 FFT Round-Trip Test

**Purpose**: Verify that FFT operations are numerically stable and reversible.

**Physical Principle**: A forward FFT followed by an inverse FFT should recover the original function to machine precision.

**Test Implementation**:
```cpp
// Test FFT round-trip for wavefunctions
mygrid->gh_fftb(psi1, psi_r);  // G -> r
mygrid->gh_fftf(psi_r, psi2);  // r -> G

double fft_error = 0.0;
for (int i = 0; i < ncoeff; i++) {
    fft_error += std::abs(psi1[i] - psi2[i]);
}
fft_error /= ncoeff;

assert(fft_error < 1e-14);

// Test FFT round-trip for density
mygrid->rc_pfft3f(0, rho);  // r -> G
mygrid->cr_pfft3b(0, rho);  // G -> r

double density_fft_error = 0.0;
for (int i = 0; i < nfft3d; i++) {
    density_fft_error += std::abs(rho_original[i] - rho[i]);
}
density_fft_error /= nfft3d;

assert(density_fft_error < 1e-14);
```

**Expected Result**: FFT round-trip errors should be at machine precision level.

**Failure Indication**: FFT library issues or numerical instabilities.

### 1.4 Orthogonality Test

**Purpose**: Verify that wavefunctions maintain orthonormality throughout the SCF process.

**Physical Principle**: The overlap matrix should be the identity matrix: ⟨ψ_i|ψ_j⟩ = δ_ij.

**Test Implementation**:
```cpp
// Test wavefunction orthogonality
mygrid->ggm_sym_Multiply(psi1, psi1, overlap);

double ortho_error = 0.0;
for (int i = 0; i < neall; i++) {
    for (int j = 0; j < neall; j++) {
        double expected = (i == j) ? 1.0 : 0.0;
        ortho_error += std::abs(overlap[i*neall + j] - expected);
    }
}
ortho_error /= (neall * neall);

assert(ortho_error < 1e-12);

// Test for BAND (k-point dependent)
for (int k = 0; k < nkpoints; k++) {
    mygrid->ggm_sym_Multiply(psi1_k[k], psi1_k[k], overlap_k);
    // ... same orthogonality check for each k-point
}
```

**Expected Result**: Orthogonality should be maintained to high precision.

**Failure Indication**: Orthogonalization algorithm issues or numerical instabilities.

## 2. Convergence Verification Tests

### 2.1 SCF Convergence Test

**Purpose**: Verify that the SCF loop converges properly and monitors the correct convergence criteria.

**Test Implementation**:
```cpp
// Monitor convergence criteria
bool converged = (std::abs(deltae) < control.tolerances(0)) && 
                 (deltac < control.tolerances(1)) && 
                 (deltar < control.tolerances(2));

// Verify convergence behavior
if (iteration > 1) {
    // Energy should generally decrease or converge
    assert(E_current <= E_previous + energy_tolerance || 
           std::abs(E_current - E_previous) < energy_tolerance);
    
    // Convergence criteria should be reasonable
    assert(deltae >= 0.0 || std::abs(deltae) < 1e-6);
    assert(deltac >= 0.0);
    assert(deltar >= 0.0);
}

// Verify maximum iteration limit
assert(iteration <= control.loop(1));
```

**Expected Result**: SCF should converge within the specified tolerance and iteration limits.

**Failure Indication**: Convergence algorithm issues or parameter problems.

### 2.2 K-point Convergence Test (BAND)

**Purpose**: Verify that k-point sampling converges properly with respect to the k-point mesh density.

**Test Implementation**:
```cpp
// Test k-point convergence
std::vector<int> k_meshes = {2, 4, 6, 8, 10};
std::vector<double> energies;

for (int mesh : k_meshes) {
    // Set k-point mesh
    control.set_kpoint_mesh(mesh, mesh, mesh);
    
    // Run SCF calculation
    double energy = run_scf_calculation();
    energies.push_back(energy);
}

// Check convergence with respect to k-point mesh
for (int i = 1; i < energies.size(); i++) {
    double energy_diff = std::abs(energies[i] - energies[i-1]);
    double mesh_ratio = (double)(k_meshes[i] * k_meshes[i] * k_meshes[i]) / 
                       (double)(k_meshes[i-1] * k_meshes[i-1] * k_meshes[i-1]);
    
    // Energy difference should decrease with finer mesh
    if (i > 1) {
        assert(energy_diff <= previous_energy_diff * 1.5);  // Allow some variation
    }
    previous_energy_diff = energy_diff;
}
```

**Expected Result**: Energy should converge with respect to k-point mesh density.

**Failure Indication**: K-point sampling issues or convergence problems.

## 3. Performance Verification Tests

### 3.1 MPI Scalability Test

**Purpose**: Verify that the parallel implementation scales efficiently with the number of MPI ranks.

**Test Implementation**:
```cpp
// Test MPI scalability
std::vector<int> nranks_list = {1, 2, 4, 8, 16};
std::vector<double> timings;

for (int nranks : nranks_list) {
    // Set up MPI with nranks
    setup_mpi(nranks);
    
    // Run benchmark calculation
    double start_time = MPI_Wtime();
    run_benchmark_calculation();
    double end_time = MPI_Wtime();
    
    timings.push_back(end_time - start_time);
}

// Calculate speedup and efficiency
for (int i = 1; i < nranks_list.size(); i++) {
    double speedup = timings[0] / timings[i];
    double efficiency = speedup / nranks_list[i];
    
    // Efficiency should be reasonable (at least 70% for moderate parallelism)
    if (nranks_list[i] <= 8) {
        assert(efficiency > 0.7);
    } else {
        assert(efficiency > 0.5);  // Allow lower efficiency for high parallelism
    }
}
```

**Expected Result**: Parallel efficiency should be reasonable for the given system size.

**Failure Indication**: Communication bottlenecks or load balancing issues.

### 3.2 Memory Usage Test

**Purpose**: Verify that memory usage scales reasonably with system size.

**Test Implementation**:
```cpp
// Test memory usage scaling
std::vector<int> system_sizes = {32, 64, 128, 256};
std::vector<size_t> memory_usage;

for (int size : system_sizes) {
    // Set up system with given size
    setup_system(size);
    
    // Measure memory usage
    size_t mem_usage = get_memory_usage();
    memory_usage.push_back(mem_usage);
}

// Check memory scaling (should be roughly O(N^3) for plane-wave methods)
for (int i = 1; i < system_sizes.size(); i++) {
    double size_ratio = (double)(system_sizes[i] * system_sizes[i] * system_sizes[i]) / 
                       (double)(system_sizes[i-1] * system_sizes[i-1] * system_sizes[i-1]);
    double memory_ratio = (double)memory_usage[i] / (double)memory_usage[i-1];
    
    // Memory should scale roughly as N^3 (allow some variation)
    assert(memory_ratio <= size_ratio * 1.5);
    assert(memory_ratio >= size_ratio * 0.5);
}
```

**Expected Result**: Memory usage should scale reasonably with system size.

**Failure Indication**: Memory leaks or inefficient memory management.

## 4. Numerical Stability Tests

### 4.1 NaN/Inf Detection Test

**Purpose**: Verify that numerical operations do not produce NaN or infinite values.

**Test Implementation**:
```cpp
// Check for NaN/Inf in key arrays
bool check_nan_inf(const std::string& name, const double* array, int size) {
    for (int i = 0; i < size; i++) {
        if (std::isnan(array[i]) || std::isinf(array[i])) {
            std::cerr << "ERROR: " << name << "[" << i << "] = " << array[i] << std::endl;
            return true;
        }
    }
    return false;
}

// Test key arrays throughout SCF
assert(!check_nan_inf("psi1", psi1, ncoeff));
assert(!check_nan_inf("dn", dn, nfft3d));
assert(!check_nan_inf("rho", rho, nfft3d));
assert(!check_nan_inf("Hpsi", Hpsi, ncoeff));

// Test energy components
for (int i = 0; i < 10; i++) {
    assert(!std::isnan(E[i]) && !std::isinf(E[i]));
}
```

**Expected Result**: No NaN or infinite values should be produced.

**Failure Indication**: Numerical instabilities or algorithm issues.

### 4.2 Energy Monotonicity Test

**Purpose**: Verify that the energy generally decreases during SCF iterations.

**Test Implementation**:
```cpp
// Track energy during SCF iterations
std::vector<double> energy_history;

for (int iteration = 0; iteration < max_iterations; iteration++) {
    double current_energy = E[0];
    energy_history.push_back(current_energy);
    
    if (iteration > 0) {
        double energy_change = current_energy - energy_history[iteration-1];
        
        // Energy should generally decrease or be very close to previous value
        if (energy_change > energy_tolerance) {
            // Allow occasional small increases due to mixing
            if (energy_change > 1e-6) {
                std::cerr << "WARNING: Energy increased by " << energy_change 
                          << " at iteration " << iteration << std::endl;
            }
        }
    }
}
```

**Expected Result**: Energy should generally decrease or converge.

**Failure Indication**: Convergence issues or algorithm problems.

## 5. Algorithm-Specific Tests

### 5.1 Density Mixing Test

**Purpose**: Verify that different density mixing algorithms work correctly.

**Test Implementation**:
```cpp
// Test different mixing algorithms
std::vector<int> algorithms = {0, 1, 2, 3, 4};  // Simple, Broyden, Johnson, Anderson, Local-TF

for (int alg : algorithms) {
    // Set mixing algorithm
    control.set_scf_algorithm(alg);
    
    // Run SCF calculation
    bool converged = run_scf_calculation();
    
    // All algorithms should converge for simple systems
    if (system_type == "simple") {
        assert(converged);
    }
    
    // Verify final energy is reasonable
    assert(std::abs(E[0]) < 1e6);  // Energy should not be unreasonably large
}
```

**Expected Result**: All mixing algorithms should work correctly.

**Failure Indication**: Mixing algorithm implementation issues.

### 5.2 Eigensolver Test

**Purpose**: Verify that the eigensolver produces correct eigenvalues and eigenvectors.

**Test Implementation**:
```cpp
// Test eigensolver for simple systems
// For a hydrogen atom, we expect specific energy levels
if (system_type == "hydrogen") {
    // Check that lowest eigenvalue is reasonable
    double lowest_eigenvalue = eigenvalues[0];
    assert(lowest_eigenvalue < -0.5);  // Should be negative and reasonable
    
    // Check eigenvalue ordering
    for (int i = 1; i < neall; i++) {
        assert(eigenvalues[i] >= eigenvalues[i-1]);
    }
}
```

**Expected Result**: Eigensolver should produce physically reasonable results.

**Failure Indication**: Eigensolver algorithm issues.

## 6. Integration Tests

### 6.1 End-to-End Test

**Purpose**: Verify that the complete SCF calculation produces physically reasonable results.

**Test Implementation**:
```cpp
// Test complete SCF calculation for known systems
struct TestSystem {
    std::string name;
    double expected_energy;
    double tolerance;
};

std::vector<TestSystem> test_systems = {
    {"H2", -1.17, 0.01},
    {"H2O", -76.4, 0.1},
    {"CH4", -40.5, 0.1}
};

for (const auto& test : test_systems) {
    // Set up test system
    setup_test_system(test.name);
    
    // Run SCF calculation
    run_scf_calculation();
    
    // Check final energy
    double energy_error = std::abs(E[0] - test.expected_energy);
    assert(energy_error < test.tolerance);
    
    // Check convergence
    assert(converged);
    
    // Check physical properties
    assert(E[1] > 0.0);  // Kinetic energy positive
    assert(E[2] > 0.0);  // Hartree energy positive
}
```

**Expected Result**: Known systems should produce expected energies and properties.

**Failure Indication**: Fundamental algorithm or implementation issues.

## 7. Automated Test Suite

### 7.1 Test Runner

**Purpose**: Automate the execution of all verification tests.

**Implementation**:
```cpp
class PWDFTTestSuite {
public:
    void run_all_tests() {
        std::cout << "Running PWDFT verification tests..." << std::endl;
        
        int passed = 0;
        int total = 0;
        
        // Physics-based tests
        total++; if (test_density_conservation()) passed++;
        total++; if (test_energy_conservation()) passed++;
        total++; if (test_fft_round_trip()) passed++;
        total++; if (test_orthogonality()) passed++;
        
        // Convergence tests
        total++; if (test_scf_convergence()) passed++;
        total++; if (test_kpoint_convergence()) passed++;
        
        // Performance tests
        total++; if (test_mpi_scalability()) passed++;
        total++; if (test_memory_usage()) passed++;
        
        // Numerical stability tests
        total++; if (test_nan_inf_detection()) passed++;
        total++; if (test_energy_monotonicity()) passed++;
        
        // Algorithm-specific tests
        total++; if (test_density_mixing()) passed++;
        total++; if (test_eigensolver()) passed++;
        
        // Integration tests
        total++; if (test_end_to_end()) passed++;
        
        std::cout << "Tests passed: " << passed << "/" << total << std::endl;
        
        if (passed == total) {
            std::cout << "All tests passed!" << std::endl;
        } else {
            std::cout << "Some tests failed!" << std::endl;
            exit(1);
        }
    }
};
```

### 7.2 Continuous Integration

**Purpose**: Integrate verification tests into the development workflow.

**Implementation**:
```yaml
# .github/workflows/pwdft-tests.yml
name: PWDFT Verification Tests

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v2
    
    - name: Setup environment
      run: |
        sudo apt-get update
        sudo apt-get install -y mpich libfftw3-dev libblas-dev liblapack-dev
    
    - name: Build PWDFT
      run: |
        cd build
        cmake ..
        make -j4
    
    - name: Run verification tests
      run: |
        ./test_verification_suite
    
    - name: Run performance benchmarks
      run: |
        ./run_performance_benchmarks
```

## 8. Reporting and Documentation

### 8.1 Test Results Format

**Purpose**: Provide clear reporting of test results.

**Implementation**:
```cpp
struct TestResult {
    std::string test_name;
    bool passed;
    double value;
    double expected;
    double tolerance;
    std::string message;
};

void generate_test_report(const std::vector<TestResult>& results) {
    std::ofstream report("verification_report.md");
    
    report << "# PWDFT Verification Report\n\n";
    report << "Generated: " << get_current_time() << "\n\n";
    
    int passed = 0;
    for (const auto& result : results) {
        if (result.passed) passed++;
        
        report << "## " << result.test_name << "\n";
        report << "- Status: " << (result.passed ? "PASSED" : "FAILED") << "\n";
        report << "- Value: " << result.value << "\n";
        report << "- Expected: " << result.expected << "\n";
        report << "- Tolerance: " << result.tolerance << "\n";
        if (!result.passed) {
            report << "- Message: " << result.message << "\n";
        }
        report << "\n";
    }
    
    report << "## Summary\n";
    report << "Tests passed: " << passed << "/" << results.size() << "\n";
    report << "Success rate: " << (100.0 * passed / results.size()) << "%\n";
}
```

## 9. Conclusion

These verification protocols provide comprehensive testing of the PWDFT implementation, ensuring that:

1. **Physical correctness** is maintained through conservation laws and physical principles
2. **Numerical stability** is achieved through proper error checking and bounds validation
3. **Performance** is verified through scalability and efficiency tests
4. **Algorithm correctness** is validated through specific tests for each implemented deviation

The automated test suite can be integrated into the development workflow to catch regressions and ensure code quality. Regular execution of these tests provides confidence in the implementation's correctness and reliability. 