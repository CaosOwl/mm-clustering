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
  output->SetCompressionSettings(config->GetCompressionLevel());
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
                   &ChanOutput,
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

  //Histograms
  output->cd();
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
    Creator->GenerateToy(ChanOutput,            
                         StripsOutput,
                         StripsOutputProcessed,
                         config->GetNClusters(),          //number of clusters to generate
                         config->MinimizationSwitch());   //If minimization procedure must be applied or not

    //assign true
    for(UInt_t j(0);j < config->GetNClusters();++j)
     {
      TruePositions[j] = Creator->GetPosition(j);
      Sigmas[j]        = Creator->GetSigma(j);
      Charges[j]       = Creator->GetCharge(j);
     }

    //save histogram
    if(config->SaveWaveformsSwitch())
     {
      TString foldername;
      foldername.Form("histograms/event_%i", i);
      output->mkdir(foldername);output->cd(foldername);
      TH1F* stripsphys = new TH1F(TString::Format("stripsphys_evt_%i", i),
                                  TString::Format("strip physical for event %i", i),
                                  config->GetNumberOfStrips(),
                                  -0.5,
                                  config->GetNumberOfStrips() -0.5);
      TH1F* stripsreco = new TH1F(TString::Format("stripsreco_evt_%i", i),
                                  TString::Format("strip reconstructed for event %i", i),
                                  config->GetNumberOfStrips(),
                                  -0.5,
                                  config->GetNumberOfStrips() -0.5);
      TH1F* chanout   = new TH1F(TString::Format("channel_evt_%i", i),
                                 TString::Format("channel output for event %i", i),
                                 config->GetNumberOfChannels(),
                                 -0.5,
                                 config->GetNumberOfChannels() -0.5);
      //fill it      
      for(UInt_t n(0); n < config->GetNumberOfStrips(); ++n){stripsphys->SetBinContent(n, StripsOutput[n]);stripsreco->SetBinContent(n, StripsOutputProcessed[n]);}
      for(UInt_t n(0); n < config->GetNumberOfChannels(); ++n)chanout->SetBinContent(n, ChanOutput[n]);
            
      stripsphys->Write();stripsreco->Write();chanout->Write();
      output->cd();       
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

  
  output->cd();
  TTree* timetree = new TTree("time", "time taken by the program");
  MyTime.SaveInTree(timetree);
  //save import parameter in info tree
  TTree* infotree = new TTree("InfoTree", "Parameter used for the toy");
  config->SaveParamterInTree(infotree);
  Creator->SaveMultiplexMapToTree(infotree);
  //write to file
  clustree->Write();
  timetree->Write();
  infotree->Write();
  output->Close();

  return 0;

 }
