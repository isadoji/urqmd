// ============================================================
// plot_observables.C
// Histogramas de pT, eta, phi desde el TTree generado por urqmd2root.C
//
// Uso:
//   root -l -b -q 'plot_observables.C("Bi_11GeV_MB.root")'
//   root -l -b -q 'plot_observables.C("Bi_11GeV_MB.root","participants",true)'
//   root -l -b -q 'plot_observables.C("Bi_11GeV_MB.root","spectators",true)'
//   root -l -b -q 'plot_observables.C("Bi_11GeV_MB.root","all",false,101)'
//
// Parámetros:
//   infile       : archivo ROOT generado por urqmd2root.C
//   select       : "all" | "participants" (ncl>0) | "spectators" (ncl==0)
//   charged_only : true = solo partículas cargadas
//   pid          : filtro por ityp de UrQMD  (-1 = sin filtro)
// ============================================================

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLatex.h"
#include "TStyle.h"
#include "TMath.h"
#include <cstdio>
#include <cmath>

#define MAXPART 600000

void plot_observables(TString infile       = "Bi_11GeV_MB.root",
                      TString select       = "all",
                      Bool_t  charged_only = false,
                      Int_t   pid          = -1)
{
    TFile *fin = TFile::Open(infile);
    if(!fin || fin->IsZombie()){
        printf("[ERROR] No se puede abrir: %s\n", infile.Data()); return;
    }
    TTree *tree = (TTree*)fin->Get("T");
    if(!tree){
        printf("[ERROR] TTree 'T' no encontrado en %s\n", infile.Data()); return;
    }

    // ── Tag para nombres de archivo ──────────────────────────────────────
    TString tag = infile;
    tag.ReplaceAll(".root", "");
    Int_t sl = tag.Last('/');
    if(sl >= 0) tag = tag(sl+1, tag.Length()-sl-1);
    if(select != "all") tag += "_" + select;
    if(charged_only)    tag += "_charged";
    if(pid >= 0)        tag += Form("_pid%d", pid);

    // ── Histogramas ──────────────────────────────────────────────────────
    TH1F *hpT  = new TH1F("hpT",  "", 60,  0.0,          3.0);
    TH1F *heta = new TH1F("heta", "", 80, -8.0,          8.0);
    TH1F *hphi = new TH1F("hphi", "", 72, -TMath::Pi(), TMath::Pi());

    // ── Ramas ────────────────────────────────────────────────────────────
    Int_t          npart;
    static Float_t Px[MAXPART], Py[MAXPART], Pz[MAXPART];
    static Float_t chg[MAXPART], ncl[MAXPART];
    static Int_t   ityp[MAXPART];

    tree->SetBranchStatus("*", 0);
    tree->SetBranchStatus("npart",      1);  tree->SetBranchAddress("npart",      &npart);
    tree->SetBranchStatus("Px",         1);  tree->SetBranchAddress("Px",          Px);
    tree->SetBranchStatus("Py",         1);  tree->SetBranchAddress("Py",          Py);
    tree->SetBranchStatus("Pz",         1);  tree->SetBranchAddress("Pz",          Pz);
    tree->SetBranchStatus("charge",     1);  tree->SetBranchAddress("charge",      chg);
    tree->SetBranchStatus("numbercoll", 1);  tree->SetBranchAddress("numbercoll",  ncl);
    if(pid >= 0){
        tree->SetBranchStatus("ityp",   1);  tree->SetBranchAddress("ityp",        ityp);
    }

    // ── Loop ─────────────────────────────────────────────────────────────
    Long64_t nEv = tree->GetEntries();
    printf("Eventos: %lld  |  select=%s  |  charged=%s  |  pid=%d\n",
           nEv, select.Data(), charged_only ? "yes" : "no", pid);

    for(Long64_t j = 0; j < nEv; j++){
        tree->GetEntry(j);
        for(Int_t i = 0; i < npart; i++){
            if(select == "participants" && ncl[i] <= 0.0f) continue;
            if(select == "spectators"   && ncl[i] >  0.0f) continue;
            if(charged_only             && chg[i] == 0.0f) continue;
            if(pid >= 0                 && ityp[i] != pid) continue;

            Float_t pT  = TMath::Sqrt(Px[i]*Px[i] + Py[i]*Py[i]);
            Float_t P   = TMath::Sqrt(Px[i]*Px[i] + Py[i]*Py[i] + Pz[i]*Pz[i]);
            if(P < 1e-10f || TMath::Abs(P - TMath::Abs(Pz[i])) < 1e-10f) continue;
            Float_t eta = 0.5f * TMath::Log((P + Pz[i]) / (P - Pz[i]));
            Float_t phi = TMath::ATan2(Py[i], Px[i]);

            hpT ->Fill(pT);
            heta->Fill(eta);
            hphi->Fill(phi);
        }
        if((j+1) % 500 == 0 || j+1 == nEv)
            printf("\r  Procesados %lld / %lld eventos", j+1, nEv);
    }
    printf("\n");

    // ── Estilo ───────────────────────────────────────────────────────────
    gStyle->SetOptStat(0);   gStyle->SetOptTitle(0);
    gStyle->SetPadTickX(1);  gStyle->SetPadTickY(1);
    gStyle->SetPadLeftMargin(0.14);   gStyle->SetPadRightMargin(0.06);
    gStyle->SetPadBottomMargin(0.13); gStyle->SetPadTopMargin(0.06);

    auto Draw = [&](TH1F *h, TString obs, TString xtitle, Bool_t logy){
        TCanvas *c = new TCanvas("c_"+obs, "", 800, 600);
        if(logy) c->SetLogy();
        h->SetLineColor(kBlue+1);
        h->SetLineWidth(2);
        h->GetXaxis()->SetTitle(xtitle);
        h->GetXaxis()->CenterTitle();
        h->GetXaxis()->SetTitleSize(0.052);
        h->GetXaxis()->SetLabelSize(0.042);
        h->GetYaxis()->SetTitle("Particles / bin");
        h->GetYaxis()->CenterTitle();
        h->GetYaxis()->SetTitleSize(0.052);
        h->GetYaxis()->SetLabelSize(0.042);
        h->GetYaxis()->SetTitleOffset(1.4);
        h->Draw("HIST");
        TLatex lat;
        lat.SetNDC(); lat.SetTextSize(0.038);
        lat.DrawLatex(0.16, 0.88, tag);
        TString out = obs + "_" + tag;
        c->SaveAs(out + ".pdf");
        c->SaveAs(out + ".png");
        printf("Guardado: %s.png\n", out.Data());
        delete c;
    };

    Draw(hpT,  "pt",  "p_{T}  [GeV/c]",  kTRUE);
    Draw(heta, "eta", "#eta",             kFALSE);
    Draw(hphi, "phi", "#phi  [rad]",      kFALSE);

    fin->Close();
}
