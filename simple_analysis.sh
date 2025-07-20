#!/bin/bash

# Simple PWDFT Commit Analysis Script
# This script analyzes commits from improved_SCF branch and shows where they will be moved

echo "=== PWDFT Commit Analysis Script ==="
echo "Analyzing commits from last 7 days on improved_SCF branch..."
echo

# Get list of commits from last 7 days that are not on master
COMMITS=$(git log --oneline --since="7 days ago" improved_SCF --not master)

if [ -z "$COMMITS" ]; then
    echo "No commits found from last 7 days on improved_SCF that are not on master"
    exit 0
fi

echo "Found commits to reorganize:"
echo "$COMMITS"
echo

# Function to check if a commit contains specific content patterns
check_commit_content() {
    local commit_hash=$1
    local pattern=$2
    git show --stat $commit_hash | grep -q "$pattern" || git show --name-only $commit_hash | grep -q "$pattern"
}

echo "=== Detailed Analysis ==="
echo

WF_INIT_COMMITS=""
DOC_COMMITS=""
AUTOCI_COMMITS=""
IMPROVED_SCF_COMMITS=""

# Process each commit
while read -r line; do
    commit_hash=$(echo "$line" | cut -d' ' -f1)
    commit_msg=$(echo "$line" | cut -d' ' -f2-)
    
    echo "Analyzing: $commit_msg"
    
    # Check for wavefunction initialization content
    if check_commit_content $commit_hash "wavefunction\|psi\|cpsi\|atomic\|restart\|fallback" || \
       check_commit_content $commit_hash "Nwpw/band/lib/cpsi\|Nwpw/pspw/lib/psi" || \
       check_commit_content $commit_hash "atomic keyword\|restart wavefunction"; then
        echo "  → wf_initialization"
        WF_INIT_COMMITS="$WF_INIT_COMMITS $commit_hash"
        
    # Check for documentation content
    elif check_commit_content $commit_hash "doc/\|README\|\.rst\|\.md" || \
         check_commit_content $commit_hash "documentation\|tutorial\|installation" || \
         check_commit_content $commit_hash "formatting\|syntax\|instructions"; then
        echo "  → add-comprehensive-documentation"
        DOC_COMMITS="$DOC_COMMITS $commit_hash"
        
    # Check for CI/testing content
    elif check_commit_content $commit_hash "test/\|\.sh\|aurora\|CI\|validation" || \
         check_commit_content $commit_hash "AURORA_TEST\|test_suite\|runner" || \
         check_commit_content $commit_hash "gpu_tile\|SYCL\|device" || \
         check_commit_content $commit_hash "tier[0-9]\|run_tests"; then
        echo "  → autoCI"
        AUTOCI_COMMITS="$AUTOCI_COMMITS $commit_hash"
        
    # Check for SCF improvements content
    elif check_commit_content $commit_hash "SCF\|convergence\|minimizer\|adaptive\|diagonalization" || \
         check_commit_content $commit_hash "energy\|optimization\|algorithm\|preconditioning" || \
         check_commit_content $commit_hash "Local-TF\|enhanced\|system-aware\|defaults" || \
         check_commit_content $commit_hash "NaN\|fallback\|recovery\|robust"; then
        echo "  → improved_SCF"
        IMPROVED_SCF_COMMITS="$IMPROVED_SCF_COMMITS $commit_hash"
        
    else
        # Default to improved_SCF for unclear cases
        echo "  → improved_SCF (default)"
        IMPROVED_SCF_COMMITS="$IMPROVED_SCF_COMMITS $commit_hash"
    fi
    echo
done <<< "$COMMITS"

echo "=== Summary ==="
echo

# Count commits in each category
WF_INIT_COUNT=$(echo $WF_INIT_COMMITS | wc -w)
DOC_COUNT=$(echo $DOC_COMMITS | wc -w)
AUTOCI_COUNT=$(echo $AUTOCI_COMMITS | wc -w)
IMPROVED_SCF_COUNT=$(echo $IMPROVED_SCF_COMMITS | wc -w)

echo "Commits to be moved to wf_initialization ($WF_INIT_COUNT):"
for commit in $WF_INIT_COMMITS; do
    git log --oneline -1 $commit
done
echo

echo "Commits to be moved to add-comprehensive-documentation ($DOC_COUNT):"
for commit in $DOC_COMMITS; do
    git log --oneline -1 $commit
done
echo

echo "Commits to be moved to autoCI ($AUTOCI_COUNT):"
for commit in $AUTOCI_COMMITS; do
    git log --oneline -1 $commit
done
echo

echo "Commits to remain on improved_SCF ($IMPROVED_SCF_COUNT):"
for commit in $IMPROVED_SCF_COMMITS; do
    git log --oneline -1 $commit
done
echo

echo "=== Final Branch State ==="
echo "After reorganization:"
echo "- wf_initialization: Will contain $WF_INIT_COUNT commits from last 7 days"
echo "- add-comprehensive-documentation: Will contain $DOC_COUNT commits from last 7 days"
echo "- autoCI: Will contain $AUTOCI_COUNT commits from last 7 days"
echo "- improved_SCF: Will contain $IMPROVED_SCF_COUNT commits from last 7 days"
echo "- symmetry_k_points: Unchanged (as requested)"
echo
echo "To proceed with reorganization, run: ./reorganize_commits.sh" 