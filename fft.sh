##############################
#  FFT CONTEXT EXTRACTION
##############################

# 1. Fortran workspace declarations (wsave) in FFTPACK stubs
grep -R --exclude-dir=tests --exclude-dir=build \
     -n "DIMENSION.*wsave" Nwpw/nwpwlib/fftpack/

# 2. C/C++ FFT buffer allocations (new/malloc)
grep -R --exclude-dir=tests --exclude-dir=build -En \
     "new\s+double\s*\*\s*(wsave|tmpx|tmpy|tmpz)" Nwpw
grep -R --exclude-dir=tests --exclude-dir=build -En \
     "malloc\([^)]*(wsave|tmpx|tmpy|tmpz)" Nwpw

# 3. FFTPACK entry-point calls (CURRENT branch)
grep -R --exclude-dir=tests --exclude-dir=build -nE \
     "(drffti_|dcffti_|drfftf_|dcfftb_|dcffti_|dcfftb_)" Nwpw/nwpwlib

# 4. FFTPACK entry-point calls (MASTER HEAD)
git grep -nE "(drffti_|dcffti_|drfftf_|dcfftb_|dcffti_|dcfftb_)" \
    master -- Nwpw/nwpwlib

# 5. Workspace-related diffs between master and HEAD
git diff master HEAD -- Nwpw/nwpwlib/fftpack/*.f \
    | grep -E "DIMENSION|new|malloc|wsave|tmpx|tmpy|tmpz"

# 6. Quick tabular view of all alloc/decl lines
echo; echo "=== FFT VARIABLES & SIZES (curr) ==="
grep -R --exclude-dir=tests --exclude-dir=build -En \
     "(DIMENSION.*wsave|double\s+(wsave|tmpx|tmpy|tmpz))" Nwpw \
  | sed 's/:/  |  /g'

