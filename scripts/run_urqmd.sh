#!/usr/bin/env bash
# Corre un job de UrQMD en un directorio de trabajo aislado.
# Uso: run_urqmd.sh INPUTFILE WORKDIR
set -euo pipefail

INPUTFILE="$(readlink -f "${1:?Falta INPUTFILE}")"
WORKDIR="${2:?Falta WORKDIR}"

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
source "${REPO_DIR}/config.sh"

if [[ ! -f "${INPUTFILE}" ]]; then
    echo "ERROR: no existe ${INPUTFILE}" >&2; exit 1
fi
if [[ ! -x "${URQMD_BIN}" ]]; then
    echo "ERROR: binario no encontrado o no ejecutable: ${URQMD_BIN}" >&2; exit 1
fi

mkdir -p "${WORKDIR}"
WORKDIR="$(readlink -f "${WORKDIR}")"

cp "${INPUTFILE}" "${WORKDIR}/inputfile"

export ftn09="${WORKDIR}/inputfile"
export ftn13="${WORKDIR}/urqmd.f13"
export ftn14="${WORKDIR}/urqmd.f14"
export ftn15="${WORKDIR}/urqmd.f15"
export ftn16="${WORKDIR}/urqmd.f16"
export ftn19="${WORKDIR}/urqmd.f19"
export ftn20="${WORKDIR}/urqmd.f20"
export OMP_NUM_THREADS=1

echo "[$(date +%T)] Iniciando job en ${WORKDIR}"
cd "${WORKDIR}"
"${URQMD_BIN}" < /dev/null
echo "[$(date +%T)] Terminado: ${WORKDIR}"
