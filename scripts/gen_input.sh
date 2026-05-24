#!/usr/bin/env bash
# Genera un inputfile concreto sustituyendo los marcadores @...@ de la plantilla.
# Uso: gen_input.sh OUTFILE SEED
set -euo pipefail

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
source "${REPO_DIR}/config.sh"

OUTFILE="${1:?Falta OUTFILE}"
SEED="${2:?Falta SEED}"

sed \
    -e "s/@PRO_A@/${PRO_A}/g" \
    -e "s/@PRO_Z@/${PRO_Z}/g" \
    -e "s/@TAR_A@/${TAR_A}/g" \
    -e "s/@TAR_Z@/${TAR_Z}/g" \
    -e "s/@NEV@/${NEV}/g" \
    -e "s/@IMP@/${IMP}/g" \
    -e "s/@ECM@/${ECM}/g" \
    -e "s/@TIM_TOTAL@/${TIM_TOTAL}/g" \
    -e "s/@TIM_STEP@/${TIM_STEP}/g" \
    -e "s/@SEED@/${SEED}/g" \
    "${REPO_DIR}/input/template.inp" > "${OUTFILE}"
