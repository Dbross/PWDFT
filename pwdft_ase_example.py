#!/usr/bin/env python3
"""
Example of generating ASE input using the PWDFT calculator.

This script demonstrates how to:
1. Generate surface slabs using ASE (following pynta's approach)
2. Set up the PWDFT ASE calculator with appropriate parameters
3. Create PWDFT input files (.nwxi) that can be used directly
4. Run calculations and extract results
"""

import sys
import os
import numpy as np
from ase import Atoms
from ase.io import write
import ase.build

# Add the PWDFT ASE calculator to the path
sys.path.append("pynta/ase_pwdft")
from pwdft import PWDFT

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

def setup_pwdft_calculator(metal, command=None, **kwargs):
    """
    Set up PWDFT ASE calculator with appropriate parameters.
    
    Parameters:
    -----------
    metal : str
        Chemical symbol of the metal
    command : str
        Command to run PWDFT (if None, uses default)
    **kwargs : dict
        Additional calculator parameters
    
    Returns:
    --------
    PWDFT
        Configured PWDFT calculator
    """
    
    # Default command if not provided
    if command is None:
        command = 'pwdft < PREFIX.nwx > PREFIX.nwxo'
    
    # Default parameters following pynta's approach and PWDFT examples
    default_params = {
        'label': f'{metal}_slab',
        'echo': True,
        'charge': 0,
        'nwpw': {
            'xc': 'pbe96',           # Exchange-correlation functional
            'cutoff': 60.0,          # Plane wave cutoff in Ry
            'scf': 'ks-grassmann-cg anderson alpha 0.15',  # SCF method
            'smear': 'methfessel-paxton',  # Smearing method
            'temperature': 300,      # Temperature in K
            'loop': '20 20',         # SCF and geometry optimization loops
            'monkhorst-pack': '3 3 1',  # k-point mesh
        }
    }
    
    # Update with any provided kwargs
    for key, value in kwargs.items():
        if key == 'nwpw' and 'nwpw' in default_params:
            default_params['nwpw'].update(value)
        else:
            default_params[key] = value
    
    # Create calculator
    calc = PWDFT(command=command, **default_params)
    
    return calc

def main():
    """Example usage showing how to generate and set up slabs for PWDFT."""
    
    print("=== PWDFT ASE Calculator Example ===\n")
    
    # Example 1: Generate Cu(111) surface and set up PWDFT calculator
    print("1. Generating Cu(111) surface...")
    
    # Generate the slab
    cu_slab = generate_slab_for_pwdft(
        metal='Cu',
        surface_type='fcc111',
        lattice_constant=3.615,  # Cu lattice constant in Angstroms
        size=(3, 3, 4),          # 3x3x4 unit cells
        vacuum=8.0
    )
    
    print(f"   Generated Cu(111) slab with {len(cu_slab)} atoms")
    print(f"   Cell dimensions: {cu_slab.cell.diagonal()}")
    print(f"   Periodic boundary conditions: {cu_slab.pbc}")
    
    # Set up PWDFT calculator
    cu_calc = setup_pwdft_calculator(
        metal='Cu',
        # You can specify a custom command here:
        # command='mpiexec -n 4 /path/to/pwdft < PREFIX.nwx > PREFIX.nwxo',
        nwpw={
            'xc': 'pbe96',
            'cutoff': 60.0,
            'scf': 'ks-grassmann-cg anderson alpha 0.15',
            'smear': 'methfessel-paxton',
            'temperature': 300,
            'loop': '20 20',
            'monkhorst-pack': '3 3 1'
        }
    )
    
    # Attach calculator to slab
    cu_slab.calc = cu_calc
    
    # Save the slab in various formats
    write('cu_111_slab.xyz', cu_slab)
    print("   Slab saved as cu_111_slab.xyz")
    
    # The calculator will automatically generate the PWDFT input file
    # when we request a calculation
    print("\n2. Generating PWDFT input file...")
    
    # This will trigger the calculator to write the input file
    try:
        energy = cu_slab.get_potential_energy()
        print(f"   Energy calculation completed: {energy:.6f} eV")
        print("   PWDFT input file generated: cu_slab.nwx")
        print("   PWDFT output file generated: cu_slab.nwxo")
    except Exception as e:
        print(f"   Note: Energy calculation failed (this is expected if PWDFT is not installed): {e}")
        print("   However, the PWDFT input file (.nwx) was still generated!")
    
    # Example 2: Generate Pt(111) surface with different parameters
    print("\n3. Generating Pt(111) surface...")
    
    pt_slab = generate_slab_for_pwdft(
        metal='Pt',
        surface_type='fcc111',
        lattice_constant=3.924,  # Pt lattice constant in Angstroms
        size=(2, 2, 3),          # 2x2x3 unit cells
        vacuum=10.0
    )
    
    print(f"   Generated Pt(111) slab with {len(pt_slab)} atoms")
    
    # Set up calculator with different parameters
    pt_calc = setup_pwdft_calculator(
        metal='Pt',
        nwpw={
            'xc': 'pbe96',
            'cutoff': 70.0,      # Higher cutoff for Pt
            'scf': 'ks-grassmann-cg anderson alpha 0.15',
            'smear': 'methfessel-paxton',
            'temperature': 300,
            'loop': '20 20',
            'monkhorst-pack': '3 3 1'
        }
    )
    
    pt_slab.calc = pt_calc
    write('pt_111_slab.xyz', pt_slab)
    print("   Slab saved as pt_111_slab.xyz")
    
    # Example 3: Show how to add adsorbates
    print("\n4. Adding adsorbates to Cu surface...")
    
    # Add a hydrogen atom on top of the Cu surface
    from ase.build import add_adsorbate
    
    # Add H atom 1.5 Angstroms above the surface at 'ontop' site
    add_adsorbate(cu_slab, 'H', 1.5, 'ontop')
    
    print(f"   Added H adsorbate. Total atoms: {len(cu_slab)}")
    
    # Set up calculator for the system with adsorbate
    cu_h_calc = setup_pwdft_calculator(
        metal='Cu',
        label='cu_111_with_H',
        nwpw={
            'cutoff': 60,
            'xc': 'PBE',
            'loop': '10 250',
            'mult': 1
        }
    )
    
    cu_slab.calc = cu_h_calc
    write('cu_111_with_H.xyz', cu_slab)
    print("   Slab with adsorbate saved as cu_111_with_H.xyz")
    
    # Example 4: Show different PWDFT parameters
    print("\n5. Example PWDFT parameters:")
    
    # Lattice optimization parameters (higher precision)
    lattice_opt_params = {
        'nwpw': {
            'cutoff': 80,        # Higher cutoff
            'xc': 'PBE',
            'loop': '20 500',    # More iterations, tighter convergence
            'mult': 1
        }
    }
    
    print("   Lattice optimization parameters:")
    for key, value in lattice_opt_params.items():
        print(f"     {key}: {value}")
    
    # Surface calculation parameters
    surface_params = {
        'nwpw': {
            'cutoff': 60,
            'xc': 'PBE',
            'loop': '10 250',
            'mult': 1,
            'monkhorst-pack': '3 3 1'  # K-points for surface
        }
    }
    
    print("   Surface calculation parameters:")
    for key, value in surface_params.items():
        print(f"     {key}: {value}")
    
    print("\n=== Summary ===")
    print("Files created:")
    print("- cu_111_slab.xyz: Clean Cu(111) surface")
    print("- pt_111_slab.xyz: Clean Pt(111) surface") 
    print("- cu_111_with_H.xyz: Cu(111) surface with H adsorbate")
    print("- *.nwxi: PWDFT input files (generated when calculator is attached)")
    print("\nTo run PWDFT calculations:")
    print("1. Make sure PWDFT is installed and in your PATH")
    print("2. Set the correct command in setup_pwdft_calculator()")
    print("3. Run: pwdft < filename.nwxi > filename.nwxo")
    print("\nThe ASE calculator will automatically:")
    print("- Generate the PWDFT input file (.nwxi)")
    print("- Run the calculation (if PWDFT is available)")
    print("- Read the results from the output file (.nwxo)")

if __name__ == "__main__":
    main() 