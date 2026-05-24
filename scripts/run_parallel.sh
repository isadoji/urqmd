#!/usr/bin/env bash
# Genera N_JOBS inputs con semillas distintas y los corre en paralelo.
# Número de procesos simultáneos controlado por N_WORKERS (en config.sh).
set -euo pipefail

REPO_DIR="$(cd "$(dirname "$0")/.." && pwd)"
source "${REPO_DIR}/config.sh"

OUTDIR="${REPO_DIR}/output/${RUN_TAG}"
mkdir -p "${OUTDIR}"

echo "============================================="
echo "Sistema : ${SPECIES}+${SPECIES}  sqrt(s)=${ECM} GeV"
echo "Central.: ${CENT_LABEL}  b=[${IMP_MIN}, ${IMP_MAX}] fm"
echo "Jobs    : ${N_JOBS} × ${NEV} ev = $((N_JOBS * NEV)) eventos totales"
echo "Workers : ${N_WORKERS} paralelos"
echo "Salida  : ${OUTDIR}"
echo "============================================="

_run_job() {
    local job_id
    job_id="$(printf '%04d' "$1")"
    local seed=$(( SEED_BASE + $1 ))
    local workdir="${OUTDIR}/run_${job_id}"
    local inputfile="${workdir}/inputfile"

    mkdir -p "${workdir}"
    "${REPO_DIR}/scripts/gen_input.sh" "${inputfile}" "${seed}"
    "${REPO_DIR}/scripts/run_urqmd.sh" "${inputfile}" "${workdir}"
}
export -f _run_job
export REPO_DIR OUTDIR SEED_BASE

# GNU Parallel si está disponible, bash puro en caso contrario
if command -v parallel &>/dev/null; then
    seq 0 $(( N_JOBS - 1 )) | parallel -j "${N_WORKERS}" _run_job {}
else
    running=0
    for i in $(seq 0 $(( N_JOBS - 1 ))); do
        _run_job "$i" &
        running=$(( running + 1 ))
        if (( running >= N_WORKERS )); then
            wait -n 2>/dev/null || wait
            running=$(( running - 1 ))
        fi
    done
    wait
fi

echo "Todos los jobs terminados. Archivos en: ${OUTDIR}"
