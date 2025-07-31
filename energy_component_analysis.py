#!/usr/bin/env python3
"""
Energy Component Analysis Script
Compares individual energy components between Band and PSPW modules for H2
"""

import json
import numpy as np

# Energy conversion factors
RY_TO_EV = 13.6057  # 1 Ry = 13.6057 eV

def load_energies(json_file):
    """Load energy components from JSON file"""
    with open(json_file, 'r') as f:
        data = json.load(f)
    
    # Extract energies array
    if 'band' in data:
        energies = data['band']['energies']
        module = 'Band'
    elif 'pspw' in data:
        energies = data['pspw']['energies']
        module = 'PSPW'
    else:
        raise ValueError("No band or pspw energies found in JSON")
    
    return energies, module

def extract_energy_components(band_energies, pspw_energies):
    """Extract individual energy components from energy arrays"""
    
    # Based on the output format, the energy components are in this order:
    # [0] = total energy
    # [1] = total orbital energy  
    # [2] = hartree energy
    # [3] = exc-corr energy
    # [4] = ion-ion energy
    # [5] = kinetic (planewave)
    # [6] = V_local (planewave)
    # [7] = V_nl (planewave)
    # [8] = V_Coul (planewave)
    # [9] = V_xc (planewave)
    
    components = {
        'Total Energy': (band_energies[0], pspw_energies[0]),
        'Total Orbital Energy': (band_energies[1], pspw_energies[1]),
        'Hartree Energy': (band_energies[2], pspw_energies[2]),
        'Exchange-Correlation': (band_energies[3], pspw_energies[3]),
        'Ion-Ion Energy': (band_energies[4], pspw_energies[4]),
        'Kinetic Energy': (band_energies[5], pspw_energies[5]),
        'V_local': (band_energies[6], pspw_energies[6]),
        'V_nl': (band_energies[7], pspw_energies[7]),
        'V_Coul': (band_energies[8], pspw_energies[8]),
        'V_xc': (band_energies[9], pspw_energies[9])
    }
    
    return components

def create_comparison_table(components):
    """Create a formatted comparison table"""
    
    print("Energy Component Analysis: Band vs PSPW for H₂")
    print("=" * 80)
    print(f"{'Component':<20} {'Band (Ry)':<12} {'Band (eV)':<12} {'PSPW (Ry)':<12} {'PSPW (eV)':<12} {'Ratio':<10}")
    print("-" * 80)
    
    ratios = {}
    
    for component, (band_ry, pspw_ry) in components.items():
        band_ev = band_ry * RY_TO_EV
        pspw_ev = pspw_ry * RY_TO_EV
        
        if abs(pspw_ry) > 1e-10:  # Avoid division by zero
            ratio = abs(band_ry / pspw_ry)
        else:
            ratio = float('inf') if abs(band_ry) > 1e-10 else 1.0
            
        ratios[component] = ratio
        
        print(f"{component:<20} {band_ry:<12.6f} {band_ev:<12.6f} {pspw_ry:<12.6f} {pspw_ev:<12.6f} {ratio:<10.2f}")
    
    print("-" * 80)
    
    # Find the component with the largest ratio
    max_ratio_component = max(ratios.items(), key=lambda x: x[1] if x[1] != float('inf') else 0)
    print(f"\n🔍 **LARGEST DISCREPANCY**: {max_ratio_component[0]} (Ratio: {max_ratio_component[1]:.2f})")
    
    return ratios

def analyze_discrepancies(components):
    """Analyze which components show significant discrepancies"""
    
    print("\n" + "=" * 60)
    print("DISCREPANCY ANALYSIS")
    print("=" * 60)
    
    significant_components = []
    
    for component, (band_ry, pspw_ry) in components.items():
        if abs(pspw_ry) > 1e-10:
            ratio = abs(band_ry / pspw_ry)
            if ratio > 1.1:  # More than 10% difference
                significant_components.append((component, ratio, band_ry, pspw_ry))
    
    # Sort by ratio (largest first)
    significant_components.sort(key=lambda x: x[1], reverse=True)
    
    print("Components with >10% difference (sorted by ratio):")
    print(f"{'Component':<20} {'Ratio':<10} {'Band (Ry)':<15} {'PSPW (Ry)':<15}")
    print("-" * 60)
    
    for component, ratio, band_ry, pspw_ry in significant_components:
        print(f"{component:<20} {ratio:<10.2f} {band_ry:<15.6f} {pspw_ry:<15.6f}")
    
    return significant_components

def main():
    """Main analysis function"""
    
    # Load energy data
    try:
        band_energies, _ = load_energies('tests/tier1/2.4.1_H2_band/h2-energy.json')
        pspw_energies, _ = load_energies('tests/tier1/2.1.1_H2_energy/h2-energy.json')
        
        print(f"✅ Loaded Band energies: {len(band_energies)} components")
        print(f"✅ Loaded PSPW energies: {len(pspw_energies)} components")
        
    except Exception as e:
        print(f"❌ Error loading energy data: {e}")
        return
    
    # Extract components
    components = extract_energy_components(band_energies, pspw_energies)
    
    # Create comparison table
    ratios = create_comparison_table(components)
    
    # Analyze discrepancies
    significant_components = analyze_discrepancies(components)
    
    # Summary
    print("\n" + "=" * 60)
    print("SUMMARY")
    print("=" * 60)
    print(f"• Total Energy: Band = {components['Total Energy'][0]:.6f} Ry, PSPW = {components['Total Energy'][1]:.6f} Ry")
    print(f"• Energy Ratio: {abs(components['Total Energy'][0] / components['Total Energy'][1]):.2f}x")
    print(f"• Components with >10% difference: {len(significant_components)}")
    
    if significant_components:
        print(f"• Primary culprit: {significant_components[0][0]} ({significant_components[0][1]:.2f}x)")
    
    print("\n" + "=" * 60)
    print("NEXT STEPS")
    print("=" * 60)
    print("1. Focus debugging on the component with the largest ratio")
    print("2. Check for volume normalization issues in that component")
    print("3. Compare wavefunction normalization between Band and PSPW")
    print("4. Verify density normalization in energy calculations")

if __name__ == "__main__":
    main() 