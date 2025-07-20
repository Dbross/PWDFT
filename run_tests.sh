#!/bin/bash

# Default to Tier 1 if no argument is given
TIER_TO_RUN="tier1"
if [ "$1" == "--tier" ] && [ -n "$2" ]; then
  TIER_TO_RUN="tier$2"
fi

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
        export PWDFT_BIN="$(dirname "$0")/build/pwdft"
    else
        echo "ERROR: 'pwdft' executable not found in PATH or build/"
        exit 1
    fi
fi
echo "Using pwdft executable at: $PWDFT_BIN"

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
    ./run.sh
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