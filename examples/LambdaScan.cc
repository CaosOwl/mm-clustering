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
#include <TGraph.h>
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
  TString outputname = config->CreateOutputName("ToyCluster");

  TFile* output = new TFile(outputname +"-LambdaScan.root", "RECREATE");  
  output->SetCompressionSettings(config->GetCompressionLevel());

  //fix the branches
  UInt_t ChanOutput[config->GetNumberOfChannels()];
  UInt_t StripsOutput[config->GetNumberOfStrips()];
  Double_t StripsOutputProcessed[config->GetNumberOfStrips()];
  Double_t TruePositions[5];
  Double_t Sigmas[5];
  UInt_t   Charges[5];
  
  //main creator object
  Micromega::ToyDataCreator* Creator;
  if(config->UserFileSwitch())
   {
    const TString targetfile(config->GetTargetFile().c_str());
    Creator = new Micromega::ToyDataCreator(config->GetNumberOfChannels(),
                                            config->GetMultiplexFactor(),
                                            config->GetMPVCharge(),
                                            config->GetSigma(),
                                            targetfile
                                            );
    //noise as well
    Creator->InitializeNoiseFromROOTFile(config->GetTargetFile().c_str(), "MM3X_sigma");
    Creator->SetNoiseMethod(Micromega::NoiseMethod::FROMFILE);
   }
  else
   {
    const Micromega::MapMethod mapmethod = config->MultiplexFromFileSwitch() ?
     Micromega::MapMethod::TXT : Micromega::MapMethod::ALGO;
    
    Creator = new Micromega::ToyDataCreator(config->GetNumberOfChannels(),
                                            config->GetMultiplexFactor(),
                                            config->GetMPVCharge(),
                                            config->GetSigma(),
                                            mapmethod
                                            );
   }
  //set verbose
  Creator->SetVerbose(config->GetVerbose());

  //Histograms
  output->cd();

  UInt_t ScanPoints = 15;
  vector<Double_t> minima(ScanPoints, 0);
  //vector<Double_t> scanpoints = {5e-4, 2e-3, 3e-3, 4e-3, 5e-3, 1e-2, 5e-2, 0.1, 0.5, 1};
  vector<Double_t> scanpoints = {1e-6, 5e-6, 1e-5, 5e-4, 2e-3, 3e-3, 4e-3, 5e-3, 1e-2, 5e-2};

  TProfile* scan = new TProfile("resultLambdaScan", "Lambda scan; lambda; minimum", ScanPoints, scanpoints[0], scanpoints[ScanPoints - 1]);

  
  //Create new random number generator
  gRandom = new TRandom2(time(0));

  MyTime.AddCheckPoint("start_loop");

  ///MAIN LOOP
  for(unsigned int i(0); i < config->GetNEvent(); ++i)
   {

        //Generate Toy
    Creator->GenerateToy(ChanOutput,            
                         StripsOutput,
                         StripsOutputProcessed,
                         config->GetNClusters());

    //activate scan
    Creator->DoLambdaScan(ChanOutput,
                          minima,
                          scanpoints);

    for(UInt_t i(0), i < ScanPoints; ++i)
     {
      scan->Fill(scanpoints[i], minima[i]);
     }

    if(i%(1000) == 1)
     {
      std::cout << "#nevent: " << i-1 << "\n";
     }

   }

  MyTime.AddCheckPoint("finish loop");

  //save results in a TGraph
  //normalize
  for(auto min : minima) min /= config->GetNEvent();
  TGraph* result = new TGraph(ScanPoints, &scanpoints[0], &minima[0]);
  result->SetName("LambdaScanResults");

  //record time of the program
  MyTime.End();
  //Print results
  MyTime.Print(std::cout);

  
  output->cd();
  result->Write();
  TTree* timetree = new TTree("time", "time taken by the program");
  MyTime.SaveInTree(timetree);
  //save import parameter in info tree
  TTree* infotree = new TTree("InfoTree", "Parameter used for the toy");
  config->SaveParamterInTree(infotree);
  Creator->SaveParameterInTree(infotree);
  //write to file
  timetree->Write();
  infotree->Write();
  output->Close();

  return 0;

 }
