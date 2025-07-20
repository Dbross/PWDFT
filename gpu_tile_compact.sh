#!/bin/bash

# Simple GPU tile script for Aurora testing
# This script assigns GPU tiles to MPI ranks

# Get MPI rank from environment
MPI_RANK=${OMPI_COMM_WORLD_RANK:-0}
MPI_SIZE=${OMPI_COMM_WORLD_SIZE:-1}

# Calculate GPU and tile assignment
# Aurora has 6 GPUs with 2 tiles each (0.0, 0.1, 1.0, 1.1, 2.0, 2.1, etc.)
GPU_ID=$((MPI_RANK / 2))
TILE_ID=$((MPI_RANK % 2))

# Set GPU affinity mask
export ZE_AFFINITY_MASK="${GPU_ID}.${TILE_ID}"

echo "Rank $MPI_RANK: Using GPU $GPU_ID, Tile $TILE_ID (ZE_AFFINITY_MASK=$ZE_AFFINITY_MASK)"

# Execute the command
exec "$@" 