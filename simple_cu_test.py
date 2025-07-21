#!/usr/bin/env python3
"""
Simple test with one Cu atom to test NaN detection.
"""

import os
import subprocess
import tempfile

def create_simple_cu_input():
    """Create a very simple test input with one Cu atom."""
    
    input_content = """echo

start pspw_pbe_Cu

memory 500 mb

charge 0

geometry noautoz nocenter noautosym
system molecule
end

Cu 0.000000 0.000000 0.000000
end

nwpw
  xc pbe96
  cutoff 20.0
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

def run_simple_test():
    """Run a simple test with one Cu atom."""
    
    print("Simple Cu Atom Test")
    print("=" * 30)
    
    # Find PWDFT executable
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
    
    input_content = create_simple_cu_input()
    
    # Create temporary directory for test
    with tempfile.TemporaryDirectory() as temp_dir:
        os.chdir(temp_dir)
        
        # Write input file
        with open("test_input.nw", "w") as f:
            f.write(input_content)
        
        print("Running PWDFT with simple Cu atom...")
        
        # Run PWDFT
        try:
            result = subprocess.run(
                [pwdft_executable, "test_input.nw"],
                capture_output=True,
                text=True,
                timeout=60  # 1 minute timeout
            )
            
            print(f"Exit code: {result.returncode}")
            
            # Analyze output
            output = result.stdout + result.stderr
            print(f"Output length: {len(output)} characters")
            
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
                # Print more of the output for debugging
                print("Last 1000 characters of output:")
                print(output[-1000:])
                return False
                
        except subprocess.TimeoutExpired:
            print("❌ Test timed out")
            return False
        except Exception as e:
            print(f"❌ Test failed with exception: {e}")
            return False

if __name__ == "__main__":
    success = run_simple_test()
    exit(0 if success else 1) 