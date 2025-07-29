#!/bin/bash

# PWDFT Phase 4 Verification Test Runner
# This script runs comprehensive verification tests for the PWDFT implementation

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PWDFT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PWDFT_ROOT/build"
VERIFICATION_DIR="$SCRIPT_DIR"
H2_TEST_DIR="$PWDFT_ROOT/tests/tier1/2.4.1_H2_band"
LOG_DIR="$VERIFICATION_DIR/logs"

# Create log directory
mkdir -p "$LOG_DIR"

# Timestamp for logs
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
LOG_FILE="$LOG_DIR/verification_${TIMESTAMP}.log"

# Function to print colored output
print_status() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}" | tee -a "$LOG_FILE"
}

# Function to run command and log output
run_command() {
    local cmd="$1"
    local description="$2"
    
    print_status "$BLUE" "Running: $description"
    print_status "$YELLOW" "Command: $cmd"
    
    if eval "$cmd" 2>&1 | tee -a "$LOG_FILE"; then
        print_status "$GREEN" "✅ SUCCESS: $description"
        return 0
    else
        print_status "$RED" "❌ FAILED: $description"
        return 1
    fi
}

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Function to check MPI availability
check_mpi() {
    if ! command_exists mpirun; then
        print_status "$RED" "❌ ERROR: mpirun not found. Please install MPI."
        exit 1
    fi
    
    if ! command_exists mpicxx; then
        print_status "$RED" "❌ ERROR: mpicxx not found. Please install MPI C++ compiler."
        exit 1
    fi
    
    print_status "$GREEN" "✅ MPI environment detected"
}

# Function to check PWDFT build
check_pwdft_build() {
    if [ ! -d "$BUILD_DIR" ]; then
        print_status "$RED" "❌ ERROR: Build directory not found: $BUILD_DIR"
        print_status "$YELLOW" "Please run: cd $PWDFT_ROOT && mkdir -p build && cd build && make"
        exit 1
    fi
    
    if [ ! -f "$BUILD_DIR/pwdft" ] && [ ! -f "$BUILD_DIR/lib/libpwdft.a" ] && [ ! -f "$BUILD_DIR/lib/libpwdft.so" ]; then
        # Check for individual component libraries
        if [ ! -f "$BUILD_DIR/nwpwlib/libnwpwlib.a" ] && [ ! -f "$BUILD_DIR/pspw/libpspw.a" ] && [ ! -f "$BUILD_DIR/band/libband.a" ]; then
            print_status "$RED" "❌ ERROR: PWDFT libraries not found in build directory"
            print_status "$YELLOW" "Please build PWDFT first: cd $BUILD_DIR && make"
            exit 1
        fi
    fi
    
    print_status "$GREEN" "✅ PWDFT build detected"
}

# Function to run H2 band test
run_h2_band_test() {
    print_status "$BLUE" "Running H2 Band Test (Reference Test)"
    
    if [ ! -d "$H2_TEST_DIR" ]; then
        print_status "$RED" "❌ ERROR: H2 test directory not found: $H2_TEST_DIR"
        return 1
    fi
    
    cd "$H2_TEST_DIR"
    
    # Clean previous results
    run_command "rm -f h2_band.out debug.log" "Cleaning previous H2 test results"
    
    # Run H2 band test
    run_command "./run.sh > h2_band.out 2> debug.log" "Running H2 band test"
    
    # Check for successful completion
    if [ -f "h2-energy.json" ]; then
        local energy=$(grep '"energy":' h2-energy.json | tail -1 | sed 's/.*"energy": \([0-9.-]*\).*/\1/')
        print_status "$GREEN" "✅ H2 band test completed successfully"
        print_status "$GREEN" "   Final energy: $energy Hartree"
        
        # Check for reasonable energy range
        if (( $(echo "$energy > -2.0 && $energy < 2.0" | bc -l) )); then
            print_status "$GREEN" "✅ Energy in reasonable range for H2"
        else
            print_status "$YELLOW" "⚠️  Energy outside expected range for H2"
        fi
    elif grep -q "Final energy:" h2_band.out; then
        local energy=$(grep "Final energy:" h2_band.out | tail -1 | awk '{print $3}')
        print_status "$GREEN" "✅ H2 band test completed successfully"
        print_status "$GREEN" "   Final energy: $energy Hartree"
        
        # Check for reasonable energy range
        if (( $(echo "$energy > -2.0 && $energy < 2.0" | bc -l) )); then
            print_status "$GREEN" "✅ Energy in reasonable range for H2"
        else
            print_status "$YELLOW" "⚠️  Energy outside expected range for H2"
        fi
    else
        print_status "$RED" "❌ H2 band test failed - no final energy found"
        return 1
    fi
    
    # Check for errors in debug log
    if [ -s debug.log ]; then
        print_status "$YELLOW" "⚠️  Debug log contains messages:"
        tail -10 debug.log | tee -a "$LOG_FILE"
    fi
    
    cd "$VERIFICATION_DIR"
}

# Function to build verification suite
build_verification_suite() {
    print_status "$BLUE" "Building Verification Suite"
    
    cd "$VERIFICATION_DIR"
    
    # Clean previous build
    run_command "make clean" "Cleaning previous verification build"
    
    # Build verification suite
    run_command "make all" "Building verification suite"
    
    if [ -f "verification_suite" ]; then
        print_status "$GREEN" "✅ Verification suite built successfully"
    else
        print_status "$RED" "❌ Verification suite build failed"
        return 1
    fi
}

# Function to run physics-based smoke tests
run_physics_tests() {
    print_status "$BLUE" "Running Physics-Based Smoke Tests"
    
    cd "$VERIFICATION_DIR"
    
    # Test density conservation
    run_command "mpirun -np 1 ./verification_suite --test-density-conservation" "Density conservation test"
    
    # Test energy conservation
    run_command "mpirun -np 1 ./verification_suite --test-energy-conservation" "Energy conservation test"
    
    # Test FFT round-trip
    run_command "mpirun -np 1 ./verification_suite --test-fft-round-trip" "FFT round-trip test"
    
    # Test orthogonality
    run_command "mpirun -np 1 ./verification_suite --test-orthogonality" "Wavefunction orthogonality test"
}

# Function to run convergence verification tests
run_convergence_tests() {
    print_status "$BLUE" "Running Convergence Verification Tests"
    
    cd "$VERIFICATION_DIR"
    
    # Test SCF convergence
    run_command "mpirun -np 1 ./verification_suite --test-scf-convergence" "SCF convergence test"
    
    # Test k-point convergence
    run_command "mpirun -np 1 ./verification_suite --test-kpoint-convergence" "K-point convergence test"
}

# Function to run performance verification tests
run_performance_tests() {
    print_status "$BLUE" "Running Performance Verification Tests"
    
    cd "$VERIFICATION_DIR"
    
    # Test MPI scalability
    run_command "mpirun -np 4 ./verification_suite --test-mpi-scalability" "MPI scalability test"
    
    # Test memory usage
    run_command "mpirun -np 1 ./verification_suite --test-memory-usage" "Memory usage test"
}

# Function to run numerical stability tests
run_stability_tests() {
    print_status "$BLUE" "Running Numerical Stability Tests"
    
    cd "$VERIFICATION_DIR"
    
    # Test NaN/Inf detection
    run_command "mpirun -np 1 ./verification_suite --test-nan-inf-detection" "NaN/Inf detection test"
    
    # Test energy monotonicity
    run_command "mpirun -np 1 ./verification_suite --test-energy-monotonicity" "Energy monotonicity test"
}

# Function to run all verification tests
run_all_verification_tests() {
    print_status "$BLUE" "Running All Verification Tests"
    
    cd "$VERIFICATION_DIR"
    
    # Run complete verification suite
    run_command "mpirun -np 1 ./verification_suite" "Complete verification suite"
}

# Function to generate verification report
generate_report() {
    print_status "$BLUE" "Generating Verification Report"
    
    local report_file="$LOG_DIR/verification_report_${TIMESTAMP}.md"
    
    cat > "$report_file" << EOF
# PWDFT Phase 4 Verification Report

**Date**: $(date)
**Timestamp**: $TIMESTAMP
**PWDFT Root**: $PWDFT_ROOT

## Test Summary

### H2 Band Test (Reference)
- **Status**: $(grep -q "H2 band test completed successfully" "$LOG_FILE" && echo "✅ PASSED" || echo "❌ FAILED")
- **Final Energy**: $(grep '"energy":' "$LOG_FILE" | tail -1 | sed 's/.*"energy": \([0-9.-]*\).*/\1/' 2>/dev/null || grep "Final energy:" "$LOG_FILE" | tail -1 | awk '{print $3}' 2>/dev/null || echo "N/A")

### Physics-Based Smoke Tests
- **Density Conservation**: $(grep -q "Density conservation.*PASS" "$LOG_FILE" && echo "✅ PASSED" || echo "❌ FAILED")
- **Energy Conservation**: $(grep -q "Energy conservation.*PASS" "$LOG_FILE" && echo "✅ PASSED" || echo "❌ FAILED")
- **FFT Round-Trip**: $(grep -q "FFT round-trip.*PASS" "$LOG_FILE" && echo "✅ PASSED" || echo "❌ FAILED")
- **Orthogonality**: $(grep -q "Orthogonality.*PASS" "$LOG_FILE" && echo "✅ PASSED" || echo "❌ FAILED")

### Convergence Verification Tests
- **SCF Convergence**: $(grep -q "SCF convergence.*PASS" "$LOG_FILE" && echo "✅ PASSED" || echo "❌ FAILED")
- **K-point Convergence**: $(grep -q "K-point convergence.*PASS" "$LOG_FILE" && echo "✅ PASSED" || echo "❌ FAILED")

### Performance Verification Tests
- **MPI Scalability**: $(grep -q "MPI scalability.*PASS" "$LOG_FILE" && echo "✅ PASSED" || echo "❌ FAILED")
- **Memory Usage**: $(grep -q "Memory usage.*PASS" "$LOG_FILE" && echo "✅ PASSED" || echo "❌ FAILED")

### Numerical Stability Tests
- **NaN/Inf Detection**: $(grep -q "NaN/Inf detection.*PASS" "$LOG_FILE" && echo "✅ PASSED" || echo "❌ FAILED")
- **Energy Monotonicity**: $(grep -q "Energy monotonicity.*PASS" "$LOG_FILE" && echo "✅ PASSED" || echo "❌ FAILED")

## Detailed Log

\`\`\`
$(cat "$LOG_FILE")
\`\`\`

## Recommendations

$(if grep -q "❌ FAILED" "$report_file"; then
    echo "- Review failed tests and address implementation issues"
    echo "- Check numerical stability and convergence criteria"
    echo "- Verify MPI communication patterns"
else
    echo "- All tests passed successfully"
    echo "- PWDFT implementation verified for correctness and performance"
    echo "- Ready for production use"
fi)

EOF

    print_status "$GREEN" "✅ Verification report generated: $report_file"
}

# Main execution
main() {
    print_status "$BLUE" "=========================================="
    print_status "$BLUE" "PWDFT Phase 4 Verification Test Runner"
    print_status "$BLUE" "=========================================="
    print_status "$BLUE" "Date: $(date)"
    print_status "$BLUE" "PWDFT Root: $PWDFT_ROOT"
    print_status "$BLUE" "Log File: $LOG_FILE"
    
    # Check prerequisites
    check_mpi
    check_pwdft_build
    
    # Parse command line arguments
    case "${1:-all}" in
        "h2"|"h2-band")
            run_h2_band_test
            ;;
        "physics"|"smoke")
            build_verification_suite
            run_physics_tests
            ;;
        "convergence")
            build_verification_suite
            run_convergence_tests
            ;;
        "performance")
            build_verification_suite
            run_performance_tests
            ;;
        "stability")
            build_verification_suite
            run_stability_tests
            ;;
        "verification"|"suite")
            build_verification_suite
            run_all_verification_tests
            ;;
        "all"|"")
            run_h2_band_test
            build_verification_suite
            run_all_verification_tests
            ;;
        "report")
            generate_report
            ;;
        "help"|"-h"|"--help")
            print_status "$BLUE" "Usage: $0 [test_type]"
            print_status "$BLUE" "Test types:"
            print_status "$BLUE" "  h2, h2-band     - Run H2 band reference test"
            print_status "$BLUE" "  physics, smoke  - Run physics-based smoke tests"
            print_status "$BLUE" "  convergence     - Run convergence verification tests"
            print_status "$BLUE" "  performance     - Run performance verification tests"
            print_status "$BLUE" "  stability       - Run numerical stability tests"
            print_status "$BLUE" "  verification    - Run complete verification suite"
            print_status "$BLUE" "  all             - Run all tests (default)"
            print_status "$BLUE" "  report          - Generate verification report"
            print_status "$BLUE" "  help            - Show this help message"
            exit 0
            ;;
        *)
            print_status "$RED" "❌ ERROR: Unknown test type: $1"
            print_status "$YELLOW" "Use '$0 help' for usage information"
            exit 1
            ;;
    esac
    
    # Generate report if not already done
    if [ "${1:-all}" != "report" ]; then
        generate_report
    fi
    
    print_status "$GREEN" "=========================================="
    print_status "$GREEN" "Verification test run completed"
    print_status "$GREEN" "Check log file: $LOG_FILE"
    print_status "$GREEN" "Check report: $LOG_DIR/verification_report_${TIMESTAMP}.md"
    print_status "$GREEN" "=========================================="
}

# Run main function with all arguments
main "$@" 