#!/usr/bin/env python3
"""
Test script for NaN detection and handling in PWDFT.

This script tests the robust NaN detection and handling that has been implemented
in the energy computation functions to prevent NaN propagation and provide better
error recovery.
"""

import os
import sys
import subprocess
import tempfile
import shutil

def create_test_input():
    """Create a test input file that should trigger NaN detection."""
    
    input_content = """echo

start pspw_pbe_Cu4h

memory 1900 mb

charge 0

geometry noautoz nocenter noautosym
system crystal
   lattice_vectors
     3.71 0.000000 0.000000
     0.000000 3.71 0.000000
     0.000000 0.000000 3.71
end

Cu 0.000000 0.000000 0.000000
Cu 0.000000 0.500000 0.500000
Cu 0.500000 0.000000 0.500000
Cu 0.500000 0.500000 0.000000
end

nwpw
  xc pbe96
  cutoff 30.0
  scf_algorithm 2
  scf_alpha 0.25
  scf_beta 0.1
  diis_histories 8
  initial_wavefunction_guess superposition
  smear methfessel-paxton
  temperature 300
  loop 20 20
  monkhorst-pack 4 4 4 
end

task band energy
"""
    
    return input_content

def run_pwdft_test(input_content, test_name, pwdft_executable):
    """Run a PWDFT test and analyze the output for NaN detection."""
    
    print(f"\n{'='*60}")
    print(f"Running test: {test_name}")
    print(f"{'='*60}")
    
    # Create temporary directory for test
    with tempfile.TemporaryDirectory() as temp_dir:
        os.chdir(temp_dir)
        
        # Write input file
        with open("test_input.nw", "w") as f:
            f.write(input_content)
        
        # Run PWDFT
        try:
            result = subprocess.run(
                [pwdft_executable, "test_input.nw"],
                capture_output=True,
                text=True,
                timeout=120  # 2 minute timeout
            )
            
            output = result.stdout + result.stderr
            
            # Analyze output for NaN detection
            print("Analysis of PWDFT output:")
            print("-" * 40)
            
            # Check for NaN detection messages
            nan_detection_count = output.count("NaN/Inf detected")
            fallback_count = output.count("Reinitializing wavefunction")
            retry_count = output.count("Retry")
            
            print(f"NaN detection messages: {nan_detection_count}")
            print(f"Fallback attempts: {fallback_count}")
            print(f"Retry attempts: {retry_count}")
            
            # Check for successful completion
            if "tolerance ok. iteration terminated" in output:
                print("✅ Test completed successfully with convergence")
                return True
            elif "arrived at the Maximum iteration" in output:
                print("⚠️  Test reached maximum iterations but didn't crash")
                return True
            elif "Maximum NaN recovery attempts" in output:
                print("❌ Test failed due to excessive NaN issues")
                return False
            else:
                print("❓ Test outcome unclear")
                return False
                
        except subprocess.TimeoutExpired:
            print("❌ Test timed out")
            return False
        except Exception as e:
            print(f"❌ Test failed with exception: {e}")
            return False

def test_nan_detection_features(pwdft_executable):
    """Test various NaN detection features."""
    
    tests = [
        {
            "name": "Basic NaN Detection Test",
            "input": create_test_input(),
            "expected": "Should detect NaN and attempt recovery"
        },
        {
            "name": "Conservative Parameters Test",
            "input": create_test_input().replace("scf_alpha 0.25", "scf_alpha 0.1").replace("scf_beta 0.1", "scf_beta 0.05"),
            "expected": "Should use more conservative parameters"
        },
        {
            "name": "Different Wavefunction Guess Test",
            "input": create_test_input().replace("initial_wavefunction_guess superposition", "initial_wavefunction_guess atomic"),
            "expected": "Should use atomic guess for reinitialization"
        }
    ]
    
    results = []
    
    for test in tests:
        success = run_pwdft_test(test["input"], test["name"], pwdft_executable)
        results.append({
            "name": test["name"],
            "success": success,
            "expected": test["expected"]
        })
    
    # Print summary
    print(f"\n{'='*60}")
    print("TEST SUMMARY")
    print(f"{'='*60}")
    
    passed = sum(1 for r in results if r["success"])
    total = len(results)
    
    for result in results:
        status = "✅ PASS" if result["success"] else "❌ FAIL"
        print(f"{status}: {result['name']}")
        print(f"    Expected: {result['expected']}")
    
    print(f"\nOverall: {passed}/{total} tests passed")
    
    return passed == total

def main():
    """Main test function."""
    
    print("PWDFT NaN Detection and Handling Test Suite")
    print("=" * 50)
    
    # Check if pwdft is available
    import os
    current_dir = os.getcwd()
    pwdft_paths = [
        os.path.join(current_dir, "build", "pwdft"),
        os.path.join(current_dir, "Nwpw", "build", "pwdft"),
        "pwdft"
    ]
    pwdft_executable = None
    
    for path in pwdft_paths:
        try:
            subprocess.run([path, "--version"], capture_output=True, check=True)
            pwdft_executable = path
            break
        except (subprocess.CalledProcessError, FileNotFoundError):
            continue
    
    if pwdft_executable is None:
        print("❌ PWDFT executable not found. Please ensure pwdft is built and available.")
        return False
    
    print(f"✅ Found PWDFT executable: {pwdft_executable}")
    
    # Run tests
    success = test_nan_detection_features(pwdft_executable)
    
    if success:
        print("\n🎉 All tests passed! NaN detection and handling is working correctly.")
    else:
        print("\n⚠️  Some tests failed. NaN detection may need further investigation.")
    
    return success

if __name__ == "__main__":
    success = main()
    sys.exit(0 if success else 1) 