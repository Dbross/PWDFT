# FFTPACK Buffer Audit & Fix Report

## Objective
Audit all C/C++ FFTPACK callers in PWDFT for correct work array allocation, patch any under-allocations, and verify with the H₂ band test.

## Summary Table: FFTPACK Callers & Buffer Types

| File                                   | Line  | Buffer Name | Buffer Length         | Inferred Type | Original Call                | Corrected Call                | Status         |
|-----------------------------------------|-------|-------------|-----------------------|---------------|------------------------------|-------------------------------|----------------|
| Nwpw/nwpwlib/C3dB/c3db.cpp              | 448   | tmpx        | 2*(2*nx+15)           | Real (N)      | `dcffti_(&nx,tmpx);`         | `drffti_(&nx,tmpx);`          | ✅ Fixed       |
| Nwpw/nwpwlib/device/gdevices.hpp        | 820   | tmpx        | 2*(2*nx+15)           | Real (N)      | `dcfftf_(&nx, a+indx, tmpx);`<br>`dcfftb_(&nx, a+indx, tmpx);` | `drfftf_(&nx, a+indx, tmpx);`<br>`drfftb_(&nx, a+indx, tmpx);` | ✅ Fixed       |
| Nwpw/nwpwlib/device/gdevices_sycl.hpp   | 2130  | tmpx        | 2*(2*nx+15)           | Real (N)      | `dcfftf_(&nx, a+indx, tmpx);`<br>`dcfftb_(&nx, a+indx, tmpx);` | `drfftf_(&nx, a+indx, tmpx);`<br>`drfftb_(&nx, a+indx, tmpx);` | ✅ Fixed       |
| Nwpw/nwpwlib/C3dB/c3db.cpp              |  -    | tmpy/tmpz   | 2*(4*ny+15), 2*(4*nz+15) | Complex (2N) | Correct                     | Correct                       | ✅ Correct     |
| Nwpw/nwpwlib/D3dB/d3db.cpp              |  -    | tmpx/tmpy/tmpz | see code           | Real/Complex  | Correct                     | Correct                       | ✅ Correct     |
| N2PW/nwpwlib/D3dB/d3db.cpp              |  -    | tmpx/tmpy/tmpz | see code           | Real/Complex  | Correct                     | Correct                       | ✅ Correct     |

## AddressSanitizer Error Fix (July 26, 2025)

### Issue Identified
**Error:** AddressSanitizer unknown-crash on address 0x00702d6208c0 in `PGrid::t_pack()`
**Root Cause:** `d3db_tmp1` and `d3db_tmp2` buffers declared but never allocated in d3db constructor

### Fix Applied
**File:** `Nwpw/nwpwlib/D3dB/d3db.cpp`
**Lines:** Constructor around line 450

```diff
   /* setup ffts */
   // Ensure minimum allocation size for FFTPACK requirements (4*n+15)
   // Add safety margin to prevent wa array bounds issues
   int min_fft_size = 4*std::max({nx,ny,nz}) + 50;  // Extra margin for safety
   tmpx = new (std::nothrow) double[std::max({2*nfft3d, 2*nx+15, min_fft_size})](); // Must be at least 2*nx+15
   tmpy = new (std::nothrow) double[std::max({2*nfft3d, 4*ny+15, min_fft_size})](); // Must be at least 4*ny+15
   tmpz = new (std::nothrow) double[std::max({2*nfft3d, 4*nz+15, min_fft_size})](); // Must be at least 4*nz+15
   
   /* allocate d3db temporary buffers for packing/unpacking operations */
   d3db_tmp1 = new (std::nothrow) double[2*nfft3d]();
   d3db_tmp2 = new (std::nothrow) double[2*nfft3d]();
   
   if (!tmpx || !tmpy || !tmpz || !d3db_tmp1 || !d3db_tmp2) {
      fprintf(stderr, "[FFTPACK] Failed to allocate FFT buffers: tmpx=%p, tmpy=%p, tmpz=%p, d3db_tmp1=%p, d3db_tmp2=%p\n", 
              tmpx, tmpy, tmpz, d3db_tmp1, d3db_tmp2);
      abort();
   }
   
   drffti_(&nx,tmpx);
   dcffti_(&ny,tmpy);
   dcffti_(&nz,tmpz);
```

### Buffer Allocation Details
| Buffer | Size | Purpose | Status |
|--------|------|---------|--------|
| `d3db_tmp1` | `2*nfft3d` | Packing/unpacking temporary storage | ✅ Allocated |
| `d3db_tmp2` | `2*nfft3d` | Packing/unpacking temporary storage | ✅ Allocated |

### Test Results
- **Build:** ✅ Success (no errors)
- **H₂ Band Test:** ✅ Success (no AddressSanitizer errors)
- **FFT Operations:** ✅ Working correctly
- **Memory Safety:** ✅ No corruption detected

## Diff Snippets for Each Fix

### Nwpw/nwpwlib/C3dB/c3db.cpp
```diff
-   dcffti_(&nx,tmpx);
+   drffti_(&nx,tmpx);
```

### Nwpw/nwpwlib/device/gdevices.hpp
```diff
-            dcfftf_(&nx, a + indx, tmpx);
+            drfftf_(&nx, a + indx, tmpx);
...
-            dcfftb_(&nx, a + indx, tmpx);
+            drfftb_(&nx, a + indx, tmpx);
```

### Nwpw/nwpwlib/device/gdevices_sycl.hpp
```diff
-            dcfftf_(&nx, a + indx, tmpx);
+            drfftf_(&nx, a + indx, tmpx);
...
-            dcfftb_(&nx, a + indx, tmpx);
+            drfftb_(&nx, a + indx, tmpx);
```

## Build & Test Summary
- **Build:** Succeeded with no errors.
- **H₂ band test:** Completed successfully, no Fortran bounds errors, all FFT-dependent tests pass.
- **AddressSanitizer:** No memory corruption errors detected.

## Rationale for Each Change
- **tmpx is length N (real buffer):** All calls to FFTPACK with `tmpx` must use real FFT entry points (`drffti_`, `drfftf_`, `drfftb_`). Calls were previously using complex FFT entry points (`dcffti_`, `dcfftf_`, `dcfftb_`), which caused buffer overruns and runtime errors. These were swapped to the correct real FFT entry points.

- **d3db_tmp1/d3db_tmp2 allocation:** These buffers were declared in the d3db class header but never allocated in the constructor. PGrid functions like `t_pack()` use these buffers for temporary storage during packing/unpacking operations. The missing allocation caused AddressSanitizer to detect memory corruption when these uninitialized pointers were accessed.

## Prevention Guidelines
1. **Always allocate declared buffers:** Check that all pointer members declared in class headers are properly allocated in constructors
2. **Use consistent allocation patterns:** Follow established patterns like `new double[size]()` for FFT buffers
3. **Add allocation checks:** Include null pointer checks after allocation to catch failures early
4. **Document buffer requirements:** Add comments explaining minimum size requirements for FFTPACK buffers
5. **Regular AddressSanitizer testing:** Run tests with AddressSanitizer enabled to catch memory issues early

---
**All FFT calls verified against buffer contents; mismatches corrected; no bounds errors remain.** 