// ============================================================
// urqmd2root.C
// Convierte archivos .f14 de UrQMD a un ROOT TTree.
// Layout de ramas compatible con CalcBfield_3cent.C.
//
// Uso:
//   root -l -b -q 'urqmd2root.C("output/Bi_11GeV_MB")'
//   root -l -b -q 'urqmd2root.C("output/Bi_11GeV_0-20","Bi_0-20.root")'
// ============================================================

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"
#include "TSystemDirectory.h"
#include "TList.h"
#include "TSystemFile.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cstdio>

#define MAXPART 600000

// ── Estructura interna por partícula ────────────────────────────────────────
struct Particle {
    float t, x, y, z, e, px, py, pz, mass, chg, ncl;
    int   itype, iiso;
};

// ── Ramas del TTree ─────────────────────────────────────────────────────────
namespace G {
    Int_t   npart, nspec;
    Float_t b, nev_f;
    Float_t t_arr[MAXPART], X[MAXPART], Y[MAXPART], Z_arr[MAXPART];
    Float_t E[MAXPART], Px[MAXPART], Py[MAXPART], Pz[MAXPART];
    Float_t m_arr[MAXPART], charge[MAXPART], numbercoll[MAXPART];
    Int_t   ityp[MAXPART], iso[MAXPART];
}

// ── Vuelca el buffer al TTree ────────────────────────────────────────────────
void FlushEvent(TTree *tree, std::vector<Particle> &buf,
                Long64_t &n_ev, Long64_t &n_part_tot)
{
    if(buf.empty()) return;
    G::npart = (Int_t)buf.size();
    if(G::npart > MAXPART) G::npart = MAXPART;
    G::nspec = 0;
    for(Int_t i = 0; i < G::npart; i++){
        G::t_arr[i]      = buf[i].t;
        G::X[i]          = buf[i].x;
        G::Y[i]          = buf[i].y;
        G::Z_arr[i]      = buf[i].z;
        G::E[i]          = buf[i].e;
        G::Px[i]         = buf[i].px;
        G::Py[i]         = buf[i].py;
        G::Pz[i]         = buf[i].pz;
        G::m_arr[i]      = buf[i].mass;
        G::charge[i]     = buf[i].chg;
        G::numbercoll[i] = buf[i].ncl;
        G::ityp[i]       = buf[i].itype;
        G::iso[i]        = buf[i].iiso;
        if(buf[i].ncl == 0.0f) G::nspec++;
    }
    tree->Fill();
    n_ev++;
    n_part_tot += G::npart;
    buf.clear();
}

// ── Función principal ────────────────────────────────────────────────────────
void urqmd2root(TString run_dir = "output/Bi_11GeV_MB", TString outfile = "")
{
    // Auto-nombre del archivo de salida
    if(outfile == ""){
        TString base = run_dir;
        while(base.EndsWith("/")) base.Remove(base.Length()-1, 1);
        Int_t sl = base.Last('/');
        if(sl >= 0) base = base(sl+1, base.Length()-sl-1);
        outfile = base + ".root";
    }

    // Buscar archivos urqmd.f14 en subdirectorios run_NNNN/
    std::vector<std::string> f14list;
    TSystemDirectory sdir("sdir", run_dir);
    TList *entries = sdir.GetListOfFiles();
    if(!entries){
        printf("[ERROR] No se puede abrir: %s\n", run_dir.Data()); return;
    }
    TIter next(entries);
    TSystemFile *ent;
    while((ent = (TSystemFile*)next())){
        TString ename = ent->GetName();
        if(!ent->IsDirectory() || ename == "." || ename == "..") continue;
        TString f14 = run_dir + "/" + ename + "/urqmd.f14";
        if(!gSystem->AccessPathName(f14))
            f14list.push_back(f14.Data());
    }
    if(f14list.empty()){
        printf("[ERROR] No hay archivos urqmd.f14 en %s\n", run_dir.Data()); return;
    }
    std::sort(f14list.begin(), f14list.end());
    printf("Convirtiendo %zu archivo(s)  →  %s\n\n", f14list.size(), outfile.Data());

    // Crear TTree
    TFile fout(outfile, "RECREATE");
    TTree *tree = new TTree("T", "UrQMD final-state particles");

    tree->Branch("npart",      &G::npart,       "npart/I");
    tree->Branch("nspec",      &G::nspec,       "nspec/I");
    tree->Branch("b",          &G::b,           "b/F");
    tree->Branch("nev",        &G::nev_f,       "nev/F");
    tree->Branch("time",        G::t_arr,       "time[npart]/F");
    tree->Branch("X",           G::X,           "X[npart]/F");
    tree->Branch("Y",           G::Y,           "Y[npart]/F");
    tree->Branch("Z",           G::Z_arr,       "Z[npart]/F");
    tree->Branch("E",           G::E,           "E[npart]/F");
    tree->Branch("Px",          G::Px,          "Px[npart]/F");
    tree->Branch("Py",          G::Py,          "Py[npart]/F");
    tree->Branch("Pz",          G::Pz,          "Pz[npart]/F");
    tree->Branch("m",           G::m_arr,       "m[npart]/F");
    tree->Branch("charge",      G::charge,      "charge[npart]/F");
    tree->Branch("numbercoll",  G::numbercoll,  "numbercoll[npart]/F");
    tree->Branch("ityp",        G::ityp,        "ityp[npart]/I");
    tree->Branch("iso",         G::iso,         "iso[npart]/I");

    std::vector<Particle> buf;
    buf.reserve(5000);
    Long64_t n_ev = 0, n_part_tot = 0;

    // Parseo de archivos f14
    for(const auto &path : f14list){
        std::ifstream fin(path);
        if(!fin){ printf("[WARN] No se puede abrir %s\n", path.c_str()); continue; }

        G::b     = -1.0f;
        G::nev_f = (Float_t)n_ev;
        bool in_event = false;
        std::string line;

        while(std::getline(fin, line)){
            if(line.empty()) continue;
            char c = line[0];

            if(c == 'U'){
                // Inicio de nuevo evento: vuelca el anterior
                FlushEvent(tree, buf, n_ev, n_part_tot);
                G::b     = -1.0f;
                G::nev_f = (Float_t)n_ev;
                in_event = true;
            }
            else if(c == 'i'){
                // impact_parameter_real/min/max(fm):  b_val  ...
                const char *p = strstr(line.c_str(), "fm):");
                if(p) sscanf(p + 4, " %f", &G::b);
            }
            else if(c == 'p' || c == 't' || c == 'e' || c == 'o'){
                continue;   // cabeceras: proyectil, transformación, ecuación, opciones
            }
            else if(in_event){
                // Línea numérica: cuenta columnas para distinguir
                // contadores (2 u 8 cols) de líneas de partícula (≥15 cols)
                std::istringstream iss(line);
                double v; std::vector<double> vals;
                while(iss >> v) vals.push_back(v);
                if((int)vals.size() >= 15){
                    Particle p;
                    p.t     = (float)vals[0];   p.x     = (float)vals[1];
                    p.y     = (float)vals[2];   p.z     = (float)vals[3];
                    p.e     = (float)vals[4];   p.px    = (float)vals[5];
                    p.py    = (float)vals[6];   p.pz    = (float)vals[7];
                    p.mass  = (float)vals[8];
                    p.itype = (int)vals[9];
                    p.iiso  = (int)vals[10];
                    p.chg   = (float)vals[11];
                    // vals[12] = lcl#,  vals[13] = ncl,  vals[14] = hist
                    p.ncl   = (float)vals[13];
                    buf.push_back(p);
                }
            }
        }
        FlushEvent(tree, buf, n_ev, n_part_tot);   // último evento del archivo
        printf("  OK  %s\n", path.c_str());
    }

    fout.cd();
    tree->Write();
    fout.Close();
    printf("\nListo: %lld eventos  |  %lld partículas  →  %s\n",
           n_ev, n_part_tot, outfile.Data());
}
