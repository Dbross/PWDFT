#!/bin/bash

# PWDFT Comprehensive Analysis Script
# This script analyzes the two-phase reorganization without making changes

echo "=== PWDFT Comprehensive Analysis Script ==="
echo "Analyzing two-phase reorganization..."
echo

# Function to check if a commit contains specific content patterns
check_commit_content() {
    local commit_hash=$1
    local pattern=$2
    git show --stat $commit_hash | grep -q "$pattern" || git show --name-only $commit_hash | grep -q "$pattern"
}

# Get all commits on improved_SCF that are not on master (entire history)
echo "=== Phase 1: Global Reorganization Analysis ==="
echo "Identifying all commits on improved_SCF that are not on master..."
ALL_COMMITS=$(git log --oneline improved_SCF --not master)

if [ -z "$ALL_COMMITS" ]; then
    echo "No commits found on improved_SCF that are not on master"
    exit 0
fi

echo "Found $(echo "$ALL_COMMITS" | wc -l) commits to analyze"
echo

# Initialize commit tracking
DOC_COMMITS=""
CI_COMMITS=""
RECENT_WF_INIT_COMMITS=""
RECENT_IMPROVED_SCF_COMMITS=""
ALREADY_MOVED_COMMITS=""

echo "=== Phase 1a: Documentation Commits Analysis ==="

# Process all commits for documentation content
while read -r line; do
    commit_hash=$(echo "$line" | cut -d' ' -f1)
    commit_msg=$(echo "$line" | cut -d' ' -f2-)
    
    # Check for documentation content
    if check_commit_content $commit_hash "doc/\|README\|\.rst\|\.md" || \
       check_commit_content $commit_hash "documentation\|tutorial\|installation" || \
       check_commit_content $commit_hash "formatting\|syntax\|instructions" || \
       check_commit_content $commit_hash "\.txt\|\.html\|\.css\|\.js" || \
       check_commit_content $commit_hash "api/\|developers/\|faq/\|introduction/\|theory/\|tutorials/"; then
        
        echo "Found documentation commit: $commit_msg"
        DOC_COMMITS="$DOC_COMMITS $commit_hash"
        ALREADY_MOVED_COMMITS="$ALREADY_MOVED_COMMITS $commit_hash"
    fi
done <<< "$ALL_COMMITS"

echo "=== Phase 1b: CI & Testing Commits Analysis ==="

# Process all commits for CI/testing content
while read -r line; do
    commit_hash=$(echo "$line" | cut -d' ' -f1)
    commit_msg=$(echo "$line" | cut -d' ' -f2-)
    
    # Check if already moved to documentation
    if echo "$ALREADY_MOVED_COMMITS" | grep -q "$commit_hash"; then
        continue
    fi
    
    # Check for CI/testing content
    if check_commit_content $commit_hash "test/\|\.sh\|aurora\|CI\|validation" || \
       check_commit_content $commit_hash "AURORA_TEST\|test_suite\|runner" || \
       check_commit_content $commit_hash "gpu_tile\|SYCL\|device" || \
       check_commit_content $commit_hash "tier[0-9]\|run_tests" || \
       check_commit_content $commit_hash "\.yml\|\.yaml\|\.json" || \
       check_commit_content $commit_hash "github\|gitlab\|workflow\|pipeline" || \
       check_commit_content $commit_hash "\.cpp\|\.hpp\|\.c\|\.h" || \
       check_commit_content $commit_hash "test_\|_test\|spec_\|_spec"; then
        
        echo "Found CI/testing commit: $commit_msg"
        CI_COMMITS="$CI_COMMITS $commit_hash"
        ALREADY_MOVED_COMMITS="$ALREADY_MOVED_COMMITS $commit_hash"
    fi
done <<< "$ALL_COMMITS"

echo "=== Phase 2: Recent Feature Commits Analysis ==="
echo "Identifying commits from last 7 days..."

# Get commits from last 7 days that are not on master
RECENT_COMMITS=$(git log --oneline --since="7 days ago" improved_SCF --not master)

if [ -z "$RECENT_COMMITS" ]; then
    echo "No commits found from last 7 days on improved_SCF that are not on master"
else
    echo "Found $(echo "$RECENT_COMMITS" | wc -l) recent commits to categorize"
    echo
    
    # Process recent commits for feature categorization
    while read -r line; do
        commit_hash=$(echo "$line" | cut -d' ' -f1)
        commit_msg=$(echo "$line" | cut -d' ' -f2-)
        
        # Check if already moved in Phase 1
        if echo "$ALREADY_MOVED_COMMITS" | grep -q "$commit_hash"; then
            echo "Already moved in Phase 1: $commit_msg"
            continue
        fi
        
        echo "Analyzing recent commit: $commit_msg"
        
        # Check for wavefunction initialization content
        if check_commit_content $commit_hash "wavefunction\|psi\|cpsi\|atomic\|restart\|fallback" || \
           check_commit_content $commit_hash "Nwpw/band/lib/cpsi\|Nwpw/pspw/lib/psi" || \
           check_commit_content $commit_hash "atomic keyword\|restart wavefunction" || \
           check_commit_content $commit_hash "Fermi energy\|band calculation\|Pneb"; then
            echo "  → Categorized as wf_initialization"
            RECENT_WF_INIT_COMMITS="$RECENT_WF_INIT_COMMITS $commit_hash"
            
        # Check for SCF improvements content
        elif check_commit_content $commit_hash "SCF\|convergence\|minimizer\|adaptive\|diagonalization" || \
             check_commit_content $commit_hash "energy\|optimization\|algorithm\|preconditioning" || \
             check_commit_content $commit_hash "Local-TF\|enhanced\|system-aware\|defaults" || \
             check_commit_content $commit_hash "NaN\|fallback\|recovery\|robust" || \
             check_commit_content $commit_hash "Methfessel-Paxton\|smearing\|mixing"; then
            echo "  → Categorized as improved_SCF"
            RECENT_IMPROVED_SCF_COMMITS="$RECENT_IMPROVED_SCF_COMMITS $commit_hash"
            
        else
            # Default to improved_SCF for unclear cases
            echo "  → Categorized as improved_SCF (default)"
            RECENT_IMPROVED_SCF_COMMITS="$RECENT_IMPROVED_SCF_COMMITS $commit_hash"
        fi
        echo
    done <<< "$RECENT_COMMITS"
fi

# Count commits in each category
DOC_COUNT=$(echo $DOC_COMMITS | wc -w)
CI_COUNT=$(echo $CI_COMMITS | wc -w)
RECENT_WF_COUNT=$(echo $RECENT_WF_INIT_COMMITS | wc -w)
RECENT_SCF_COUNT=$(echo $RECENT_IMPROVED_SCF_COMMITS | wc -w)

echo "=== Summary ==="
echo
echo "Phase 1 - Global Reorganization:"
echo "Documentation commits to be moved (all history): $DOC_COUNT"
echo "CI/Testing commits to be moved (all history): $CI_COUNT"
echo
echo "Phase 2 - Recent Feature Commits:"
echo "Recent wf_initialization commits: $RECENT_WF_COUNT"
echo "Recent improved_SCF commits: $RECENT_SCF_COUNT"
echo
echo "=== Detailed Breakdown ==="
echo

echo "Documentation commits ($DOC_COUNT):"
for commit in $DOC_COMMITS; do
    git log --oneline -1 $commit
done
echo

echo "CI/Testing commits ($CI_COUNT):"
for commit in $CI_COMMITS; do
    git log --oneline -1 $commit
done
echo

echo "Recent wf_initialization commits ($RECENT_WF_COUNT):"
for commit in $RECENT_WF_INIT_COMMITS; do
    git log --oneline -1 $commit
done
echo

echo "Recent improved_SCF commits ($RECENT_SCF_COUNT):"
for commit in $RECENT_IMPROVED_SCF_COMMITS; do
    git log --oneline -1 $commit
done
echo

echo "=== Final Branch State ==="
echo "After reorganization:"
echo "- add-comprehensive-documentation: Will contain $DOC_COUNT commits (all history)"
echo "- autoCI: Will contain $CI_COUNT commits (all history)"
echo "- wf_initialization: Will contain $RECENT_WF_COUNT commits (recent only)"
echo "- improved_SCF: Will contain $RECENT_SCF_COUNT commits (recent only)"
echo "- symmetry_k_points: Unchanged (as requested)"
echo
echo "To proceed with reorganization, run: ./comprehensive_reorganization.sh" 