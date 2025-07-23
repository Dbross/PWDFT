#!/bin/bash
PWDFT_EXECUTABLE=${PWDFT_BIN:-../../../build/pwdft}
rm ./pspw_pbe_Cu4h.movecs
$PWDFT_EXECUTABLE < cu_band.nw > cu_band.out 2>debug.log
