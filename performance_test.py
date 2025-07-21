#!/usr/bin/env python3
"""
Simple performance test for PWDFT NaN detection overhead.
"""

import os
import subprocess
import time

def create_simple_test_input():
    """Create a very simple test input for quick performance testing."""
    
    input_content = """echo

start pspw_pbe_H2

memory 500 mb

charge 0

geometry noautoz nocenter noautosym
system molecule
end

H 0.0 0.0 0.0
H 0.74 0.0 0.0
end

nwpw
  xc pbe96
  cutoff 30.0
  scf_algorithm 0
  scf_alpha 0.25
  scf_beta 0.1
  diis_histories 5
  initial_wavefunction_guess superposition
  loop 5 5
end

task band energy
"""
    
    return input_content

def run_performance_test(pwdft_executable):
    """Run a simple performance test."""
    
    print("Performance Test: Simple H2 molecule")
    print("=" * 50)
    
    input_content = create_simple_test_input()
    
    # Create temporary directory for test
    import tempfile
    with tempfile.TemporaryDirectory() as temp_dir:
        os.chdir(temp_dir)
        
        # Write input file
        with open("test_input.nw", "w") as f:
            f.write(input_content)
        
        # Run PWDFT with timing
        start_time = time.time()
        try:
            result = subprocess.run(
                [pwdft_executable, "test_input.nw"],
                capture_output=True,
                text=True,
                timeout=60  # 1 minute timeout
            )
            end_time = time.time()
            
            execution_time = end_time - start_time
            print(f"Execution time: {execution_time:.2f} seconds")
            print(f"Exit code: {result.returncode}")
            
            # Check for NaN detection messages
            output = result.stdout + result.stderr
            nan_detection_count = output.count("NaN/Inf detected")
            fallback_count = output.count("Reinitializing wavefunction")
            
            print(f"NaN detection messages: {nan_detection_count}")
            print(f"Fallback attempts: {fallback_count}")
            
            # Check for successful completion
            if "tolerance ok. iteration terminated" in output:
                print("✅ Test completed successfully")
                return True, execution_time
            elif "arrived at the Maximum iteration" in output:
                print("⚠️  Test reached maximum iterations")
                return True, execution_time
            else:
                print("❓ Test outcome unclear")
                return False, execution_time
                
        except subprocess.TimeoutExpired:
            print("❌ Test timed out")
            return False, 60.0
        except Exception as e:
            print(f"❌ Test failed with exception: {e}")
            return False, 0.0

def main():
    """Main performance test function."""
    
    print("PWDFT Performance Test")
    print("=" * 30)
    
    # Find PWDFT executable
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
        print("❌ PWDFT executable not found.")
        return False
    
    print(f"✅ Found PWDFT executable: {pwdft_executable}")
    
    # Run performance test
    success, execution_time = run_performance_test(pwdft_executable)
    
    # Performance analysis
    print(f"\nPerformance Analysis:")
    print(f"- Execution time: {execution_time:.2f} seconds")
    
    if execution_time < 10:
        print("✅ Performance is good - execution time under 10 seconds")
    elif execution_time < 30:
        print("⚠️  Performance is acceptable - execution time under 30 seconds")
    else:
        print("❌ Performance is poor - execution time over 30 seconds")
    
    return success

if __name__ == "__main__":
    success = main()
    exit(0 if success else 1) 