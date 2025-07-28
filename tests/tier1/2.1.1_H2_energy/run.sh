#!/bin/bash
rm -f h2-energy.movecs
PWDFT_EXECUTABLE=${PWDFT_BIN:-../../../build/pwdft}
mpirun -n 2 $PWDFT_EXECUTABLE < h2_energy.nw > h2_energy.out 
