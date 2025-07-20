#!/bin/bash

# PWDFT State-Based Analysis Script
# This script analyzes the state-based reorganization without making changes

echo "=== PWDFT State-Based Analysis Script ==="
echo "Analyzing state-based reorganization..."
echo

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

echo "=== Phase 1: State-Based Branches Analysis ==="

# Analyze documentation files
echo "=== Phase 1a: Documentation Files Analysis ==="
echo "Files that will be included in docs_final_state:"

# Check for docs directory
if [ -d "docs" ]; then
    echo "✓ docs/ directory found"
else
    echo "✗ docs/ directory not found"
fi

# Check for markdown and rst files
md_files=$(find . -name "*.md" -o -name "*.rst" | head -10)
if [ -n "$md_files" ]; then
    echo "✓ Found .md/.rst files:"
    echo "$md_files" | head -5
    if [ $(echo "$md_files" | wc -l) -gt 5 ]; then
        echo "... and $(($(echo "$md_files" | wc -l) - 5)) more"
    fi
else
    echo "✗ No .md/.rst files found"
fi

# Check for README files
readme_files=$(find . -name "README*" | head -5)
if [ -n "$readme_files" ]; then
    echo "✓ Found README files:"
    echo "$readme_files"
else
    echo "✗ No README files found"
fi
echo

# Analyze testing files
echo "=== Phase 1b: Testing Files Analysis ==="
echo "Files that will be included in ci_final_state:"

# Check for tests directory
if [ -d "tests" ]; then
    echo "✓ tests/ directory found"
else
    echo "✗ tests/ directory not found"
fi

# Check for run_tests.sh
if [ -f "run_tests.sh" ]; then
    echo "✓ run_tests.sh found"
else
    echo "✗ run_tests.sh not found"
fi

# Check for aurora-related files
aurora_files=$(find . -name "*aurora*" -o -name "*AURORA*" | head -10)
if [ -n "$aurora_files" ]; then
    echo "✓ Found aurora-related files:"
    echo "$aurora_files" | head -5
    if [ $(echo "$aurora_files" | wc -l) -gt 5 ]; then
        echo "... and $(($(echo "$aurora_files" | wc -l) - 5)) more"
    fi
else
    echo "✗ No aurora-related files found"
fi
echo

echo "=== Phase 2: Feature Branch Rebuilding Analysis ==="

# Analyze improved_SCF branch
echo "=== Phase 2a: improved_SCF Branch Analysis ==="
COMMITS_TO_PROCESS=$(git log --oneline improved_SCF --not master --format="%H")

if [ -n "$COMMITS_TO_PROCESS" ]; then
    echo "Found $(echo "$COMMITS_TO_PROCESS" | wc -l) commits to analyze on improved_SCF"
    echo
    
    CORE_CODE_COMMITS=""
    DOCS_TEST_COMMITS=""
    
    while read -r commit_hash; do
        commit_msg=$(git log --oneline -1 $commit_hash)
        
        if commit_has_only_docs_or_tests "$commit_hash"; then
            DOCS_TEST_COMMITS="$DOCS_TEST_COMMITS $commit_hash"
            echo "Will skip (docs/test only): $commit_msg"
        else
            CORE_CODE_COMMITS="$CORE_CODE_COMMITS $commit_hash"
            echo "Will keep (core code): $commit_msg"
        fi
    done <<< "$COMMITS_TO_PROCESS"
    
    CORE_COUNT=$(echo $CORE_CODE_COMMITS | wc -w)
    DOCS_TEST_COUNT=$(echo $DOCS_TEST_COMMITS | wc -w)
    
    echo
    echo "improved_SCF summary:"
    echo "- Core code commits to keep: $CORE_COUNT"
    echo "- Docs/test commits to skip: $DOCS_TEST_COUNT"
else
    echo "No commits found on improved_SCF that are not on master"
fi
echo

# Analyze wf_initialization branch
echo "=== Phase 2b: wf_initialization Branch Analysis ==="
WF_COMMITS_TO_PROCESS=$(git log --oneline wf_initialization --not master --format="%H")

if [ -n "$WF_COMMITS_TO_PROCESS" ]; then
    echo "Found $(echo "$WF_COMMITS_TO_PROCESS" | wc -l) commits to analyze on wf_initialization"
    echo
    
    WF_CORE_CODE_COMMITS=""
    WF_DOCS_TEST_COMMITS=""
    
    while read -r commit_hash; do
        commit_msg=$(git log --oneline -1 $commit_hash)
        
        if commit_has_only_docs_or_tests "$commit_hash"; then
            WF_DOCS_TEST_COMMITS="$WF_DOCS_TEST_COMMITS $commit_hash"
            echo "Will skip (docs/test only): $commit_msg"
        else
            WF_CORE_CODE_COMMITS="$WF_CORE_CODE_COMMITS $commit_hash"
            echo "Will keep (core code): $commit_msg"
        fi
    done <<< "$WF_COMMITS_TO_PROCESS"
    
    WF_CORE_COUNT=$(echo $WF_CORE_CODE_COMMITS | wc -w)
    WF_DOCS_TEST_COUNT=$(echo $WF_DOCS_TEST_COMMITS | wc -w)
    
    echo
    echo "wf_initialization summary:"
    echo "- Core code commits to keep: $WF_CORE_COUNT"
    echo "- Docs/test commits to skip: $WF_DOCS_TEST_COUNT"
else
    echo "No commits found on wf_initialization that are not on master"
fi
echo

echo "=== Final State Preview ==="
echo "After reorganization:"
echo "- docs_final_state: Will contain final state of all documentation files"
echo "- ci_final_state: Will contain final state of all testing infrastructure"
echo "- improved_SCF: Will contain $CORE_COUNT commits (core code only)"
echo "- wf_initialization: Will contain $WF_CORE_COUNT commits (core code only)"
echo "- symmetry_k_points: Unchanged (as requested)"
echo
echo "To proceed with reorganization, run: ./state_based_reorganization.sh" 