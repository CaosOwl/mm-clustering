//root
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TH2F.h>
#include <TH1.h>
#include <TCanvas.h>
#include <TProfile2D.h>
#include <TParameter.h>
#include <TRandom3.h>
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
#include "MyConfiguration.hh"

//globals
const char* treename     = "ClusterTree";
const char* infotreename = "InfoTree";


int main (int argc, char *argv[])
 {

  //define inputs
  if(argc < 3)
   {
    std::cerr << "INVALID NUMBER OF INPUTS \n";
    std::cout << "Usage: \n 1: input (.root) \n";
    std::cout << "Usage: \n 2: Number of events to produce \n";
    return 1;
   }
  //start clock to time the analysis
  Time::TimeCheck MyTime(50);

  const UInt_t Nevent = atoi(argv[2]);

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

  //main parameters
  const UInt_t NumberOfChannels = 64;//Utils::GetParameter(InfoTree->GetUserInfo(), "NumberOfChannels", 1);
  const UInt_t MultiplexFactor = 5;//Utils::GetParameter(InfoTree->GetUserInfo(), "NumberOfChannels", 1);
  const UInt_t DistanceBetweenEvent = 2;
  const UInt_t QualityCut[] = {150, 250};

  //Connect branch
  TString planebranch("MM3X");
  Double_t ChanOutput[NumberOfChannels];
  Double_t RecoPos(-1000);
  Utils::SafeSetBranchAddress(MyTree, planebranch, ChanOutput, 1);
  Utils::SafeSetBranchAddress(MyTree, planebranch+"_recopos", RecoPos, 1);

  //DEFINE OUTPUTS
  TString outname;
  outname = inputname(inputname.Last('/') + 1, inputname.Last('.')) + planebranch + "-output.root";
  TFile* output  = new TFile(outname, "RECREATE");

  //collect multiplex map
  UInt_t ReverseMultiplexMap[NumberOfChannels * MultiplexFactor];
  Utils::ImportReverseMultiplexMap(ReverseMultiplexMap, InfoTree);

  //new tree
  TTree* outtree = new TTree("ClusterTree","ClusterTree");
  Double_t Chan_total[NumberOfChannels];
  Double_t StripsOutput[NumberOfChannels * MultiplexFactor];
  Double_t Chan_1[NumberOfChannels];
  Double_t Chan_2[NumberOfChannels];
  Double_t TruePosition[5];
  outtree->Branch(planebranch+"_Chan1", &Chan_1, planebranch + TString::Format("_Chan1UInt_t[%i]/i", NumberOfChannels));
  outtree->Branch(planebranch+"_Chan2", &Chan_2, planebranch + TString::Format("_Chan2UInt_t[%i]/i", NumberOfChannels));
  outtree->Branch(planebranch+"_ChanOutput", &Chan_total, planebranch + TString::Format("_ChanOutputUInt_t[%i]/i", NumberOfChannels));
  outtree->Branch(planebranch+"_StripsOutput", &StripsOutput, planebranch + TString::Format("_StripsOutputUInt_t[%i]/i", NumberOfChannels * MultiplexFactor));  
  outtree->Branch(planebranch+"TruePosition", &TruePosition, "TruePositionDouble_t[5]/D");

  //Create new random number generator
  gRandom = new TRandom3(time(0));

  MyTime.AddCheckPoint("Finished input");

  MyTime.AddCheckPoint("start loop");

  ///MAIN LOOP
  for(unsigned int i(0); i < Nevent; ++i)
   {
    //reset
    RecoPos = -1000;
    UInt_t event1 = i % MyTree->GetEntries();
    while(RecoPos > QualityCut[1] || RecoPos < QualityCut[0]) //skip bad events
     {
      //event1 = gRandom->Uniform(MyTree->GetEntries());
      ++event1;
      //First event
      MyTree->GetEntry(event1);
      //std::cout << RecoPos << "\n";
     }
    //save first event
    for(UInt_t chan(0); chan < NumberOfChannels; ++chan) {Chan_1[chan] = ChanOutput[chan]; Chan_total[chan] = ChanOutput[chan];}
    TruePosition[0] = RecoPos;

    //second event
    RecoPos = -1000;
    UInt_t event2(event1+DistanceBetweenEvent);
    while(RecoPos > QualityCut[1] || RecoPos < QualityCut[0]) //skip bad events
     {
      //event2 = gRandom->Uniform(MyTree->GetEntries());
      ++event2;
      //First event
      MyTree->GetEntry(event2);
     }

    //save second event
    for(UInt_t chan(0); chan < NumberOfChannels; ++chan) {Chan_2[chan] = ChanOutput[chan]; Chan_total[chan] += ChanOutput[chan];}
    TruePosition[1] = RecoPos;

    //also reconstruct the strips
    for(UInt_t strip(0); strip < NumberOfChannels*MultiplexFactor; ++strip) {StripsOutput[strip] = ChanOutput[ReverseMultiplexMap[strip]];}

    //fill it
    outtree->Fill();


    if(i%(10000) == 1)
     {
      std::cout << "#nevent: " << i-1 << "\n";
     }

   }

  MyTime.AddCheckPoint("finish loop");

  //write to file
  Utils::SaveGitHashInTree(InfoTree);
  InfoTree->Write();
  outtree->Write();
  output->Write();
  output->Close();



  MyTime.AddCheckPoint("finish program");
  //record time of the program
  MyTime.End();
  //Print results
  MyTime.Print(std::cout);

  return 0;

 }
