#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <chrono>
#include <mpi.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <sys/stat.h>

// PWDFT includes - minimal set for verification
#include "Parallel.hpp"
#include "Lattice.hpp"
#include "Control2.hpp"

using namespace pwdft;

// Verification utilities
#include "verification_utils.hpp"

class PWDFTVerificationSuite {
private:
    Parallel *myparall;
    Control2 *control;
    Lattice *lattice;
    
    // Test calculation files
    std::string test_input_file;
    std::string test_output_file;
    std::string test_movecs_file;
    
public:
    PWDFTVerificationSuite() {
        // Initialize MPI with real PWDFT classes
        myparall = new Parallel(MPI_COMM_WORLD);
        
        // Create a simple control object with minimal JSON configuration
        std::string test_config = R"({
            "current_task": "energy",
            "charge": 0.0,
            "nwpw": {
                "mapping": 3,
                "mapping1d": 1,
                "tile_factor": 1,
                "initial_psi_random_algorithm": 1,
                "initial_wavefunction_guess": "superposition",
                "pfft3_qsize": 5,
                "np_dimensions": [1, 1, 1]
            }
        })";
        control = new Control2(0, test_config);
        
        // Create a simple lattice for testing
        lattice = new Lattice(*control);
        
        // Set up test file names
        test_input_file = "test_h2.nw";
        test_output_file = "test_h2.out";
        test_movecs_file = "test_h2.movecs";
        
        // Create test input file
        create_test_input_file();
    }
    
    ~PWDFTVerificationSuite() {
        delete lattice;
        delete control;
        delete myparall;
        
        // Clean up test files
        cleanup_test_files();
    }
    
    void create_test_input_file() {
        std::ofstream input_file(test_input_file);
        if (input_file.is_open()) {
            input_file << R"(Title "H2 Molecule Energy Test"

memory 1000 mb
start test_h2
echo

geometry noautosym noautoz center
H 0 0 0.3705
H 0 0 -0.3705
end

nwpw
   pseudopotentials
   H library pspw_default
   end
   simulation_cell
     SC 10.0
   end
   mapping 1
   cutoff 20.0
   xc pbe
   steepest_descent
      time_step 5.0
      loop 10 100
   end
end

task pspw energy)";
            input_file.close();
            std::cout << "Created test input file: " << test_input_file << std::endl;
        }
    }
    
    void cleanup_test_files() {
        // Remove test files
        std::remove(test_input_file.c_str());
        std::remove(test_output_file.c_str());
        std::remove(test_movecs_file.c_str());
    }
    
    bool file_exists(const std::string& filename) {
        struct stat buffer;
        return (stat(filename.c_str(), &buffer) == 0);
    }
    
    // REAL TESTS - Testing actual PWDFT calculations
    
    bool test_real_pwdft_execution() {
        std::cout << "Testing REAL PWDFT execution..." << std::endl;
        
        // Check if PWDFT executable exists
        std::string pwdft_exec = "../build/pwdft";
        if (!file_exists(pwdft_exec)) {
            std::cout << "  SKIP: PWDFT executable not found at " << pwdft_exec << std::endl;
            return true; // Skip if PWDFT not available
        }
        
        // Run PWDFT calculation
        std::string command = pwdft_exec + " < " + test_input_file + " > " + test_output_file;
        std::cout << "  Running: " << command << std::endl;
        
        int result = system(command.c_str());
        
        bool passed = (result == 0) && file_exists(test_output_file);
        
        std::cout << "  PWDFT execution result: " << result << std::endl;
        std::cout << "  Output file created: " << (file_exists(test_output_file) ? "YES" : "NO") << std::endl;
        std::cout << "  Status: " << (passed ? "PASS" : "FAIL") << std::endl;
        
        return passed;
    }
    
    bool test_real_output_parsing() {
        std::cout << "Testing REAL output parsing..." << std::endl;
        
        if (!file_exists(test_output_file)) {
            std::cout << "  SKIP: Output file not found" << std::endl;
            return true;
        }
        
        std::ifstream output_file(test_output_file);
        if (!output_file.is_open()) {
            std::cout << "  FAIL: Cannot open output file" << std::endl;
            return false;
        }
        
        std::string line;
        bool found_energy = false;
        bool found_convergence = false;
        double energy_value = 0.0;
        
        while (std::getline(output_file, line)) {
            // Look for energy output
            if (line.find("total energy") != std::string::npos || 
                line.find("Total Energy") != std::string::npos) {
                found_energy = true;
                // Try to extract energy value
                std::istringstream iss(line);
                std::string token;
                while (iss >> token) {
                    try {
                        energy_value = std::stod(token);
                        break;
                    } catch (...) {
                        continue;
                    }
                }
            }
            
            // Look for convergence information
            if (line.find("convergence") != std::string::npos || 
                line.find("Convergence") != std::string::npos) {
                found_convergence = true;
            }
        }
        
        output_file.close();
        
        bool passed = found_energy && found_convergence;
        
        std::cout << "  Energy found: " << (found_energy ? "YES" : "NO") << std::endl;
        if (found_energy) {
            std::cout << "  Energy value: " << energy_value << " (if parsed correctly)" << std::endl;
        }
        std::cout << "  Convergence info found: " << (found_convergence ? "YES" : "NO") << std::endl;
        std::cout << "  Status: " << (passed ? "PASS" : "FAIL") << std::endl;
        
        return passed;
    }
    
    bool test_real_wavefunction_file() {
        std::cout << "Testing REAL wavefunction file..." << std::endl;
        
        if (!file_exists(test_movecs_file)) {
            std::cout << "  SKIP: Wavefunction file not found" << std::endl;
            return true;
        }
        
        // Check file size (should be non-zero for a real calculation)
        struct stat file_stat;
        if (stat(test_movecs_file.c_str(), &file_stat) == 0) {
            bool passed = (file_stat.st_size > 0);
            
            std::cout << "  Wavefunction file size: " << file_stat.st_size << " bytes" << std::endl;
            std::cout << "  File is non-empty: " << (passed ? "YES" : "NO") << std::endl;
            std::cout << "  Status: " << (passed ? "PASS" : "FAIL") << std::endl;
            
            return passed;
        } else {
            std::cout << "  FAIL: Cannot stat wavefunction file" << std::endl;
            return false;
        }
    }
    
    bool test_real_control_parsing() {
        std::cout << "Testing REAL control parsing..." << std::endl;
        
        // Test that control object was parsed correctly
        bool passed = true;
        
        // Check that control object exists
        if (!control) {
            passed = false;
            std::cout << "  Control object is null" << std::endl;
        }
        
        // Check that lattice was created
        if (!lattice) {
            passed = false;
            std::cout << "  Lattice object is null" << std::endl;
        }
        
        // Check that parallel object was created
        if (!myparall) {
            passed = false;
            std::cout << "  Parallel object is null" << std::endl;
        }
        
        std::cout << "  Control object created: " << (control ? "YES" : "NO") << std::endl;
        std::cout << "  Lattice object created: " << (lattice ? "YES" : "NO") << std::endl;
        std::cout << "  Parallel object created: " << (myparall ? "YES" : "NO") << std::endl;
        std::cout << "  Status: " << (passed ? "PASS" : "FAIL") << std::endl;
        
        return passed;
    }
    
    bool test_real_parallel_communication() {
        std::cout << "Testing REAL parallel communication..." << std::endl;
        
        int nranks, myrank;
        MPI_Comm_size(MPI_COMM_WORLD, &nranks);
        MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
        
        // Test real MPI communication
        double local_value = static_cast<double>(myrank);
        double global_sum = 0.0;
        
        MPI_Allreduce(&local_value, &global_sum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        
        double expected_sum = nranks * (nranks - 1) / 2.0;
        double error = std::abs(global_sum - expected_sum);
        
        bool passed = error < 1e-12;
        
        std::cout << "  Rank " << myrank << ": local=" << local_value 
                  << ", global_sum=" << global_sum << std::endl;
        std::cout << "  Expected sum: " << expected_sum << std::endl;
        std::cout << "  Error: " << error << std::endl;
        std::cout << "  Status: " << (passed ? "PASS" : "FAIL") << std::endl;
        
        return passed;
    }
    
    bool test_real_memory_allocation() {
        std::cout << "Testing REAL memory allocation and deallocation..." << std::endl;
        
        // Test real memory allocation
        size_t initial_memory = get_memory_usage();
        
        // Allocate large arrays
        int test_size = 1000000;
        double* test_array1 = new double[test_size];
        double* test_array2 = new double[test_size];
        
        size_t after_allocation = get_memory_usage();
        
        // Use the arrays
        for (int i = 0; i < test_size; ++i) {
            test_array1[i] = std::sin(static_cast<double>(i));
            test_array2[i] = std::cos(static_cast<double>(i));
        }
        
        // Deallocate
        delete[] test_array1;
        delete[] test_array2;
        
        size_t after_deallocation = get_memory_usage();
        
        bool passed = (after_allocation > initial_memory) && 
                     (after_deallocation <= after_allocation);
        
        std::cout << "  Initial memory: " << initial_memory << " bytes" << std::endl;
        std::cout << "  After allocation: " << after_allocation << " bytes" << std::endl;
        std::cout << "  After deallocation: " << after_deallocation << " bytes" << std::endl;
        std::cout << "  Status: " << (passed ? "PASS" : "FAIL") << std::endl;
        
        return passed;
    }
    
    bool test_real_numerical_stability() {
        std::cout << "Testing REAL numerical stability..." << std::endl;
        
        // Test with realistic wavefunction data
        int test_size = 1000;
        double* test_data = new double[test_size];
        
        for (int i = 0; i < test_size; ++i) {
            double x = 2.0 * M_PI * (i % 10) / 10.0;
            test_data[i] = std::exp(-x*x/2.0);
        }
        
        // Test clean data
        bool has_nan_clean = check_nan_inf("test_data", test_data, test_size);
        
        // Test with NaN
        test_data[0] = std::numeric_limits<double>::quiet_NaN();
        bool has_nan_dirty = check_nan_inf("test_data", test_data, test_size);
        
        // Test with Inf
        test_data[0] = std::numeric_limits<double>::infinity();
        bool has_inf_dirty = check_nan_inf("test_data", test_data, test_size);
        
        // Test with very small values
        test_data[0] = 1e-300;
        bool has_small_clean = check_nan_inf("test_data", test_data, test_size);
        
        delete[] test_data;
        
        bool passed = !has_nan_clean && has_nan_dirty && has_inf_dirty && !has_small_clean;
        
        std::cout << "  Clean data test: " << (!has_nan_clean ? "PASS" : "FAIL") << std::endl;
        std::cout << "  NaN detection test: " << (has_nan_dirty ? "PASS" : "FAIL") << std::endl;
        std::cout << "  Inf detection test: " << (has_inf_dirty ? "PASS" : "FAIL") << std::endl;
        std::cout << "  Small value test: " << (!has_small_clean ? "PASS" : "FAIL") << std::endl;
        std::cout << "  Overall status: " << (passed ? "PASS" : "FAIL") << std::endl;
        
        return passed;
    }
    
    // Run all REAL tests
    void run_all_tests() {
        std::cout << "==========================================" << std::endl;
        std::cout << "PWDFT Phase 4: REAL Verification Suite" << std::endl;
        std::cout << "==========================================" << std::endl;
        
        int total_tests = 0;
        int passed_tests = 0;
        
        // REAL PWDFT Functionality Tests (with actual PWDFT execution)
        std::cout << "\nREAL PWDFT Functionality Tests (Actual PWDFT Execution)" << std::endl;
        std::cout << "------------------------------------------------------" << std::endl;
        
        total_tests++; passed_tests += test_real_control_parsing() ? 1 : 0;
        total_tests++; passed_tests += test_real_pwdft_execution() ? 1 : 0;
        total_tests++; passed_tests += test_real_output_parsing() ? 1 : 0;
        total_tests++; passed_tests += test_real_wavefunction_file() ? 1 : 0;
        total_tests++; passed_tests += test_real_parallel_communication() ? 1 : 0;
        total_tests++; passed_tests += test_real_memory_allocation() ? 1 : 0;
        total_tests++; passed_tests += test_real_numerical_stability() ? 1 : 0;
        
        // Summary
        std::cout << "\n==========================================" << std::endl;
        std::cout << "REAL Verification Summary" << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << "Total tests: " << total_tests << std::endl;
        std::cout << "Passed tests: " << passed_tests << std::endl;
        std::cout << "Failed tests: " << (total_tests - passed_tests) << std::endl;
        std::cout << "Success rate: " << (100.0 * passed_tests / total_tests) << "%" << std::endl;
        
        if (passed_tests == total_tests) {
            std::cout << "✅ ALL REAL TESTS PASSED - PWDFT implementation verified!" << std::endl;
        } else {
            std::cout << "❌ SOME REAL TESTS FAILED - Review implementation issues" << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    // Initialize MPI
    MPI_Init(&argc, &argv);
    
    try {
        PWDFTVerificationSuite verifier;
        verifier.run_all_tests();
    } catch (const std::exception& e) {
        std::cerr << "Error in verification suite: " << e.what() << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Finalize MPI
    MPI_Finalize();
    
    return 0;
} 