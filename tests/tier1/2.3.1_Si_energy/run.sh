#!/bin/bash
PWDFT_EXECUTABLE=${PWDFT_BIN:-../../../build/pwdft}
$PWDFT_EXECUTABLE < si_energy.nw > si_energy.out 