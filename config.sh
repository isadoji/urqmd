#!/usr/bin/env bash
# =============================================================================
# config.sh -- UrQMD simulation parameters
# Edit here to change species, energy, centrality, statistics, etc.
# =============================================================================

# --- Colliding species ---
# Bismuth-209:  PRO_A=209  PRO_Z=83
# Gold-197:     PRO_A=197  PRO_Z=79
# Lead-208:     PRO_A=208  PRO_Z=82
PRO_A=209
PRO_Z=83
TAR_A=209
TAR_Z=83
SPECIES="Bi"

# --- Center-of-mass energy [GeV] ---
ECM=11          # sqrt(sNN): NICA range 4-11, RHIC=200, LHC=5020

# --- Impact parameter [fm] ---
# Minimum bias (b sampled 0..16 weighted by geometry):
#   CENT_LABEL="MB"   IMP="-16.0"
#
# Fixed centrality range (Bi+Bi b_max~14.2 fm, b_c=14.2*sqrt(fraction)):
#   0-10%: "0.0 4.5"   0-20%: "0.0 6.4"   20-40%: "6.4 9.0"
#   40-60%: "9.0 11.0"   60-80%: "11.0 12.8"
CENT_LABEL="MB"
IMP="-16.0"

# --- UrQMD output files ---
# Space-separated list from: f14 f15 f16 f19 f20
#
# f14  particle snapshot at each time step (requires cto 41 1, added automatically)
#      With tim T T (single step) this IS the final state. Use for pT/eta/phi.
# f15  collision-by-collision history (before/after each interaction).
#      Useful for collision studies; NOT the final-state particle list.
# f16  reaction cross-section table
# f19/f20  debug / diagnostics
OUTPUTS="f14"

# --- Statistics ---
NEV=10            # events per job
N_JOBS=4         # independent jobs  ->  total = N_JOBS x NEV
N_WORKERS=4      # parallel processes (set <= available cores)

SEED_BASE=12345  # base seed; job i uses SEED_BASE + i

# --- Simulation time [fm/c] ---
# Final state only:   TIM_TOTAL=200  TIM_STEP=200  (fast)
# Time evolution:     TIM_TOTAL=200  TIM_STEP=1    (200 snapshots)
TIM_TOTAL=200
TIM_STEP=200

# --- UrQMD binary ---
# normal mode -> Au, Bi (nmax=40000)
# lhc mode    -> Pb at LHC energies (nmax=100000)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
URQMD_DIR="${SCRIPT_DIR}/urqmd-3.4"
URQMD_BIN="${URQMD_DIR}/urqmd.x86_64"
# URQMD_BIN="${URQMD_DIR}/urqmd.x86_64.lhc"   # use for Pb+Pb LHC

# --- Run tag (names the output directories) ---
RUN_TAG="${SPECIES}_${ECM}GeV_${CENT_LABEL}"
