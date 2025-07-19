#!/usr/bin/env python3
"""
ASE PWDFT example with PBS job script integration.

This script demonstrates how to use the PWDFT ASE calculator with the PBS job script
that provides the command via environment variable.
"""

import sys
import os
import numpy as np
from ase import Atoms
from ase.io import write
from ase.constraints import FixAtoms
from ase.optimize import BFGS
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
    print(f"   Z-threshold: {threshold_z:.3f} Å")
    
    return atoms

def setup_pwdft_calculator(label, **kwargs):
    """Set up PWDFT calculator with command from environment variable."""
    
    # Get command from environment variable (set by PBS script)
    command = os.environ.get('PWDFT_COMMAND')
    if not command:
        # Fallback command if environment variable is not set
        command = 'nwpw PREFIX.nwx > PREFIX.out'
        print("Warning: PWDFT_COMMAND not found in environment, using fallback")
    
    print(f"Using PWDFT command: {command}")
    
    default_params = {
        'command': command,
        'label': label,
        'echo': True,
        'charge': 0,
        'nwpw': {
            'xc': 'pbe96',
            'cutoff': 60.0,
            'scf': 'ks-grassmann-cg anderson alpha 0.15',
            'smear': 'methfessel-paxton',
            'temperature': 300,
            'loop': '20 20',
            'monkhorst-pack': '3 3 1',
        }
    }
    
    # Update with any provided kwargs
    for key, value in kwargs.items():
        if key == 'nwpw' and 'nwpw' in default_params:
            default_params['nwpw'].update(value)
        else:
            default_params[key] = value
    
    return PWDFT(**default_params)

def main():
    """Example of slab optimization with PBS job script integration."""
    
    print("=== ASE PWDFT with PBS Job Script Integration ===\n")
    
    # Step 1: Generate clean slab
    print("1. Generating clean Cu(111) slab...")
    slab = generate_slab(
        metal='Cu',
        surface_type='fcc111',
        lattice_constant=3.615,
        size=(3, 3, 6),  # 6 layers for good bottom fixing
        vacuum=8.0
    )
    
    print(f"   Generated slab with {len(slab)} atoms")
    print(f"   Cell dimensions: {slab.cell.diagonal()}")
    
    # Save initial structure
    write('cu_slab_initial.xyz', slab)
    print("   Initial structure saved as cu_slab_initial.xyz")
    
    # Step 2: Optimize clean slab (all atoms free)
    print("\n2. Optimizing clean slab (all atoms free)...")
    
    clean_calc = setup_pwdft_calculator('cu_clean')
    slab.calc = clean_calc
    
    # Optimize without constraints
    optimizer = BFGS(slab, trajectory='cu_clean_opt.traj')
    optimizer.run(fmax=0.01, steps=50)  # Force convergence 0.01 eV/Å
    
    print(f"   Clean slab optimization completed in {optimizer.nsteps} steps")
    write('cu_slab_clean_optimized.xyz', slab)
    print("   Optimized clean slab saved as cu_slab_clean_optimized.xyz")
    
    # Step 3: Add adsorbate
    print("\n3. Adding adsorbate to optimized slab...")
    
    from ase.build import add_adsorbate
    
    # Add H atom on top of the surface
    add_adsorbate(slab, 'H', 1.5, 'ontop')
    
    print(f"   Added H adsorbate. Total atoms: {len(slab)}")
    write('cu_slab_with_H.xyz', slab)
    print("   Slab with adsorbate saved as cu_slab_with_H.xyz")
    
    # Step 4: Fix bottom half and optimize adsorbate + top half
    print("\n4. Fixing bottom half and optimizing adsorbate + top half...")
    
    # Apply constraints to fix bottom half
    slab = fix_bottom_half(slab, fraction=0.5)
    
    # Set up calculator for adsorbate optimization
    adsorbate_calc = setup_pwdft_calculator('cu_with_H')
    slab.calc = adsorbate_calc
    
    # Optimize with constraints
    optimizer = BFGS(slab, trajectory='cu_with_H_opt.traj')
    optimizer.run(fmax=0.01, steps=100)
    
    print(f"   Adsorbate optimization completed in {optimizer.nsteps} steps")
    write('cu_slab_final_optimized.xyz', slab)
    print("   Final optimized structure saved as cu_slab_final_optimized.xyz")
    
    # Step 5: Calculate final energy
    print("\n5. Calculating final energy...")
    
    final_energy = slab.get_potential_energy()
    print(f"   Final total energy: {final_energy:.6f} eV")
    
    # Calculate forces on free atoms
    forces = slab.get_forces()
    free_forces = forces[~slab.constraints[0].index]  # Forces on free atoms
    max_force = np.max(np.linalg.norm(free_forces, axis=1))
    print(f"   Maximum force on free atoms: {max_force:.6f} eV/Å")
    
    print("\n=== Summary ===")
    print("• Clean slab optimized (all atoms free)")
    print("• Adsorbate added to relaxed slab")
    print("• Bottom half fixed during adsorbate optimization")
    print("• Uses PWDFT command from PBS job script environment")
    print("• All calculations use your specified PWDFT keywords")
    print("• Trajectory files saved for analysis")

if __name__ == "__main__":
    main() 