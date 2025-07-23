#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# --- Build Log ---
# This file will now contain the output of the 'make' commands.
# It will be overwritten for each commit, letting you verify the build.
BUILD_LOG="bisect_build.log"

echo "---" > $BUILD_LOG
echo "Checking commit $(git rev-parse --short HEAD)" >> $BUILD_LOG
echo "Build started at: $(date)" >> $BUILD_LOG

# 1. Set the correct environment path.
export PATH=$PATH:/Users/brossdh/src/PWDFT/build2

# 2. Force a clean rebuild of the project.
#    'make clean' removes old compiled files.
#    'make' then rebuilds from scratch for the current commit.
#    We append all output (stdout & stderr) to the log file.
cd build2
make clean >> $BUILD_LOG 2>&1
make >> $BUILD_LOG 2>&1

echo "Build finished. Running test..." >> $BUILD_LOG
cd ..

# 3. Run the test script and check for the PASS status.
#    This command correctly determines if the test passed.
sh run_tests.sh | grep -A 3 "2.1.1_H2_energy" | grep -q "STATUS: PASSED"
