# UrQMD — event generator for heavy-ion collisions at NICA energies

Generates Bi+Bi events at 11 GeV with UrQMD 3.4 and computes
observables (pT, eta, phi) using local parallel processing.
The repository is self-contained: clone it, compile, and run.

---

## Build UrQMD

The UrQMD 3.4 source tar (204 MB) is not stored in this repository.
Download it from the official site and place it at the repo root:

```
http://urqmd.org/download/urqmd-3.4.tar
```

Then build:

```bash
git clone https://github.com/isadoji/urqmd.git
cd urqmd

# 1. Place urqmd-3.4.tar here, then extract
tar xf urqmd-3.4.tar          # creates urqmd-3.4/

# 2. Apply the gfortran patch (fixes compilation with gfortran >= 10)
patch urqmd-3.4/mk/Linux.mk patches/Linux.mk.patch

# 3. Compile (normal mode — Bi+Bi, Au+Au)
cd urqmd-3.4 && make
# -> urqmd-3.4/urqmd.x86_64

# 4. Compile LHC mode (Pb+Pb at LHC energies, nmax=100000)
make lhc
# -> urqmd-3.4/urqmd.x86_64.lhc

cd ..
```

`config.sh` automatically picks up `urqmd-3.4/urqmd.x86_64` relative
to the repository root — no path editing required after cloning.

### Patch: `patches/Linux.mk.patch`

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
│   ├── read_f14.py            # parser for .f14 output (19-column UrQMD 3.4 format)
│   └── plot_observables.py    # pT, eta, phi histograms via matplotlib
├── patches/
│   └── Linux.mk.patch         # gfortran >= 10 fix for urqmd-3.4/mk/Linux.mk
└── urqmd-3.4/                 # NOT in git — extract tar and apply patch
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
config.sh  ->  run_parallel.sh  ->  output/*/urqmd.f14  ->  plot_observables.py
 (edit)         (generate + run N jobs)  (final state)        (pT, eta, phi)
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

For LHC mode set `URQMD_BIN="${URQMD_DIR}/urqmd.x86_64.lhc"` in `config.sh`.

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

## Step 3: Plot pT, eta, phi

```bash
# All particles
python3 analysis/plot_observables.py output/Bi_11GeV_0-20

# Charged particles only
python3 analysis/plot_observables.py output/Bi_11GeV_0-20 --charged-only

# Participants only  (ncl > 0)
python3 analysis/plot_observables.py output/Bi_11GeV_0-20 --select participants

# Spectators only   (ncl == 0)
python3 analysis/plot_observables.py output/Bi_11GeV_0-20 --select spectators --charged-only

# Filter by UrQMD internal particle type (ityp)
python3 analysis/plot_observables.py output/Bi_11GeV_0-20 --pid 101 --charged-only
```

| `--select` | Condición | Descripción |
|------------|-----------|-------------|
| `all` (default) | — | Todas las partículas |
| `participants` | `ncl > 0` | Nucleones que tuvieron al menos una colisión |
| `spectators` | `ncl == 0` | Nucleones que no colisionaron |

Plots are saved to `output/<RUN_TAG>/plots/`. The filename includes the active filters:

```
pt_Bi_11GeV_0-20.png                          # all particles
pt_Bi_11GeV_0-20_participants.png             # --select participants
pt_Bi_11GeV_0-20_spectators_cargadas.png      # --select spectators --charged-only
eta_Bi_11GeV_0-20.png
phi_Bi_11GeV_0-20.png
```

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

- gfortran >= 10 (to compile UrQMD from source)
- Python >= 3.8: `numpy`, `matplotlib`
- GNU Parallel (optional; script works without it)
- ROOT >= 6.0 (optional, for additional `.C` macro analysis)

## References

- Bass et al., Prog. Part. Nucl. Phys. 41, 255 (1998)
- Bleicher et al., J. Phys. G 25, 1859 (1999)
- Skokov et al., PRC 80, 034902 (2009)
- Bzdak & Skokov, PLB 710, 171 (2012)
