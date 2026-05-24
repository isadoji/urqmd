#!/usr/bin/env bash
# Runs one UrQMD job in an isolated work directory.
# Usage: run_urqmd.sh INPUTFILE WORKDIR
set -euo pipefail

INPUTFILE="$(readlink -f "${1:?Falta INPUTFILE}")"
WORKDIR="${2:?Falta WORKDIR}"

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
source "${REPO_DIR}/config.sh"

if [[ ! -f "${INPUTFILE}" ]]; then
    echo "ERROR: inputfile not found: ${INPUTFILE}" >&2; exit 1
fi
if [[ ! -x "${URQMD_BIN}" ]]; then
    echo "ERROR: binary not found or not executable: ${URQMD_BIN}" >&2; exit 1
fi

mkdir -p "${WORKDIR}"
WORKDIR="$(readlink -f "${WORKDIR}")"

TARGET="${WORKDIR}/inputfile"
if [[ "$(readlink -f "${INPUTFILE}")" != "$(readlink -f "${TARGET}")" ]]; then
    cp "${INPUTFILE}" "${TARGET}"
fi

# ftn09 (input) and ftn13 (tables) are always required
export ftn09="${WORKDIR}/inputfile"
export ftn13="${WORKDIR}/urqmd.f13"

# Export only the ftn units listed in OUTPUTS so UrQMD writes only those files.
# Note: ftn14 being set forces UrQMD to write f14 regardless of the input file.
_want() { echo "${OUTPUTS}" | grep -qw "$1"; }
_want f14 && export ftn14="${WORKDIR}/urqmd.f14" || true
_want f15 && export ftn15="${WORKDIR}/urqmd.f15" || true
_want f16 && export ftn16="${WORKDIR}/urqmd.f16" || true
_want f19 && export ftn19="${WORKDIR}/urqmd.f19" || true
_want f20 && export ftn20="${WORKDIR}/urqmd.f20" || true

export OMP_NUM_THREADS=1

echo "[$(date +%T)] Starting job in ${WORKDIR}  (OUTPUTS=${OUTPUTS})"
cd "${WORKDIR}"
"${URQMD_BIN}" < /dev/null
echo "[$(date +%T)] Done: ${WORKDIR}"
