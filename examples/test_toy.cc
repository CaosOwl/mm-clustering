//root
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TH2F.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TProfile2D.h>
#include <TParameter.h>
#include <TRandom2.h>
#include <TProfile.h>
//TMVA ROOT
#ifdef TMVA_h
#include <TMVA/Tools.h>
#include <TMVA/Reader.h>
#include <TMVA/MethodCuts.h>
using namespace TMVA;
#endif
//std library
#include <vector>
#include <iostream>
#include <fstream>
//user
#include "utility.hh"
#include "TimeCheck.hh"
#include "ClusteringFunction.hh"

//globals
const char* treename     = "ClusterTree";
const char* infotreename = "InfoTree";


int main (int argc, char *argv[])
 {

  //define inputs
  if(argc < 2)
   {
    std::cerr << "INVALID NUMBER OF INPUTS \n";
    std::cout << "Usage: \n 1-N: inputs (.root) \n";
   }
  //start clock to time the analysis
  Time::TimeCheck MyTime(50);

  //Define inputs
  TString inputname(argv[1]);
  TFile* input = new TFile(inputname, "READ");
  if(!input)
   {
    std::cerr << "invalid or corrupted input file \n";
    return 1;
   }
  
  //Define tree
  TTree* MyTree = (TTree*)(input->Get(treename));
  if(!MyTree)
   {
    std::cerr << "invalid Tree given" << treename << "\n";
    return 1;    
   }

  //Info tree
  TTree* InfoTree = (TTree*)(input->Get(infotreename));
  if(!InfoTree)
   {
    std::cerr << "invalid Tree given: " << infotreename << "\n";
    return 1;    
   }

  //define important parameters
  const UInt_t Nevents          = 1000;//MyTree->getEntries();
  const UInt_t NumberOfChannels = Utils::GetIntParameter(InfoTree->GetUserInfo(), "NumberOfChannels");
  const UInt_t MultiplexFactor  = Utils::GetIntParameter(InfoTree->GetUserInfo(), "MultiplexFactor");
  const UInt_t NumberOfStrips   = Utils::GetIntParameter(InfoTree->GetUserInfo(), "NumberOfStrips");
  const UInt_t MPVCharge        = Utils::GetIntParameter(InfoTree->GetUserInfo(), "MPVCharge");
  const UInt_t Sigma            = Utils::GetIntParameter(InfoTree->GetUserInfo(), "Sigma");  

  //Connect branch
  UInt_t   Chan[NumberOfChannels];
  UInt_t   StripsPhysical[NumberOfStrips];
  Double_t StripsProcessed[NumberOfStrips];
  Double_t TruePositions[5];
  Double_t Sigmas[5];
  UInt_t Charges[5];
   
  MyTree->SetBranchAddress("ChanOutput",       Chan);
  MyTree->SetBranchAddress("Strips_Physical",  StripsPhysical);
  MyTree->SetBranchAddress("Strips_Processed", StripsProcessed);
  MyTree->SetBranchAddress("TruePosition", TruePositions);
  MyTree->SetBranchAddress("Sigma", Sigmas);
  MyTree->SetBranchAddress("Charge", Charges);

    

  //DEFINE OUTPUTS
  TString outname;
  outname = inputname(inputname.Last('/') + 1, inputname.Last('.')) + "-analysed.root";
  TFile* output = new TFile(outname, "RECREATE");
  TTree* newinfotree = new TTree("InfoTree-analysis", "Infotree-analysis");
  //Utils::AddInfoParameters(newinfotree, InfoTree->GetUserInfo());  

  //Histograms
  output->mkdir("summary");
  output->cd("summary");
  TH1F* Amplitude = new TH1F("Amplitude", "Amplitude of the Clusters; signal output", 1000, 0, 4000);
  TH1F* SigmaOut  = new TH1F("Sigma",     "Sigma of the Clusters; sigma [strips]",    15, 0, 15);
  TH1F* NCluster  = new TH1F("NCluster",  "Number of clusters; NClusters",            10, 0, 10);
  TH1F* chi2      = new TH1F("chi2",      "$chi^2 of the fit; #chi^2",                1000, 0, 10);
  TH2F* residuals = new TH2F("resvsdist", "residual of the hits vs distance between hist; distance [strip]; residual [strip]",20, -0.5, 20, 600, 0., 3);
  TProfile* errorprofile = new TProfile("err_profile", "residual against number of strips; distance [strips]; residual [strips]", 20, -0.5, 19.5);
  TH1F* distance    = new TH1F("distance", "Distance of clusters; distance [strips];",20, -0.5, 19.5);
  TH1F* efficiency = new TH1F("efficiency", "efficiency as function of distance; distance [strips]; efficiency [%]",20, -0.5, 19.5);
  output->cd();
  //Create new random number generator
  gRandom = new TRandom2(time(0));

  MyTime.AddCheckPoint("Start Loop");

  ///MAIN LOOP
  for(unsigned int i(0); i < Nevents; ++i)
   {
    //import event
    MyTree->GetEntry(i);

    //fit peaks
    FitPlane plane;
    plane.true1 = FitClus(TruePositions[0], Sigmas[0], Charges[0]);
    plane.true2 = FitClus(TruePositions[1], Sigmas[1], Charges[1]);
    TCanvas* canv = FitPeaks(StripsPhysical,
                             NumberOfStrips,
                             plane,
                             FitMethod::SPECTRUM);
    plane.histo->SetName(TString::Format("hist-%i", i));
    canv->SetName(TString::Format("%s-canv", plane.histo->GetName()));

    distance->Fill(plane.distance());

    if(plane.residual1() < 3 && plane.residual2() < 3)
     {
      residuals->Fill(plane.distance(), plane.residual1());
      residuals->Fill(plane.distance(), plane.residual2());
      errorprofile->Fill(plane.distance(), plane.residual1());
      errorprofile->Fill(plane.distance(), plane.residual2());
      efficiency->Fill(plane.distance());
     }

    //compare to actual solutions
    std::cout << "event " << i << "\n";
    plane.Print(std::cout);

    canv->Write();

   }
  for(UInt_t n(0); n < 20; ++n)
   {
    efficiency->SetBinContent(n, 100. * efficiency->GetBinContent(n) / distance->GetBinContent(n));
   }

  MyTime.AddCheckPoint("finish loop");

  //record time of the program
  MyTime.End();
  //Print results
  MyTime.Print(std::cout);

  //save results  
  MyTime.SaveInTree(newinfotree);
  newinfotree->Write();
  output->Write();
  output->Close();

  return 0;

 }
