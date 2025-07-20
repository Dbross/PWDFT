#!/bin/bash

# Default to Tier 1 if no argument is given
TIER_TO_RUN="tier1"
AURORA_MODE=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --tier)
      TIER_TO_RUN="tier$2"
      shift 2
      ;;
    --aurora)
      AURORA_MODE=true
      shift
      ;;
    *)
      echo "Usage: $0 [--tier N] [--aurora]"
      echo "  --tier N: Run tier N tests (default: tier1)"
      echo "  --aurora: Use Aurora-specific settings (MPI, GPU tiling)"
      exit 1
      ;;
  esac
done

TEST_BASE_DIR="tests/${TIER_TO_RUN}"
if [ ! -d "$TEST_BASE_DIR" ]; then
  echo "ERROR: Test directory '$TEST_BASE_DIR' not found."
  exit 1
fi

# Find the pwdft executable
export PWDFT_BIN=$(which pwdft)
if [ -z "$PWDFT_BIN" ]; then
    # Look for it in a standard build directory relative to script
    if [ -f "$(dirname "$0")/build/pwdft" ]; then
        export PWDFT_BIN="$(cd "$(dirname "$0")" && pwd)/build/pwdft"
    else
        echo "ERROR: 'pwdft' executable not found in PATH or build/"
        exit 1
    fi
fi
echo "Using pwdft executable at: $PWDFT_BIN"

# Aurora-specific setup
if [ "$AURORA_MODE" = true ]; then
    echo "Running in Aurora mode with MPI and GPU tiling..."
    
    # Source Aurora environment if available
    if [ -f "aurora_config.sh" ]; then
        source aurora_config.sh
    fi
    
    # Set up MPI and GPU tiling
    export OMP_NUM_THREADS=1
    export ZE_AFFINITY_MASK=0.0
    
    # Use MPI for Aurora runs
    MPI_CMD="mpirun -np 2"
    echo "Using MPI command: $MPI_CMD"
else
    MPI_CMD=""
    echo "Running in local mode..."
fi

PASSED_COUNT=0
FAILED_COUNT=0
FAILED_TESTS=""
TEST_DIRS=$(find "$TEST_BASE_DIR" -mindepth 1 -maxdepth 1 -type d | sort)

for test_dir in $TEST_DIRS; do
  test_name=$(basename "$test_dir")
  echo "============================================================"
  echo "RUNNING TEST: $test_name"
  echo "============================================================"

  cd "$test_dir"

  # Clean up previous run
  rm -f *.out *.xml *.move* tmp/* 2>/dev/null || true

  # Run the test
  if [ -f run.sh ]; then
    chmod +x run.sh
    
    if [ "$AURORA_MODE" = true ]; then
      # For Aurora mode, modify the run.sh to use MPI
      echo "Running with Aurora settings (MPI + GPU tiling)..."
      # Create a temporary run script with MPI that preserves output redirection
      cat > run_aurora.sh << EOF
#!/bin/bash
# Find the main .nw file (prefer the one without suffixes)
if [ -f "h2_energy.nw" ]; then
  NW_FILE="h2_energy.nw"
  OUT_FILE="h2_energy.out"
elif [ -f "si_energy.nw" ]; then
  NW_FILE="si_energy.nw"
  OUT_FILE="si_energy.out"
elif [ -f "al_energy.nw" ]; then
  NW_FILE="al_energy.nw"
  OUT_FILE="al_energy.out"
else
  # Fallback to first .nw file
  NW_FILE=\$(ls *.nw | head -1)
  OUT_FILE=\$(basename \$NW_FILE .nw).out
fi
$MPI_CMD $PWDFT_BIN < \$NW_FILE > \$OUT_FILE
EOF
      chmod +x run_aurora.sh
      ./run_aurora.sh
      rm -f run_aurora.sh
    else
      ./run.sh
    fi
  else
    echo "No run.sh found, skipping execution."
  fi

  # Validate the result
  if [ -f validate.sh ]; then
    chmod +x validate.sh
    ./validate.sh
    if [ $? -eq 0 ]; then
      echo "STATUS: PASSED"
      ((PASSED_COUNT++))
    else
      echo "STATUS: FAILED"
      ((FAILED_COUNT++))
      FAILED_TESTS="$FAILED_TESTS $test_name"
      # Archive the failed test directory for debugging
      tar -czf "../../${test_name}_failed.tar.gz" . 2>/dev/null || true
    fi
  else
    echo "STATUS: NO VALIDATION (SKIPPED)"
  fi

  cd - > /dev/null
  echo ""
done

echo "============================================================"
echo "TEST SUMMARY FOR TIER: ${TIER_TO_RUN#tier}"
if [ "$AURORA_MODE" = true ]; then
  echo "MODE: Aurora (MPI + GPU tiling)"
fi
echo "============================================================"
echo "PASSED: $PASSED_COUNT"
echo "FAILED: $FAILED_COUNT"
if [ $FAILED_COUNT -gt 0 ]; then
  echo "Failed tests:$FAILED_TESTS"
  exit 1
else
  echo "All tests passed!"
  exit 0
fi 