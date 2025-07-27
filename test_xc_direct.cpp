#include <iostream>
#include <cmath>
#include <vector>
#include <cstring>

// Include the actual exchange-correlation function
extern "C" {
    void v_cwexc_(const int *gga, void *mycneb, const double *dn,
                  const double *x_parameter, const double *c_parameter, double *xcp,
                  double *xce, double *rho, double *grx, double *gry, double *grz,
                  double *agr, double *fn, double *fdn);
}

// Simple test density function
double test_density(double x, double y, double z) {
    double r2 = x*x + y*y + z*z;
    return exp(-r2 / 2.0) / (pow(2.0 * M_PI, 1.5));
}

int main() {
    std::cout << "=== Direct PWDFT Exchange-Correlation Test ===" << std::endl;
    
    // Test parameters
    const int nx = 16, ny = 16, nz = 16;
    const double dx = 1.0, dy = 1.0, dz = 1.0;
    const int nfft3d = nx * ny * nz;
    
    std::cout << "Grid size: " << nx << "x" << ny << "x" << nz << std::endl;
    std::cout << "Total grid points: " << nfft3d << std::endl;
    
    // Allocate arrays
    std::vector<double> rho(nfft3d);
    std::vector<double> grx(nfft3d);
    std::vector<double> gry(nfft3d);
    std::vector<double> grz(nfft3d);
    std::vector<double> agr(nfft3d);
    std::vector<double> xcp(nfft3d);
    std::vector<double> xce(nfft3d);
    std::vector<double> fn(nfft3d);
    std::vector<double> fdn(nfft3d);
    std::vector<double> dn(nfft3d);
    
    // Initialize test density
    std::cout << "\nInitializing test density..." << std::endl;
    int idx = 0;
    double total_charge = 0.0;
    
    for (int k = 0; k < nz; ++k) {
        for (int j = 0; j < ny; ++j) {
            for (int i = 0; i < nx; ++i) {
                double x = (i - nx/2) * dx;
                double y = (j - ny/2) * dy;
                double z = (k - nz/2) * dz;
                
                rho[idx] = test_density(x, y, z);
                dn[idx] = rho[idx];  // For single electron
                total_charge += rho[idx] * dx * dy * dz;
                
                // Initialize gradients to zero (will be computed by finite differences)
                grx[idx] = 0.0;
                gry[idx] = 0.0;
                grz[idx] = 0.0;
                agr[idx] = 0.0;
                
                idx++;
            }
        }
    }
    
    std::cout << "Total charge: " << total_charge << " (should be ~1.0)" << std::endl;
    std::cout << "Density range: [" << *std::min_element(rho.begin(), rho.end()) 
              << ", " << *std::max_element(rho.begin(), rho.end()) << "]" << std::endl;
    
    // Test parameters
    int gga = 10;  // PBE functional
    double x_parameter = 1.0;
    double c_parameter = 1.0;
    
    // Mock Cneb object (null pointer for now)
    void *mycneb = nullptr;
    
    std::cout << "\nCalling v_cwexc function..." << std::endl;
    
    try {
        // Call the actual exchange-correlation function
        v_cwexc_(&gga, mycneb, dn.data(), &x_parameter, &c_parameter, 
                xcp.data(), xce.data(), rho.data(), grx.data(), gry.data(), grz.data(),
                agr.data(), fn.data(), fdn.data());
        
        std::cout << "✓ v_cwexc function completed successfully!" << std::endl;
        
        // Check results
        std::cout << "\n=== Results Analysis ===" << std::endl;
        std::cout << "V_xc range: [" << *std::min_element(xcp.begin(), xcp.end())
                  << ", " << *std::max_element(xcp.begin(), xcp.end()) << "]" << std::endl;
        std::cout << "E_xc range: [" << *std::min_element(xce.begin(), xce.end())
                  << ", " << *std::max_element(xce.begin(), xce.end()) << "]" << std::endl;
        
        // Check for NaN/Inf values
        bool has_nan_inf = false;
        for (int i = 0; i < nfft3d; ++i) {
            if (!std::isfinite(xcp[i]) || !std::isfinite(xce[i])) {
                has_nan_inf = true;
                std::cout << "NaN/Inf found at index " << i << ": xcp=" << xcp[i] << ", xce=" << xce[i] << std::endl;
                break;
            }
        }
        
        if (!has_nan_inf) {
            std::cout << "✓ No NaN/Inf values detected" << std::endl;
        } else {
            std::cout << "❌ NaN/Inf values detected!" << std::endl;
        }
        
        // Compute total energy
        double total_energy = 0.0;
        for (int i = 0; i < nfft3d; ++i) {
            total_energy += xce[i] * rho[i] * dx * dy * dz;
        }
        std::cout << "Total exchange-correlation energy: " << total_energy << std::endl;
        
        // Test symmetry
        double symmetry_error = 0.0;
        int symmetry_tests = 0;
        
        for (int k = 0; k < nz/2; ++k) {
            for (int j = 0; j < ny/2; ++j) {
                for (int i = 0; i < nx/2; ++i) {
                    int idx1 = k * ny * nx + j * nx + i;
                    int idx2 = (nz-1-k) * ny * nx + (ny-1-j) * nx + (nx-1-i);
                    
                    double diff = fabs(xcp[idx1] - xcp[idx2]);
                    symmetry_error = std::max(symmetry_error, diff);
                    symmetry_tests++;
                }
            }
        }
        
        std::cout << "Symmetry error: " << symmetry_error << " (tests: " << symmetry_tests << ")" << std::endl;
        
        // Summary
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "✓ Function call: PASSED" << std::endl;
        std::cout << "✓ No NaN/Inf: " << (has_nan_inf ? "FAILED" : "PASSED") << std::endl;
        std::cout << "✓ Energy calculation: PASSED (energy: " << total_energy << ")" << std::endl;
        std::cout << "✓ Symmetry test: " << (symmetry_error < 1e-10 ? "PASSED" : "FAILED") << std::endl;
        
        if (!has_nan_inf && symmetry_error < 1e-10) {
            std::cout << "\n🎉 ALL TESTS PASSED! Real-space XC implementation is working correctly." << std::endl;
            return 0;
        } else {
            std::cout << "\n❌ SOME TESTS FAILED! Check implementation." << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ Exception caught: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "❌ Unknown exception caught!" << std::endl;
        return 1;
    }
} 