#!/bin/bash

# Check if pw.x is available
PW_EXECUTABLE=$(which pw.x)
if [ -z "$PW_EXECUTABLE" ]; then
    echo "ERROR: 'pw.x' executable not found in PATH"
    echo "Please ensure Quantum ESPRESSO is installed and pw.x is available"
    exit 1
fi

echo "Using pw.x executable at: $PW_EXECUTABLE"

# Check if pseudopotential file exists
PSEUDO_FILE="/global/homes/b/brossdh/espresso/pseudo/H.pbe-rrkjus_psl.1.0.0.UPF"
if [ ! -f "$PSEUDO_FILE" ]; then
    echo "ERROR: Pseudopotential file not found: $PSEUDO_FILE"
    echo "Please ensure the H pseudopotential is available in the specified directory"
    exit 1
fi

echo "Using pseudopotential: $PSEUDO_FILE"

# Clean up previous run
rm -f *.out *.xml *.wfc* *.rho* *.pot* *.dat *.save 2>/dev/null || true

# Run the calculation
$PW_EXECUTABLE < h2_energy.qe.in > h2_energy.qe.out

# Check if calculation completed successfully
if [ $? -eq 0 ]; then
    echo "QE calculation completed successfully"
else
    echo "QE calculation failed"
    exit 1
fi 