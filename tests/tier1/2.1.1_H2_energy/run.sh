#!/bin/bash
rm -f h2-energy.movecs
PWDFT_EXECUTABLE=${PWDFT_BIN:-../../../build/pwdft}
$PWDFT_EXECUTABLE < h2_energy.nw > h2_energy.out 
