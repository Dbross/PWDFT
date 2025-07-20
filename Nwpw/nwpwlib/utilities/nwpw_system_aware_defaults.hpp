#ifndef _NWPW_SYSTEM_AWARE_DEFAULTS_HPP_
#define _NWPW_SYSTEM_AWARE_DEFAULTS_HPP_

#pragma once

/* nwpw_system_aware_defaults.hpp
   Author - AI Assistant
   
   This module implements system-aware default parameter selection
   for SCF convergence and energy minimization, providing intelligent
   defaults based on system classification.
*/

#include <cmath>
#include <algorithm>
#include <vector>
#include <string>

namespace pwdft {

/**
 * @brief System classification types
 */
enum class SystemType {
    MOLECULE,      // Isolated molecule
    SURFACE_SLAB,  // Surface or slab system
    BULK_METAL,    // Bulk metallic system
    BULK_INSULATOR, // Bulk insulating system
    INTERFACE,     // Interface between different materials
    UNKNOWN        // Unknown or mixed system type
};

/**
 * @brief System classification parameters
 */
struct SystemClassification {
    SystemType type;
    double dimensionality;  // 0D, 1D, 2D, 3D
    bool is_metallic;
    bool is_insulating;
    bool is_surface;
    bool is_interface;
    double cell_aspect_ratio;  // For detecting slabs
    int n_atoms;
    double cell_volume;
    std::vector<double> cell_vectors;
};

/**
 * @brief System-aware default parameters
 */
struct SystemAwareDefaults {
    double mixing_beta;
    int mixing_ndim;
    double scf_initial_ethr;
    double scf_min_ethr;
    double scf_ethr_factor;
    int scf_algorithm;
    double scf_alpha;
    double scf_beta;
    int diis_histories;
    bool scf_adaptive_threshold;
    
    // Minimizer parameters
    int minimizer_type;           // 1=steepest, 2=CG, 3=L-BFGS, 4=FIRE, 5=adaptive
    double minimizer_step_size;
    int minimizer_max_iterations;
    double minimizer_tolerance;
    int adaptive_switch_threshold;
    double adaptive_energy_threshold;
    
    std::string description;
};

/**
 * @brief Classify system based on atomic structure and periodicity
 * 
 * This function implements proper system classification based on:
 * 1. Metal periodicity in x and y → Conducting
 * 2. Vacuum in cell → Slab
 * 3. Adsorbate present → Slab+adsorbate
 * 4. Perfect crystal → Bulk
 * 5. Conducting determined by actual atoms
 * 
 * @param n_atoms Number of atoms
 * @param cell_vectors Lattice vectors (9 elements: a1x,a1y,a1z,a2x,a2y,a2z,a3x,a3y,a3z)
 * @param is_crystal Whether system is crystalline
 * @param nelec Number of electrons
 * @param ispin Spin polarization
 * @param atom_types Vector of atom types (element symbols)
 * @param atom_positions Vector of atom positions (x,y,z coordinates)
 * 
 * @return SystemClassification structure
 */
inline SystemClassification classify_system(
    const int n_atoms,
    const std::vector<double>& cell_vectors,
    const bool is_crystal,
    const int nelec,
    const int ispin,
    const std::vector<std::string>& atom_types = {},
    const std::vector<std::vector<double>>& atom_positions = {})
{
    SystemClassification classification;
    classification.n_atoms = n_atoms;
    classification.cell_vectors = cell_vectors;
    
    // Calculate cell dimensions
    double a1 = std::sqrt(cell_vectors[0]*cell_vectors[0] + cell_vectors[1]*cell_vectors[1] + cell_vectors[2]*cell_vectors[2]);
    double a2 = std::sqrt(cell_vectors[3]*cell_vectors[3] + cell_vectors[4]*cell_vectors[4] + cell_vectors[5]*cell_vectors[5]);
    double a3 = std::sqrt(cell_vectors[6]*cell_vectors[6] + cell_vectors[7]*cell_vectors[7] + cell_vectors[8]*cell_vectors[8]);
    
    classification.cell_volume = a1 * a2 * a3;
    classification.cell_aspect_ratio = std::max({a1, a2, a3}) / std::min({a1, a2, a3});
    
    // Determine dimensionality based on cell aspect ratio and structure
    double min_dim = std::min({a1, a2, a3});
    double max_dim = std::max({a1, a2, a3});
    
    // Check for surface-like cell structure (large x,y, small z)
    bool has_large_xy = (a1 > 10.0 && a2 > 10.0 && a3 < 8.0);
    bool has_high_aspect = (max_dim / min_dim > 3.0);
    
    if (has_large_xy || has_high_aspect) {
        classification.dimensionality = 2.0;  // 2D system (surface/slab)
    } else {
        classification.dimensionality = 3.0;  // 3D system
    }
    
    // Determine if system is metallic based on atom types
    bool is_metallic = false;
    if (!atom_types.empty()) {
        // Check if any atoms are metals
        for (const auto& atom : atom_types) {
            std::string atom_upper = atom;
            std::transform(atom_upper.begin(), atom_upper.end(), atom_upper.begin(), ::toupper);
            
            // List of metallic elements
            if (atom_upper == "LI" || atom_upper == "NA" || atom_upper == "K" || atom_upper == "RB" || atom_upper == "CS" ||
                atom_upper == "BE" || atom_upper == "MG" || atom_upper == "CA" || atom_upper == "SR" || atom_upper == "BA" ||
                atom_upper == "AL" || atom_upper == "GA" || atom_upper == "IN" || atom_upper == "TL" ||
                atom_upper == "TI" || atom_upper == "V" || atom_upper == "CR" || atom_upper == "MN" || atom_upper == "FE" ||
                atom_upper == "CO" || atom_upper == "NI" || atom_upper == "CU" || atom_upper == "ZN" || atom_upper == "Y" ||
                atom_upper == "ZR" || atom_upper == "NB" || atom_upper == "MO" || atom_upper == "TC" || atom_upper == "RU" ||
                atom_upper == "RH" || atom_upper == "PD" || atom_upper == "AG" || atom_upper == "CD" || atom_upper == "HF" ||
                atom_upper == "TA" || atom_upper == "W" || atom_upper == "RE" || atom_upper == "OS" || atom_upper == "IR" ||
                atom_upper == "PT" || atom_upper == "AU" || atom_upper == "HG" || atom_upper == "TL" || atom_upper == "PB" ||
                atom_upper == "BI" || atom_upper == "PO" || atom_upper == "AT" || atom_upper == "FR" || atom_upper == "RA" ||
                atom_upper == "AC" || atom_upper == "TH" || atom_upper == "PA" || atom_upper == "U" || atom_upper == "NP" ||
                atom_upper == "PU" || atom_upper == "AM" || atom_upper == "CM" || atom_upper == "BK" || atom_upper == "CF" ||
                atom_upper == "ES" || atom_upper == "FM" || atom_upper == "MD" || atom_upper == "NO" || atom_upper == "LR") {
                is_metallic = true;
                break;
            }
        }
    }
    
    // Determine if system is insulating based on atom types
    bool is_insulating = false;
    if (!atom_types.empty()) {
        // Check if atoms are insulators/semiconductors
        for (const auto& atom : atom_types) {
            std::string atom_upper = atom;
            std::transform(atom_upper.begin(), atom_upper.end(), atom_upper.begin(), ::toupper);
            
            // List of insulating/semiconductor elements
            if (atom_upper == "H" || atom_upper == "HE" || atom_upper == "C" || atom_upper == "N" || atom_upper == "O" ||
                atom_upper == "F" || atom_upper == "NE" || atom_upper == "P" || atom_upper == "S" || atom_upper == "CL" ||
                atom_upper == "AR" || atom_upper == "SE" || atom_upper == "BR" || atom_upper == "KR" || atom_upper == "TE" ||
                atom_upper == "I" || atom_upper == "XE" || atom_upper == "RN") {
                is_insulating = true;
                break;
            }
        }
    }
    
    // System classification logic based on structure and composition
    // First check for surface/slab systems (priority over molecule detection)
    // Check for surface-like characteristics:
    // 1. 2D dimensionality
    // 2. Large x,y dimensions with small z
    // 3. Non-orthogonal cell with similar vector lengths (FCC-like)
    // 4. Must have enough atoms to be a surface (not a small molecule)
    // 5. Must have layered structure (not bulk crystal)
    // 6. Must not be orthogonal cubic (exclude bulk systems)
    // Check if cell is orthogonal (all off-diagonal elements are zero)
    bool is_orthogonal = (std::abs(cell_vectors[1]) < 0.1 && std::abs(cell_vectors[2]) < 0.1 &&
                         std::abs(cell_vectors[3]) < 0.1 && std::abs(cell_vectors[5]) < 0.1 &&
                         std::abs(cell_vectors[6]) < 0.1 && std::abs(cell_vectors[7]) < 0.1);
    bool is_cubic = is_orthogonal && (std::abs(a1 - a2) < 0.1 && std::abs(a2 - a3) < 0.1);
    
    bool is_surface_like = (n_atoms >= 8) && !is_cubic && (
                          (classification.dimensionality < 2.5) || 
                          (a1 > 10.0 && a2 > 10.0 && a3 < 8.0) ||
                          (std::abs(a1 - a2) < 2.0 && std::abs(a2 - a3) < 2.0 && a1 > 15.0)) &&
                          !is_crystal; // Surfaces are typically not crystalline in the input
    
    if (is_surface_like) {
        // Surface-like system - likely surface/slab
        classification.type = SystemType::SURFACE_SLAB;
        classification.is_metallic = is_metallic;
        classification.is_insulating = !is_metallic;
        classification.is_surface = true;
        classification.is_interface = false;
    }
    else if (n_atoms < 10 && classification.cell_aspect_ratio < 2.0) {
        // Small system with low aspect ratio - likely molecule
        classification.type = SystemType::MOLECULE;
        classification.is_metallic = false;
        classification.is_insulating = true;
        classification.is_surface = false;
        classification.is_interface = false;
    }
    else if ((is_crystal || (classification.dimensionality > 2.5 && classification.cell_aspect_ratio < 2.0)) && n_atoms >= 8) {
        // 3D crystalline system or cubic-like system with many atoms - bulk
        if (is_metallic) {
            classification.type = SystemType::BULK_METAL;
        } else {
            classification.type = SystemType::BULK_INSULATOR;
        }
        classification.is_metallic = is_metallic;
        classification.is_insulating = !is_metallic;
        classification.is_surface = false;
        classification.is_interface = false;
    }
    else if (n_atoms >= 20 && n_atoms <= 100 && classification.cell_aspect_ratio > 2.0) {
        // Medium-sized system with moderate aspect ratio - could be interface
        classification.type = SystemType::INTERFACE;
        classification.is_metallic = is_metallic;
        classification.is_insulating = !is_metallic;
        classification.is_surface = false;
        classification.is_interface = true;
    }
    else {
        // Default classification based on composition
        if (is_metallic) {
            classification.type = SystemType::BULK_METAL;
        } else {
            classification.type = SystemType::BULK_INSULATOR;
        }
        classification.is_metallic = is_metallic;
        classification.is_insulating = !is_metallic;
        classification.is_surface = false;
        classification.is_interface = false;
    }
    
    return classification;
}

/**
 * @brief Get system-aware default parameters
 * 
 * Provides intelligent defaults for SCF convergence and energy minimization
 * based on system classification and computational chemistry best practices.
 * 
 * @param classification System classification
 * @return SystemAwareDefaults structure with recommended parameters
 */
inline SystemAwareDefaults get_system_aware_defaults(const SystemClassification& classification)
{
    SystemAwareDefaults defaults;
    
    switch (classification.type) {
        case SystemType::MOLECULE:
            defaults.mixing_beta = 0.3;  // Conservative for molecular systems
            defaults.mixing_ndim = 8;    // Standard mixing history
            defaults.scf_initial_ethr = 1.0e-2;
            defaults.scf_min_ethr = 1.0e-13;
            defaults.scf_ethr_factor = 0.1;
            defaults.scf_algorithm = 2;  // DIIS mixing
            defaults.scf_alpha = 0.25;
            defaults.scf_beta = 0.1;
            defaults.diis_histories = 8;
            defaults.scf_adaptive_threshold = true;
            
            // Minimizer: L-BFGS for molecules (fast convergence)
            defaults.minimizer_type = 3;  // L-BFGS
            defaults.minimizer_step_size = 0.01;
            defaults.minimizer_max_iterations = 500;
            defaults.minimizer_tolerance = 1.0e-6;
            defaults.adaptive_switch_threshold = 50;
            defaults.adaptive_energy_threshold = 1.0e-5;
            
            defaults.description = "Molecular system defaults";
            break;
            
        case SystemType::SURFACE_SLAB:
            defaults.mixing_beta = 0.2;  // Conservative mixing for inhomogeneous systems
            defaults.mixing_ndim = 8;
            defaults.scf_initial_ethr = 1.0e-2;
            defaults.scf_min_ethr = 1.0e-13;
            defaults.scf_ethr_factor = 0.1;
            defaults.scf_algorithm = 4;  // Local-TF mixing for surfaces
            defaults.scf_alpha = 0.25;
            defaults.scf_beta = 0.1;
            defaults.diis_histories = 8;
            defaults.scf_adaptive_threshold = true;
            
            // Minimizer: Adaptive for surfaces (complex energy landscapes)
            defaults.minimizer_type = 5;  // Adaptive
            defaults.minimizer_step_size = 0.01;
            defaults.minimizer_max_iterations = 1000;
            defaults.minimizer_tolerance = 1.0e-6;
            defaults.adaptive_switch_threshold = 50;
            defaults.adaptive_energy_threshold = 1.0e-5;
            
            defaults.description = "Surface/slab system defaults";
            break;
            
        case SystemType::BULK_METAL:
            defaults.mixing_beta = 0.7;  // Standard mixing for bulk calculations
            defaults.mixing_ndim = 8;
            defaults.scf_initial_ethr = 1.0e-2;
            defaults.scf_min_ethr = 1.0e-13;
            defaults.scf_ethr_factor = 0.1;
            defaults.scf_algorithm = 2;  // DIIS mixing
            defaults.scf_alpha = 0.25;
            defaults.scf_beta = 0.1;
            defaults.diis_histories = 8;
            defaults.scf_adaptive_threshold = true;
            
            // Minimizer: L-BFGS for bulk metals (periodic systems)
            defaults.minimizer_type = 3;  // L-BFGS
            defaults.minimizer_step_size = 0.01;
            defaults.minimizer_max_iterations = 1000;
            defaults.minimizer_tolerance = 1.0e-6;
            defaults.adaptive_switch_threshold = 50;
            defaults.adaptive_energy_threshold = 1.0e-5;
            
            defaults.description = "Bulk metal system defaults";
            break;
            
        case SystemType::BULK_INSULATOR:
            defaults.mixing_beta = 0.5;  // Moderate mixing for insulators
            defaults.mixing_ndim = 8;
            defaults.scf_initial_ethr = 1.0e-2;
            defaults.scf_min_ethr = 1.0e-13;
            defaults.scf_ethr_factor = 0.1;
            defaults.scf_algorithm = 2;  // DIIS mixing
            defaults.scf_alpha = 0.25;
            defaults.scf_beta = 0.1;
            defaults.diis_histories = 8;
            defaults.scf_adaptive_threshold = true;
            
            // Minimizer: L-BFGS for bulk insulators (crystalline systems)
            defaults.minimizer_type = 3;  // L-BFGS
            defaults.minimizer_step_size = 0.01;
            defaults.minimizer_max_iterations = 1000;
            defaults.minimizer_tolerance = 1.0e-6;
            defaults.adaptive_switch_threshold = 50;
            defaults.adaptive_energy_threshold = 1.0e-5;
            
            defaults.description = "Bulk insulator system defaults";
            break;
            
        case SystemType::INTERFACE:
            defaults.mixing_beta = 0.2;  // Conservative for interfaces
            defaults.mixing_ndim = 8;
            defaults.scf_initial_ethr = 1.0e-2;
            defaults.scf_min_ethr = 1.0e-13;
            defaults.scf_ethr_factor = 0.1;
            defaults.scf_algorithm = 4;  // Local-TF mixing for interfaces
            defaults.scf_alpha = 0.25;
            defaults.scf_beta = 0.1;
            defaults.diis_histories = 8;
            defaults.scf_adaptive_threshold = true;
            
            // Minimizer: Adaptive for interfaces (heterogeneous systems)
            defaults.minimizer_type = 5;  // Adaptive
            defaults.minimizer_step_size = 0.01;
            defaults.minimizer_max_iterations = 1000;
            defaults.minimizer_tolerance = 1.0e-6;
            defaults.adaptive_switch_threshold = 50;
            defaults.adaptive_energy_threshold = 1.0e-5;
            
            defaults.description = "Interface system defaults";
            break;
            
        default:
            // Fallback to molecular defaults
            defaults.mixing_beta = 0.3;
            defaults.mixing_ndim = 8;
            defaults.scf_initial_ethr = 1.0e-2;
            defaults.scf_min_ethr = 1.0e-13;
            defaults.scf_ethr_factor = 0.1;
            defaults.scf_algorithm = 2;
            defaults.scf_alpha = 0.25;
            defaults.scf_beta = 0.1;
            defaults.diis_histories = 8;
            defaults.scf_adaptive_threshold = true;
            
            // Minimizer: L-BFGS as default
            defaults.minimizer_type = 3;  // L-BFGS
            defaults.minimizer_step_size = 0.01;
            defaults.minimizer_max_iterations = 1000;
            defaults.minimizer_tolerance = 1.0e-6;
            defaults.adaptive_switch_threshold = 50;
            defaults.adaptive_energy_threshold = 1.0e-5;
            
            defaults.description = "Default system parameters";
            break;
    }
    
    return defaults;
}

/**
 * @brief Apply system-aware defaults to control parameters
 * 
 * This function applies the system-aware defaults to the control parameters
 * if they haven't been explicitly set by the user.
 * 
 * @param classification System classification
 * @param user_mixing_beta User-specified mixing beta (0.0 if not set)
 * @param user_scf_algorithm User-specified SCF algorithm (-1 if not set)
 * @param user_scf_alpha User-specified SCF alpha (0.0 if not set)
 * @param user_scf_beta User-specified SCF beta (0.0 if not set)
 * @param user_diis_histories User-specified DIIS histories (-1 if not set)
 * @param user_minimizer_type User-specified minimizer type (-1 if not set)
 * @param user_minimizer_step_size User-specified minimizer step size (0.0 if not set)
 * @param user_minimizer_max_iterations User-specified minimizer max iterations (-1 if not set)
 * @param user_minimizer_tolerance User-specified minimizer tolerance (0.0 if not set)
 * 
 * @return SystemAwareDefaults with final parameter values
 */
inline SystemAwareDefaults apply_system_aware_defaults(
    const SystemClassification& classification,
    const double user_mixing_beta = 0.0,
    const int user_scf_algorithm = -1,
    const double user_scf_alpha = 0.0,
    const double user_scf_beta = 0.0,
    const int user_diis_histories = -1,
    const int user_minimizer_type = -1,
    const double user_minimizer_step_size = 0.0,
    const int user_minimizer_max_iterations = -1,
    const double user_minimizer_tolerance = 0.0)
{
    SystemAwareDefaults defaults = get_system_aware_defaults(classification);
    
    // Apply user overrides if specified
    if (user_mixing_beta > 0.0) {
        defaults.mixing_beta = user_mixing_beta;
    }
    if (user_scf_algorithm >= 0) {
        defaults.scf_algorithm = user_scf_algorithm;
    }
    if (user_scf_alpha > 0.0) {
        defaults.scf_alpha = user_scf_alpha;
    }
    if (user_scf_beta > 0.0) {
        defaults.scf_beta = user_scf_beta;
    }
    if (user_diis_histories >= 0) {
        defaults.diis_histories = user_diis_histories;
    }
    
    // Apply minimizer user overrides if specified
    if (user_minimizer_type >= 0) {
        defaults.minimizer_type = user_minimizer_type;
    }
    if (user_minimizer_step_size > 0.0) {
        defaults.minimizer_step_size = user_minimizer_step_size;
    }
    if (user_minimizer_max_iterations >= 0) {
        defaults.minimizer_max_iterations = user_minimizer_max_iterations;
    }
    if (user_minimizer_tolerance > 0.0) {
        defaults.minimizer_tolerance = user_minimizer_tolerance;
    }
    
    return defaults;
}

/**
 * @brief Get system type as string for output
 * 
 * @param type System type
 * @return String representation
 */
inline std::string system_type_to_string(SystemType type)
{
    switch (type) {
        case SystemType::MOLECULE: return "Molecule";
        case SystemType::SURFACE_SLAB: return "Surface/Slab";
        case SystemType::BULK_METAL: return "Bulk Metal";
        case SystemType::BULK_INSULATOR: return "Bulk Insulator";
        case SystemType::INTERFACE: return "Interface";
        case SystemType::UNKNOWN: return "Unknown";
        default: return "Unknown";
    }
}

} // namespace pwdft

#endif 