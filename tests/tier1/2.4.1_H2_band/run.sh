#!/bin/bash
PWDFT_EXECUTABLE=${PWDFT_BIN:-../../../build/pwdft}
rm h2-energy.movecs
$PWDFT_EXECUTABLE < h2_band.nw > h2_band.out  2> debug.out
