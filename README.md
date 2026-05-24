# Observables en NICA — UrQMD local con paralelismo

Generación de eventos Bi+Bi a 11 GeV con UrQMD y cálculo de observables
(pT, η, φ) usando procesamiento paralelo local.

---

## Rutas locales

| Recurso | Ruta |
|---------|------|
| Binario UrQMD (normal) | `/home/isadoji/Software/UrQMD/urqmd-3.4-lhc/urqmd.x86_64` |
| Binario UrQMD (LHC) | `/home/isadoji/Software/UrQMD/urqmd-3.4-lhc/urqmd.x86_64.lhc` |
| Runs de referencia (Bi) | `/home/isadoji/Software/UrQMD/urqmd_runs/Bi9MB/` |
| Scripts de referencia | `/home/isadoji/github/nica/Bfield/` |

---

## Sistemas y centralidades

| Sistema | √sNN | Pro (A, Z) | Tar (A, Z) | Modo binario |
|---------|------|-----------|-----------|--------------|
| Bi+Bi | 11 GeV | 209, 83 | 209, 83 | `normal` |
| Au+Au | 200 GeV | 197, 79 | 197, 79 | `normal` |
| Pb+Pb | 5020 GeV | 208, 82 | 208, 82 | `lhc` |

**Parámetro de impacto para Bi+Bi** (`b_c = 14.2 · √(fracción)` fm):

| Centralidad | IMP_MIN | IMP_MAX |
|-------------|---------|---------|
| 0–10% | 0.0 | 4.5 |
| 0–20% | 0.0 | 6.4 |
| 20–40% | 6.4 | 9.0 |
| 40–60% | 9.0 | 11.0 |
| 60–80% | 11.0 | 12.8 |

---

## Estructura del repositorio

```
urqmd/
├── config.sh                  # <-- editar aquí: energía, especie, centralidad
├── input/
│   └── template.inp           # plantilla con marcadores @...@
├── scripts/
│   ├── gen_input.sh           # genera inputfile concreto (sustituye @...@)
│   ├── run_urqmd.sh           # corre un job de UrQMD
│   └── run_parallel.sh        # lanza N_JOBS en paralelo
├── analysis/
│   ├── read_f14.py            # lector de .f14 / .f15
│   └── plot_observables.py    # histogramas de pT, η, φ
└── output/                    # creado en tiempo de corrida (.gitignored)
    └── Bi_11GeV_0-20/
        ├── run_0000/
        │   ├── inputfile
        │   ├── urqmd.f15
        │   └── ...
        └── run_0001/ ...
```

---

## Flujo de trabajo

```
1. Editar config.sh           →  especie, energía, centralidad, estadística
2. ./scripts/run_parallel.sh  →  genera inputs y corre N_JOBS en paralelo
3. python3 analysis/plot_observables.py output/Bi_11GeV_0-20  →  pT, η, φ
```

---

## Paso 1: Editar config.sh

Variables principales:

```bash
PRO_A=209  PRO_Z=83   # proyectil: Bismuto-209
TAR_A=209  TAR_Z=83   # blanco
ECM=11                # √sNN [GeV]

CENT_LABEL="0-20"
IMP_MIN=0.0
IMP_MAX=6.4

NEV=10        # eventos por job
N_JOBS=4      # jobs totales  →  40 eventos
N_WORKERS=4   # procesos paralelos (≤ núcleos disponibles)
```

Con `TIM_TOTAL=200  TIM_STEP=200` el `.f15` contiene solo el estado final
(una instantánea, más rápido). Para evolución temporal usar `TIM_STEP=1`.

---

## Paso 2: Generar eventos

```bash
cd /home/isadoji/github/urqmd
chmod +x scripts/*.sh
./scripts/run_parallel.sh
```

El script:
1. Lee `config.sh`.
2. Llama a `gen_input.sh` para crear un inputfile por job (semilla única).
3. Lanza `run_urqmd.sh` en paralelo con `N_WORKERS` procesos simultáneos.
4. Cada job escribe su salida en `output/<RUN_TAG>/run_NNNN/`.

Usa GNU Parallel si está instalado; de lo contrario usa bash puro (`& + wait`).

---

## Paso 3: Graficar pT, η, φ

```bash
# Todas las partículas
python3 analysis/plot_observables.py output/Bi_11GeV_0-20

# Solo partículas cargadas
python3 analysis/plot_observables.py output/Bi_11GeV_0-20 --charged-only

# Filtrar por tipo de partícula (ityp interno de UrQMD)
python3 analysis/plot_observables.py output/Bi_11GeV_0-20 --pid 101 --charged-only
```

Los plots se guardan en `output/<RUN_TAG>/plots/`:

```
pt_Bi_11GeV_0-20.png
eta_Bi_11GeV_0-20.png
phi_Bi_11GeV_0-20.png
```

---

## Formato del archivo de salida .f14 / .f15

Cabecera de evento:
```
UQMD version ...
projectile: (mass, char) A Z   target: (mass, char) A Z
impact_parameter_real/min/max(fm): b bmin bmax  total_cross_section: σ
event# N  random seed: S  total_time: T  Delta(t): dt
...
pvec: r0 rx ry rz p0 px py pz m ityp 2i3 chg lcl# ncl or
     npart  nstep
```

Línea de partícula (23 columnas):
```
t  X  Y  Z  E  Px  Py  Pz  m  ityp  2i3  chg  lcl#  ncl  or
frezeT  frezeX  frezeY  frezeZ  frezeE  frezePx  frezePy  frezePz  counter
```

Observables calculados desde los momentos:
```
pT  = sqrt(Px² + Py²)
P   = sqrt(Px² + Py² + Pz²)
η   = 0.5 · ln((P + Pz) / (P − Pz))
φ   = atan2(Py, Px)
```

---

## Referencia de parámetros del input de UrQMD

| Parámetro | Descripción | Ejemplo |
|-----------|-------------|---------|
| `pro A Z` | proyectil (masa, carga) | `pro 209 83` |
| `tar A Z` | blanco | `tar 209 83` |
| `nev N` | eventos por corrida | `nev 10` |
| `imp bmin bmax` | rango b [fm] | `imp 0.0 6.4` |
| `ecm E` | √sNN [GeV] | `ecm 11` |
| `tim T dt` | tiempo total y paso [fm/c] | `tim 200 200` |
| `eos 0` | EOS hadrónica | — |
| `rsd S` | semilla aleatoria | `rsd 12345` |
| `cto 41 1` | activa string formation | — |
| `f14` / `f15` | habilitar salidas | `f15` (estado final) |

---

## Dependencias

- UrQMD 3.4 — `/home/isadoji/Software/UrQMD/urqmd-3.4-lhc/`
- Python ≥ 3.8: `numpy`, `matplotlib`
- GNU Parallel (opcional; el script funciona sin él)
- ROOT ≥ 6.0 (opcional, para análisis adicional con macros `.C`)

## Referencias

- Bass et al., Prog. Part. Nucl. Phys. 41, 255 (1998)
- Bleicher et al., J. Phys. G 25, 1859 (1999)
- Skokov et al., PRC 80, 034902 (2009)
- Bzdak & Skokov, PLB 710, 171 (2012)
