#!/bin/bash
if ! grep -q "JOB DONE" si_vcrelax.out; then
  echo "FAIL: vc-relax calculation did not complete."
  exit 1
fi

# 1. Validate final lattice constant
FINAL_A=$(grep "new unit-cell volume" si_vcrelax.out | tail -1 | awk '{ a = ($5 * 2.0)**(1.0/3.0); print a }')
BENCHMARK_A="10.26"
TOLERANCE_A="0.1"
IS_A_OK=$(echo "$FINAL_A $BENCHMARK_A $TOLERANCE_A" | awk '{ diff = $1 - $2; if (diff < 0) diff = -diff; if (diff < $3) print "1"; else print "0"; }')

if [ "$IS_A_OK" != "1" ]; then
  echo "FAIL: Final lattice constant $FINAL_A is not within tolerance of $BENCHMARK_A."
  exit 1
fi
echo "PASS: Final lattice constant $FINAL_A is correct."

# 2. Validate band gap
if grep -q "highest occupied, lowest unoccupied level" si_vcrelax.out; then
   HOMO=$(grep "highest occupied" si_vcrelax.out | tail -1 | awk '{print $5}')
   LUMO=$(grep "lowest unoccupied" si_vcrelax.out | tail -1 | awk '{print $5}')
   BAND_GAP_RY=$(echo "$LUMO $HOMO" | awk '{print $1 - $2}')
   BAND_GAP_EV=$(echo "$BAND_GAP_RY" | awk '{print $1 * 13.6057}')
   BENCHMARK_GAP_EV="0.5"
   TOLERANCE_GAP_EV="0.2"
   IS_GAP_OK=$(echo "$BAND_GAP_EV $BENCHMARK_GAP_EV $TOLERANCE_GAP_EV" | awk '{ diff = $1 - $2; if (diff < 0) diff = -diff; if (diff < $3) print "1"; else print "0"; }')

   if [ "$IS_GAP_OK" = "1" ]; then
     echo "PASS: Band gap $BAND_GAP_EV eV is within tolerance."
     exit 0
   else
     echo "FAIL: Band gap $BAND_GAP_EV eV is NOT within tolerance."
     exit 1
   fi
else
  echo "FAIL: Could not determine band gap from output."
  exit 1
fi 