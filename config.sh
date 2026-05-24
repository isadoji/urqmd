#!/usr/bin/env bash
# =============================================================================
# config.sh — parámetros de simulación UrQMD
# Edita aquí para cambiar sistema, energía, centralidad, estadística, etc.
# =============================================================================

# --- Especie colisionante ---
# Bismuto-209:  PRO_A=209  PRO_Z=83
# Oro-197:      PRO_A=197  PRO_Z=79
# Plomo-208:    PRO_A=208  PRO_Z=82
PRO_A=209
PRO_Z=83
TAR_A=209
TAR_Z=83
SPECIES="Bi"

# --- Energía del centro de masa [GeV] ---
ECM=11          # sqrt(sNN): NICA range 4-11, RHIC=200, LHC=5020

# --- Centralidad: rango de parámetro de impacto [fm] ---
# Bi+Bi b_max ≈ 14.2 fm  →  b_c = 14.2·sqrt(fracción)
#   0-10%:  0.0 -  4.5      20-40%:  6.4 -  9.0
#   0-20%:  0.0 -  6.4      40-60%:  9.0 - 11.0
#                            60-80%: 11.0 - 12.8
CENT_LABEL="0-20"
IMP_MIN=0.0
IMP_MAX=6.4

# --- Estadística ---
NEV=1           # eventos por job
N_JOBS=1         # jobs independientes  →  total = N_JOBS × NEV
N_WORKERS=4      # procesos paralelos simultáneos (≤ núcleos disponibles)

SEED_BASE=12345  # semilla base; job i usa SEED_BASE + i

# --- Tiempo de simulación [fm/c] ---
# Estado final únicamente:  TIM_TOTAL=200  TIM_STEP=200  (rápido)
# Evolución temporal:       TIM_TOTAL=200  TIM_STEP=1    (genera 200 snapshots)
TIM_TOTAL=200
TIM_STEP=200

# --- Binario de UrQMD ---
# Modo normal → Au, Bi (nmax=40000)
# Modo lhc    → Pb a energías LHC (nmax=100000)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
URQMD_DIR="${SCRIPT_DIR}/urqmd-3.4"
URQMD_BIN="${URQMD_DIR}/urqmd.x86_64"
# URQMD_BIN="${URQMD_DIR}/urqmd.x86_64.lhc"   # usar para Pb+Pb LHC

# --- Etiqueta de corrida (nombra los directorios de salida) ---
RUN_TAG="${SPECIES}_${ECM}GeV_${CENT_LABEL}"
