#ifndef _NWPW_SYSTEM_AWARE_DEFAULTS_HPP_
#define _NWPW_SYSTEM_AWARE_DEFAULTS_HPP_

#pragma once

/* nwpw_system_aware_defaults.hpp
   Author - AI Assistant (based on Quantum Espresso implementation)
   
   This module implements system-aware default parameter selection
   for SCF convergence, similar to Quantum Espresso's intelligent defaults.
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
    std::string description;
};

/**
 * @brief Classify system based on geometry and cell parameters
 * 
 * @param n_atoms Number of atoms
 * @param cell_vectors Lattice vectors (9 elements: a1x,a1y,a1z,a2x,a2y,a2z,a3x,a3y,a3z)
 * @param is_crystal Whether system is crystalline
 * @param nelec Number of electrons
 * @param ispin Spin polarization
 * 
 * @return SystemClassification structure
 */
inline SystemClassification classify_system(
    const int n_atoms,
    const std::vector<double>& cell_vectors,
    const bool is_crystal,
    const int nelec,
    const int ispin)
{
    SystemClassification classification;
    classification.n_atoms = n_atoms;
    classification.cell_vectors = cell_vectors;
    
    // Calculate cell volume and aspect ratios
    double a1 = std::sqrt(cell_vectors[0]*cell_vectors[0] + cell_vectors[1]*cell_vectors[1] + cell_vectors[2]*cell_vectors[2]);
    double a2 = std::sqrt(cell_vectors[3]*cell_vectors[3] + cell_vectors[4]*cell_vectors[4] + cell_vectors[5]*cell_vectors[5]);
    double a3 = std::sqrt(cell_vectors[6]*cell_vectors[6] + cell_vectors[7]*cell_vectors[7] + cell_vectors[8]*cell_vectors[8]);
    
    classification.cell_volume = a1 * a2 * a3;
    classification.cell_aspect_ratio = std::max({a1, a2, a3}) / std::min({a1, a2, a3});
    
    // Determine dimensionality
    double min_dim = std::min({a1, a2, a3});
    double max_dim = std::max({a1, a2, a3});
    
    if (max_dim / min_dim > 10.0) {
        // Very anisotropic - likely 1D or 2D
        if (std::min({a1, a2, a3}) < 5.0) {
            classification.dimensionality = 1.0;  // 1D system
        } else {
            classification.dimensionality = 2.0;  // 2D system
        }
    } else {
        classification.dimensionality = 3.0;  // 3D system
    }
    
    // Classify system type
    if (!is_crystal || n_atoms < 10) {
        // Small system or non-crystalline - likely molecule
        classification.type = SystemType::MOLECULE;
        classification.is_metallic = false;
        classification.is_insulating = true;
        classification.is_surface = false;
        classification.is_interface = false;
    }
    else if (classification.dimensionality < 2.5 && classification.cell_aspect_ratio > 5.0) {
        // Slab or surface system
        classification.type = SystemType::SURFACE_SLAB;
        classification.is_metallic = false;  // Default assumption
        classification.is_insulating = true;
        classification.is_surface = true;
        classification.is_interface = false;
    }
    else if (classification.cell_volume > 1000.0 && n_atoms > 50) {
        // Large bulk system
        classification.type = SystemType::BULK_METAL;  // Default assumption
        classification.is_metallic = true;
        classification.is_insulating = false;
        classification.is_surface = false;
        classification.is_interface = false;
    }
    else {
        // Default to bulk insulator
        classification.type = SystemType::BULK_INSULATOR;
        classification.is_metallic = false;
        classification.is_insulating = true;
        classification.is_surface = false;
        classification.is_interface = false;
    }
    
    return classification;
}

/**
 * @brief Get system-aware default parameters
 * 
 * Based on Quantum Espresso's intelligent defaults and the roadmap specifications.
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
            defaults.mixing_ndim = 8;    // QE default mixing history
            defaults.scf_initial_ethr = 1.0e-2;
            defaults.scf_min_ethr = 1.0e-13;
            defaults.scf_ethr_factor = 0.1;
            defaults.scf_algorithm = 2;  // DIIS mixing
            defaults.scf_alpha = 0.25;
            defaults.scf_beta = 0.1;
            defaults.diis_histories = 8;
            defaults.scf_adaptive_threshold = true;
            defaults.description = "Molecular system defaults";
            break;
            
        case SystemType::SURFACE_SLAB:
            defaults.mixing_beta = 0.2;  // QE default for inhomogeneous systems
            defaults.mixing_ndim = 8;
            defaults.scf_initial_ethr = 1.0e-2;
            defaults.scf_min_ethr = 1.0e-13;
            defaults.scf_ethr_factor = 0.1;
            defaults.scf_algorithm = 4;  // Local-TF mixing for surfaces
            defaults.scf_alpha = 0.25;
            defaults.scf_beta = 0.1;
            defaults.diis_histories = 8;
            defaults.scf_adaptive_threshold = true;
            defaults.description = "Surface/slab system defaults";
            break;
            
        case SystemType::BULK_METAL:
            defaults.mixing_beta = 0.7;  // QE default for standard calculations
            defaults.mixing_ndim = 8;
            defaults.scf_initial_ethr = 1.0e-2;
            defaults.scf_min_ethr = 1.0e-13;
            defaults.scf_ethr_factor = 0.1;
            defaults.scf_algorithm = 2;  // DIIS mixing
            defaults.scf_alpha = 0.25;
            defaults.scf_beta = 0.1;
            defaults.diis_histories = 8;
            defaults.scf_adaptive_threshold = true;
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
 * 
 * @return SystemAwareDefaults with final parameter values
 */
inline SystemAwareDefaults apply_system_aware_defaults(
    const SystemClassification& classification,
    const double user_mixing_beta = 0.0,
    const int user_scf_algorithm = -1,
    const double user_scf_alpha = 0.0,
    const double user_scf_beta = 0.0,
    const int user_diis_histories = -1)
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