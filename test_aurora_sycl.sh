#!/bin/bash

# Test script for Aurora SYCL device selection
# This script tests the integration with gpu_tile_compact.sh

set -e

echo "=== Aurora SYCL Device Selection Test ==="
echo

# Check if we're on Aurora
if [[ -f /usr/bin/udevadm ]]; then
    num_gpu=$(/usr/bin/udevadm info /sys/module/i915/drivers/pci:i915/* |& grep -v Unknown | grep -c "P: /devices" || echo "0")
    echo "Detected $num_gpu Intel GPUs on this node"
else
    echo "Not on Aurora (no udevadm found)"
    num_gpu=0
fi

echo

# Test 1: Basic SYCL device availability
echo "Test 1: Basic SYCL Device Availability"
echo "----------------------------------------"
./test_sycl_device
echo

# Test 2: Single rank with Aurora GPU tiling
echo "Test 2: Single Rank with Aurora GPU Tiling"
echo "-------------------------------------------"
if [ $num_gpu -gt 0 ]; then
    echo "Testing with 1 rank on Aurora GPU..."
    mpiexec -n 1 --ppn 1 --host localhost --depth=1 --cpu-bind depth \
        --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
        ./gpu_tile_compact.sh ./test_sycl_device
else
    echo "No GPUs detected, skipping Aurora GPU test"
fi
echo

# Test 3: Multiple ranks with Aurora GPU tiling
echo "Test 3: Multiple Ranks with Aurora GPU Tiling"
echo "----------------------------------------------"
if [ $num_gpu -gt 0 ]; then
    echo "Testing with 4 ranks on Aurora GPU..."
    mpiexec -n 4 --ppn 4 --host localhost --depth=1 --cpu-bind depth \
        --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
        ./gpu_tile_compact.sh ./test_sycl_device
else
    echo "No GPUs detected, skipping Aurora GPU test"
fi
echo

# Test 4: PWDFT with Aurora GPU tiling
echo "Test 4: PWDFT with Aurora GPU Tiling"
echo "-------------------------------------"
if [ $num_gpu -gt 0 ]; then
    echo "Testing PWDFT with 2 ranks on Aurora GPU..."
    
    # Create a minimal test input
    cat > test_aurora.nw << EOF
title "Aurora SYCL Test"

geometry
  H 0.0 0.0 0.0
  H 0.74 0.0 0.0
end

nwpw
  simulation_cell
    boundary_conditions periodic
    unita 10.0 0.0 0.0
             0.0 10.0 0.0
             0.0 0.0 10.0
  end
  xc pbe
  minimizer 2
  scf_algorithm broyden
  scf_alpha 0.25
  scf_beta 0.5
  lmbfgs_size 3
end

task pspw energy
EOF
    
    mpiexec -n 2 --ppn 2 --host localhost --depth=1 --cpu-bind depth \
        --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
        ./gpu_tile_compact.sh ./pwdft test_aurora.nw
else
    echo "No GPUs detected, skipping PWDFT Aurora test"
fi
echo

echo "=== All Tests Completed ===" 