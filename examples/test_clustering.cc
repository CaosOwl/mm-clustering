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
    std::cerr << "invalid Tree given\n";
    return 1;    
   }

  //Connect branch
  MicromegasPlane plane("MM1X", MyTree);
  //Initialize Multiplexing map
  plane.InitializeMultiplexingMap((TTree*)input->Get(infotreename));

    

  //DEFINE OUTPUTS
  TString outname;
  outname = inputname(inputname.Last('/') + 1, inputname.Last('.')) + "-output.root";
  TFile* output = new TFile(outname, "RECREATE");

  //Histograms

  //Create new random number generator
  TRandom2* myrand = new TRandom2(time(0));

  MyTime.AddCheckPoint("Finished input");

  MyTime.AddCheckPoint("start loop");

  ///MAIN LOOP
  for(unsigned int i(0); i < MyTree->GetEntries(); ++i)
   {
    //import event
    MyTree->GetEntry(i);

   }

  MyTime.AddCheckPoint("finish loop");


  //write to file
  output->Write();



  MyTime.AddCheckPoint("finish program");
  //record time of the program
  MyTime.End();
  //Print results
  MyTime.Print(std::cout);

  return 0;

 }
