#!/bin/bash
# Run all three calculations for CO adsorption energy
PWDFT_EXECUTABLE=${PWDFT_BIN:-../../../build/pwdft}

echo "Running CO on Pt(111) adsorption calculation..."
$PWDFT_EXECUTABLE < Pt111_CO_relax.nw > Pt111_CO_relax.out

echo "Running clean Pt(111) slab calculation..."
$PWDFT_EXECUTABLE < Pt111_slab.nw > Pt111_slab.out

echo "Running gas-phase CO calculation..."
$PWDFT_EXECUTABLE < CO_gas.nw > CO_gas.out 