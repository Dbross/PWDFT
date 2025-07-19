#!/usr/bin/env python3
"""
Comprehensive example of generating slabs for PWDFT using ASE functions.

This script demonstrates:
1. How to generate slabs using ASE (following pynta's approach)
2. How to set up the ASE calculator with appropriate keywords for PWDFT
3. How to save the slab in formats compatible with PWDFT
"""

import ase.build
from ase.io import write, read
from ase.calculators.espresso import Espresso
import numpy as np

def generate_slab_for_pwdft(metal, surface_type, lattice_constant, 
                           size=(3, 3, 4), vacuum=8.0, pbc=(True, True, False)):
    """
    Generate a slab for PWDFT using ASE functions, following pynta's approach.
    
    Parameters:
    -----------
    metal : str
        Chemical symbol of the metal (e.g., 'Cu', 'Pt', 'Fe')
    surface_type : str
        Surface type (e.g., 'fcc111', 'bcc110', 'fcc100', 'hcp0001')
    lattice_constant : float
        Lattice constant in Angstroms
    size : tuple
        Size of the slab in unit cells (x, y, z)
    vacuum : float
        Vacuum height in Angstroms
    pbc : tuple
        Periodic boundary conditions (x, y, z)
    
    Returns:
    --------
    ase.Atoms
        The generated slab
    """
    
    # Get the appropriate ASE build function
    slab_type = getattr(ase.build, surface_type)
    
    # Generate the slab using ASE
    # For HCP structures, we might need both 'a' and 'c' parameters
    if surface_type == 'hcp0001':
        # For HCP, c/a ratio is typically sqrt(8/3) ≈ 1.633
        c_over_a = np.sqrt(8/3)
        c = lattice_constant * c_over_a
        slab = slab_type(symbol=metal, size=size, a=lattice_constant, 
                        c=c, vacuum=vacuum)
    else:
        slab = slab_type(symbol=metal, size=size, a=lattice_constant, 
                        vacuum=vacuum)
    
    # Set periodic boundary conditions
    slab.pbc = pbc
    
    return slab

def setup_pwdft_calculator(metal, kpts=(3, 3, 1), ecutwfc=40, **kwargs):
    """
    Set up ASE calculator with PWDFT-compatible parameters.
    
    This follows the same approach as pynta's software_kwargs.
    
    Parameters:
    -----------
    metal : str
        Chemical symbol of the metal
    kpts : tuple
        K-point mesh (x, y, z)
    ecutwfc : float
        Wavefunction cutoff energy in Ry
    **kwargs : dict
        Additional calculator parameters
    
    Returns:
    --------
    ase.calculators.espresso.Espresso
        Configured calculator
    """
    
    # Default parameters following pynta's approach
    default_params = {
        'kpts': kpts,
        'tprnfor': True,
        'occupations': 'smearing',
        'smearing': 'marzari-vanderbilt',
        'degauss': 0.01,
        'ecutwfc': ecutwfc,
        'nosym': True,
        'conv_thr': 1e-6,
        'mixing_mode': 'local-TF',
        'pseudopotentials': {
            'Cu': 'Cu.pbe-spn-kjpaw_psl.1.0.0.UPF',
            'Pt': 'Pt.pbe-spn-kjpaw_psl.1.0.0.UPF',
            'Fe': 'Fe.pbe-spn-kjpaw_psl.1.0.0.UPF',
            'Ru': 'Ru.pbe-spn-kjpaw_psl.1.0.0.UPF',
            'H': 'H.pbe-kjpaw_psl.1.0.0.UPF',
            'O': 'O.pbe-n-kjpaw_psl.1.0.0.UPF',
            'C': 'C.pbe-n-kjpaw_psl.1.0.0.UPF',
            'N': 'N.pbe-n-kjpaw_psl.1.0.0.UPF',
        }
    }
    
    # Update with any provided kwargs
    default_params.update(kwargs)
    
    # Create calculator
    calc = Espresso(**default_params)
    
    return calc

def main():
    """Example usage showing how to generate and set up slabs for PWDFT."""
    
    # Example: Generate Cu(111) surface
    print("=== Generating Cu(111) surface for PWDFT ===")
    
    # Generate the slab
    cu_slab = generate_slab_for_pwdft(
        metal='Cu',
        surface_type='fcc111',
        lattice_constant=3.615,  # Cu lattice constant in Angstroms
        size=(3, 3, 4),  # 3x3x4 unit cells
        vacuum=8.0
    )
    
    print(f"Generated Cu(111) slab with {len(cu_slab)} atoms")
    print(f"Cell dimensions: {cu_slab.cell.diagonal()}")
    print(f"Periodic boundary conditions: {cu_slab.pbc}")
    
    # Set up calculator with PWDFT-compatible parameters
    calc = setup_pwdft_calculator(
        metal='Cu',
        kpts=(3, 3, 1),  # Appropriate for surface calculations
        ecutwfc=40,      # Wavefunction cutoff in Ry
        # You can add more parameters as needed
    )
    
    # Attach calculator to slab
    cu_slab.calc = calc
    
    # Save the slab in various formats
    write('cu_111_slab.xyz', cu_slab)
    write('cu_111_slab.pwi', cu_slab)  # Quantum ESPRESSO input format
    
    print("Slab saved as cu_111_slab.xyz and cu_111_slab.pwi")
    
    # Example: Generate Pt(111) surface with different parameters
    print("\n=== Generating Pt(111) surface for PWDFT ===")
    
    pt_slab = generate_slab_for_pwdft(
        metal='Pt',
        surface_type='fcc111',
        lattice_constant=3.924,  # Pt lattice constant in Angstroms
        size=(2, 2, 3),  # 2x2x3 unit cells
        vacuum=10.0
    )
    
    print(f"Generated Pt(111) slab with {len(pt_slab)} atoms")
    
    # Set up calculator with different parameters
    pt_calc = setup_pwdft_calculator(
        metal='Pt',
        kpts=(4, 4, 1),  # Higher k-point density for Pt
        ecutwfc=50,      # Higher cutoff for Pt
    )
    
    pt_slab.calc = pt_calc
    write('pt_111_slab.xyz', pt_slab)
    write('pt_111_slab.pwi', pt_slab)
    
    print("Slab saved as pt_111_slab.xyz and pt_111_slab.pwi")
    
    # Example: Show how to modify parameters for different calculations
    print("\n=== Example: Lattice optimization parameters ===")
    
    # For lattice optimization, pynta uses different parameters
    lattice_opt_params = {
        'kpts': (25, 25, 25),  # Much denser k-point mesh
        'ecutwfc': 70,         # Higher cutoff
        'degauss': 0.02,       # Different smearing
        'mixing_mode': 'plain' # Different mixing
    }
    
    print("Lattice optimization parameters:")
    for key, value in lattice_opt_params.items():
        print(f"  {key}: {value}")
    
    # Example: Show how to add adsorbates (following pynta's approach)
    print("\n=== Example: Adding adsorbates ===")
    
    # Add a hydrogen atom on top of the Cu surface
    from ase.build import add_adsorbate
    
    # Add H atom 1.5 Angstroms above the surface at 'ontop' site
    add_adsorbate(cu_slab, 'H', 1.5, 'ontop')
    
    print(f"Added H adsorbate. Total atoms: {len(cu_slab)}")
    write('cu_111_with_H.xyz', cu_slab)
    write('cu_111_with_H.pwi', cu_slab)
    
    print("Slab with adsorbate saved as cu_111_with_H.xyz and cu_111_with_H.pwi")
    
    print("\n=== Summary ===")
    print("Files created:")
    print("- cu_111_slab.xyz/.pwi: Clean Cu(111) surface")
    print("- pt_111_slab.xyz/.pwi: Clean Pt(111) surface") 
    print("- cu_111_with_H.xyz/.pwi: Cu(111) surface with H adsorbate")
    print("\nThese files can be used directly with PWDFT/Quantum ESPRESSO")

if __name__ == "__main__":
    main() 