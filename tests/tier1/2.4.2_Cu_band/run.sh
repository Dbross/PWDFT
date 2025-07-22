#!/bin/bash
PWDFT_EXECUTABLE=${PWDFT_BIN:-../../../build/pwdft}
$PWDFT_EXECUTABLE < cu_band.nw > cu_band.out 