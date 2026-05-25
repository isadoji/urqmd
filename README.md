# UrQMD — event generator for heavy-ion collisions at NICA energies

Generates Bi+Bi events at 11 GeV with UrQMD 4.0 and computes
observables (pT, eta, phi) using local parallel processing.
The repository is self-contained: clone it, compile, and run.

---

## Build UrQMD

### UrQMD 4.0 (default)

The UrQMD 4.0 source tar is not stored in this repository.
Download it from the official site and place it at the repo root:

```
https://itp.uni-frankfurt.de/~bleicher/urqmddownload/urqmd-4.0.tar.gz
```

Then build:

```bash
git clone https://github.com/isadoji/urqmd.git
cd urqmd

# 1. Place urqmd-4.0.tar.gz here, then extract
tar xzf urqmd-4.0.tar.gz          # creates urqmd-4.0/

# 2. Compile (UrQMD 4.0 mk/Linux.mk already includes gfortran >= 10 flags)
cd urqmd-4.0 && make
# -> urqmd-4.0/urqmd.x86_64

cd ..
```

`config.sh` automatically picks up `urqmd-4.0/urqmd.x86_64` relative
to the repository root — no path editing required after cloning.

> **Note:** UrQMD 4.0 does not support the `lhc` compilation mode (the target was
> removed in this release). For Pb+Pb at LHC energies, use UrQMD 3.4 (see below).

---

### UrQMD 3.4 (alternative)

If you need the 3.4 release (e.g., for LHC-mode or reproducibility):

```
http://urqmd.org/download/urqmd-3.4.tar
```

```bash
# 1. Extract
tar xf urqmd-3.4.tar          # creates urqmd-3.4/

# 2. Apply the gfortran >= 10 patch
patch urqmd-3.4/mk/Linux.mk patches/Linux.mk.patch

# 3. Compile (normal mode — Bi+Bi, Au+Au)
cd urqmd-3.4 && make
# -> urqmd-3.4/urqmd.x86_64

# 4. Compile LHC mode (Pb+Pb, nmax=100000)
make lhc
# -> urqmd-3.4/urqmd.x86_64.lhc

cd ..
```

To switch `config.sh` to 3.4, change the `URQMD_DIR` line:

```bash
URQMD_DIR="${SCRIPT_DIR}/urqmd-3.4"
# URQMD_BIN="${URQMD_DIR}/urqmd.x86_64.lhc"   # use for Pb+Pb LHC
```

#### Patch: `patches/Linux.mk.patch`

The original `GNUmakefile` does not compile with gfortran >= 10.
`patches/Linux.mk.patch` adds the required flags to `mk/Linux.mk`:

```makefile
FFLAGS = -O3 -mcmodel=medium -std=legacy -fallow-argument-mismatch -ffixed-line-length-none
```

| Flag | Reason |
|------|--------|
| `-std=legacy` | Allows obsolete Fortran extensions used in UrQMD |
| `-fallow-argument-mismatch` | Suppresses type-mismatch errors in mixed integer/real calls |
| `-ffixed-line-length-none` | Allows fixed-form Fortran lines of arbitrary length |

---

## Repository layout

```
urqmd/
├── config.sh                  # <-- edit here: species, energy, centrality
├── input/
│   └── template.inp           # UrQMD input template with @...@ placeholders
├── scripts/
│   ├── gen_input.sh           # fills template -> concrete inputfile per job
│   ├── run_urqmd.sh           # runs one UrQMD job (ftn* env vars)
│   └── run_parallel.sh        # launches N_JOBS in parallel
├── analysis/
│   ├── read_f14.py            # parser for .f14 output; iter_events/iter_all generators
│   ├── plot_observables.py    # pT, eta, phi histograms via matplotlib
│   ├── build_dataset.py       # HDF5 feature builder for ML (constant RAM, any scale)
│   ├── urqmd2root.C           # converts .f14 → ROOT TTree (compatible with CalcBfield)
│   └── plot_observables.C     # pT, eta, phi histograms via ROOT
├── patches/
│   └── Linux.mk.patch         # gfortran >= 10 fix for urqmd-3.4/mk/Linux.mk (not needed for 4.0)
└── urqmd-4.0/                 # NOT in git — extract tar and compile
└── output/                    # created at runtime (.gitignored)
    └── Bi_11GeV_0-20/
        └── run_0000/
            ├── inputfile
            ├── urqmd.f14
            └── plots/
```

---

## Workflow

```
config.sh  ->  run_parallel.sh  ->  output/*/urqmd.f14  ->  plot_observables.py   (Python plots)
 (edit)         (generate + run N jobs)  (final state)    |-> urqmd2root.C         (ROOT TTree)
                                                          |     -> plot_observables.C  (ROOT plots)
                                                          `-> build_dataset.py     (HDF5 for ML)
```

---

## Step 1: Edit config.sh

```bash
PRO_A=209  PRO_Z=83   # projectile: Bismuth-209
TAR_A=209  TAR_Z=83   # target
ECM=11                # sqrt(sNN) [GeV]

CENT_LABEL="0-20"
IMP_MIN=0.0
IMP_MAX=6.4

NEV=10        # events per job
N_JOBS=4      # independent jobs  ->  40 events total
N_WORKERS=4   # parallel processes (set <= available cores)
```

### Supported systems

| System | sqrt(sNN) | Pro (A, Z) | Tar (A, Z) | Binary mode |
|--------|-----------|-----------|-----------|-------------|
| Bi+Bi  | 11 GeV    | 209, 83   | 209, 83   | `normal`    |
| Au+Au  | 200 GeV   | 197, 79   | 197, 79   | `normal`    |
| Pb+Pb  | 5020 GeV  | 208, 82   | 208, 82   | `lhc`       |

LHC mode (Pb+Pb at 5020 GeV) requires UrQMD 3.4 — see the alternative build section above.

### Impact parameter for Bi+Bi (b_max = 14.2 fm)

Set `IMP` in `config.sh` — single negative value for minimum bias,
two positive values for a fixed centrality range:

```bash
# Minimum bias: b sampled from 0 to 16 fm weighted by geometry
CENT_LABEL="MB"  ;  IMP="-16.0"

# Fixed centrality range [fm]
CENT_LABEL="0-20"  ;  IMP="0.0 6.4"
```

**Fixed centrality ranges for Bi+Bi** (b_max = 14.2 fm):

| Centrality | IMP            |
|------------|----------------|
| MB         | `-16.0`        |
| 0–10%      | `0.0 4.5`      |
| 0–20%      | `0.0 6.4`      |
| 20–40%     | `6.4 9.0`      |
| 40–60%     | `9.0 11.0`     |
| 60–80%     | `11.0 12.8`    |

Formula: `b_c = 14.2 * sqrt(fraction)` fm.

---

## Step 2: Generate events

```bash
chmod +x scripts/*.sh
./scripts/run_parallel.sh
```

The script:
1. Reads `config.sh`.
2. Calls `gen_input.sh` to produce one inputfile per job (unique seed).
3. Launches `run_urqmd.sh` with `N_WORKERS` simultaneous processes.
4. Each job writes output to `output/<RUN_TAG>/run_NNNN/`.

Uses GNU Parallel if available; falls back to plain bash (`& + wait`).

---

## Step 3: Analysis

Three back-ends are available. Choose based on your goal:

| | Python (matplotlib) | ROOT (CERN) | HDF5 (ML) |
|---|---|---|---|
| **Script** | `plot_observables.py` | `urqmd2root.C` + `plot_observables.C` | `build_dataset.py` |
| **Input** | `output/*/urqmd.f14` | `output/*/urqmd.f14` → `.root` | `output/*/urqmd.f14` |
| **Output** | PNG histograms | PNG + PDF + ROOT TTree | HDF5 feature matrix |
| **Best for** | Quick checks | Standard HEP analysis, CalcBfield | ML training (1M+ events) |
| **RAM usage** | All events in memory | TTree lazy read | Constant (batch write) |
| **Requires** | `numpy`, `matplotlib` | ROOT >= 6 (`root_env`) | `h5py` (`ml` env) |

### Particle selection (all three back-ends)

| `--select` / `select` | Condition | Description |
|-----------------------|-----------|-------------|
| `all` (default) | — | All final-state particles |
| `participants` | `ncl > 0` | Nucleons that had at least one collision |
| `spectators` | `ncl == 0` | Nucleons that did not collide |

---

### Option A — Python / matplotlib

Reads `.f14` files directly. Best for quick exploratory plots.

```bash
# All particles
python3 analysis/plot_observables.py output/Bi_11GeV_0-20

# Participants, charged only
python3 analysis/plot_observables.py output/Bi_11GeV_0-20 \
    --select participants --charged-only

# Spectators, charged only
python3 analysis/plot_observables.py output/Bi_11GeV_0-20 \
    --select spectators --charged-only

# Filter by UrQMD particle type (ityp)
python3 analysis/plot_observables.py output/Bi_11GeV_0-20 --pid 101
```

Plots saved to `output/<RUN_TAG>/plots/` — filename includes active filters:

```
pt_Bi_11GeV_0-20.png
pt_Bi_11GeV_0-20_participants_cargadas.png
eta_Bi_11GeV_0-20.png
phi_Bi_11GeV_0-20.png
```

---

### Option B — ROOT / CERN

Two-step: convert `.f14` to a ROOT TTree, then plot. The TTree is compatible
with `CalcBfield_3cent.C` and any standard HEP analysis macro.

**Step B-1: convert**

```bash
# conda root_env (ROOT >= 6)
root -l -b -q 'analysis/urqmd2root.C("output/Bi_11GeV_MB")'
# -> output/Bi_11GeV_MB/Bi_11GeV_MB.root  (TTree "T")

# Custom output path
root -l -b -q 'analysis/urqmd2root.C("output/Bi_11GeV_0-20","output/Bi_11GeV_0-20/Bi_0-20.root")'
```

**TTree "T" branch layout**

| Branch | Type | Description |
|--------|------|-------------|
| `npart` | `Int_t` | Total particles per event |
| `nspec` | `Int_t` | Spectators (ncl == 0) per event |
| `b` | `Float_t` | Impact parameter [fm] |
| `nev` | `Float_t` | Event counter |
| `time[npart]` | `Float_t[]` | Time [fm/c] |
| `X/Y/Z[npart]` | `Float_t[]` | Position [fm] |
| `E/Px/Py/Pz[npart]` | `Float_t[]` | 4-momentum [GeV] |
| `m[npart]` | `Float_t[]` | Mass [GeV] |
| `charge[npart]` | `Float_t[]` | Electric charge |
| `numbercoll[npart]` | `Float_t[]` | Collisions per nucleon; 0 = spectator |
| `ityp[npart]` | `Int_t[]` | UrQMD particle type |
| `iso[npart]` | `Int_t[]` | Isospin projection |

**Step B-2: plot**

```bash
# All particles
root -l -b -q 'analysis/plot_observables.C("output/Bi_11GeV_MB/Bi_11GeV_MB.root")'

# Participants, charged only
root -l -b -q 'analysis/plot_observables.C("output/Bi_11GeV_MB/Bi_11GeV_MB.root","participants",true)'

# Spectators, charged only
root -l -b -q 'analysis/plot_observables.C("output/Bi_11GeV_MB/Bi_11GeV_MB.root","spectators",true)'

# Filter by ityp
root -l -b -q 'analysis/plot_observables.C("output/Bi_11GeV_MB/Bi_11GeV_MB.root","all",false,101)'
```

Saves `pt_<tag>.png/pdf`, `eta_<tag>.png/pdf`, `phi_<tag>.png/pdf` in the current directory.

---

### Option C — HDF5 for ML

Reads `.f14` one event at a time (constant RAM) and writes per-event histogram
feature vectors to an HDF5 file in batches. Designed for 1M+ events.

```bash
# conda ml env (pip install h5py)

# Participants, charged only
python3 analysis/build_dataset.py output/Bi_11GeV_0-20 features_0-20.h5 \
    --select participants --charged-only

# All particles, minimum bias
python3 analysis/build_dataset.py output/Bi_11GeV_MB features_MB.h5

# Spectators, quick test
python3 analysis/build_dataset.py output/Bi_11GeV_0-20 features_spec.h5 \
    --select spectators --max-events 10000
```

**HDF5 file structure**

| Dataset | Shape | Description |
|---------|-------|-------------|
| `pt_hist`  | `(N, 60)` | pT histogram per event, 0–3 GeV/c |
| `eta_hist` | `(N, 80)` | η histogram per event, −8 to 8 |
| `phi_hist` | `(N, 72)` | φ histogram per event, −π to π |
| `b`        | `(N,)`    | Impact parameter [fm] — regression target |
| `pt_edges` / `eta_edges` / `phi_edges` | 1-D | Bin boundaries |

Total: **212 features per event** (60 + 80 + 72), gzip-compressed.

**Load for ML training**

```python
import h5py, numpy as np

with h5py.File("features_0-20.h5", "r") as h5:
    X = np.hstack([h5["pt_hist"][:], h5["eta_hist"][:], h5["phi_hist"][:]])
    b = h5["b"][:]   # regression target

# Batched loading for PyTorch / Keras (avoids loading full file)
with h5py.File("features_0-20.h5", "r") as h5:
    for i in range(0, len(h5["b"]), 1000):
        X_batch = np.hstack([h5["pt_hist"][i:i+1000],
                             h5["eta_hist"][i:i+1000],
                             h5["phi_hist"][i:i+1000]])
```

**Options**

| Flag | Default | Description |
|------|---------|-------------|
| `--select` | `all` | `all` / `participants` / `spectators` |
| `--charged-only` | off | Charged particles only |
| `--pid N` | off | Filter by UrQMD ityp |
| `--batch N` | 500 | Events per disk flush |
| `--max-events N` | all | Stop after N events |

---

## UrQMD output files

Output is controlled by `OUTPUTS` in `config.sh`.
In UrQMD, listing `fXX` in the input file **suppresses** that unit (`bf` flag logic);
output is enabled by exporting the corresponding `ftnXX` environment variable.
`gen_input.sh` and `run_urqmd.sh` handle this automatically based on `OUTPUTS`.

| File | Content | Use case |
|------|---------|----------|
| `f14` | Particle snapshot at each time step (requires `cto 41 1`). With `tim T T` this is the final state. | **pT / eta / phi** |
| `f15` | Collision-by-collision history (before and after each interaction). | Collision studies |
| `f16` | Reaction cross-section table. | Cross-section checks |
| `f19/f20` | Debug and diagnostics. | Debugging |

### f14 format (19 columns per particle line)

```
t  X  Y  Z  E  Px  Py  Pz  m  ityp  iso  chg  lcl#  ncl  hist  frezeT  frezeX  step  counter
```

Observables computed from momenta:

```
pT  = sqrt(Px^2 + Py^2)
P   = sqrt(Px^2 + Py^2 + Pz^2)
eta = 0.5 * ln((P + Pz) / (P - Pz))
phi = atan2(Py, Px)
```

---

## UrQMD input reference

| Parameter    | Description                        | Example       |
|--------------|------------------------------------|---------------|
| `pro A Z`    | projectile (mass, charge)          | `pro 209 83`  |
| `tar A Z`    | target                             | `tar 209 83`  |
| `nev N`      | events per run                     | `nev 10`      |
| `imp -b`     | minimum bias (b ~ 0 to b, weighted)| `imp -16.0`   |
| `imp b1 b2`  | fixed centrality range [fm]        | `imp 0.0 6.4` |
| `ecm E`      | sqrt(sNN) [GeV]                    | `ecm 11`      |
| `tim T dt`   | total time and step [fm/c]         | `tim 200 200` |
| `eos 0`      | hadronic equation of state         | —             |
| `rsd S`      | random seed                        | `rsd 12345`   |
| `cto 41 1`   | extended f14 output (auto-added when f14 is in OUTPUTS) | —  |

---

## Dependencies

| Component | Requirement | Used by |
|-----------|-------------|---------|
| gfortran >= 10 | compile UrQMD | `make` |
| Python >= 3.8: `numpy`, `matplotlib` | Option A plots | `plot_observables.py` |
| Python >= 3.8: `h5py` | Option C ML dataset | `build_dataset.py` |
| ROOT >= 6.0 (`root_env`) | Option B ROOT analysis | `urqmd2root.C`, `plot_observables.C` |
| GNU Parallel | parallel job launch | `run_parallel.sh` (optional, falls back to bash) |

## References

- Bass et al., Prog. Part. Nucl. Phys. 41, 255 (1998)
- Bleicher et al., J. Phys. G 25, 1859 (1999)
- Skokov et al., PRC 80, 034902 (2009)
- Bzdak & Skokov, PLB 710, 171 (2012)
