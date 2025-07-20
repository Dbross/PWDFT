#!/bin/bash

# PWDFT State-Based Repository Reorganization Script
# This script creates state-based branches for documentation and testing,
# then rebuilds feature branches to contain only core code changes.

set -e  # Exit on any error

echo "=== PWDFT State-Based Repository Reorganization Script ==="
echo "Starting state-based reorganization..."
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

# Function to check if a file is documentation
is_documentation_file() {
    local file_path="$1"
    [[ "$file_path" =~ ^docs/ ]] || \
    [[ "$file_path" =~ \.(md|rst)$ ]] || \
    [[ "$file_path" =~ README ]] || \
    [[ "$file_path" =~ documentation ]] || \
    [[ "$file_path" =~ tutorial ]] || \
    [[ "$file_path" =~ installation ]] || \
    [[ "$file_path" =~ \.(txt|html|css|js)$ ]]
}

# Function to check if a file is testing
is_testing_file() {
    local file_path="$1"
    [[ "$file_path" =~ ^tests/ ]] || \
    [[ "$file_path" =~ run_tests\.sh ]] || \
    [[ "$file_path" =~ aurora ]] || \
    [[ "$file_path" =~ \.(sh|yml|yaml|json)$ ]] || \
    [[ "$file_path" =~ test_ ]] || \
    [[ "$file_path" =~ _test ]] || \
    [[ "$file_path" =~ CI ]] || \
    [[ "$file_path" =~ validation ]] || \
    [[ "$file_path" =~ AURORA_TEST ]] || \
    [[ "$file_path" =~ test_suite ]] || \
    [[ "$file_path" =~ runner ]] || \
    [[ "$file_path" =~ gpu_tile ]] || \
    [[ "$file_path" =~ SYCL ]] || \
    [[ "$file_path" =~ device ]] || \
    [[ "$file_path" =~ tier[0-9] ]] || \
    [[ "$file_path" =~ github ]] || \
    [[ "$file_path" =~ gitlab ]] || \
    [[ "$file_path" =~ workflow ]] || \
    [[ "$file_path" =~ pipeline ]]
}

# Function to check if a commit contains only documentation/testing files
commit_has_only_docs_or_tests() {
    local commit_hash="$1"
    local has_core_code=false
    
    # Get list of files modified in this commit
    local files=$(git show --name-only --pretty=format: $commit_hash)
    
    while read -r file; do
        if [ -n "$file" ]; then
            if ! is_documentation_file "$file" && ! is_testing_file "$file"; then
                has_core_code=true
                break
            fi
        fi
    done <<< "$files"
    
    ! $has_core_code
}

# Function to create backup branch
create_backup() {
    local branch_name="$1"
    local backup_name="backup_${branch_name}_$(date +%Y%m%d_%H%M%S)"
    
    echo "Creating backup of $branch_name as $backup_name"
    git checkout "$branch_name"
    git checkout -b "$backup_name"
    git push origin "$backup_name"
    echo "✓ Created backup branch: $backup_name"
    echo
}

# Create backup branches before making changes
echo "=== Creating Backup Branches ==="
create_backup "improved_SCF"
create_backup "wf_initialization"

# Phase 1: Create State-Based Branches
echo "=== Phase 1: Creating State-Based Branches ==="

# Create Documentation Branch
echo "=== Phase 1a: Creating docs_final_state Branch ==="
git checkout master
git checkout -b docs_final_state

echo "Copying documentation files from improved_SCF..."
git checkout improved_SCF -- docs/ 2>/dev/null || echo "No docs/ directory found"
git checkout improved_SCF -- *.md *.rst 2>/dev/null || echo "No .md/.rst files found"
git checkout improved_SCF -- README* 2>/dev/null || echo "No README files found"

# Check if there are any documentation files to commit
if git diff --cached --quiet; then
    echo "No documentation files found to commit"
else
    git add .
    git commit -m "feat: Add final state of all documentation"
    echo "✓ Created docs_final_state branch with documentation"
fi

git push origin docs_final_state
echo

# Create Testing Branch
echo "=== Phase 1b: Creating ci_final_state Branch ==="
git checkout master
git checkout -b ci_final_state

echo "Copying testing files from improved_SCF..."
git checkout improved_SCF -- tests/ 2>/dev/null || echo "No tests/ directory found"
git checkout improved_SCF -- run_tests.sh 2>/dev/null || echo "No run_tests.sh found"

# Find and copy aurora-related files
echo "Finding aurora-related files..."
aurora_files=$(git ls-tree -r --name-only improved_SCF | grep -i aurora || true)
if [ -n "$aurora_files" ]; then
    echo "Found aurora files: $aurora_files"
    for file in $aurora_files; do
        git checkout improved_SCF -- "$file" 2>/dev/null || echo "Could not checkout $file"
    done
fi

# Check if there are any testing files to commit
if git diff --cached --quiet; then
    echo "No testing files found to commit"
else
    git add .
    git commit -m "feat: Add final state of all testing infrastructure"
    echo "✓ Created ci_final_state branch with testing infrastructure"
fi

git push origin ci_final_state
echo

# Phase 2: Rebuild Feature Branches
echo "=== Phase 2: Rebuilding Feature Branches ==="

# Clean improved_SCF
echo "=== Phase 2a: Rebuilding improved_SCF Branch ==="
git checkout master
git checkout -b improved_SCF_rebuilt

echo "Processing commits from improved_SCF branch..."
# Get all commits on improved_SCF that are not on master
COMMITS_TO_PROCESS=$(git log --oneline improved_SCF --not master --format="%H")

if [ -n "$COMMITS_TO_PROCESS" ]; then
    echo "Found $(echo "$COMMITS_TO_PROCESS" | wc -l) commits to process"
    
    while read -r commit_hash; do
        commit_msg=$(git log --oneline -1 $commit_hash)
        
        if commit_has_only_docs_or_tests "$commit_hash"; then
            echo "Skipping docs/test-only commit: $commit_msg"
        else
            echo "Cherry-picking core code commit: $commit_msg"
            if git cherry-pick "$commit_hash"; then
                echo "✓ Successfully cherry-picked"
            else
                echo "✗ Failed to cherry-pick - aborting"
                git cherry-pick --abort
                echo "Continuing with next commit..."
            fi
        fi
    done <<< "$COMMITS_TO_PROCESS"
else
    echo "No commits found to process"
fi

# Replace original improved_SCF branch
echo "Replacing original improved_SCF branch..."
git checkout improved_SCF
git reset --hard improved_SCF_rebuilt
git push origin improved_SCF --force
echo "✓ Rebuilt improved_SCF branch with core code only"
echo

# Clean wf_initialization
echo "=== Phase 2b: Rebuilding wf_initialization Branch ==="
git checkout master
git checkout -b wf_initialization_rebuilt

echo "Processing commits from wf_initialization branch..."
# Get all commits on wf_initialization that are not on master
WF_COMMITS_TO_PROCESS=$(git log --oneline wf_initialization --not master --format="%H")

if [ -n "$WF_COMMITS_TO_PROCESS" ]; then
    echo "Found $(echo "$WF_COMMITS_TO_PROCESS" | wc -l) commits to process"
    
    while read -r commit_hash; do
        commit_msg=$(git log --oneline -1 $commit_hash)
        
        if commit_has_only_docs_or_tests "$commit_hash"; then
            echo "Skipping docs/test-only commit: $commit_msg"
        else
            echo "Cherry-picking core code commit: $commit_msg"
            if git cherry-pick "$commit_hash"; then
                echo "✓ Successfully cherry-picked"
            else
                echo "✗ Failed to cherry-pick - aborting"
                git cherry-pick --abort
                echo "Continuing with next commit..."
            fi
        fi
    done <<< "$WF_COMMITS_TO_PROCESS"
else
    echo "No commits found to process"
fi

# Replace original wf_initialization branch
echo "Replacing original wf_initialization branch..."
git checkout wf_initialization
git reset --hard wf_initialization_rebuilt
git push origin wf_initialization --force
echo "✓ Rebuilt wf_initialization branch with core code only"
echo

# Cleanup temporary branches
echo "=== Cleanup ==="
echo "Removing temporary branches..."
git branch -D improved_SCF_rebuilt 2>/dev/null || echo "improved_SCF_rebuilt already removed"
git branch -D wf_initialization_rebuilt 2>/dev/null || echo "wf_initialization_rebuilt already removed"

# Return to improved_SCF branch
git checkout improved_SCF

# Count commits in each branch
echo "=== Final Summary ==="
echo

DOCS_COMMITS=$(git log --oneline docs_final_state --not master | wc -l)
CI_COMMITS=$(git log --oneline ci_final_state --not master | wc -l)
IMPROVED_SCF_COMMITS=$(git log --oneline improved_SCF --not master | wc -l)
WF_INIT_COMMITS=$(git log --oneline wf_initialization --not master | wc -l)

echo "Reorganization Complete!"
echo "Summary:"
echo "- docs_final_state: $DOCS_COMMITS commits (state-based documentation)"
echo "- ci_final_state: $CI_COMMITS commits (state-based testing)"
echo "- improved_SCF: $IMPROVED_SCF_COMMITS commits (core code only)"
echo "- wf_initialization: $WF_INIT_COMMITS commits (core code only)"
echo "- symmetry_k_points: Unchanged (as requested)"
echo
echo "All branches are now ready for CI validation against master branch."
echo "You can create pull requests for each feature branch."
echo
echo "Note: Backup branches have been created for safety." 