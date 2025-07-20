# PWDFT State-Based Repository Reorganization

This directory contains scripts for a state-based reorganization of the PWDFT repository, designed to create clean, focused branches for documentation and testing while filtering feature branches to contain only core code changes.

## Overview

The reorganization follows a two-phase approach:

### Phase 1: Create State-Based Branches
- **docs_final_state**: Contains the final state of all documentation files in a single commit
- **ci_final_state**: Contains the final state of all testing infrastructure in a single commit

### Phase 2: Rebuild Feature Branches
- **improved_SCF**: Rebuilt to contain only commits with core code changes
- **wf_initialization**: Rebuilt to contain only commits with core code changes

## Scripts

### 1. `state_based_analysis.sh`
**Purpose**: Preview the reorganization without making changes
**Usage**: `./state_based_analysis.sh`
**Output**: Shows which files and commits will be affected

### 2. `state_based_reorganization.sh`
**Purpose**: Perform the complete state-based reorganization
**Usage**: `./state_based_reorganization.sh`
**Requirements**: Must be run from the `improved_SCF` branch

## Analysis Results

Based on the state-based analysis, the reorganization will result in:

### Phase 1 - State-Based Branches
- **docs_final_state**: Single commit with final state of all documentation files
- **ci_final_state**: Single commit with final state of all testing infrastructure

### Phase 2 - Feature Branch Rebuilding
- **improved_SCF**: 35 commits (core code only, 28 docs/test commits filtered out)
- **wf_initialization**: 10 commits (core code only, 0 docs/test commits filtered out)

### Final State
- **symmetry_k_points**: Unchanged (as requested)

## File Classification Rules

### Documentation Files
Files matching these patterns are classified as documentation:
- `docs/` directory
- Files ending in `.md` or `.rst`
- Files containing `README`
- Files containing `documentation`, `tutorial`, `installation`
- Files ending in `.txt`, `.html`, `.css`, `.js`

### Testing Files
Files matching these patterns are classified as testing:
- `tests/` directory
- `run_tests.sh` script
- Files containing `aurora`
- Files ending in `.sh`, `.yml`, `.yaml`, `.json`
- Files containing `test_`, `_test`, `CI`, `validation`
- Files containing `AURORA_TEST`, `test_suite`, `runner`
- Files containing `gpu_tile`, `SYCL`, `device`
- Files containing `tier[0-9]`, `github`, `gitlab`, `workflow`, `pipeline`

### Core Code Files
All other files not matching the documentation or testing patterns above.

## Key Features

### State-Based Approach
- **Single Commits**: Documentation and testing branches contain single commits with final state
- **Clean History**: No complex commit history in state-based branches
- **Easy Review**: Simple to understand what each branch contains

### Intelligent Filtering
- **Commit Analysis**: Analyzes each commit to determine if it contains only docs/test files
- **Selective Cherry-picking**: Only keeps commits with core code changes
- **Preserves History**: Maintains chronological order of core code commits

### Safety Features
1. **Backup Branches**: Creates timestamped backups before any changes
2. **Error Handling**: Exits on any error with automatic cherry-pick abort
3. **Branch Validation**: Ensures correct starting branch
4. **Conflict Detection**: Handles cherry-pick conflicts gracefully
5. **Temporary Branches**: Uses temporary branches for rebuilding

## Workflow

### 1. Analysis Phase (Recommended)
```bash
./state_based_analysis.sh
```
Review the output to understand what will happen.

### 2. Reorganization Phase
```bash
./state_based_reorganization.sh
```

This will:
- Create backup branches for `improved_SCF` and `wf_initialization`
- **Phase 1a**: Create `docs_final_state` branch with final documentation state
- **Phase 1b**: Create `ci_final_state` branch with final testing state
- **Phase 2a**: Rebuild `improved_SCF` with core code commits only
- **Phase 2b**: Rebuild `wf_initialization` with core code commits only
- Clean up temporary branches
- Push all changes to origin

## Key Benefits

### Clean Separation
- **Documentation**: All documentation in one state-based branch
- **Testing**: All testing infrastructure in one state-based branch
- **Core Code**: Feature branches contain only relevant code changes

### Simplified Review Process
- **State-based branches**: Easy to review final state
- **Feature branches**: Clean commit history for code review
- **Clear boundaries**: No mixed content in any branch

### CI-Ready Structure
- **Focused branches**: Each branch has a single, clear purpose
- **Clean history**: Easy to understand what each branch contains
- **Proper isolation**: No cross-contamination between concerns

## Post-Reorganization Actions

1. **Create Pull Requests**:
   - `docs_final_state` → master
   - `ci_final_state` → master
   - `improved_SCF` → master
   - `wf_initialization` → master

2. **Review Strategy**:
   - **State-based branches**: Review final state, not commit history
   - **Feature branches**: Review commit history for code changes
   - **Dependencies**: Consider merge order based on dependencies

3. **Testing Strategy**:
   - Test each branch independently
   - Verify no functionality is lost
   - Ensure proper isolation

## Comparison with Previous Approaches

### State-Based vs. Commit-Based
- **State-based**: Single commits with final state (simpler review)
- **Commit-based**: Full commit history (more detailed history)

### Filtering vs. Categorization
- **Filtering**: Removes unwanted commits (cleaner feature branches)
- **Categorization**: Moves commits to appropriate branches (preserves all history)

## Troubleshooting

### Cherry-pick Conflicts
If conflicts occur during rebuilding:
1. Script will abort the cherry-pick
2. Manually resolve conflicts in temporary branch
3. Complete cherry-pick manually
4. Continue with script

### Wrong File Classification
If files are misclassified:
1. Review classification rules in script
2. Modify patterns as needed
3. Re-run analysis to verify
4. Proceed with reorganization

### Backup Recovery
If something goes wrong:
1. Use backup branches: `backup_improved_SCF_YYYYMMDD_HHMMSS`
2. Reset branches: `git reset --hard backup_improved_SCF_YYYYMMDD_HHMMSS`
3. Force push: `git push origin <branch> --force`

## Notes

- The `symmetry_k_points` branch is intentionally left unchanged
- State-based branches start fresh from master
- Feature branches are rebuilt from master with filtered commits
- Commits are processed in chronological order
- Temporary branches are automatically cleaned up

## Expected Outcome

After reorganization, you'll have:
- **State-based branches** with final documentation and testing state
- **Clean feature branches** containing only core code changes
- **Proper separation of concerns** (docs, CI, features)
- **CI-ready branches** for validation against master
- **Simplified review process** for pull requests 