#!/usr/bin/env python3
"""
Genera histogramas de pT, eta y phi a partir de archivos .f14/.f15 de UrQMD.

Uso:
    python3 analysis/plot_observables.py output/Bi_11GeV_0-20
    python3 analysis/plot_observables.py output/Bi_11GeV_0-20 --charged-only
    python3 analysis/plot_observables.py output/Bi_11GeV_0-20 --pid 101 --charged-only
    python3 analysis/plot_observables.py output/Bi_11GeV_0-20 --select participants
    python3 analysis/plot_observables.py output/Bi_11GeV_0-20 --select spectators --charged-only
"""

import argparse
import sys
from pathlib import Path

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

sys.path.insert(0, str(Path(__file__).parent))
from read_f14 import read_all


def collect_arrays(events, charged_only=False, pid=None, select="all"):
    """
    Concatena pT, eta, phi de todos los eventos aplicando filtros.

    select : "all" | "participants" | "spectators"
        participants -> ncl > 0  (nucleones que colisionaron al menos una vez)
        spectators   -> ncl == 0 (nucleones que no colisionaron)
        all          -> sin filtro por ncl
    """
    pT_list, eta_list, phi_list = [], [], []
    for ev in events:
        mask = np.ones(len(ev["pT"]), dtype=bool)
        if select == "participants":
            mask &= (ev["ncl"] > 0)
        elif select == "spectators":
            mask &= (ev["ncl"] == 0)
        if charged_only:
            mask &= (ev["chg"] != 0)
        if pid is not None:
            mask &= (ev["ityp"] == pid)
        mask &= np.isfinite(ev["eta"])
        pT_list.append(ev["pT"][mask])
        eta_list.append(ev["eta"][mask])
        phi_list.append(ev["phi"][mask])
    return (np.concatenate(pT_list),
            np.concatenate(eta_list),
            np.concatenate(phi_list))


def plot_pt(pT, tag, outdir):
    fig, ax = plt.subplots()
    bins = np.linspace(0, 3.0, 61)
    ax.hist(pT, bins=bins, histtype="step", linewidth=1.5, color="steelblue")
    ax.set_xlabel(r"$p_T$ [GeV/c]")
    ax.set_ylabel("Partículas / bin")
    ax.set_title(f"Distribución $p_T$ — {tag}")
    ax.set_yscale("log")
    fig.tight_layout()
    path = outdir / f"pt_{tag}.png"
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"  Guardado: {path}")


def plot_eta(eta, tag, outdir):
    fig, ax = plt.subplots()
    bins = np.linspace(-8, 8, 81)
    ax.hist(eta, bins=bins, histtype="step", linewidth=1.5, color="firebrick")
    ax.set_xlabel(r"$\eta$ (pseudorapidez)")
    ax.set_ylabel("Partículas / bin")
    ax.set_title(f"Distribución $\\eta$ — {tag}")
    fig.tight_layout()
    path = outdir / f"eta_{tag}.png"
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"  Guardado: {path}")


def plot_phi(phi, tag, outdir):
    fig, ax = plt.subplots()
    bins = np.linspace(-np.pi, np.pi, 73)
    ax.hist(phi, bins=bins, histtype="step", linewidth=1.5, color="seagreen")
    ax.set_xlabel(r"$\phi$ [rad]")
    ax.set_ylabel("Partículas / bin")
    ax.set_title(f"Distribución $\\phi$ — {tag}")
    ax.set_xticks([-np.pi, -np.pi/2, 0, np.pi/2, np.pi])
    ax.set_xticklabels([r"$-\pi$", r"$-\pi/2$", "0", r"$\pi/2$", r"$\pi$"])
    fig.tight_layout()
    path = outdir / f"phi_{tag}.png"
    fig.savefig(path, dpi=150)
    plt.close(fig)
    print(f"  Guardado: {path}")


def main():
    parser = argparse.ArgumentParser(description="Plots pT, eta, phi de UrQMD")
    parser.add_argument("run_dir", help="Directorio con subdirectorios run_NNNN/")
    parser.add_argument("--charged-only", action="store_true",
                        help="Solo partículas cargadas")
    parser.add_argument("--pid", type=int, default=None,
                        help="Filtrar por ityp (ID interno de UrQMD)")
    parser.add_argument("--select", choices=["all", "participants", "spectators"],
                        default="all",
                        help="all: todas las partículas (default); "
                             "participants: ncl>0; spectators: ncl==0")
    parser.add_argument("--max-events", type=int, default=None,
                        help="Límite de eventos a leer")
    args = parser.parse_args()

    run_dir = Path(args.run_dir)
    if not run_dir.exists():
        sys.exit(f"ERROR: no existe {run_dir}")

    tag = run_dir.name
    if args.select != "all":
        tag += f"_{args.select}"
    if args.charged_only:
        tag += "_cargadas"
    if args.pid is not None:
        tag += f"_pid{args.pid}"

    outdir = run_dir / "plots"
    outdir.mkdir(exist_ok=True)

    print(f"Leyendo eventos de {run_dir} ...")
    events = read_all(run_dir, max_events=args.max_events)
    if not events:
        sys.exit("ERROR: no se encontraron eventos")

    pT, eta, phi = collect_arrays(events,
                                  charged_only=args.charged_only,
                                  pid=args.pid,
                                  select=args.select)
    n_ev = len(events)
    n_part = len(pT)
    print(f"Eventos: {n_ev}   Partículas (post-filtro): {n_part}")

    print("Generando gráficas...")
    plot_pt(pT, tag, outdir)
    plot_eta(eta, tag, outdir)
    plot_phi(phi, tag, outdir)
    print("Listo.")


if __name__ == "__main__":
    main()
