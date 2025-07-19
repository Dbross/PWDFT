#include <iostream>
#include <cmath>
#include "Nwpw/nwpwlib/utilities/nwpw_scf_adaptive_threshold.hpp"

int main() {
    std::cout << "Testing adaptive threshold function..." << std::endl;
    
    // Test parameters
    bool adaptive_enabled = true;
    int iteration = 5;
    double dr2 = 1.0e-4;  // Small density residual
    int nelec = 2;
    double current_ethr = 1.0e-2;
    double initial_ethr = 1.0e-2;
    double min_ethr = 1.0e-13;
    double ethr_factor = 0.1;
    
    // Test the function
    double new_ethr = pwdft::get_scf_diagonalization_threshold(
        adaptive_enabled, iteration, dr2, nelec,
        current_ethr, initial_ethr, min_ethr, ethr_factor);
    
    std::cout << "Input parameters:" << std::endl;
    std::cout << "  iteration: " << iteration << std::endl;
    std::cout << "  dr2: " << dr2 << std::endl;
    std::cout << "  nelec: " << nelec << std::endl;
    std::cout << "  current_ethr: " << current_ethr << std::endl;
    std::cout << "  new_ethr: " << new_ethr << std::endl;
    
    if (new_ethr < current_ethr) {
        std::cout << "SUCCESS: Adaptive threshold reduced from " << current_ethr << " to " << new_ethr << std::endl;
    } else {
        std::cout << "INFO: Adaptive threshold unchanged or increased" << std::endl;
    }
    
    return 0;
} 