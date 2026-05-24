"""
Lector de archivos .f14 / .f15 de UrQMD.

Formato de línea de partícula (23 columnas):
  t  X  Y  Z  E  Px  Py  Pz  m  ityp  2i3  chg  lcl#  ncl  or
  frezeT  frezeX  frezeY  frezeZ  frezeE  frezePx  frezePy  frezePz  counter

Observables calculados:
  pT  = sqrt(Px^2 + Py^2)
  P   = sqrt(Px^2 + Py^2 + Pz^2)
  eta = 0.5 * ln((P + Pz) / (P - Pz))
  phi = atan2(Py, Px)
"""

import numpy as np
from pathlib import Path


_COLS = [
    "t", "X", "Y", "Z", "E", "Px", "Py", "Pz", "m",
    "ityp", "iso", "chg", "lcl", "ncl", "hist",
    "frezeT", "frezeX", "frezeY", "frezeZ",
    "frezeE", "frezePx", "frezePy", "frezePz", "counter",
]


def _derived(particles):
    """Añade pT, P, eta, phi a un dict de arrays numpy."""
    px, py, pz = particles["Px"], particles["Py"], particles["Pz"]
    pT = np.sqrt(px**2 + py**2)
    P  = np.sqrt(px**2 + py**2 + pz**2)
    with np.errstate(divide="ignore", invalid="ignore"):
        eta = np.where(np.abs(P - pz) > 1e-10,
                       0.5 * np.log((P + pz) / (P - pz)),
                       np.nan)
    phi = np.arctan2(py, px)
    particles.update({"pT": pT, "P": P, "eta": eta, "phi": phi})
    return particles


def read_events(filepath, max_events=None):
    """
    Lee un archivo .f14 o .f15 y devuelve una lista de eventos.
    Cada evento es un dict con arrays numpy de las 23 columnas + pT, eta, phi.
    Incluye 'b' (parámetro de impacto del evento).

    Parameters
    ----------
    filepath : str | Path
    max_events : int | None — límite de eventos a leer (None = todos)

    Returns
    -------
    list[dict]
    """
    filepath = Path(filepath)
    events = []
    current_particles = {c: [] for c in _COLS}
    b_val = np.nan
    in_event = False

    with open(filepath) as f:
        for line in f:
            c = line[0] if line else ""

            if c == "U":   # cabecera principal (empieza nuevo evento)
                if in_event and current_particles["E"]:
                    p = {k: np.array(v, dtype=np.float64)
                         for k, v in current_particles.items()}
                    p["b"] = b_val
                    events.append(_derived(p))
                    if max_events and len(events) >= max_events:
                        return events
                current_particles = {c: [] for c in _COLS}
                b_val = np.nan
                in_event = True

            elif c == "i":  # línea con parámetro de impacto
                try:
                    b_val = float(line[37:42])
                except (ValueError, IndexError):
                    pass

            elif c in ("p", "t", "e", "o"):  # cabeceras, saltar
                continue

            elif in_event and line.strip() and c not in ("#", "\n"):
                parts = line.split()
                if len(parts) >= 23:
                    try:
                        for col, val in zip(_COLS, parts[:24]):
                            current_particles[col].append(float(val))
                    except ValueError:
                        pass

    # último evento
    if in_event and current_particles["E"]:
        p = {k: np.array(v, dtype=np.float64)
             for k, v in current_particles.items()}
        p["b"] = b_val
        events.append(_derived(p))

    return events


def read_all(output_dir, pattern="*/urqmd.f1[45]", max_events=None):
    """
    Lee todos los archivos .f14 o .f15 en output_dir y concatena los eventos.
    """
    files = sorted(Path(output_dir).glob(pattern))
    if not files:
        files = sorted(Path(output_dir).glob("*/urqmd.f15"))
    if not files:
        files = sorted(Path(output_dir).glob("*/urqmd.f14"))
    if not files:
        raise FileNotFoundError(f"No se encontraron archivos f14/f15 en {output_dir}")

    all_events = []
    for fp in files:
        evs = read_events(fp, max_events=max_events)
        all_events.extend(evs)
        if max_events and len(all_events) >= max_events:
            break
    print(f"Leídos {len(all_events)} eventos de {len(files)} archivos")
    return all_events
