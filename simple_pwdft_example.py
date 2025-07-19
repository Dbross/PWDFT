#!/usr/bin/env python3
"""
Simple example of using PWDFT ASE calculator for single-point calculations.

This script demonstrates how to:
1. Generate a slab using ASE
2. Set up PWDFT calculator with specific keywords
3. Perform a single-point calculation (not geometry optimization)
4. Generate .nwx input files
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

def generate_simple_slab(metal, surface_type, lattice_constant, size=(2, 2, 3), vacuum=8.0):
    """Generate a simple slab for testing."""
    slab_type = getattr(ase.build, surface_type)
    slab = slab_type(symbol=metal, size=size, a=lattice_constant, vacuum=vacuum)
    slab.pbc = (True, True, False)  # Periodic in x,y, not in z
    return slab

def main():
    """Example of single-point calculation with PWDFT."""
    
    print("=== PWDFT Single-Point Calculation Example ===\n")
    
    # Generate a simple Cu(111) slab
    print("1. Generating Cu(111) slab...")
    cu_slab = generate_simple_slab(
        metal='Cu',
        surface_type='fcc111',
        lattice_constant=3.615,
        size=(2, 2, 3),  # Small slab for testing
        vacuum=8.0
    )
    
    print(f"   Generated slab with {len(cu_slab)} atoms")
    print(f"   Cell dimensions: {cu_slab.cell.diagonal()}")
    
    # Set up PWDFT calculator with your specific keywords
    print("\n2. Setting up PWDFT calculator...")
    
    calc = PWDFT(
        command='nwpw',  # PWDFT executable
        label='cu_slab',
        echo=True,
        charge=0,
        nwpw={
            'xc': 'pbe96',           # Exchange-correlation functional
            'cutoff': 60.0,          # Plane wave cutoff in Ry
            'scf': 'ks-grassmann-cg anderson alpha 0.15',  # SCF method
            'smear': 'methfessel-paxton',  # Smearing method
            'temperature': 300,      # Temperature in K
            'loop': '20 20',         # SCF iterations (20 max, 20 convergence)
            'monkhorst-pack': '3 3 1',  # k-point mesh
        }
    )
    
    # Attach calculator to slab
    cu_slab.calc = calc
    
    # Save the slab structure
    write('cu_slab.xyz', cu_slab)
    print("   Slab structure saved as cu_slab.xyz")
    
    # Perform single-point calculation
    print("\n3. Performing single-point calculation...")
    print("   Note: This will generate cu_slab.nwx input file")
    
    try:
        # This triggers the calculator to write the input file and run the calculation
        energy = cu_slab.get_potential_energy()
        print(f"   ✓ Calculation completed successfully!")
        print(f"   ✓ Total energy: {energy:.6f} eV")
        print(f"   ✓ Input file: cu_slab.nwx")
        print(f"   ✓ Output file: cu_slab.nwxo")
        
        # You can also get forces if needed
        forces = cu_slab.get_forces()
        print(f"   ✓ Forces calculated for {len(forces)} atoms")
        
    except Exception as e:
        print(f"   ⚠ Calculation failed: {e}")
        print("   However, the input file cu_slab.nwx should still be generated")
        print("   You can run it manually with: nwpw < cu_slab.nwx > cu_slab.nwxo")
    
    print("\n=== Summary ===")
    print("• Single-point calculation (not geometry optimization)")
    print("• Uses your specified keywords:")
    print("  - xc pbe96")
    print("  - cutoff 60.0")
    print("  - scf ks-grassmann-cg anderson alpha 0.15")
    print("  - smear methfessel-paxton")
    print("  - temperature 300")
    print("  - loop 20 20")
    print("  - monkhorst-pack 3 3 1")
    print("• Generates .nwx input files (not .nwxi)")
    print("• No geometry optimization - just energy calculation")

if __name__ == "__main__":
    main() 