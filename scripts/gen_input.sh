#!/usr/bin/env bash
# Generates a concrete inputfile by replacing @...@ placeholders.
# Usage: gen_input.sh OUTFILE SEED
set -euo pipefail

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
source "${REPO_DIR}/config.sh"

OUTFILE="${1:?missing OUTFILE}"
SEED="${2:?missing SEED}"

# cto 41 1 enables extended f14 output; only needed when f14 is requested.
# In UrQMD, listing "fXX" in the input SUPPRESSES that unit (bf flag logic).
# Output is controlled exclusively via ftn* env vars in run_urqmd.sh.
if echo "${OUTPUTS}" | grep -qw "f14"; then
    CTO41="cto 41 1"
else
    CTO41=""
fi

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
    -e "s/@CTO41@/${CTO41}/g" \
    "${REPO_DIR}/input/template.inp" > "${OUTFILE}"
