#!/usr/bin/env bash
set -e

##############################
# FFT VARIABLE & ALLOCATION AUDIT
##############################

# 1. C/C++ declarations of FFT buffers
echo; echo "=== FFT Buffer Declarations (current) ==="
grep -R --exclude-dir={tests,build,Nwpw/build} -nE \
     "double\s+(wsave|tmpx|tmpy|tmpz|tmp1|tmp2|d3db::tmpz|c3db::tmpz)" Nwpw

echo; echo "=== FFT Buffer Declarations (master) ==="
git grep -nE \
    "double\s+(wsave|tmpx|tmpy|tmpz|tmp1|tmp2|d3db::tmpz|c3db::tmpz)" \
    master -- Nwpw

# 2. Dynamic allocations (malloc/new)
echo; echo "=== FFT Buffer Allocations (current) ==="
grep -R --exclude-dir={tests,build,Nwpw/build} -nE \
     "malloc\([^)]*(wsave|tmpx|tmpy|tmpz|tmp1|tmp2)" Nwpw
grep -R --exclude-dir={tests,build,Nwpw/build} -nE \
     "new\s+double\s*\[\s*[0-9]+\s*\]" Nwpw

echo; echo "=== FFT Buffer Allocations (master) ==="
git grep -nE \
    "malloc\([^)]*(wsave|tmpx|tmpy|tmpz|tmp1|tmp2)" \
    master -- Nwpw
git grep -nE \
    "new\s+double\s*\[\s*[0-9]+\s*\]" \
    master -- Nwpw

# 3. Fortran wsave declarations
echo; echo "=== Fortran wsave Declarations (current) ==="
grep -R --exclude-dir={tests,build,Nwpw/build} -n \
     "DIMENSION.*wsave" Nwpw/nwpwlib/fftpack

echo; echo "=== Fortran wsave Declarations (master) ==="
git grep -n "DIMENSION.*wsave" master -- Nwpw/nwpwlib/fftpack

# 4. FFTPACK entry-point calls
echo; echo "=== FFTPACK Calls (current) ==="
grep -R --exclude-dir={tests,build,Nwpw/build} -nE \
     "(drffti_|dcffti_|drfftf_|dcfftb_)" Nwpw

echo; echo "=== FFTPACK Calls (master) ==="
git grep -nE "(drffti_|dcffti_|drfftf_|dcfftb_)" master -- Nwpw

