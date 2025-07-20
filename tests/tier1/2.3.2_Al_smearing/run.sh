#!/bin/bash
PWDFT_EXECUTABLE=${PWDFT_BIN:-../../../build/pwdft}
$PWDFT_EXECUTABLE < al_energy.nw > al_energy.out 