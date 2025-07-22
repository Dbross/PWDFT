#!/usr/bin/env python3
"""
Performance Testing Script for PWDFT SCF Adaptive Diagonalization Threshold

This script automates the testing of the adaptive threshold feature by:
1. Running multiple test cases with and without adaptive threshold
2. Parsing the output to extract performance metrics
3. Generating a comprehensive performance report
"""

import subprocess
import re
import json
import time
import os
from pathlib import Path

class PWDFTPerformanceTester:
    def __init__(self, pwdft_path="./build/pwdft"):
        self.pwdft_path = pwdft_path
        self.results = {}
        
    def create_test_case(self, name, adaptive_enabled=True, **kwargs):
        """Create a test case with specified parameters"""
        test_content = f"""memory 900 mb
start {name}

geometry noautosym noautoz
H 0 0 0
H 0 0 0.8
end

nwpw
   simulation_cell
     SC 20.0
     ngrid 48 48 48
   end
   scf_adaptive_threshold {'true' if adaptive_enabled else 'false'}
   scf_initial_ethr {kwargs.get('scf_initial_ethr', '1.0e-2')}
   scf_min_ethr {kwargs.get('scf_min_ethr', '1.0e-13')}
   scf_ethr_factor {kwargs.get('scf_ethr_factor', '0.1')}
   minimizer {kwargs.get('minimizer', 8)}
   loop {kwargs.get('loop_inner', 5)} {kwargs.get('loop_outer', 20)}
   tolerances {kwargs.get('energy_tol', '1.0e-5')} {kwargs.get('density_tol', '1.0e-5')} {kwargs.get('ion_tol', '1.0e-4')}
   scf_algorithm {kwargs.get('scf_algorithm', 0)}
   scf_alpha {kwargs.get('scf_alpha', '0.25')}
   print medium
end
task pspw energy
"""
        
        filename = f"test_{name}.nw"
        with open(filename, 'w') as f:
            f.write(test_content)
        return filename
    
    def run_test(self, input_file, output_file=None):
        """Run a PWDFT test case"""
        if output_file is None:
            output_file = f"{input_file}.out"
            
        print(f"Running {input_file}...")
        start_time = time.time()
        
        try:
            result = subprocess.run(
                [self.pwdft_path, input_file],
                capture_output=True,
                text=True,
                timeout=300  # 5 minute timeout
            )
            
            run_time = time.time() - start_time
            
            # Write output to file
            with open(output_file, 'w') as f:
                f.write(result.stdout)
                if result.stderr:
                    f.write("\n=== STDERR ===\n")
                    f.write(result.stderr)
            
            return {
                'success': result.returncode == 0,
                'run_time': run_time,
                'output_file': output_file,
                'stdout': result.stdout,
                'stderr': result.stderr,
                'return_code': result.returncode
            }
            
        except subprocess.TimeoutExpired:
            return {
                'success': False,
                'run_time': time.time() - start_time,
                'output_file': output_file,
                'error': 'Timeout'
            }
        except Exception as e:
            return {
                'success': False,
                'run_time': time.time() - start_time,
                'output_file': output_file,
                'error': str(e)
            }
    
    def parse_output(self, output_file):
        """Parse PWDFT output to extract performance metrics"""
        try:
            with open(output_file, 'r') as f:
                content = f.read()
        except FileNotFoundError:
            return None
            
        metrics = {
            'converged': False,
            'iterations': 0,
            'final_energy': None,
            'adaptive_adjustments': 0,
            'run_time': None,
            'error': None
        }
        
        # Check for convergence
        if "*** tolerance ok. iteration terminated" in content:
            metrics['converged'] = True
        elif "*** arrived at the Maximum iteration.  terminated" in content:
            metrics['error'] = "Maximum iterations reached"
        
        # Extract iteration count
        iter_match = re.search(r'(\d+)\s+[-\d\.]+e[+-]\d+\s+[-\d\.]+e[+-]\d+\s+[-\d\.]+e[+-]\d+\s*\*\*', content)
        if iter_match:
            metrics['iterations'] = int(iter_match.group(1))
        
        # Extract final energy
        energy_match = re.search(r'total\s+energy\s+:\s+([-\d\.]+e[+-]\d+)', content)
        if energy_match:
            metrics['final_energy'] = float(energy_match.group(1))
        
        # Count adaptive threshold adjustments
        adaptive_matches = re.findall(r'Adaptive threshold adjusted to:', content)
        metrics['adaptive_adjustments'] = len(adaptive_matches)
        
        # Extract run time
        time_match = re.search(r'total\s+:\s+([\d\.]+e[+-]\d+)', content)
        if time_match:
            metrics['run_time'] = float(time_match.group(1))
        
        return metrics
    
    def run_performance_comparison(self):
        """Run comprehensive performance comparison"""
        test_cases = [
            # Basic SCF tests
            {
                'name': 'scf_adaptive',
                'adaptive': True,
                'minimizer': 8,
                'description': 'SCF with adaptive threshold'
            },
            {
                'name': 'scf_baseline', 
                'adaptive': False,
                'minimizer': 8,
                'description': 'SCF without adaptive threshold'
            },
            # L-BFGS tests for comparison
            {
                'name': 'lbfgs_adaptive',
                'adaptive': True,
                'minimizer': 2,
                'description': 'L-BFGS with adaptive threshold'
            },
            {
                'name': 'lbfgs_baseline',
                'adaptive': False, 
                'minimizer': 2,
                'description': 'L-BFGS without adaptive threshold'
            }
        ]
        
        results = {}
        
        for test_case in test_cases:
            print(f"\n{'='*60}")
            print(f"Testing: {test_case['description']}")
            print(f"{'='*60}")
            
            # Create test file
            input_file = self.create_test_case(
                test_case['name'],
                adaptive_enabled=test_case['adaptive'],
                minimizer=test_case['minimizer']
            )
            
            # Run test
            run_result = self.run_test(input_file)
            
            # Parse output
            metrics = self.parse_output(run_result['output_file'])
            
            # Store results
            results[test_case['name']] = {
                'test_case': test_case,
                'run_result': run_result,
                'metrics': metrics
            }
            
            # Clean up input file
            os.remove(input_file)
        
        return results
    
    def generate_report(self, results):
        """Generate a comprehensive performance report"""
        report = []
        report.append("# PWDFT SCF Adaptive Diagonalization Threshold Performance Report")
        report.append("")
        report.append(f"Generated: {time.strftime('%Y-%m-%d %H:%M:%S')}")
        report.append("")
        
        # Summary table
        report.append("## Test Results Summary")
        report.append("")
        report.append("| Test Case | Adaptive | Minimizer | Converged | Iterations | Run Time (s) | Adaptive Adjustments |")
        report.append("|-----------|----------|-----------|-----------|------------|--------------|---------------------|")
        
        for name, result in results.items():
            test_case = result['test_case']
            metrics = result['metrics']
            run_result = result['run_result']
            
            if metrics:
                report.append(f"| {test_case['description']} | {'Yes' if test_case['adaptive'] else 'No'} | {test_case['minimizer']} | {'Yes' if metrics['converged'] else 'No'} | {metrics['iterations']} | {metrics['run_time']:.3f} | {metrics['adaptive_adjustments']} |")
            else:
                report.append(f"| {test_case['description']} | {'Yes' if test_case['adaptive'] else 'No'} | {test_case['minimizer']} | Failed | - | {run_result['run_time']:.3f} | - |")
        
        report.append("")
        
        # Performance analysis
        report.append("## Performance Analysis")
        report.append("")
        
        # Compare SCF tests
        scf_adaptive = results.get('scf_adaptive', {}).get('metrics')
        scf_baseline = results.get('scf_baseline', {}).get('metrics')
        
        if scf_adaptive and scf_baseline:
            if scf_adaptive['converged'] and scf_baseline['converged']:
                iteration_improvement = ((scf_baseline['iterations'] - scf_adaptive['iterations']) / scf_baseline['iterations']) * 100
                time_improvement = ((scf_baseline['run_time'] - scf_adaptive['run_time']) / scf_baseline['run_time']) * 100
                
                report.append(f"### SCF Convergence Comparison")
                report.append("")
                report.append(f"- **Iteration Improvement**: {iteration_improvement:+.1f}% ({scf_baseline['iterations']} → {scf_adaptive['iterations']} iterations)")
                report.append(f"- **Time Improvement**: {time_improvement:+.1f}% ({scf_baseline['run_time']:.3f}s → {scf_adaptive['run_time']:.3f}s)")
                report.append(f"- **Adaptive Adjustments**: {scf_adaptive['adaptive_adjustments']} threshold adjustments made")
                report.append("")
        
        # Detailed results
        report.append("## Detailed Results")
        report.append("")
        
        for name, result in results.items():
            test_case = result['test_case']
            metrics = result['metrics']
            run_result = result['run_result']
            
            report.append(f"### {test_case['description']}")
            report.append("")
            
            if metrics:
                report.append(f"- **Convergence**: {'✅ Converged' if metrics['converged'] else '❌ Failed'}")
                report.append(f"- **Iterations**: {metrics['iterations']}")
                report.append(f"- **Final Energy**: {metrics['final_energy']}")
                report.append(f"- **Run Time**: {metrics['run_time']:.3f} seconds")
                report.append(f"- **Adaptive Adjustments**: {metrics['adaptive_adjustments']}")
            else:
                report.append(f"- **Status**: ❌ Failed to parse output")
                report.append(f"- **Error**: {run_result.get('error', 'Unknown error')}")
                report.append(f"- **Run Time**: {run_result['run_time']:.3f} seconds")
            
            report.append("")
        
        return "\n".join(report)

def main():
    """Main function to run performance testing"""
    tester = PWDFTPerformanceTester()
    
    print("PWDFT SCF Adaptive Diagonalization Threshold Performance Testing")
    print("=" * 70)
    
    # Run performance comparison
    results = tester.run_performance_comparison()
    
    # Generate report
    report = tester.generate_report(results)
    
    # Save report
    with open('performance_report.md', 'w') as f:
        f.write(report)
    
    print("\n" + "=" * 70)
    print("Performance testing completed!")
    print("Report saved to: performance_report.md")
    print("=" * 70)
    
    # Print summary to console
    print("\nQuick Summary:")
    for name, result in results.items():
        test_case = result['test_case']
        metrics = result['metrics']
        if metrics and metrics['converged']:
            print(f"✅ {test_case['description']}: {metrics['iterations']} iterations, {metrics['run_time']:.3f}s")
        else:
            print(f"❌ {test_case['description']}: Failed")

if __name__ == "__main__":
    main() 