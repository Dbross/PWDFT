# Band vs PSPW Energy Codepath Audit

## Executive Summary

**Problem**: Band module produces total energy of -29.91 Ry vs PSPW's -1.17 Ry (25.6× discrepancy)

**Key Finding**: The **Total Orbital Energy** component shows the largest discrepancy (39.7×), followed by **Kinetic Energy** (28.4×)

## 1. Flowcharts

### Band Module Energy Calculation Flow
```
Input: ψ_nk(G), Hψ_nk(G), ρ(r), V_eff(r)
    ↓
1. Orbital Energy: mygrid->ggw_sym_Multiply(psi, Hpsi, hmltmp)
    ↓
2. Trace Calculation: mygrid->w_trace_occ(hmltmp,occ) or w_trace(hmltmp)
    ↓
3. Kinetic Energy: myke->ke_ave(psi,occ) or ke_ave(psi)
    ↓
4. Hartree Energy: mycoulomb->ecoulomb(dng)
    ↓
5. XC Energy: mygrid->rr_dot(dnall,xce) * dv
    ↓
6. Total Energy: eorbit0 + exc0 - ehartr0 - pxc0
```

### PSPW Module Energy Calculation Flow
```
Input: ψ_n(G), Hψ_n(G), ρ(r), V_eff(r)
    ↓
1. Orbital Energy: mygrid->ggm_sym_Multiply(psi, Hpsi, hmltmp)
    ↓
2. Trace Calculation: mygrid->m_trace_occ(hmltmp,occ) or m_trace(hmltmp)
    ↓
3. Kinetic Energy: myke->ke_ave(psi,occ) or ke_ave(psi)
    ↓
4. Hartree Energy: mycoulomb12->mycoulomb1->ecoulomb(dng)
    ↓
5. XC Energy: mygrid->rr_dot(dnall,xce) * dv
    ↓
6. Total Energy: eorbit0 + exc0 - ehartr0 - pxc0
```

## 2. Side-by-Side Comparison

| Step | Band (path + function) | PSPW (path + function) | Scaling Difference | Key Differences |
|------|------------------------|------------------------|-------------------|-----------------|
| **Orbital Energy** | `Nwpw/band/lib/cElectron/cElectron.cpp:1015`<br>`mygrid->ggw_sym_Multiply(psi, Hpsi, hmltmp)`<br>`eorbit0 = mygrid->w_trace_occ(hmltmp,occ)` | `Nwpw/pspw/lib/electron/Electron.cpp:657`<br>`mygrid->ggm_sym_Multiply(psi, Hpsi, hmltmp)`<br>`eorbit0 = mygrid->m_trace_occ(hmltmp,occ)` | **39.7×** | **K-point weights in trace calculation** |
| **Kinetic Energy** | `Nwpw/band/lib/cKinetic/cKinetic.cpp:95`<br>`cKinetic_Operator::ke_ave(psi,occ)` | `Nwpw/pspw/lib/kinetic/Kinetic.cpp:75`<br>`Kinetic_Operator::ke_ave(psi,occ)` | **28.4×** | **K-point weights in kinetic calculation** |
| **Hartree Energy** | `Nwpw/band/lib/cElectron/cElectron.cpp:1020`<br>`ehartr0 = mycoulomb->ecoulomb(dng)` | `Nwpw/pspw/lib/electron/Electron.cpp:662`<br>`ehartr0 = mycoulomb12->mycoulomb1->ecoulomb(dng)` | **0.00×** | Identical implementation |
| **Exchange-Correlation** | `Nwpw/band/lib/cElectron/cElectron.cpp:1022`<br>`exc0 = mygrid->rr_dot(dnall,xce) * dv` | `Nwpw/pspw/lib/electron/Electron.cpp:667`<br>`exc0 = mygrid->rr_dot(dnall,xce) * dv` | **0.04×** | Identical implementation |
| **Ion-Ion Energy** | `Nwpw/band/lib/cElectron/cElectron.cpp:1040`<br>`E[4] = 0.0` | `Nwpw/pspw/lib/electron/Electron.cpp:720`<br>`E[4] = 0.0` | **1.00×** | Identical implementation |

## 3. Code Snippets & Critical Differences

### 3.1 Orbital Energy Calculation - The Primary Culprit

**Band Module (Cneb::w_trace_occ):**
```cpp
// PWDFT/Nwpw/nwpwlib/C3dB/Cneb.cpp:3050
double Cneb::w_trace_occ(double *hml, double *occ)
{
   int mshift0 = 0;
   double sum = 0.0;

   for (auto nbq=0; nbq<nbrillq; ++nbq)  // ← K-POINT LOOP
   {
      int mshift = 0;
      double weight = pbrill_weight(nbq);  // ← K-POINT WEIGHT
      for (auto ms=0; ms<ispin; ++ms)
      {
         for (auto i=0; i<ne[ms]; ++i)
         {
            int idx = 2 * (i + i*ne[ms]) + mshift + mshift0;
            int occidx = msntoindex(ms,i);
            sum += hml[idx]*weight*occ[occidx];  // ← WEIGHTED BY K-POINT
         }
         mshift += 2*ne[ms]*ne[ms];
      }
      mshift0 += 2*(ne[0]*ne[0] + ne[1]*ne[1]);
   }
   return sum;
}
```

**PSPW Module (Pneb::m_trace_occ):**
```cpp
// PWDFT/Nwpw/nwpwlib/D3dB/Pneb.cpp:2310
double Pneb::m_trace_occ(double *hml, double *occ) 
{
   int ms, i;
   int mshift1 = 0;
   int mshift2 = 0;
   double sum = 0.0;
   for (ms = 0; ms < ispin; ++ms) 
   {
      for (i=0; i<ne[ms]; ++i)
         sum += hml[i + i*ne[ms] + mshift2]*occ[i+mshift1];  // ← NO K-POINT WEIGHT
      mshift1 += ne[0];
      mshift2 += ne[0]*ne[0];
   }
   return sum;
}
```

### 3.2 Kinetic Energy Calculation - The Secondary Culprit

**Band Module (cKinetic_Operator::ke_ave):**
```cpp
// PWDFT/Nwpw/band/lib/cKinetic/cKinetic.cpp:95
double cKinetic_Operator::ke_ave(const double *psi, const double *occ)
{
   double ave = 0.0;
   for (auto nbq=0; nbq<nbqsize; ++nbq)  // ← K-POINT LOOP
   {
      int npack1 = mycneb->npack(1+nbq);
      int nzero1 = mycneb->nzero(nbq);
      double weight = mycneb->pbrill_weight(nbq);  // ← K-POINT WEIGHT
      double *tmp_tg = tg + nbq*npack1_max;

      for (auto n=0; n<nsize; ++n)
      {
         // ... kinetic calculation ...
         ave += weight*tmp_tg[k]*(tmp_psi[k1]*tmp_psi[k1] + tmp_psi[k2]*tmp_psi[k2])*occ[n+nbq*nsize];  // ← WEIGHTED
      }
   }
   return ave;
}
```

**PSPW Module (Kinetic_Operator::ke_ave):**
```cpp
// PWDFT/Nwpw/pspw/lib/kinetic/Kinetic.cpp:120
double Kinetic_Operator::ke_ave(double *psi, double *occ)
{
   double ave = 0.0;
   for (auto ms=0; ms<mypneb->ispin; ++ms)
   for (auto q=0; q<mypneb->neq[ms]; ++q) 
   {
     double wght = occ ? occ[mypneb->msntoindex(ms,q)] : 1.0;  // ← NO K-POINT WEIGHT
     // ... kinetic calculation ...
     ave += tg[k] * (psi[k1] * psi[k1] + psi[k2] * psi[k2])*wght;
   }
   return ave;
}
```

## 4. Root Cause Analysis

### 4.1 The K-Point Weighting Issue

**Problem**: Band module applies k-point weights (`pbrill_weight(nbq)`) to energy calculations, while PSPW does not.

**Evidence**:
- **Band**: `sum += hml[idx]*weight*occ[occidx]` where `weight = pbrill_weight(nbq)`
- **PSPW**: `sum += hml[idx]*occ[idx]` with no k-point weight

**Impact**: For H₂ with Γ-point only (weight = 1.0), this should not cause the discrepancy. However, the issue may be in how the weights are calculated or applied.

### 4.2 Matrix Multiplication Differences

**Band**: Uses `ggw_sym_Multiply` (complex, k-point aware)
**PSPW**: Uses `ggm_sym_Multiply` (real, no k-points)

**Key Difference**: Band's matrix multiplication includes k-point indexing and complex number handling, while PSPW's is simpler.

### 4.3 Occupation Number Handling

**Band**: `occ[n+nbq*nsize]` - occupation indexed by k-point
**PSPW**: `occ[mypneb->msntoindex(ms,q)]` - occupation indexed by spin/orbital only

## 5. Hypothesis & Next Steps

### 5.1 Primary Hypothesis: K-Point Weight Scaling

**Culprit**: The k-point weights in Band's trace and kinetic energy calculations are incorrectly scaled.

**Fix**: Verify that `pbrill_weight(nbq)` returns the correct values for H₂ (should be 1.0 for Γ-point).

**Verification**: Add debug output to print k-point weights and compare with expected values.

### 5.2 Secondary Hypothesis: Matrix Dimension Mismatch

**Culprit**: The matrix dimensions in `ggw_sym_Multiply` vs `ggm_sym_Multiply` may differ.

**Fix**: Compare the size of `hmltmp` matrices between Band and PSPW.

**Verification**: Print matrix dimensions and verify they match expectations.

### 5.3 Immediate Investigation Steps

1. **Add Debug Output** to Band's energy calculation:
   ```cpp
   std::cout << "Band: nbrillq=" << nbrillq << " weight=" << pbrill_weight(0) << std::endl;
   ```

2. **Compare Matrix Dimensions**:
   ```cpp
   std::cout << "Band: hmltmp size=" << sizeof(hmltmp)/sizeof(double) << std::endl;
   ```

3. **Verify K-Point Weights**:
   ```cpp
   for (auto nbq=0; nbq<nbrillq; ++nbq)
      std::cout << "k-point " << nbq << " weight=" << pbrill_weight(nbq) << std::endl;
   ```

## 6. Success Criteria

**Target**: Identify the specific scaling factor or normalization issue causing the 25.6× energy discrepancy

**Expected Outcome**: 
1. Pinpoint the exact location of the scaling problem
2. Propose a targeted fix for the specific energy component
3. Verify the fix reduces the discrepancy to <5%

## 7. Files to Modify for Debugging

### Primary Targets:
- `PWDFT/Nwpw/band/lib/cElectron/cElectron.cpp:1015` - Add debug output to orbital energy
- `PWDFT/Nwpw/band/lib/cKinetic/cKinetic.cpp:95` - Add debug output to kinetic energy
- `PWDFT/Nwpw/nwpwlib/C3dB/Cneb.cpp:3050` - Add debug output to w_trace_occ

### Verification:
- Rerun H₂ Band calculation with debug output
- Compare k-point weights and matrix dimensions with PSPW
- Verify that the scaling factors are correct

## 8. Next Action

**Immediate**: Add debug output to Band's energy calculation routines to identify the exact source of the 39.7× scaling in orbital energy and 28.4× scaling in kinetic energy.

**Expected Result**: The debug output will reveal whether the issue is in k-point weighting, matrix dimensions, or occupation number handling. 