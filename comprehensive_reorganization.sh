#!/bin/bash

# PWDFT Comprehensive Repository Reorganization Script
# This script performs a two-phase reorganization:
# Phase 1: Isolate all documentation and CI commits from entire history
# Phase 2: Categorize recent commits (last 7 days) into feature branches

set -e  # Exit on any error

echo "=== PWDFT Comprehensive Repository Reorganization Script ==="
echo "Starting two-phase reorganization..."
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

# Function to create and initialize a branch
create_branch() {
    local branch_name=$1
    echo "Creating branch: $branch_name"
    git checkout master
    git checkout -b $branch_name
    git push origin $branch_name
    echo "✓ Created and pushed $branch_name branch"
    echo
}

# Create backup branch before making changes
echo "=== Creating backup branch ==="
BACKUP_BRANCH="backup_improved_SCF_$(date +%Y%m%d_%H%M%S)"
git checkout -b $BACKUP_BRANCH
git push origin $BACKUP_BRANCH
echo "Created backup branch: $BACKUP_BRANCH"
echo

# Get all commits on improved_SCF that are not on master (entire history)
echo "=== Phase 1: Global Reorganization by Content ==="
echo "Identifying all commits on improved_SCF that are not on master..."
ALL_COMMITS=$(git log --oneline improved_SCF --not master --format="%H")

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

echo "=== Phase 1a: Isolating Documentation Commits ==="
create_branch "add-comprehensive-documentation"

# Process all commits for documentation content
echo "Analyzing all commits for documentation content..."
while read -r commit_hash; do
    commit_msg=$(git log --oneline -1 $commit_hash)
    
    # Check for documentation content
    if check_commit_content $commit_hash "doc/\|README\|\.rst\|\.md" || \
       check_commit_content $commit_hash "documentation\|tutorial\|installation" || \
       check_commit_content $commit_hash "formatting\|syntax\|instructions" || \
       check_commit_content $commit_hash "\.txt\|\.html\|\.css\|\.js" || \
       check_commit_content $commit_hash "api/\|developers/\|faq/\|introduction/\|theory/\|tutorials/"; then
        
        echo "Found documentation commit: $commit_msg"
        DOC_COMMITS="$DOC_COMMITS $commit_hash"
        ALREADY_MOVED_COMMITS="$ALREADY_MOVED_COMMITS $commit_hash"
        
        # Cherry-pick to documentation branch
        cherry_pick_to_branch $commit_hash "add-comprehensive-documentation"
    fi
done <<< "$ALL_COMMITS"

echo "=== Phase 1b: Isolating CI & Testing Commits ==="
create_branch "autoCI"

# Process all commits for CI/testing content
echo "Analyzing all commits for CI/testing content..."
while read -r commit_hash; do
    commit_msg=$(git log --oneline -1 $commit_hash)
    
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
        
        # Cherry-pick to CI branch
        cherry_pick_to_branch $commit_hash "autoCI"
    fi
done <<< "$ALL_COMMITS"

echo "=== Phase 2: Reclassify Recent Feature Commits ==="
echo "Identifying commits from last 7 days..."

# Get commits from last 7 days that are not on master
RECENT_COMMITS=$(git log --oneline --since="7 days ago" improved_SCF --not master --format="%H")

if [ -z "$RECENT_COMMITS" ]; then
    echo "No commits found from last 7 days on improved_SCF that are not on master"
else
    echo "Found $(echo "$RECENT_COMMITS" | wc -l) recent commits to categorize"
    echo
    
    echo "=== Phase 2a: Categorizing Recent Commits ==="
    
    # Process recent commits for feature categorization
    while read -r commit_hash; do
        commit_msg=$(git log --oneline -1 $commit_hash)
        
        # Check if already moved in Phase 1
        if echo "$ALREADY_MOVED_COMMITS" | grep -q "$commit_hash"; then
            echo "Skipping already moved commit: $commit_msg"
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
    
    echo "=== Phase 2b: Moving Recent Commits to Feature Branches ==="
    
    # Move recent wf_initialization commits
    if [ -n "$RECENT_WF_INIT_COMMITS" ]; then
        echo "Moving recent wf_initialization commits..."
        create_branch "wf_initialization"
        
        for commit in $RECENT_WF_INIT_COMMITS; do
            cherry_pick_to_branch $commit "wf_initialization"
        done
    fi
    
    # Move recent improved_SCF commits
    if [ -n "$RECENT_IMPROVED_SCF_COMMITS" ]; then
        echo "Moving recent improved_SCF commits..."
        create_branch "improved_SCF_new"
        
        for commit in $RECENT_IMPROVED_SCF_COMMITS; do
            cherry_pick_to_branch $commit "improved_SCF_new"
        done
    fi
fi

echo "=== Final Cleanup ==="

# Reset original improved_SCF branch to master
echo "Resetting improved_SCF branch to master..."
git checkout improved_SCF
git reset --hard master

# If we created a new improved_SCF branch, merge it
if [ -n "$RECENT_IMPROVED_SCF_COMMITS" ]; then
    echo "Merging recent SCF commits back to improved_SCF..."
    git merge improved_SCF_new --no-edit
    git push origin improved_SCF --force
    echo "✓ Merged recent SCF commits to improved_SCF"
else
    git push origin improved_SCF --force
    echo "✓ Reset improved_SCF to master"
fi

# Count commits in each category
DOC_COUNT=$(echo $DOC_COMMITS | wc -w)
CI_COUNT=$(echo $CI_COMMITS | wc -w)
RECENT_WF_COUNT=$(echo $RECENT_WF_INIT_COMMITS | wc -w)
RECENT_SCF_COUNT=$(echo $RECENT_IMPROVED_SCF_COMMITS | wc -w)

echo
echo "=== Reorganization Complete ==="
echo "Summary:"
echo "- Backup branch created: $BACKUP_BRANCH"
echo "- add-comprehensive-documentation: $DOC_COUNT commits moved (all history)"
echo "- autoCI: $CI_COUNT commits moved (all history)"
echo "- wf_initialization: $RECENT_WF_COUNT commits moved (recent only)"
echo "- improved_SCF: $RECENT_SCF_COUNT commits retained (recent only)"
echo "- symmetry_k_points: Unchanged (as requested)"
echo
echo "All branches are now ready for CI validation against master branch."
echo "You can create pull requests for each feature branch."
echo
echo "Note: The original improved_SCF branch has been reset to master and"
echo "contains only the recent SCF-related commits." 