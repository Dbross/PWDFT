#!/bin/bash
PWDFT_EXECUTABLE=${PWDFT_BIN:-../../../build/pwdft}
rm ./pspw_pbe_Cu4h.movecs
mpirun -n 12  $PWDFT_EXECUTABLE < cu_band.nw > cu_band.out 2>debug.log
