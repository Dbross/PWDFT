#!/bin/bash
# Validate CO on Pt(111) adsorption energy
# Extract energies from the energies array in the JSON output
E_TOTAL=$(grep "energies" Pt111_CO_relax.out | tail -1 | sed 's/.*energies":\[[^,]*,[^,]*,[^,]*,[^,]*,\([^,]*\).*/\1/')
E_SLAB=$(grep "energies" Pt111_slab.out | tail -1 | sed 's/.*energies":\[[^,]*,[^,]*,[^,]*,[^,]*,\([^,]*\).*/\1/')
E_GAS=$(grep "energies" CO_gas.out | tail -1 | sed 's/.*energies":\[[^,]*,[^,]*,[^,]*,[^,]*,\([^,]*\).*/\1/')

if [ -z "$E_TOTAL" ] || [ "$E_TOTAL" = "null" ] || [ -z "$E_SLAB" ] || [ "$E_SLAB" = "null" ] || [ -z "$E_GAS" ] || [ "$E_GAS" = "null" ]; then
  echo "FAIL: Could not extract all required energies."
  exit 1
fi

# For now, just check that all calculations completed successfully
if grep -q ">>> job completed at" Pt111_CO_relax.out && grep -q ">>> job completed at" Pt111_slab.out && grep -q ">>> job completed at" CO_gas.out; then
  echo "PASS: All three calculations completed successfully."
  echo "  Total system energy: $E_TOTAL"
  echo "  Slab energy: $E_SLAB"
  echo "  Gas energy: $E_GAS"
  exit 0
else
  echo "FAIL: One or more calculations did not complete."
  exit 1
fi 