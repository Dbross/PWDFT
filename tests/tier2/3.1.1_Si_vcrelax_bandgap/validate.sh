#!/bin/bash
if ! grep -q ">>> job completed at" si_vcrelax.out; then
  echo "FAIL: vc-relax calculation did not complete."
  exit 1
fi

# For now, just check that the calculation completed successfully
if grep -q ">>> job completed at" si_vcrelax.out; then
  echo "PASS: Si variable-cell relaxation calculation completed successfully."
  exit 0
else
  echo "FAIL: Si variable-cell relaxation calculation did not complete."
  exit 1
fi 