#!/bin/bash

# PWDFT Commit Reorganization Script
# This script reorganizes commits from improved_SCF branch into appropriate feature branches
# based on commit content analysis from the last 7 days.

set -e  # Exit on any error

echo "=== PWDFT Commit Reorganization Script ==="
echo "Starting reorganization of commits from improved_SCF branch..."
echo

# Store current branch
CURRENT_BRANCH=$(git branch --show-current)
echo "Current branch: $CURRENT_BRANCH"

# Ensure we're on improved_SCF branch
if [ "$CURRENT_BRANCH" != "improved_SCF" ]; then
    echo "Error: Must be on improved_SCF branch to run this script"
    echo "Current branch: $CURRENT_BRANCH"
    exit 1
fi

# Get list of commits from last 7 days that are not on master
echo "Identifying commits from last 7 days..."
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

# Function to cherry-pick a commit to a branch
cherry_pick_to_branch() {
    local commit_hash=$1
    local target_branch=$2
    local commit_msg=$(git log --oneline -1 $commit_hash)
    
    echo "Cherry-picking: $commit_msg to $target_branch"
    
    # Checkout target branch
    git checkout $target_branch
    
    # Ensure branch is up to date with master
    git reset --hard master
    
    # Cherry-pick the commit
    if git cherry-pick $commit_hash; then
        echo "✓ Successfully cherry-picked to $target_branch"
    else
        echo "✗ Failed to cherry-pick to $target_branch - manual intervention may be needed"
        git cherry-pick --abort
        return 1
    fi
    
    # Push to origin
    git push origin $target_branch
    echo
}

# Initialize commit categorization strings
WF_INIT_COMMITS=""
DOC_COMMITS=""
AUTOCI_COMMITS=""
IMPROVED_SCF_COMMITS=""

echo "=== Categorizing commits ==="

# Process each commit and categorize it
while read -r line; do
    commit_hash=$(echo "$line" | cut -d' ' -f1)
    commit_msg=$(echo "$line" | cut -d' ' -f2-)
    
    echo "Analyzing: $commit_msg"
    
    # Check for wavefunction initialization content
    if check_commit_content $commit_hash "wavefunction\|psi\|cpsi\|atomic\|restart\|fallback" || \
       check_commit_content $commit_hash "Nwpw/band/lib/cpsi\|Nwpw/pspw/lib/psi" || \
       check_commit_content $commit_hash "atomic keyword\|restart wavefunction"; then
        WF_INIT_COMMITS="$WF_INIT_COMMITS $commit_hash"
        echo "  → Categorized as wf_initialization"
        
    # Check for documentation content
    elif check_commit_content $commit_hash "doc/\|README\|\.rst\|\.md" || \
         check_commit_content $commit_hash "documentation\|tutorial\|installation" || \
         check_commit_content $commit_hash "formatting\|syntax\|instructions"; then
        DOC_COMMITS="$DOC_COMMITS $commit_hash"
        echo "  → Categorized as add-comprehensive-documentation"
        
    # Check for CI/testing content
    elif check_commit_content $commit_hash "test/\|\.sh\|aurora\|CI\|validation" || \
         check_commit_content $commit_hash "AURORA_TEST\|test_suite\|runner" || \
         check_commit_content $commit_hash "gpu_tile\|SYCL\|device" || \
         check_commit_content $commit_hash "tier[0-9]\|run_tests"; then
        AUTOCI_COMMITS="$AUTOCI_COMMITS $commit_hash"
        echo "  → Categorized as autoCI"
        
    # Check for SCF improvements content
    elif check_commit_content $commit_hash "SCF\|convergence\|minimizer\|adaptive\|diagonalization" || \
         check_commit_content $commit_hash "energy\|optimization\|algorithm\|preconditioning" || \
         check_commit_content $commit_hash "Local-TF\|enhanced\|system-aware\|defaults" || \
         check_commit_content $commit_hash "NaN\|fallback\|recovery\|robust"; then
        IMPROVED_SCF_COMMITS="$IMPROVED_SCF_COMMITS $commit_hash"
        echo "  → Categorized as improved_SCF"
        
    else
        # Default to improved_SCF for unclear cases
        IMPROVED_SCF_COMMITS="$IMPROVED_SCF_COMMITS $commit_hash"
        echo "  → Categorized as improved_SCF (default)"
    fi
    echo
done <<< "$COMMITS"

# Count commits in each category
WF_INIT_COUNT=$(echo $WF_INIT_COMMITS | wc -w)
DOC_COUNT=$(echo $DOC_COMMITS | wc -w)
AUTOCI_COUNT=$(echo $AUTOCI_COMMITS | wc -w)
IMPROVED_SCF_COUNT=$(echo $IMPROVED_SCF_COMMITS | wc -w)

echo "=== Commit Categorization Summary ==="
echo "wf_initialization: $WF_INIT_COUNT commits"
echo "add-comprehensive-documentation: $DOC_COUNT commits"
echo "autoCI: $AUTOCI_COUNT commits"
echo "improved_SCF: $IMPROVED_SCF_COUNT commits"
echo

# Create backup branch before making changes
echo "=== Creating backup branch ==="
BACKUP_BRANCH="backup_improved_SCF_$(date +%Y%m%d_%H%M%S)"
git checkout -b $BACKUP_BRANCH
git push origin $BACKUP_BRANCH
echo "Created backup branch: $BACKUP_BRANCH"
echo

# Cherry-pick commits to wf_initialization branch
echo "=== Cherry-picking to wf_initialization branch ==="
for commit in $WF_INIT_COMMITS; do
    cherry_pick_to_branch $commit "wf_initialization"
done

# Cherry-pick commits to add-comprehensive-documentation branch
echo "=== Cherry-picking to add-comprehensive-documentation branch ==="
for commit in $DOC_COMMITS; do
    cherry_pick_to_branch $commit "add-comprehensive-documentation"
done

# Cherry-pick commits to autoCI branch
echo "=== Cherry-picking to autoCI branch ==="
for commit in $AUTOCI_COMMITS; do
    cherry_pick_to_branch $commit "autoCI"
done

# Reset improved_SCF branch to master and cherry-pick remaining commits
echo "=== Resetting improved_SCF branch ==="
git checkout improved_SCF
git reset --hard master

echo "=== Cherry-picking SCF-related commits to improved_SCF branch ==="
for commit in $IMPROVED_SCF_COMMITS; do
    cherry_pick_to_branch $commit "improved_SCF"
done

# Final push of improved_SCF branch
git push origin improved_SCF --force

echo
echo "=== Reorganization Complete ==="
echo "Summary:"
echo "- Backup branch created: $BACKUP_BRANCH"
echo "- wf_initialization: $WF_INIT_COUNT commits moved"
echo "- add-comprehensive-documentation: $DOC_COUNT commits moved"
echo "- autoCI: $AUTOCI_COUNT commits moved"
echo "- improved_SCF: $IMPROVED_SCF_COUNT commits retained"
echo
echo "All branches are now ready for CI validation against master branch."
echo "You can create pull requests for each feature branch."
echo
echo "Note: The symmetry_k_points branch was not modified as requested." 