#!/bin/bash
# 1. Check for successful completion
if ! grep -q ">>> job completed at" si_vcrelax.out; then
  echo "FAIL: Calculation did not complete successfully."
  exit 1
fi

# 2. Extract final total energy
FINAL_ENERGY=$(grep "total     energy" si_vcrelax.out | tail -1 | awk '{print $4}')
if [ "$FINAL_ENERGY" = "nan" ] || [ -z "$FINAL_ENERGY" ]; then
  echo "FAIL: Could not extract final total energy from output."
  exit 1
fi

# 3. Compare to benchmark value (in Ry) - Si dimer energy should be around -7.5 Ry
BENCHMARK_ENERGY="-7.5"
TOLERANCE="0.5"
IS_OK=$(echo "$FINAL_ENERGY $BENCHMARK_ENERGY $TOLERANCE" | awk '{ diff = $1 - $2; if (diff < 0) diff = -diff; if (diff < $3) print "1"; else print "0"; }')

if [ "$IS_OK" = "1" ]; then
  echo "PASS: Final energy $FINAL_ENERGY is within tolerance of $BENCHMARK_ENERGY."
  exit 0
else
  echo "FAIL: Final energy $FINAL_ENERGY is NOT within tolerance of $BENCHMARK_ENERGY."
  exit 1
fi 