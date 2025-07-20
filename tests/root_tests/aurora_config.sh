#!/bin/bash

# Aurora Configuration File
# Modify these settings for your specific Aurora setup

# Aurora node hostname (change if needed)
export AURORA_HOST="localhost"

# PWDFT executable path (adjust based on your build location)
export PWDFT_PATH="./build/pwdft"

# GPU tile script path (adjust if needed)
export GPU_TILE_SCRIPT="./gpu_tile_compact.sh"

# MPI settings
export MPI_NUM_RANKS_SMALL=2
export MPI_NUM_RANKS_MEDIUM=6
export MPI_NUM_RANKS_LARGE=12

# Test settings
export RUN_STRESS_TESTS=true
export RUN_LONG_TESTS=true
export RUN_PERFORMANCE_TESTS=false

# Environment variables for Aurora
export OMP_NUM_THREADS=1
export OMP_PLACES=cores
export OMP_PROC_BIND=close

# SYCL/oneAPI settings
export SYCL_DEVICE_FILTER="level_zero:gpu"
export ZE_ENABLE_PCI_ID_DEVICE_ORDER=1

# Debug settings
export DEBUG_MODE=false
export VERBOSE_OUTPUT=true

# Function to print current configuration
print_config() {
    echo "=== Aurora Configuration ==="
    echo "Host: $AURORA_HOST"
    echo "PWDFT Path: $PWDFT_PATH"
    echo "GPU Tile Script: $GPU_TILE_SCRIPT"
    echo "MPI Ranks (Small/Medium/Large): $MPI_NUM_RANKS_SMALL/$MPI_NUM_RANKS_MEDIUM/$MPI_NUM_RANKS_LARGE"
    echo "Stress Tests: $RUN_STRESS_TESTS"
    echo "Long Tests: $RUN_LONG_TESTS"
    echo "Performance Tests: $RUN_PERFORMANCE_TESTS"
    echo "Debug Mode: $DEBUG_MODE"
    echo "=========================="
}

# Function to validate configuration
validate_config() {
    local errors=0
    
    if [ ! -f "$PWDFT_PATH" ]; then
        echo "Error: PWDFT not found at $PWDFT_PATH"
        ((errors++))
    fi
    
    if [ ! -f "$GPU_TILE_SCRIPT" ]; then
        echo "Error: GPU tile script not found at $GPU_TILE_SCRIPT"
        ((errors++))
    fi
    
    if [ ! -f "test_simple.nw" ]; then
        echo "Error: test_simple.nw not found"
        ((errors++))
    fi
    
    if [ $errors -eq 0 ]; then
        echo "✓ Configuration validation passed"
        return 0
    else
        echo "✗ Configuration validation failed ($errors errors)"
        return 1
    fi
}

# Function to get MPI command template
get_mpi_cmd() {
    local num_ranks=$1
    local executable=$2
    local input_file=$3
    
    echo "mpiexec -n $num_ranks --ppn $num_ranks --host $AURORA_HOST --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=$OMP_NUM_THREADS --env OMP_PLACES=$OMP_PLACES --env OMP_PROC_BIND=$OMP_PROC_BIND $GPU_TILE_SCRIPT $executable $input_file"
}

# Function to get single rank MPI command
get_single_mpi_cmd() {
    local executable=$1
    local input_file=$2
    
    echo "mpiexec -n 1 --ppn 1 --host $AURORA_HOST --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=$OMP_NUM_THREADS --env OMP_PLACES=$OMP_PLACES --env OMP_PROC_BIND=$OMP_PROC_BIND $GPU_TILE_SCRIPT $executable $input_file"
}

# Export functions for use in other scripts
export -f print_config
export -f validate_config
export -f get_mpi_cmd
export -f get_single_mpi_cmd 