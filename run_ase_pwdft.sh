#!/usr/bin/env bash
#PBS -q prod
#PBS -A EXACATCHEM
#PBS -l select=1:ncpus=12:mpiprocs=12
#PBS -l walltime=06:00:00
#PBS -l filesystems=flare

# Job script to run a single ASE Python input file with PWDFT
# Usage: qsub run_ase_pwdft.sh

set -e  # Exit on any error

# Change to the directory from which the job was submitted
cd "$PBS_O_WORKDIR" || exit 1

# Get the hostname for the mpiexec command
HOST=$(hostname)
echo "Running on host: $HOST"
echo "Working directory: $(pwd)"
echo "Job started at: $(date)"

# Set environment variables for optimal performance
export OMP_NUM_THREADS=1
export OMP_PLACES=cores
export OMP_PROC_BIND=close

# Check if ASE Python script is provided as argument
if [ $# -eq 0 ]; then
    echo "Error: No ASE Python script specified"
    echo "Usage: qsub run_ase_pwdft.sh <python_script.py>"
    echo "Example: qsub run_ase_pwdft.sh slab_optimization_with_constraints.py"
    exit 1
fi

PYTHON_SCRIPT="$1"
echo "Running ASE Python script: $PYTHON_SCRIPT"

# Check if the Python script exists
if [ ! -f "$PYTHON_SCRIPT" ]; then
    echo "Error: Python script '$PYTHON_SCRIPT' not found"
    exit 1
fi

# Create a function to run PWDFT with the specified command format
run_pwdft() {
    local input_file="$1"
    local output_file="${input_file%.nwx}.out"
    
    echo "Running PWDFT on $input_file"
    echo "Output will be written to $output_file"
    
    mpiexec -n 12 --ppn 12 --host "$HOST" --depth=1 --cpu-bind depth \
        --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close \
        gpu_tile_compact.sh /home/brossdh/PWDFT/build_sycl/pwdft "$input_file" \
        > "$output_file"
    
    echo "PWDFT completed for $input_file"
}

# Export the function so it can be used by the Python script
export -f run_pwdft

# Set up the ASE PWDFT command environment variable
# This will be used by the ASE calculator to construct the command
export PWDFT_COMMAND="mpiexec -n 12 --ppn 12 --host $HOST --depth=1 --cpu-bind depth --env OMP_NUM_THREADS=1 --env OMP_PLACES=cores --env OMP_PROC_BIND=close gpu_tile_compact.sh /home/brossdh/PWDFT/build_sycl/pwdft PREFIX.nwx > PREFIX.out"

echo "PWDFT command template: $PWDFT_COMMAND"

# Run the ASE Python script
echo "Starting ASE Python execution..."
python "$PYTHON_SCRIPT"

# Check for any generated .nwx files and run them if needed
echo "Checking for generated PWDFT input files..."
for nwx_file in *.nwx; do
    if [ -f "$nwx_file" ]; then
        echo "Found PWDFT input file: $nwx_file"
        run_pwdft "$nwx_file"
    fi
done

echo "Job completed at: $(date)"

# Optional: Clean up temporary files
# Uncomment the following lines if you want to clean up
# echo "Cleaning up temporary files..."
# rm -f *.tmp *.log 2>/dev/null || true

echo "All done!" 