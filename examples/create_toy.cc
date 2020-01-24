//std library
#include <vector>
#include <iostream>
#include <fstream>
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
//TMVA ROOT
#ifdef TMVA_h
#include <TMVA/Tools.h>
#include <TMVA/Reader.h>
#include <TMVA/MethodCuts.h>
using namespace TMVA;
#endif
//user
#include "utility.hh"
#include "TimeCheck.hh"
#include "ToyDataCreator.hh"
#include "MyConfiguration.hh"

//globals
const char* treename     = "ClusterTree";
const char* infotreename = "InfoTreeRun";


int main (int argc, char *argv[])
 {

  //start clock to time the analysis
  Time::TimeCheck MyTime(50);

  MyTime.AddCheckPoint("Start input");

  Config::MyConfiguration* config = new Config::MyConfiguration(argc, argv);
  
  //aooky configuration
  if(!config->ApplyConfigurationToy())
   {
   std::cout << "\033[1;31mERROR\033[0m: Incorrect configuration or help called, exiting\n";
   return 1;
   }
 
  

  //Define outputs
  TString outputname;
  outputname.Form("ToyCluster%i_Chan%i_Mult%i_MPVCharge%i_Sigma%0.0f",
                  config->GetNClusters(),
                  config->GetNumberOfChannels(),
                  config->GetMultiplexFactor(),
                  config->GetMPVCharge(),
                  config->GetSigma());

  TFile* output = new TFile(outputname +".root", "RECREATE");
  //new tree
  TTree* clustree = new TTree("ClusterTree", "This tree Contains Clusters created with a toy model");
  //fix the branches
  UInt_t ChanOutput[config->GetNumberOfChannels()];
  UInt_t StripsOutput[config->GetNumberOfStrips()];
  UInt_t StripsOutputProcessed[config->GetNumberOfStrips()];
  Double_t TruePositions[5];
  Double_t Sigmas[5];
  UInt_t   Charges[5];
  
  for(UInt_t j(0);j < 5;++j)
   {
    TruePositions[j] = 0;
    Sigmas[j]        = 0;
    Charges[j]       = 0;
   }
 
  clustree->Branch("ChanOutput",
                   &StripsOutput,
                   TString::Format("ChanOutputUInt_t[%i]/i",(UInt_t)config->GetNumberOfChannels()));
  clustree->Branch("Strips_Physical",
                   &StripsOutput,
                   TString::Format("Strips_PhysicalUInt_t[%i]/i", (UInt_t)config->GetNumberOfStrips()));
  clustree->Branch("Strips_Processed",
                   &StripsOutputProcessed,
                   TString::Format("Strips_ProcessedUInt_t[%i]/i", (UInt_t)config->GetNumberOfStrips()));
  clustree->Branch("TruePosition", &TruePositions, "TruePositionDouble_t[5]/D");
  clustree->Branch("Sigma",        &Sigmas,        "SigmaDouble_t[5]/d");
  clustree->Branch("Charge",       &Charges,       "ChargeUInt_t[5]/i");

  
  //main creator object
  Micromega::ToyDataCreator Creator(config->GetNumberOfChannels(),
                                    config->GetMultiplexFactor(),
                                    config->GetMPVCharge(),
                                    config->GetSigma()
                                    );

  //Histograms
  TH1F* Amplitude = new TH1F("Amplitude", "Amplitude of the Clusters; signal output", 1000, 0, 4000);
  TH1F* SigmaOut  = new TH1F("Sigma",     "Sigma of the Clusters; sigma [strips]",    15, 0, 15);
  TH1F* NCluster  = new TH1F("NCluster",  "Number of clusters; NClusters",            10, 0, 10);
  //TH1F* chi2      = new TH1F("chi2",      "$chi^2 of the fit; #chi^2",                1000, 0, 10);

  
  //Create new random number generator
  gRandom = new TRandom2(time(0));

  MyTime.AddCheckPoint("start_loop");

  ///MAIN LOOP
  for(unsigned int i(0); i < config->GetNEvent(); ++i)
   {

    //Generate Toy
    Creator.GenerateToy(ChanOutput,            
                        StripsOutput,
                        StripsOutputProcessed,
                        config->GetNClusters(),    //number of clusters to generate
                        false);                    //If minimization procedure must be applied or not

    //assign true
    for(UInt_t j(0);j < config->GetNClusters();++j)
     {
      TruePositions[j] = Creator.GetPosition(j);
      Sigmas[j]        = Creator.GetSigma(j);
      Charges[j]       = Creator.GetCharge(j);
     }
    

    //Fill the tree
    clustree->Fill();
    //std::cout << "debug \n";


    if(i%(1000) == 1)
     {
      std::cout << "#nevent: " << i-1 << "\n";
     }

   }

  MyTime.AddCheckPoint("finish loop");

  //record time of the program
  MyTime.End();
  //Print results
  MyTime.Print(std::cout);

  TTree* timetree = new TTree("time", "time taken by the program");
  MyTime.SaveInTree(timetree);
  //write to file
  clustree->Write();
  timetree->Write();
  output->Close();

  return 0;

 }
