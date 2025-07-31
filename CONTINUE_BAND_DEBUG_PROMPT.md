# PWDFT Band Module Energy Debugging - Continue Session Prompt

## Context & Current Status

You are continuing debugging of the PWDFT Band module's γ-point H₂ test to fix wavefunction normalization and energy-scaling errors. The goal is to make Band module results match PSPW module results.

### **Recent Progress Made:**
✅ **Wavefunction normalization FIXED**: 
- Fixed expected electron count calculation in `Solid.cpp` and `cpsi.cpp`
- Removed incorrect multiplication by 2 for singlet states
- Added proper wavefunction normalization before ortho check
- **Result**: "Normalized wavefunctions: old norm=3.58027, new norm=1, scale=0.528496"
- **Result**: No more Gram-Schmidt warnings

### **Current Issue:**
The test is still not converging to a negative energy and the calculation may be hanging. The output is in `debug.log` (110MB file).

## **Repository Context**
- **PWDFT repo root**: `/Users/brossdh/src/PWDFT`
- **Current branch**: `backup_improved_SCF_20250720_162553`
- **Recent commits**: Normalization fixes committed successfully
- **Test directories**: 
  - Band H₂ test: `tests/tier1/2.4.1_H2_band/`
  - PSPW H₂ test: `tests/tier1/2.1.1_H2_energy/`

## **Objective & Success Criteria**
After your debugging, running both tests should yield:
- **Total energies** agree within 1×10⁻⁶ Ry, both ≈ –1.167455 Ry
- **Electron counts** ≈ 1.0000 (spin up/down) for both
- **Orbital energies** ≈ –10.3 eV for both

## **Next Steps to Investigate**

### **1. Check Current Test Status**
```bash
cd /Users/brossdh/src/PWDFT/tests/tier1/2.4.1_H2_band/
ls -la h2_band_norm.out debug_norm.log
tail -50 debug.log  # Check if test completed or is hanging
```

### **2. Analyze Energy Calculation Issues**
The normalization is now correct, but energy is still not negative. Investigate:
- Energy calculation in `cElectron.cpp` (Band vs PSPW differences)
- Potential energy vs kinetic energy components
- Energy scaling factors and signs

### **3. Check for Infinite Loops or Convergence Issues**
- Examine SCF convergence behavior
- Check for numerical instabilities
- Verify energy components are finite

### **4. Compare with PSPW Module**
```bash
cd /Users/brossdh/src/PWDFT/tests/tier1/2.1.1_H2_energy/
bash run.sh > h2_pspw_current.out 2> debug_pspw_current.log
```

### **5. Key Files to Examine**
- `Nwpw/band/lib/cElectron/cElectron.cpp` - Energy calculation
- `Nwpw/pspw/lib/electron/Electron.cpp` - PSPW energy calculation
- `Nwpw/band/lib/solid/Solid.cpp` - Band-specific energy components
- `debug.log` - Current test output (110MB)

## **Autonomy Policy**
- You may edit energy calculation routines in `lib/cElectron`, `lib/cpsi`, and related energy code
- Do not create new branches; commit directly with clear messages
- Use existing test scripts; do not alter them

## **Reporting Requirements**
- Show before/after energy values
- Extract electron counts and orbital energies from outputs
- Provide unified diffs of code changes
- Report convergence behavior and any remaining issues

## **Git Hygiene**
- Commit message format: `Fix energy calculation in Band γ-point H₂ test`
- Show staged diff before committing
- Use conventional commit style

## **Quality Bars**
- Energy agreement ≤ 1×10⁻⁶ Ry
- Electron count and orbital energies within 1×10⁻³ of target
- No new compilation warnings
- All existing tests pass

---

**Start by checking the current test status and analyzing the debug.log output to understand why the energy is still not converging to the expected negative value.** 