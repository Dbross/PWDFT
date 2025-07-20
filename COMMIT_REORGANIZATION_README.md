# PWDFT Commit Reorganization Scripts

This directory contains scripts to reorganize commits from the `improved_SCF` branch into their appropriate feature branches based on commit content analysis.

## Overview

The scripts analyze commits from the last 7 days on the `improved_SCF` branch and categorize them into the following branches:

- **wf_initialization**: Wavefunction initialization methods and related changes
- **add-comprehensive-documentation**: Documentation, docstrings, comments, and README files
- **autoCI**: Testing infrastructure, new tests, and CI configuration files
- **improved_SCF**: SCF convergence algorithms, energy minimizers, and core SCF improvements
- **symmetry_k_points**: Unchanged (as requested)

## Scripts

### 1. `simple_analysis.sh`
**Purpose**: Analyze commits without making any changes
**Usage**: `./simple_analysis.sh`
**Output**: Shows which commits will be moved to which branches

### 2. `reorganize_commits.sh`
**Purpose**: Perform the actual commit reorganization
**Usage**: `./reorganize_commits.sh`
**Requirements**: Must be run from the `improved_SCF` branch

## Commit Categorization Rules

### wf_initialization
Commits containing:
- `wavefunction`, `psi`, `cpsi`, `atomic`, `restart`, `fallback`
- Files in `Nwpw/band/lib/cpsi` or `Nwpw/pspw/lib/psi`
- `atomic keyword`, `restart wavefunction`

### add-comprehensive-documentation
Commits containing:
- `doc/`, `README`, `.rst`, `.md`
- `documentation`, `tutorial`, `installation`
- `formatting`, `syntax`, `instructions`

### autoCI
Commits containing:
- `test/`, `.sh`, `aurora`, `CI`, `validation`
- `AURORA_TEST`, `test_suite`, `runner`
- `gpu_tile`, `SYCL`, `device`
- `tier[0-9]`, `run_tests`

### improved_SCF
Commits containing:
- `SCF`, `convergence`, `minimizer`, `adaptive`, `diagonalization`
- `energy`, `optimization`, `algorithm`, `preconditioning`
- `Local-TF`, `enhanced`, `system-aware`, `defaults`
- `NaN`, `fallback`, `recovery`, `robust`

## Safety Features

1. **Backup Branch**: Creates a timestamped backup branch before making changes
2. **Error Handling**: Exits on any error with `set -e`
3. **Branch Validation**: Ensures you're on the correct branch before proceeding
4. **Cherry-pick Abort**: Automatically aborts failed cherry-picks

## Workflow

1. **Analysis Phase** (Recommended):
   ```bash
   ./simple_analysis.sh
   ```
   Review the output to ensure commits are categorized correctly.

2. **Reorganization Phase**:
   ```bash
   ./reorganize_commits.sh
   ```
   This will:
   - Create a backup branch
   - Categorize all commits
   - Cherry-pick commits to appropriate branches
   - Reset `improved_SCF` to master
   - Cherry-pick SCF-related commits back to `improved_SCF`
   - Push all changes to origin

## Expected Results

Based on the analysis, the reorganization will result in:

- **wf_initialization**: 16 commits
- **add-comprehensive-documentation**: 22 commits  
- **autoCI**: 17 commits
- **improved_SCF**: 6 commits
- **symmetry_k_points**: Unchanged

## Post-Reorganization

After running the reorganization script:

1. **Create Pull Requests**: Each branch is now ready for CI validation against master
2. **Review Changes**: Verify that commits are in their correct branches
3. **Test Branches**: Run tests on each branch to ensure functionality
4. **Merge Strategy**: Consider the order of merging based on dependencies

## Troubleshooting

### Cherry-pick Conflicts
If cherry-picking fails due to conflicts:
1. The script will abort the cherry-pick
2. Manually resolve conflicts in the target branch
3. Complete the cherry-pick manually
4. Continue with the script

### Wrong Categorization
If commits are categorized incorrectly:
1. Review the categorization rules in the script
2. Modify the patterns as needed
3. Re-run the analysis script to verify
4. Proceed with reorganization

### Backup Branch
If something goes wrong:
1. The backup branch contains the original state
2. Reset branches to the backup: `git reset --hard backup_improved_SCF_YYYYMMDD_HHMMSS`
3. Push the reset: `git push origin <branch> --force`

## Notes

- The `symmetry_k_points` branch is intentionally left unchanged
- All branches are reset to master before cherry-picking to ensure clean state
- The script uses force push for the final `improved_SCF` branch update
- Commits are processed in chronological order (oldest first) 