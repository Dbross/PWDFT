#!/usr/bin/env python3

import sys
import os
sys.path.insert(0, '/Users/brossdh/src/pynta')

# Mock ASE Atoms class for testing
from ase.cell import Cell

class MockAtom:
    def __init__(self, symbol):
        self.symbol = symbol

class MockAtoms:
    def __init__(self, cell, positions, symbols):
        self.cell = Cell(cell)
        self.positions = positions
        self.symbols = symbols
        self.pbc = np.array([True, True, True])
    
    def get_cell(self):
        return self.cell
    
    def get_positions(self):
        return self.positions
    
    def get_chemical_symbols(self):
        return self.symbols
    
    def get_initial_magnetic_moments(self):
        return np.array([0.0] * len(self.symbols))
    
    def get_scaled_positions(self):
        # For a cubic cell, just divide by the cell diagonal
        cell_diag = np.diagonal(self.cell.array)
        return self.positions / cell_diag
    
    def __iter__(self):
        return (MockAtom(s) for s in self.symbols)

# Mock numpy
import numpy as np

# Test the PWDFT writer directly
from ase_pwdft.pwdftio.pwdftwriter import write_pwdft_in

# Minimal 4-atom cubic cell (Cu FCC conventional cell)
a = 3.6  # Angstrom
cell = np.array([[a, 0.0, 0.0], [0.0, a, 0.0], [0.0, 0.0, a]])
positions = np.array([
    [0.0, 0.0, 0.0],
    [0.5*a, 0.5*a, 0.0],
    [0.5*a, 0.0, 0.5*a],
    [0.0, 0.5*a, 0.5*a]
])
symbols = ['Cu', 'Cu', 'Cu', 'Cu']
atoms = MockAtoms(cell, positions, symbols)

# Use simulation_cell keyword (SC format)
params = {
    'xc': 'pbe96',
    'cutoff': 10.0,  # Ry
    'nwpw': {
        'scf': 'ks-grassmann-cg anderson alpha 0.15',
        'smear': 'methfessel-paxton',
        'temperature': 300,
        'loop': '1 1',  # Only 1 iteration
        'monkhorst-pack': '1 1 1',
        'simulation_cell': {'SC': a}
    }
}

# Write input file (always use lattice_vectors, never SC)
with open("test_input_minimal.nwxi", "w") as f:
    f.write("""
echo

title \"pwdft\"
      
permanent_dir ./perm
      
scratch_dir ./perm
 
start pwdft
      
geometry units angstrom nocenter noautosym noautoz
  system crystal units angstrom
    lattice_vectors
      3.6 0.0 0.0
      0.0 3.6 0.0
      0.0 0.0 3.6
  end
  Cu 0.0 0.0 0.0
  Cu 0.5 0.5 0.0
  Cu 0.5 0.0 0.5
  Cu 0.0 0.5 0.5
end

cutoff 10.0
nwpw
  2d-hcurve
  lmbfgs
  scf ks-grassmann-cg anderson alpha 0.15
  smear methfessel-paxton
  temperature 300
  loop 1 1
  monkhorst-pack 1 1 1
end
""")

print("Minimal input file generated: test_input_minimal.nwxi")
with open('test_input_minimal.nwxi', 'r') as fd:
    print(fd.read())

# Attempt to run PWDFT
pwdft_exe = '/Users/brossdh/src/PWDFT/build/pwdft'
if os.path.isfile(pwdft_exe):
    print("Running PWDFT...")
    os.system(f"{pwdft_exe} < test_input_minimal.nwxi > test_output_minimal.nwxo")
    print("PWDFT run complete. Output:")
    with open('test_output_minimal.nwxo', 'r') as fd:
        print(fd.read())
else:
    print(f"PWDFT executable not found at {pwdft_exe}") 