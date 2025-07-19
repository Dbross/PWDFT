# Generating Slabs for PWDFT using ASE (pynta approach)

This repository contains scripts to generate surface slabs for PWDFT calculations using ASE functions, following the same approach used in the pynta package.

## Overview

The scripts demonstrate how to:
1. Generate surface slabs using ASE's build functions
2. Set up appropriate calculator parameters for PWDFT/Quantum ESPRESSO
3. Save slabs in formats compatible with PWDFT

## Available Surface Types

ASE supports the following surface types (as used in pynta):

### FCC Surfaces
- `fcc100` - FCC(100) surface
- `fcc110` - FCC(110) surface  
- `fcc111` - FCC(111) surface
- `fcc211` - FCC(211) surface

### BCC Surfaces
- `bcc100` - BCC(100) surface
- `bcc110` - BCC(110) surface
- `bcc111` - BCC(111) surface

### HCP Surfaces
- `hcp0001` - HCP(0001) surface
- `hcp10m10` - HCP(10-10) surface

### Other Surfaces
- `diamond100` - Diamond(100) surface
- `diamond111` - Diamond(111) surface

## Usage

### Basic Slab Generation

```python
from generate_slab_for_pwdft import generate_slab_for_pwdft

# Generate a Cu(111) surface
cu_slab = generate_slab_for_pwdft(
    metal='Cu',
    surface_type='fcc111',
    lattice_constant=3.615,  # Angstroms
    size=(3, 3, 4),          # Unit cells (x, y, z)
    vacuum=8.0               # Vacuum height in Angstroms
)
```

### Setting up Calculator Parameters

```python
from pwdft_slab_example import setup_pwdft_calculator

# Set up calculator with pynta-style parameters
calc = setup_pwdft_calculator(
    metal='Cu',
    kpts=(3, 3, 1),    # K-point mesh
    ecutwfc=40,        # Wavefunction cutoff in Ry
    # Additional parameters as needed
)

# Attach calculator to slab
cu_slab.calc = calc
```

## Key Parameters (following pynta approach)

### Slab Generation Parameters
- `metal`: Chemical symbol (e.g., 'Cu', 'Pt', 'Fe', 'Ru')
- `surface_type`: ASE surface function name (e.g., 'fcc111', 'bcc110')
- `lattice_constant`: Lattice constant in Angstroms
- `size`: Tuple of (x, y, z) unit cells
- `vacuum`: Vacuum height in Angstroms
- `pbc`: Periodic boundary conditions (default: (True, True, False))

### Calculator Parameters (pynta defaults)
```python
software_kwargs = {
    'kpts': (3, 3, 1),                    # K-point mesh
    'tprnfor': True,                      # Print forces
    'occupations': 'smearing',            # Occupation method
    'smearing': 'marzari-vanderbilt',     # Smearing type
    'degauss': 0.01,                      # Smearing width
    'ecutwfc': 40,                        # Wavefunction cutoff (Ry)
    'nosym': True,                        # No symmetry
    'conv_thr': 1e-6,                     # Convergence threshold
    'mixing_mode': 'local-TF',            # Mixing mode
    'pseudopotentials': {...}             # Pseudopotential files
}
```

### Lattice Optimization Parameters (pynta defaults)
```python
lattice_opt_software_kwargs = {
    'kpts': (25, 25, 25),     # Dense k-point mesh
    'ecutwfc': 70,            # High cutoff
    'degauss': 0.02,          # Different smearing
    'mixing_mode': 'plain'    # Different mixing
}
```

## Examples

### Example 1: Cu(111) Surface
```python
cu_slab = generate_slab_for_pwdft(
    metal='Cu',
    surface_type='fcc111',
    lattice_constant=3.615,
    size=(3, 3, 4),
    vacuum=8.0
)
```

### Example 2: Pt(111) Surface
```python
pt_slab = generate_slab_for_pwdft(
    metal='Pt',
    surface_type='fcc111',
    lattice_constant=3.924,
    size=(2, 2, 3),
    vacuum=10.0
)
```

### Example 3: Fe(110) Surface (BCC)
```python
fe_slab = generate_slab_for_pwdft(
    metal='Fe',
    surface_type='bcc110',
    lattice_constant=2.866,
    size=(3, 3, 4),
    vacuum=8.0
)
```

### Example 4: Ru(0001) Surface (HCP)
```python
ru_slab = generate_slab_for_pwdft(
    metal='Ru',
    surface_type='hcp0001',
    lattice_constant=2.706,
    size=(3, 3, 4),
    vacuum=8.0
)
```

## Adding Adsorbates

Following pynta's approach, you can add adsorbates using ASE's `add_adsorbate` function:

```python
from ase.build import add_adsorbate

# Add H atom 1.5 Angstroms above surface at 'ontop' site
add_adsorbate(slab, 'H', 1.5, 'ontop')

# Add O atom at 'fcc' site
add_adsorbate(slab, 'O', 1.2, 'fcc')

# Add CO molecule at 'bridge' site
add_adsorbate(slab, 'CO', 1.8, 'bridge')
```

## Available Adsorption Sites

The available adsorption sites depend on the surface type:

- **FCC(111)**: 'ontop', 'bridge', 'fcc', 'hcp'
- **FCC(100)**: 'ontop', 'bridge', 'hollow'
- **FCC(110)**: 'ontop', 'longbridge', 'shortbridge', 'hollow'
- **BCC(110)**: 'ontop', 'longbridge', 'shortbridge', 'hollow'
- **HCP(0001)**: 'ontop', 'bridge', 'fcc', 'hcp'

## Output Formats

The scripts save slabs in multiple formats:

- `.xyz`: XYZ format for visualization
- `.pwi`: Quantum ESPRESSO input format for PWDFT

## Dependencies

- ASE (Atomic Simulation Environment)
- NumPy
- Quantum ESPRESSO (for calculations)

## Running the Examples

```bash
# Run the basic slab generation example
python generate_slab_for_pwdft.py

# Run the comprehensive example with calculator setup
python pwdft_slab_example.py
```

## Integration with PWDFT

The generated `.pwi` files can be used directly with PWDFT/Quantum ESPRESSO. The ASE calculator parameters are automatically converted to the appropriate input format.

## Notes

- The approach follows pynta's methodology for surface generation
- All parameters are compatible with PWDFT/Quantum ESPRESSO
- The scripts handle different crystal structures (FCC, BCC, HCP) appropriately
- Periodic boundary conditions are set to (True, True, False) for surface calculations 