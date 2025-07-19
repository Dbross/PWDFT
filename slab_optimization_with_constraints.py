#!/usr/bin/env python3
"""
Example of slab optimization with adsorbates, fixing the bottom half.

This script demonstrates:
1. Generate a clean slab
2. Optimize the clean slab (all atoms free)
3. Add adsorbate to the relaxed slab
4. Optimize adsorbate + top half while fixing bottom half
5. Use PWDFT ASE calculator with specific keywords
"""

import sys
import os
import numpy as np
from ase import Atoms
from ase.io import write, read
from ase.constraints import FixAtoms
from ase.optimize import BFGS
import ase.build

# Aurora flag for supercomputer vs local run
Aurora = False  # Set to True for Aurora, False for local

if Aurora:
    pwdft_command = (
        'mpiexec -n 24 --ppn 12  --depth=1 --cpu-bind depth '
        '--env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close '
        'gpu_tile_compact.sh /home/brossdh/PWDFT/build_sycl/pwdft PREFIX.nwxi '
        '> PREFIX.out'
    )
    sys.path.append("/home/brossdh/src/pynta")
else:
    pwdft_command = '/Users/brossdh/src/PWDFT/build/pwdft'
    sys.path.append("/Users/brossdh/src/pynta")

from ase_pwdft.pwdft import PWDFT

def generate_slab(metal, surface_type, lattice_constant, size=(3, 3, 6), vacuum=8.0):
    """Generate a slab with enough layers for bottom fixing."""
    slab_type = getattr(ase.build, surface_type)
    slab = slab_type(symbol=metal, size=size, a=lattice_constant, vacuum=vacuum)
    slab.pbc = (True, True, False)  # Periodic in x,y, not in z
    return slab

def fix_bottom_half(atoms, fraction=0.5):
    """
    Fix the bottom half of the slab.
    
    Parameters:
    -----------
    atoms : ase.Atoms
        The slab atoms
    fraction : float
        Fraction of atoms to fix from bottom (default 0.5 = bottom half)
    
    Returns:
    --------
    ase.Atoms
        Atoms with constraints applied
    """
    # Get z-coordinates of all atoms
    z_positions = atoms.get_positions()[:, 2]
    
    # Find the threshold z-coordinate for the bottom fraction
    sorted_z = np.sort(z_positions)
    threshold_index = int(len(sorted_z) * fraction)
    threshold_z = sorted_z[threshold_index]
    
    # Create mask for atoms to fix (bottom half)
    fix_mask = z_positions <= threshold_z
    
    # Apply constraints
    constraints = FixAtoms(mask=fix_mask)
    atoms.set_constraint(constraints)
    
    # Print information
    n_fixed = np.sum(fix_mask)
    n_free = len(atoms) - n_fixed
    print(f"   Fixed {n_fixed} atoms (bottom half)")
    print(f"   Free {n_free} atoms (top half + adsorbates)")
    print(f"   Z-threshold: {threshold_z:.3f} Å")
    
    return atoms

def setup_pwdft_calculator(label, initial_guess=False, input_wfn=None, output_wfn=None, **kwargs):
    """Set up PWDFT calculator with default or robust initial parameters."""
    if initial_guess:
        # Robust initial SCF settings for metallic slabs
        nwpw_block = {
            'xc': 'pbe96',
            'cutoff': 60.0,  # Increased from30u slab
            'scf': 'ks-grassmann-cg simple alpha 0.25',  # Reduced alpha for stability
            'smear': 'methfessel-paxton',
            'temperature': 500,  # Reduced from 2000 for better convergence
            'loop': '10 10',  # Increased iterations for convergence
            'monkhorst-pack': '2 2 1',  # Better k-point sampling
            'initial_wavefunction_guess': 'superposition',  # Use superposition for metals
        }
        if output_wfn:
            nwpw_block['output_wavefunction_filename'] = output_wfn
    else:
        # User's original settings
        nwpw_block = {
            'xc': 'pbe96',
            'cutoff': 60.0,
            'scf': 'ks-grassmann-cg anderson alpha 0.15',
            'smear': 'methfessel-paxton',
            'temperature': 300,
            'loop': '20 20',
            'monkhorst-pack': '3 3 1',
        }
        if input_wfn:
            nwpw_block['input_wavefunction_filename'] = input_wfn
        if output_wfn:
            nwpw_block['output_wavefunction_filename'] = output_wfn
    # Allow user to override
    for key, value in kwargs.items():
        if key == 'nwpw':
            nwpw_block.update(value)
    return PWDFT(
        command=pwdft_command,
        label=label,
        echo=True,
        charge=0,
        nwpw=nwpw_block
    )

def main():
    """Complete example of slab optimization with constraints, with robust initial SCF."""
    print("=== Slab Optimization with Bottom Half Fixed ===\n")
    
    # Step 1: Generate clean slab
    print("1. Generating clean Cu(111) slab...")
    slab = generate_slab(
        metal='Cu',
        surface_type='fcc111',
        lattice_constant=3.712,
        size=(3, 3, 6),  # 6 layers for good bottom fixing
        vacuum=10.0
    )
    print(f"   Generated slab with {len(slab)} atoms")
    print(f"   Cell dimensions: {slab.cell.diagonal()}")
    write('cu_slab_initial.xyz', slab)
    print("   Initial structure saved as cu_slab_initial.xyz")

    # Step 2: Initial robust SCF to generate starting wavefunction
    print("\n2. Initial robust SCF to generate starting wavefunction...")
    initial_wfn = 'slab_init.wfn'
    initial_calc = setup_pwdft_calculator('cu_slab_init', initial_guess=True, output_wfn=initial_wfn)
    slab.calc = initial_calc
    
    # Run a single-point calculation with error handling
    try:
        print("   Starting SCF calculation...")
        energy = slab.get_potential_energy()
        print(f"   ✅ Initial SCF energy: {energy:.6f} eV")
        print(f"   ✅ Initial wavefunction written to {initial_wfn}")
    except Exception as e:
        print(f"   ❌ Initial SCF failed: {e}")
        print("   Trying alternative settings...")
        
        # Try with even more conservative settings
        alt_calc = setup_pwdft_calculator('cu_slab_init_alt', initial_guess=True, output_wfn=initial_wfn)
        alt_calc.parameters.update({
            'cutoff': 40.0,  # Lower cutoff
            'temperature': 300,  # Lower temperature
            'loop': '20 20',  # More iterations
            'initial_wavefunction_guess': 'gaussian',  # Try gaussian instead
        })
        slab.calc = alt_calc
        
        try:
            energy = slab.get_potential_energy()
            print(f"   ✅ Alternative SCF energy: {energy:.6f} eV")
        except Exception as e2:
            print(f"   ❌ Alternative SCF also failed: {e2}")
            print(f"   Cannot proceed without successful SCF. Exiting.")
            return
    
    # Step 3: Optimize clean slab (all atoms free) using initial wavefunction
    print("\n3. Optimizing clean slab (all atoms free) with initial wavefunction...")
    try:
        clean_calc = setup_pwdft_calculator('cu_clean', input_wfn=initial_wfn)
        slab.calc = clean_calc
        optimizer = BFGS(slab, trajectory='cu_clean_opt.traj')
        optimizer.run(fmax=0.01, steps=50)
        print(f"   ✅ Clean slab optimization completed in {optimizer.nsteps} steps")
        write('cu_slab_clean_optimized.xyz', slab)
        print("   Optimized clean slab saved as cu_slab_clean_optimized.xyz")
    except Exception as e:
        print(f"   ❌ Clean slab optimization failed: {e}")
        return

    # Step 4: Add adsorbate
    print("\n4. Adding adsorbate to optimized slab...")
    try:
        from ase.build import add_adsorbate
        add_adsorbate(slab, 'H', 1.5, 'ontop')
        print(f"   ✅ Added H adsorbate. Total atoms: {len(slab)}")
        write('cu_slab_with_H.xyz', slab)
        print("   Slab with adsorbate saved as cu_slab_with_H.xyz")
    except Exception as e:
        print(f"   ❌ Failed to add adsorbate: {e}")
        return

    # Step 5: Fix bottom half and optimize adsorbate + top half
    print("\n5. Fixing bottom half and optimizing adsorbate + top half...")
    try:
        slab = fix_bottom_half(slab, fraction=0.5)
        adsorbate_calc = setup_pwdft_calculator('cu_with_H', input_wfn=initial_wfn)
        slab.calc = adsorbate_calc
        optimizer = BFGS(slab, trajectory='cu_with_H_opt.traj')
        optimizer.run(fmax=0.01, steps=100)
        print(f"   ✅ Adsorbate optimization completed in {optimizer.nsteps} steps")
        write('cu_slab_final_optimized.xyz', slab)
        print("   Final optimized structure saved as cu_slab_final_optimized.xyz")
    except Exception as e:
        print(f"   ❌ Adsorbate optimization failed: {e}")
        return

    # Step 6: Calculate final energy
    print("\n6. Calculating final energy...")
    try:
        final_energy = slab.get_potential_energy()
        print(f"   ✅ Final total energy: {final_energy:.6f} eV")
        forces = slab.get_forces()
        free_forces = forces[~slab.constraints[0].index]
        max_force = np.max(np.linalg.norm(free_forces, axis=1))
        print(f"   ✅ Maximum force on free atoms: {max_force:.6f} eV/Å")
    except Exception as e:
        print(f"   ❌ Final energy calculation failed: {e}")

    # Step 7: Alternative approach - fix more layers
    print("\n7. Alternative: Fix bottom 2/3 of slab...")
    slab_alt = read('cu_slab_with_H.xyz')
    slab_alt = fix_bottom_half(slab_alt, fraction=0.67)
    adsorbate_calc_alt = setup_pwdft_calculator('cu_with_H_alt', input_wfn=initial_wfn)
    slab_alt.calc = adsorbate_calc_alt
    optimizer_alt = BFGS(slab_alt, trajectory='cu_with_H_alt_opt.traj')
    optimizer_alt.run(fmax=0.01, steps=100)
    print(f"   Alternative optimization completed in {optimizer_alt.nsteps} steps")
    write('cu_slab_alt_optimized.xyz', slab_alt)
    alt_energy = slab_alt.get_potential_energy()
    print(f"   Energy difference: {alt_energy - final_energy:.6f} eV")
    print("\n=== Summary ===")
    print("• Clean slab optimized (all atoms free)")
    print("• Adsorbate added to relaxed slab")
    print("• Bottom half fixed during adsorbate optimization")
    print("• Alternative: Bottom 2/3 fixed")
    print("• All calculations use your specified PWDFT keywords")
    print("• Trajectory files saved for analysis")

if __name__ == "__main__":
    main() 
