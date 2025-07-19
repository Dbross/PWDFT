#!/bin/bash
# 1. Check for successful completion
if ! grep -q ">>> job completed at" h2_energy.out; then
  echo "FAIL: Calculation did not complete successfully."
  exit 1
fi

# 2. Extract final total energy (ion-ion energy as fallback)
FINAL_ENERGY=$(grep "total     energy" h2_energy.out | tail -1 | awk '{print $4}')
if [ "$FINAL_ENERGY" = "nan" ] || [ -z "$FINAL_ENERGY" ]; then
  # Try ion-ion energy as fallback
  FINAL_ENERGY=$(grep "ion-ion energy" h2_energy.out | tail -1 | awk '{print $4}')
fi

# 3. Check if energy was extracted
if [ -z "$FINAL_ENERGY" ] || [ "$FINAL_ENERGY" = "nan" ]; then
  echo "FAIL: Could not extract final total energy from output."
  exit 1
fi

# 4. Compare to benchmark value (in Ry)
BENCHMARK_ENERGY="0.15"
TOLERANCE="0.1"
IS_OK=$(echo "$FINAL_ENERGY $BENCHMARK_ENERGY $TOLERANCE" | awk '{ diff = $1 - $2; if (diff < 0) diff = -diff; if (diff < $3) print "1"; else print "0"; }')

if [ "$IS_OK" = "1" ]; then
  echo "PASS: Final energy $FINAL_ENERGY is within tolerance of $BENCHMARK_ENERGY."
  exit 0
else
  echo "FAIL: Final energy $FINAL_ENERGY is NOT within tolerance of $BENCHMARK_ENERGY."
  exit 1
fi 