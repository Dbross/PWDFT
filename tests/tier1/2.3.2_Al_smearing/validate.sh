#!/bin/bash
if ! grep -q ">>> job completed at" al_energy.out; then
  echo "FAIL: Calculation did not complete."
  exit 1
fi

# Extract energy from the energies array in the JSON output
FINAL_ENERGY=$(grep "energies" al_energy.out | tail -1 | sed 's/.*energies":\[[^,]*,[^,]*,[^,]*,[^,]*,\([^,]*\).*/\1/')
if [ -z "$FINAL_ENERGY" ] || [ "$FINAL_ENERGY" = "null" ]; then
  echo "FAIL: Could not extract energy from output."
  exit 1
fi

BENCHMARK_ENERGY="-1.7"
TOLERANCE="0.5"
IS_OK=$(echo "$FINAL_ENERGY $BENCHMARK_ENERGY $TOLERANCE" | awk '{ diff = $1 - $2; if (diff < 0) diff = -diff; if (diff < $3) print "1"; else print "0"; }')

if [ "$IS_OK" = "1" ]; then
  echo "PASS: Energy $FINAL_ENERGY is within tolerance."
  exit 0
else
  echo "FAIL: Energy $FINAL_ENERGY is NOT within tolerance."
  exit 1
fi 