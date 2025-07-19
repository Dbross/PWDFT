#!/bin/bash
if ! grep -q ">>> job completed at" si_energy.out; then
  echo "FAIL: Calculation did not complete."
  exit 1
fi

# Extract energy from the energies array in the JSON output
FINAL_ENERGY=$(grep "energies" si_energy.out | tail -1 | sed 's/.*energies":\[[^,]*,[^,]*,[^,]*,[^,]*,\([^,]*\).*/\1/')
if [ -z "$FINAL_ENERGY" ] || [ "$FINAL_ENERGY" = "null" ]; then
  echo "FAIL: Could not extract energy from output."
  exit 1
fi

ENERGY_PER_ATOM=$(echo "$FINAL_ENERGY" | awk '{print $1 / 2.0}')
BENCHMARK_ENERGY="-2.8"
TOLERANCE="0.5"
IS_OK=$(echo "$ENERGY_PER_ATOM $BENCHMARK_ENERGY $TOLERANCE" | awk '{ diff = $1 - $2; if (diff < 0) diff = -diff; if (diff < $3) print "1"; else print "0"; }')

if [ "$IS_OK" = "1" ]; then
  echo "PASS: Energy per atom $ENERGY_PER_ATOM is within tolerance."
  exit 0
else
  echo "FAIL: Energy per atom $ENERGY_PER_ATOM is NOT within tolerance."
  exit 1
fi 