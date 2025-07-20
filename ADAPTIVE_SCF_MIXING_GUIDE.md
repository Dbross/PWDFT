# Adaptive SCF Mixing Guide

## Overview

The Adaptive SCF Mixing feature automatically detects numerical instabilities (NaN/Inf values) during SCF iterations and makes the mixing more conservative to improve convergence stability. This is particularly useful for challenging systems that may encounter convergence issues.

## Key Features

### 1. **Automatic NaN Detection**
- Monitors energy changes, SCF errors, and mixed potentials for NaN/Inf values
- Detects numerical instabilities at multiple points in the SCF process
- Provides detailed diagnostic output when issues are detected

### 2. **Conservative Parameter Adjustment**
- **Alpha Reduction**: Automatically reduces mixing parameter α by 50% each time NaN is detected
- **Beta Reduction**: Reduces mixing parameter β by 30% each time NaN is detected
- **Algorithm Switching**: Falls back to simple mixing (algorithm 0) if complex algorithms fail
- **Minimum Thresholds**: Prevents parameters from becoming too small (α ≥ 0.01, β ≥ 0.05)

### 3. **Convergence Monitoring**
- Tracks SCF error trends across iterations
- Detects when errors are consistently increasing
- Automatically makes mixing more conservative if error increases are detected

### 4. **Recovery Mechanisms**
- Up to 5 automatic recovery attempts per SCF cycle
- Resets mixing history when switching to conservative parameters
- Provides detailed status reporting

## Usage

### Basic Configuration

Enable adaptive mixing in your input file:

```bash
nwpw
   scf_adaptive_mixing true
   scf_adaptive_threshold true
   scf_initial_ethr 1.0e-2
   scf_min_ethr 1.0e-13
   scf_ethr_factor 0.1
end
```

### Advanced Configuration

```bash
nwpw
   # Enable adaptive features
   scf_adaptive_mixing true
   scf_adaptive_threshold true
   
   # Adaptive threshold parameters
   scf_initial_ethr 1.0e-2
   scf_min_ethr 1.0e-13
   scf_ethr_factor 0.1
   
   # SCF parameters (will be automatically adjusted if needed)
   minimizer 8
   loop 10 50
   tolerances 1.0e-7 1.0e-7 1.0e-4
   scf_algorithm 2
   scf_alpha 0.25
   scf_beta 0.1
   diis_histories 10
   print medium
end
```

## How It Works

### 1. **Detection Phase**
The adaptive mixing monitors three key areas:
- **Input Energy Changes**: Checks if `deltae` contains NaN/Inf
- **SCF Error**: Monitors the SCF error for numerical issues
- **Mixed Potentials**: Scans the entire potential array for NaN/Inf values

### 2. **Recovery Phase**
When NaN is detected:
1. **Parameter Reduction**: Reduces α and β by specified factors
2. **Algorithm Switching**: Falls back to simple mixing if needed
3. **History Reset**: Clears mixing history to start fresh
4. **Retry**: Attempts mixing again with conservative parameters

### 3. **Monitoring Phase**
- Tracks SCF error trends
- Detects consistent error increases
- Automatically adjusts parameters if convergence is poor

## Example Output

When adaptive mixing is active, you'll see output like:

```
[PWDFT] Adaptive SCF mixing initialized with:
  - Original alpha: 0.25
  - Original beta: 0.1
  - Algorithm: 2
  - Max NaN attempts: 5

[PWDFT] NaN/Inf detected in scf_error (value=nan) - Attempt 1/5
[PWDFT] Retrying with conservative parameters: alpha=0.125, beta=0.07
[PWDFT] Switching to simple mixing (algorithm 0) for stability

[PWDFT] SCF error increasing: 1.2e-4 -> 1.4e-4 (count: 1)
[PWDFT] Multiple error increases detected. Making mixing more conservative.
```

## Configuration Parameters

### Control Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `scf_adaptive_mixing` | `true` | Enable/disable adaptive mixing |
| `scf_adaptive_threshold` | `true` | Enable adaptive diagonalization thresholds |
| `scf_initial_ethr` | `1.0e-2` | Initial diagonalization threshold |
| `scf_min_ethr` | `1.0e-13` | Minimum diagonalization threshold |
| `scf_ethr_factor` | `0.1` | Threshold reduction factor |

### Internal Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| `max_nan_attempts` | `5` | Maximum recovery attempts |
| `alpha_reduction_factor` | `0.5` | Alpha reduction per attempt |
| `beta_reduction_factor` | `0.7` | Beta reduction per attempt |
| `min_alpha` | `0.01` | Minimum alpha value |
| `min_beta` | `0.05` | Minimum beta value |
| `max_error_increases` | `3` | Max error increases before adjustment |

## Best Practices

### 1. **For Challenging Systems**
```bash
# Start with conservative parameters
nwpw
   scf_adaptive_mixing true
   scf_alpha 0.1
   scf_beta 0.05
   scf_algorithm 0  # Start with simple mixing
end
```

### 2. **For Well-Behaved Systems**
```bash
# Use more aggressive parameters with adaptive safety net
nwpw
   scf_adaptive_mixing true
   scf_alpha 0.25
   scf_beta 0.1
   scf_algorithm 2  # Johnson mixing
end
```

### 3. **For Large Systems**
```bash
# Combine with other stability features
nwpw
   scf_adaptive_mixing true
   scf_adaptive_threshold true
   initial_wavefunction_guess atomic
   minimizer 1  # Steepest descent for stability
end
```

## Troubleshooting

### Common Issues

1. **Too Many Recovery Attempts**
   - **Symptom**: "Maximum NaN recovery attempts exceeded"
   - **Solution**: Check system setup, try different initial guess, or use more conservative starting parameters

2. **Slow Convergence**
   - **Symptom**: Many parameter reductions, slow progress
   - **Solution**: Start with more conservative parameters or check system geometry

3. **Algorithm Switching**
   - **Symptom**: "Switching to simple mixing for stability"
   - **Solution**: This is normal for challenging systems, consider using simple mixing from the start

### Debugging

Enable verbose output to see detailed adaptive mixing behavior:

```bash
nwpw
   scf_adaptive_mixing true
   print high  # For detailed output
end
```

## Performance Impact

- **Overhead**: Minimal - only active when NaN is detected
- **Memory**: Small additional storage for parameter tracking
- **Convergence**: Generally improves stability, may require more iterations but with better reliability

## Compatibility

- **Works with**: All SCF algorithms (0-4)
- **Works with**: All minimizer types
- **Works with**: Both molecule and band calculations
- **Backward compatible**: Can be disabled to use standard mixing

## Example Test Cases

### Test 1: Basic Adaptive Mixing
```bash
# File: test_adaptive_mixing.nw
title "Test Adaptive SCF Mixing"

nwpw
   scf_adaptive_mixing true
   minimizer 8
   loop 10 50
   scf_algorithm 2
   scf_alpha 0.25
   scf_beta 0.1
end

geometry
   O 0.0 0.0 0.0
   H 0.957 0.0 0.0
   H -0.24 0.927 0.0
end

task pspw energy
```

### Test 2: Challenging System
```bash
# File: test_challenging_system.nw
title "Challenging System with Adaptive Mixing"

nwpw
   scf_adaptive_mixing true
   scf_adaptive_threshold true
   initial_wavefunction_guess atomic
   minimizer 1
   loop 20 100
   tolerances 1.0e-6 1.0e-6 1.0e-3
end

# Add your challenging geometry here
```

## Conclusion

The Adaptive SCF Mixing feature provides automatic stability improvements for SCF calculations, particularly useful for:

- **Large systems** with complex electronic structure
- **Challenging geometries** that may cause convergence issues
- **High-throughput calculations** where manual parameter tuning is impractical
- **Educational use** where robust convergence is desired

The feature maintains backward compatibility while providing significant improvements in SCF stability and convergence reliability. 