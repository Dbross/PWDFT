# Best Practices for PWDFT Calculations

## Overview

This guide provides best practices for setting up and running PWDFT calculations efficiently and reliably.

## System Setup

### Hardware Requirements

**Minimum Requirements:**
- 4 GB RAM
- 2 CPU cores
- 10 GB disk space

**Recommended Requirements:**
- 16 GB RAM
- 8+ CPU cores
- 100 GB disk space (SSD preferred)

### Software Dependencies

**Required:**
- C++ compiler (GCC 7+, Clang 6+, Intel 18+)
- CMake 3.12+
- BLAS/LAPACK libraries
- FFTW3 library

**Optional:**
- MPI (for parallel calculations)
- HDF5 (for enhanced I/O)

## Input File Structure

### Basic Input Template

```bash
echo

start calculation_name

memory 1000 mb

charge 0

geometry noautoz nocenter noautosym
system crystal
   lattice_vectors
     3.71 0.000000 0.000000
     0.000000 3.71 0.000000
     0.000000 0.000000 3.71
end

Cu 0.000000 0.000000 0.000000
Cu 0.000000 0.500000 0.500000
Cu 0.500000 0.000000 0.500000
Cu 0.500000 0.500000 0.000000
end

nwpw
  xc pbe96
  cutoff 60.0
  scf ks-grassmann-cg anderson alpha 0.15
  smear methfessel-paxton
  temperature 300
  loop 20 20
  monkhorst-pack 4 4 4
end

task band energy
```

### Key Parameters

**Exchange-Correlation Functionals:**
- `pbe96`: PBE functional (recommended for most systems)
- `hse06`: HSE06 hybrid functional (more accurate, slower)
- `beef-vdw`: BEEF-vdW functional (includes van der Waals)

**Plane-Wave Cutoff:**
- **Molecules**: 30-40 Ry
- **Bulk solids**: 40-60 Ry
- **Surfaces**: 50-70 Ry
- **High accuracy**: 80-100 Ry

**SCF Algorithms:**
- `ks-grassmann-cg anderson alpha 0.15`: Conservative, stable
- `ks-grassmann-cg pulay alpha 0.25`: Standard, balanced
- `ks-grassmann-cg simple alpha 0.02`: Very conservative

## Numerical Stability and NaN Detection

### Automatic NaN Detection

PWDFT includes robust NaN (Not-a-Number) detection and fallback recovery mechanisms that work automatically:

**Features:**
- **Automatic Detection**: NaN values are detected in energy computations, matrix operations, and trace functions
- **Fallback Recovery**: Automatic wavefunction reinitialization and SCF restart
- **Performance**: Minimal overhead (<5% total computational cost)
- **Transparency**: No user intervention required

**Monitoring:**
Watch for these messages in the output:
```
*** NaN/Inf or large energy detected in band SCF (minimizer 8). Failure 1/3
*** Energy value: 1.000000e+10
*** Continuing with current iteration (failure 1/3)

*** Triggering fallback after 3 consecutive failures
*** 15 steepest descent iterations performed for stabilization
*** Energy stabilized, resetting failure counter
```

### Troubleshooting Numerical Issues

**Common Causes of NaN Values:**
1. **Insufficient k-point sampling** for metallic systems
2. **Too aggressive SCF mixing** parameters
3. **Inadequate smearing** for metallic systems
4. **Poor initial wavefunction guess**

**Solutions:**
```bash
# For metallic systems
nwpw
  smear methfessel-paxton
  temperature 300-500
  scf ks-grassmann-cg anderson alpha 0.15  # Conservative mixing
  monkhorst-pack 6 6 6  # Dense k-point sampling
end

# For challenging systems
nwpw
  scf ks-grassmann-cg simple alpha 0.02  # Very conservative
  initial_wavefunction_guess superposition
  loop 50 50  # More iterations
end
```

**Debugging Commands:**
```bash
# Check for NaN detection messages
grep "NaN/Inf detected" output.log

# Check for fallback activity
grep "Triggering fallback" output.log

# Check for energy stabilization
grep "Energy stabilized" output.log
```

## SCF Convergence

### Convergence Criteria

**Energy tolerance**: 1e-5 to 1e-6 Hartree
**Density tolerance**: 1e-5 to 1e-6
**Maximum iterations**: 20-50 for most systems

### Convergence Strategies

**Conservative Approach (Recommended for new systems):**
```bash
nwpw
  scf ks-grassmann-cg anderson alpha 0.15
  smear methfessel-paxton
  temperature 300
  loop 20 20
  tolerances 1e-5 1e-5 1e-4
end
```

**Aggressive Approach (for well-behaved systems):**
```bash
nwpw
  scf ks-grassmann-cg pulay alpha 0.25
  smear methfessel-paxton
  temperature 500
  loop 10 10
  tolerances 1e-6 1e-6 1e-5
end
```

### Convergence Troubleshooting

**If SCF doesn't converge:**
1. **Reduce mixing parameter**: `alpha 0.1` or `alpha 0.05`
2. **Increase smearing**: `temperature 500` or `temperature 1000`
3. **Use simpler mixing**: `scf ks-grassmann-cg simple alpha 0.02`
4. **Increase iterations**: `loop 50 50`
5. **Try different initial guess**: `initial_wavefunction_guess random`

## K-Point Sampling

### Guidelines by System Type

**Molecules (isolated):**
```bash
monkhorst-pack 1 1 1  # Gamma point only
```

**Bulk solids:**
```bash
monkhorst-pack 4 4 4  # Standard
monkhorst-pack 6 6 6  # High accuracy
monkhorst-pack 8 8 8  # Very high accuracy
```

**Surfaces (2D):**
```bash
monkhorst-pack 4 4 1  # Standard
monkhorst-pack 6 6 1  # High accuracy
```

**Wires (1D):**
```bash
monkhorst-pack 4 1 1  # Standard
monkhorst-pack 6 1 1  # High accuracy
```

### Convergence Testing

Always perform k-point convergence studies:

```bash
# Test different k-point meshes
monkhorst-pack 2 2 2
monkhorst-pack 4 4 4
monkhorst-pack 6 6 6
monkhorst-pack 8 8 8

# Compare total energies to determine convergence
```

## Performance Optimization

### Parallel Execution

**MPI Parallelization:**
```bash
# Run with 4 MPI processes
mpirun -np 4 ./build/pwdft input.nw

# For large systems, use more processes
mpirun -np 16 ./build/pwdft input.nw
```

**Memory Management:**
```bash
# Adjust memory allocation based on system size
memory 1000 mb   # Small systems
memory 4000 mb   # Medium systems
memory 16000 mb  # Large systems
```

### I/O Optimization

**Use scratch directories:**
```bash
scratch_dir /tmp/pwdft_scratch
permanent_dir ./results
```

**Reduce I/O frequency:**
```bash
nwpw
  print low  # Reduce output verbosity
  output_wavefunction_filename system.wfn  # Save wavefunction
end
```

## System-Specific Guidelines

### Metals

**Conservative settings for metallic systems:**
```bash
nwpw
  xc pbe96
  cutoff 60.0
  scf ks-grassmann-cg anderson alpha 0.15
  smear methfessel-paxton
  temperature 300
  loop 20 20
  monkhorst-pack 6 6 6
  initial_wavefunction_guess superposition
end
```

### Semiconductors

**Standard settings for semiconductors:**
```bash
nwpw
  xc pbe96
  cutoff 50.0
  scf ks-grassmann-cg pulay alpha 0.25
  loop 15 15
  monkhorst-pack 4 4 4
end
```

### Surfaces and Interfaces

**Settings for surface calculations:**
```bash
nwpw
  xc pbe96
  cutoff 60.0
  scf ks-grassmann-cg anderson alpha 0.15
  smear methfessel-paxton
  temperature 300
  loop 25 25
  monkhorst-pack 4 4 1
  dipole_correction true  # For charged surfaces
end
```

### Large Systems

**Optimizations for large systems:**
```bash
nwpw
  xc pbe96
  cutoff 40.0  # Reduced cutoff for speed
  scf ks-grassmann-cg simple alpha 0.1
  loop 10 10
  monkhorst-pack 2 2 2  # Reduced k-point sampling
  print low
end
```

## Validation and Testing

### Energy Convergence

**Test energy convergence with respect to:**
1. **Plane-wave cutoff**: 30, 40, 50, 60, 70 Ry
2. **K-point sampling**: 2x2x2, 4x4x4, 6x6x6, 8x8x8
3. **SCF tolerance**: 1e-4, 1e-5, 1e-6

### Physical Checks

**Verify results are physically reasonable:**
- **Total energy**: Should be negative and reasonable magnitude
- **Forces**: Should be small (< 0.01 eV/Å) for optimized structures
- **Band gap**: Should match expected values for the material
- **Density**: Should be smooth and positive everywhere

### Comparison with Reference

**Compare with:**
- **Experimental data**: Lattice constants, band gaps, etc.
- **Other codes**: VASP, Quantum ESPRESSO, etc.
- **Literature**: Published DFT results

## Common Pitfalls

### 1. Insufficient k-point sampling
- **Symptom**: Poor convergence, incorrect band structure
- **Solution**: Increase k-point mesh density

### 2. Too aggressive SCF mixing
- **Symptom**: SCF oscillations, NaN values
- **Solution**: Reduce mixing parameter, use conservative algorithm

### 3. Inadequate smearing for metals
- **Symptom**: Poor convergence, incorrect electronic structure
- **Solution**: Use Methfessel-Paxton smearing with appropriate temperature

### 4. Poor initial wavefunction guess
- **Symptom**: Slow convergence, convergence to wrong state
- **Solution**: Try different initial guesses (superposition, random, atomic)

### 5. Insufficient plane-wave cutoff
- **Symptom**: Inaccurate energies, poor convergence
- **Solution**: Increase cutoff, perform convergence study

## Advanced Features

### Adaptive SCF Mixing

**Enable adaptive mixing for challenging systems:**
```bash
nwpw
  scf_adaptive_mixing true
  scf_alpha 0.25
  scf_beta 0.1
  scf_algorithm 0
end
```

### Adaptive Diagonalization Thresholds

**Enable adaptive thresholds for better convergence:**
```bash
nwpw
  scf_adaptive_threshold true
  scf_initial_ethr 1.0e-2
  scf_min_ethr 1.0e-13
  scf_ethr_factor 0.1
end
```

### Fractional Occupation

**For metallic systems with fractional occupation:**
```bash
nwpw
  fractional true
  fractional_kT 0.001
  fractional_orbitals 4
end
```

## Monitoring and Debugging

### Output Analysis

**Key output sections to monitor:**
1. **SCF convergence**: Energy and density convergence
2. **Forces**: Atomic forces for geometry optimization
3. **Band structure**: Electronic band energies
4. **Density of states**: Electronic DOS

### Log File Analysis

**Useful grep commands:**
```bash
# Check SCF convergence
grep "tolerance ok" output.log

# Check for errors
grep -i "error\|warning\|failed" output.log

# Check timing
grep "cputime" output.log

# Check memory usage
grep "memory" output.log
```

### Performance Monitoring

**Monitor computational resources:**
- **CPU usage**: Should be near 100% for single-threaded runs
- **Memory usage**: Should be within allocated limits
- **Disk I/O**: Monitor scratch directory usage
- **Wall time**: Track total calculation time

## Conclusion

Following these best practices will help ensure reliable and efficient PWDFT calculations. Remember to:

1. **Start conservative** and optimize parameters systematically
2. **Perform convergence studies** for new system types
3. **Monitor for numerical issues** and use NaN detection features
4. **Validate results** against experimental or reference data
5. **Document parameters** used for reproducibility

For additional help, consult the main documentation or contact the development team. 