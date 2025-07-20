#!/bin/bash

# PWDFT Commit Analysis Script
# This script analyzes commits from improved_SCF branch and shows where they will be moved
# without actually performing the reorganization.

set -e

echo "=== PWDFT Commit Analysis Script ==="
echo "Analyzing commits from last 7 days on improved_SCF branch..."
echo

# Get list of commits from last 7 days that are not on master
COMMITS_TO_REORGANIZE=$(git log --oneline --since="7 days ago" improved_SCF --not master --format="%H")

if [ -z "$COMMITS_TO_REORGANIZE" ]; then
    echo "No commits found from last 7 days on improved_SCF that are not on master"
    exit 0
fi

# Function to check if a commit contains specific content patterns
check_commit_content() {
    local commit_hash=$1
    local pattern=$2
    git show --stat $commit_hash | grep -q "$pattern" || git show --name-only $commit_hash | grep -q "$pattern"
}

# Function to categorize a single commit
categorize_commit() {
    local commit_hash=$1
    local commit_msg=$(git log --oneline -1 $commit_hash)
    
    echo "Analyzing: $commit_msg"
    
    # Check for wavefunction initialization content
    if check_commit_content $commit_hash "wavefunction\|psi\|cpsi\|atomic\|restart\|fallback" || \
       check_commit_content $commit_hash "Nwpw/band/lib/cpsi\|Nwpw/pspw/lib/psi" || \
       check_commit_content $commit_hash "atomic keyword\|restart wavefunction"; then
        echo "  → wf_initialization"
        echo "wf_initialization:$commit_hash"
        return 0
        
    # Check for documentation content
    elif check_commit_content $commit_hash "doc/\|README\|\.rst\|\.md" || \
         check_commit_content $commit_hash "documentation\|tutorial\|installation" || \
         check_commit_content $commit_hash "formatting\|syntax\|instructions"; then
        echo "  → add-comprehensive-documentation"
        echo "add-comprehensive-documentation:$commit_hash"
        return 0
        
    # Check for CI/testing content
    elif check_commit_content $commit_hash "test/\|\.sh\|aurora\|CI\|validation" || \
         check_commit_content $commit_hash "AURORA_TEST\|test_suite\|runner" || \
         check_commit_content $commit_hash "gpu_tile\|SYCL\|device" || \
         check_commit_content $commit_hash "tier[0-9]\|run_tests"; then
        echo "  → autoCI"
        echo "autoCI:$commit_hash"
        return 0
        
    # Check for SCF improvements content
    elif check_commit_content $commit_hash "SCF\|convergence\|minimizer\|adaptive\|diagonalization" || \
         check_commit_content $commit_hash "energy\|optimization\|algorithm\|preconditioning" || \
         check_commit_content $commit_hash "Local-TF\|enhanced\|system-aware\|defaults" || \
         check_commit_content $commit_hash "NaN\|fallback\|recovery\|robust"; then
        echo "  → improved_SCF"
        echo "improved_SCF:$commit_hash"
        return 0
        
    else
        # Default to improved_SCF for unclear cases
        echo "  → improved_SCF (default)"
        echo "improved_SCF:$commit_hash"
        return 0
    fi
}

echo "=== Detailed Commit Analysis ==="
echo

# Process each commit and categorize it
WF_INIT_COUNT=0
DOC_COUNT=0
AUTOCI_COUNT=0
IMPROVED_SCF_COUNT=0

echo "$COMMITS_TO_REORGANIZE" | while read commit_hash; do
    categorize_commit $commit_hash
    echo
done | while read line; do
    if [[ $line == wf_initialization:* ]]; then
        WF_INIT_COUNT=$((WF_INIT_COUNT + 1))
        echo "wf_initialization: $(git log --oneline -1 ${line#wf_initialization:})"
    elif [[ $line == add-comprehensive-documentation:* ]]; then
        DOC_COUNT=$((DOC_COUNT + 1))
        echo "add-comprehensive-documentation: $(git log --oneline -1 ${line#add-comprehensive-documentation:})"
    elif [[ $line == autoCI:* ]]; then
        AUTOCI_COUNT=$((AUTOCI_COUNT + 1))
        echo "autoCI: $(git log --oneline -1 ${line#autoCI:})"
    elif [[ $line == improved_SCF:* ]]; then
        IMPROVED_SCF_COUNT=$((IMPROVED_SCF_COUNT + 1))
        echo "improved_SCF: $(git log --oneline -1 ${line#improved_SCF:})"
    else
        echo "$line"
    fi
done

echo
echo "=== Summary ==="
echo "Commits to be moved to wf_initialization: $WF_INIT_COUNT"
echo "Commits to be moved to add-comprehensive-documentation: $DOC_COUNT"
echo "Commits to be moved to autoCI: $AUTOCI_COUNT"
echo "Commits to remain on improved_SCF: $IMPROVED_SCF_COUNT"
echo
echo "To proceed with reorganization, run: ./reorganize_commits.sh" 