# UrQMD — Resumen del proyecto

**Repo:** `https://github.com/isadoji/urqmd` · Local: `/home/isadoji/github/urqmd`

## Qué hace

Genera eventos de colisiones Bi+Bi a √sNN = 11 GeV con UrQMD 4.0 y computa observables (pT, η, φ) con tres back-ends de análisis: Python, ROOT y HDF5 para ML.

---

## Estado actual (último commit: `905ad80`)

Todo funciona y está probado. Flujo completo operativo:

```
config.sh → run_parallel.sh → output/*/urqmd.f14 → [A] plot_observables.py
                                                 → [B] urqmd2root.C → plot_observables.C
                                                 → [C] build_dataset.py → HDF5
```

---

## Archivos clave

| Archivo | Rol |
|---------|-----|
| `config.sh` | Parámetros: especie, energía, centralidad, N_JOBS |
| `scripts/run_parallel.sh` | Lanza N_JOBS en paralelo |
| `input/template.inp` | Plantilla UrQMD con `@...@` |
| `analysis/read_f14.py` | Parser f14; `iter_events`/`iter_all` (generadores, RAM constante) |
| `analysis/plot_observables.py` | Plots Python; `--select`, `--charged-only`, `--pid` |
| `analysis/urqmd2root.C` | f14 → ROOT TTree en `output/<TAG>/<TAG>.root` |
| `analysis/plot_observables.C` | Plots ROOT |
| `analysis/build_dataset.py` | HDF5: 212 features/evento (pT+η+φ histos), batch write |
| `patches/Linux.mk.patch` | Fix gfortran ≥10 para compilar UrQMD 3.4 (no necesario para 4.0) |

---

## Comandos de uso

### Generar eventos
```bash
# Editar config.sh, luego:
./scripts/run_parallel.sh
# Salida: output/Bi_11GeV_MB/run_0000..run_NNNN/urqmd.f14
```

### Análisis A — Python
```bash
python3 analysis/plot_observables.py output/Bi_11GeV_MB --select participants --charged-only
```

### Análisis B — ROOT (`root_env`)
```bash
root -l -b -q 'analysis/urqmd2root.C("output/Bi_11GeV_MB")'
root -l -b -q 'analysis/plot_observables.C("output/Bi_11GeV_MB/Bi_11GeV_MB.root","participants",true)'
```

### Análisis C — HDF5 para ML (`ml` env, h5py instalado)
```bash
python3 analysis/build_dataset.py output/Bi_11GeV_MB features_MB.h5 --select participants --charged-only
```
```python
import h5py, numpy as np
with h5py.File("features_MB.h5", "r") as h5:
    X = np.hstack([h5["pt_hist"][:], h5["eta_hist"][:], h5["phi_hist"][:]])  # (N, 212)
    b = h5["b"][:]   # target de regresión
```

---

## Detalles técnicos críticos

- **f14 formato:** 19 columnas · `ncl > 0` = participantes · `ncl == 0` = espectadores
- **bf-flag invertido:** poner `fXX` en el input de UrQMD *suprime* esa salida. `gen_input.sh` inserta las unidades no pedidas para evitar que Fortran cree `fort.15/16/19/20`
- **Binario:** `urqmd-4.0/urqmd.x86_64` — no está en git, compilado localmente (Linux.mk ya incluye los flags para gfortran ≥10)
- **`output/`** — en `.gitignore`, no se sube

---

## Conda environments

| Env | Para qué |
|-----|----------|
| `root_env` | `urqmd2root.C`, `plot_observables.C` (ROOT 6.38) |
| `ml` | `build_dataset.py`, modelos ML (h5py, numpy 2.3) |

Activar con: `/home/isadoji/Software/miniconda3/envs/<env>/bin/python` o `<env>/bin/root`

---

## Próximos pasos sugeridos

1. Generar estadística grande (1M+ eventos por centralidad) ajustando `N_JOBS` en `config.sh`
2. Correr `build_dataset.py` por centralidad (0-20%, 20-40%, 60-80%, MB)
3. Entrenar modelo ML (regresión de `b` o clasificación de centralidad) con los HDF5
