#!/usr/bin/env python3
"""
Simple ASE script that just generates PWDFT input files.
No command needed - just generates .nwx files for manual execution.
"""

import sys
import numpy as np
from ase import Atoms
from ase.io import write
from ase.constraints import FixAtoms
import ase.build

# Add the PWDFT ASE calculator to the path
sys.path.append("pynta/ase_pwdft")
from pwdft import PWDFT

def generate_slab(metal, surface_type, lattice_constant, size=(3, 3, 6), vacuum=8.0):
    """Generate a slab with enough layers for bottom fixing."""
    slab_type = getattr(ase.build, surface_type)
    slab = slab_type(symbol=metal, size=size, a=lattice_constant, vacuum=vacuum)
    slab.pbc = (True, True, False)  # Periodic in x,y, not in z
    return slab

def fix_bottom_half(atoms, fraction=0.5):
    """Fix the bottom half of the slab."""
    z_positions = atoms.get_positions()[:, 2]
    sorted_z = np.sort(z_positions)
    threshold_index = int(len(sorted_z) * fraction)
    threshold_z = sorted_z[threshold_index]
    fix_mask = z_positions <= threshold_z
    constraints = FixAtoms(mask=fix_mask)
    atoms.set_constraint(constraints)
    
    n_fixed = np.sum(fix_mask)
    n_free = len(atoms) - n_fixed
    print(f"   Fixed {n_fixed} atoms (bottom half)")
    print(f"   Free {n_free} atoms (top half + adsorbates)")
    
    return atoms

def main():
    """Generate PWDFT input files for manual execution."""
    
    print("=== ASE PWDFT Input File Generator ===\n")
    
    # Step 1: Generate clean slab
    print("1. Generating clean Cu(111) slab...")
    slab = generate_slab(
        metal='Cu',
        surface_type='fcc111',
        lattice_constant=3.615,
        size=(3, 3, 6),
        vacuum=8.0
    )
    
    print(f"   Generated slab with {len(slab)} atoms")
    
    # Save initial structure
    write('cu_slab_initial.xyz', slab)
    print("   Initial structure saved as cu_slab_initial.xyz")
    
    # Step 2: Generate input file for clean slab optimization
    print("\n2. Generating PWDFT input for clean slab optimization...")
    
    clean_calc = PWDFT(
        label='cu_clean_opt',
        echo=True,
        charge=0,
        nwpw={
            'xc': 'pbe96',
            'cutoff': 60.0,
            'scf': 'ks-grassmann-cg anderson alpha 0.15',
            'smear': 'methfessel-paxton',
            'temperature': 300,
            'loop': '20 20',
            'monkhorst-pack': '3 3 1',
            'geometry': 'optimize',  # Enable geometry optimization
        }
    )
    
    slab.calc = clean_calc
    # This just generates cu_clean_opt.nwx - doesn't run anything
    slab.get_potential_energy()
    print("   Generated: cu_clean_opt.nwx")
    
    # Step 3: Add adsorbate
    print("\n3. Adding adsorbate...")
    
    from ase.build import add_adsorbate
    add_adsorbate(slab, 'H', 1.5, 'ontop')
    
    print(f"   Added H adsorbate. Total atoms: {len(slab)}")
    write('cu_slab_with_H.xyz', slab)
    
    # Step 4: Fix bottom half
    print("\n4. Fixing bottom half...")
    slab = fix_bottom_half(slab, fraction=0.5)
    
    # Step 5: Generate input file for adsorbate optimization
    print("\n5. Generating PWDFT input for adsorbate optimization...")
    
    adsorbate_calc = PWDFT(
        label='cu_with_H_opt',
        echo=True,
        charge=0,
        nwpw={
            'xc': 'pbe96',
            'cutoff': 60.0,
            'scf': 'ks-grassmann-cg anderson alpha 0.15',
            'smear': 'methfessel-paxton',
            'temperature': 300,
            'loop': '20 20',
            'monkhorst-pack': '3 3 1',
            'geometry': 'optimize',  # Enable geometry optimization
        }
    )
    
    slab.calc = adsorbate_calc
    # This just generates cu_with_H_opt.nwx - doesn't run anything
    slab.get_potential_energy()
    print("   Generated: cu_with_H_opt.nwx")
    
    print("\n=== Summary ===")
    print("Generated files:")
    print("• cu_slab_initial.xyz - Initial slab structure")
    print("• cu_slab_with_H.xyz - Slab with adsorbate")
    print("• cu_clean_opt.nwx - PWDFT input for clean slab optimization")
    print("• cu_with_H_opt.nwx - PWDFT input for adsorbate optimization")
    print("\nTo run these calculations:")
    print("1. qsub run_ase_pwdft.sh  # This will run all .nwx files")
    print("2. Or run manually: mpiexec -n 12 ... pwdft cu_clean_opt.nwx > cu_clean_opt.out")

if __name__ == "__main__":
    main() 