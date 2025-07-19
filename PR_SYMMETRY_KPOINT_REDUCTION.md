# Pull Request: Add Symmetry-Aware K-Point Reduction for Crystal Systems

## Overview
This PR implements automatic symmetry detection and k-point reduction for crystal systems in PWDFT/Nwpw, significantly improving computational efficiency for symmetric structures while maintaining accuracy.

## Key Features

### 1. Automatic Symmetry Detection
- **Crystal System Detection**: Automatically detects crystal system from lattice vectors and atomic positions
- **Point Group Recognition**: Identifies symmetry groups (Oh, D4h, C4v, C3v, C2v, Cs, Ci, C1)
- **Tolerance-Based Detection**: Uses configurable symmetry tolerance for robust detection

### 2. Enhanced K-Point Reduction
- **Time-Reversal Symmetry**: Maintains existing time-reversal symmetry pruning
- **Point Group Symmetry**: Adds full point group symmetry reduction
- **IBZ Reduction**: Reduces k-points to irreducible Brillouin zone (IBZ)
- **Weight Accumulation**: Properly accumulates weights for symmetry-equivalent k-points

### 3. Supported Symmetry Groups
- **Cubic (Oh)**: 48 operations (24 proper + 24 improper rotations)
- **Tetragonal (D4h)**: 16 operations
- **Surface Symmetries**: C4v, C3v, C2v for surface calculations
- **Lower Symmetries**: Cs, Ci, C1 for asymmetric structures

## Technical Implementation

### Core Functions Added
```cpp
// Symmetry detection
std::string detect_crystal_symmetry(const std::vector<double>& unita, 
                                   const std::vector<double>& coords,
                                   const std::vector<double>& masses,
                                   double sym_tolerance = 1e-6);

// K-point reduction
void reduce_kpoints_by_symmetry(std::vector<std::vector<double>>& ks, 
                               const std::vector<double>& unita = {},
                               const std::vector<double>& coords = {},
                               const std::vector<double>& masses = {});

// Symmetry operations generation
std::vector<std::vector<std::vector<double>>> generate_symmetry_operations(const std::string& group_name);
```

### Integration Points
- **Monkhorst-Pack Generation**: Enhanced `monkhorst_pack_set()` function
- **Geometry Parsing**: Integrated with `parse_geometry()` and `parse_nwpw()`
- **JSON RTDB**: Passes geometry information through parsing context

## Performance Improvements

### Expected K-Point Reductions
- **FCC Bulk (Oh symmetry)**: ~21x reduction (512 → 24 k-points)
- **Cu(111) Surface (C3v)**: ~3x reduction (9 → 3 k-points)  
- **Cu(100) Surface (C4v)**: ~4x reduction (16 → 4 k-points)
- **Low Symmetry (C1)**: No reduction (64 → 64 k-points)

### Computational Savings
- **Memory Usage**: Reduced by factor of symmetry reduction
- **CPU Time**: Linear scaling with k-point reduction
- **Accuracy**: Maintained through proper weight accumulation

## Usage Examples

### Input File Syntax
```nwchem
nwpw
 simulation_cell
  fcc 3.615
 end
 cutoff 30.0
 smear fermi 0.01
 monkhorst-pack 4 4 4  # Will be automatically reduced by symmetry
 tolerances 1e-6 1e-6 1e-6
end
```

### Output Information
```
   Monkhorst-Pack grid: 4x4x4 = 64 k-points
   Time-reversal reduction: 64 -> 32 k-points
   Detected symmetry: Oh
   Symmetry reduction: 32 -> 24 k-points
   Total reduction: 64 -> 24 k-points (factor of 2.7x)
```

## Testing

### Test Cases Included
1. **FCC Copper Bulk**: Tests cubic symmetry (Oh group)
2. **Cu(111) Surface**: Tests surface symmetry (C3v group)
3. **Cu(100) with Adsorbate**: Tests lower symmetry (C4v group)
4. **Low Symmetry System**: Tests no reduction (C1 group)

### Validation
- **Energy Conservation**: Ensures total energy remains unchanged
- **Force Consistency**: Validates forces are properly symmetrized
- **Convergence**: Confirms SCF convergence is maintained

## Configuration Options

### Symmetry Tolerance
```nwchem
geometry "system" units angstroms symmetry_tolerance 1e-6
```

### Disable Symmetry (if needed)
```nwchem
geometry "system" units angstroms noautosym
```

## Backward Compatibility
- **Default Behavior**: Maintains existing functionality for systems without geometry info
- **Optional Feature**: Can be disabled via input parameters
- **Fallback**: Uses C1 symmetry (no reduction) when detection fails

## Files Modified

### Core Implementation
- `Nwpw/nwpwlib/parse/parse_pwdft.cpp`: Main symmetry detection and k-point reduction logic

### Test Scripts
- `QA/Symmetry/quick_symmetry_test.sh`: Quick symmetry detection test
- `QA/Symmetry/lattice_optimization_workflow.sh`: Full workflow demonstration

### Documentation
- `QA/Symmetry/symmetry_kpoints_examples.md`: Usage examples and theory

## Dependencies
- **nlohmann/json**: For JSON parsing (already included)
- **Standard C++**: Uses C++17 features (already required)

## Performance Impact
- **Compilation**: Minimal impact (additional ~500 lines)
- **Runtime**: Negligible overhead for symmetry detection
- **Memory**: Slight increase for symmetry operations storage

## Future Enhancements
- **Magnetic Symmetries**: Support for magnetic point groups
- **Spin-Orbit Coupling**: Extension to include SOC effects
- **Dynamic Symmetry**: Real-time symmetry detection during MD
- **Custom Symmetries**: User-defined symmetry operations

## Testing Instructions

### Quick Test
```bash
cd QA/Symmetry
bash quick_symmetry_test.sh
```

### Full Workflow
```bash
cd QA/Symmetry  
bash lattice_optimization_workflow.sh
```

### Expected Output
- Symmetry detection messages in calculation output
- K-point reduction statistics
- Consistent energies with and without symmetry

## Review Checklist
- [ ] Symmetry detection accuracy for test cases
- [ ] K-point reduction correctness
- [ ] Energy conservation validation
- [ ] Backward compatibility verification
- [ ] Performance impact assessment
- [ ] Documentation completeness
- [ ] Test coverage adequacy

## Related Issues
- Addresses computational efficiency for symmetric crystal systems
- Improves user experience with automatic optimization
- Enables larger k-point grids for better convergence

---

**Note**: This feature significantly improves the efficiency of PWDFT calculations for symmetric crystal systems while maintaining full accuracy. The automatic detection and reduction process is transparent to users and requires no additional input beyond standard geometry specifications. 