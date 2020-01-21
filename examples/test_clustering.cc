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
//std library
#include <vector>
#include <iostream>
#include <fstream>
//user
#include "utility.hh"
#include "TimeCheck.hh"
#include "MicromegasPlane.hh"

//globals
const char* treename     = "ClusterTree";
const char* infotreename = "InfoTreeRun";


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

  //Connect branch
  const char* planebranch = "MM3X";
  Micromega::MicromegasPlane plane(planebranch, MyTree, InfoTree);
  TString PlaneDir(planebranch);PlaneDir += "-waveforms";

    

  //DEFINE OUTPUTS
  TString outname;
  outname = inputname(inputname.Last('/') + 1, inputname.Last('.')) + "-output.root";
  TFile* output = new TFile(outname, "RECREATE");
  output->mkdir(PlaneDir);
  output->mkdir(PlaneDir + "-selected");

  //Histograms
  TH1F* Amplitude = new TH1F("Amplitude", "Amplitude of the Clusters; signal output", 1000, 0, 4000);
  TH1F* SigmaOut  = new TH1F("Sigma",     "Sigma of the Clusters; sigma [strips]",    15, 0, 15);
  TH1F* NCluster  = new TH1F("NCluster",  "Number of clusters; NClusters",            10, 0, 10);
  TH1F* chi2      = new TH1F("chi2",      "$chi^2 of the fit; #chi^2",                1000, 0, 10);

  //Create new random number generator
  gRandom = new TRandom2(time(0));

  MyTime.AddCheckPoint("Finished input");

  MyTime.AddCheckPoint("start loop");

  ///MAIN LOOP
  for(unsigned int i(0); i < MyTree->GetEntries(); ++i)
   {
    //import event
    MyTree->GetEntry(i);

    //Generate strips
    plane.GenerateStrips(false,           //if you want to add the smearing
                         false);          //if you want to add the noise


    //save an output every 1000 events
    if(i%100 == 1)
     {
      //plane.PrintEvent(std::cout);      
      output->cd(PlaneDir);
      //Generate histogram
      TH1F* planehist = plane.GenerateStripsHistogram(Micromega::WW,
                                                      i,
                                                      true //if you want to fit it
                                                      );

      planehist->Write();      
      output->cd();

      Amplitude->Fill(plane.GetAmplitude());
      SigmaOut->Fill(plane.GetSigma());
      NCluster->Fill(plane.GetNPeaks());
      chi2->Fill(plane.GetChi2());

      //fill other histograms
      if(plane.GetChi2() < 5)
       {
        output->cd(PlaneDir + "-selected");
        planehist->Write();
        output->cd();                
       }
      
     }
    


    if(i%(10000) == 1)
     {
      std::cout << "#nevent: " << i-1 << "\n";
     }

   }

  MyTime.AddCheckPoint("finish loop");

  //write to file
  output->Write();
  output->Close();



  MyTime.AddCheckPoint("finish program");
  //record time of the program
  MyTime.End();
  //Print results
  MyTime.Print(std::cout);

  return 0;

 }
