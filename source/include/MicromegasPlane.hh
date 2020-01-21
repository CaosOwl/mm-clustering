#pragma once
//std library
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream> 
#include <ctime>

//root
#include "TTree.h"
#include "TParameter.h"
#include "TList.h"
#include "TSpectrum.h"
#include "TF1.h"
#include "TRandom2.h"
#include "TRandom3.h"

//user
#include"Globals.hh"
#include"utility.hh"
#include"Cluster.hh"


using namespace std;

namespace Micromega
{

 enum ClusterSearchType{NONE, //no cluster search
                        WW,   //Using weight and threshold
                        PEAKSEARCH //using TSpectrum and result of gaussian
 };
  
 class MicromegasPlane
 {

 public:

  //CONSTRUCTOR
  MicromegasPlane();
  MicromegasPlane(const char*, TTree*, TTree*);
  MicromegasPlane(const char*, TTree*);

  void GenerateStrips(const bool, const bool, const bool ProcessStrip = true);
  TH1F* GenerateStripsHistogram(const ClusterSearchType searchtype = WW,
                                const int event = 0,
                                const bool FitHisto = true);
  void GenerateDoubleClusterHistogram(TH1F*, TH1F*, TH1F*);
  void InitializeMultiplexingMap(TTree*, const char* mapbranchname = "MicromegasMap");
  void InitializeNoise(TTree*, const char* noisebranchname);
  void GenerateNoise();
  void GenerateSmearing();


  //PRINT FUNCTION
  void PrintChannels(std::ostream&);
  void PrintNoises(std::ostream&);
  void PrintEvent(std::ostream&);

  //get function
  UInt_t GetAmplitude() { return amplitude;}
  double GetSigma()     { return sigma;}
  double GetXpos()      { return xposition;}
  double GetChi2()      { return chi2;}
  UInt_t GetNPeaks()    { if(PeakFinder)return PeakFinder->GetNPeaks();else return clusvec.size();}


 private:

  TString name;

  TTree* tree;

  TSpectrum* PeakFinder;
 
  UInt_t ChanOutput[NCHAN];
  UInt_t ChanNoise[NCHAN];
  UInt_t ChanSmear[NCHAN];

  //strips output
  UInt_t StripsOutput[NCHAN*MFACTOR];
  UInt_t StripsOutputProcessed[NCHAN*MFACTOR];

  //globals
  UInt_t NoiseMap[NCHAN];
  UInt_t MultiplexMAP[NCHAN][MFACTOR];
  UInt_t ReverseMultiplexMAP[NCHAN*MFACTOR];

  //temporary amplitudes
  UInt_t amplitude;
  double xposition;
  double sigma;
  double chi2;

  //vector with clusters
  vector<Cluster> clusvec;

#if 0
  //Histograms

  //Event histo
  TH1F* hStrips;
  TH1F* hStripsProcessed;

  //Global Histo
  
  TH1F* hAmplitude;
  TH1F* hSigmaOut;
  TH1F* hSigmaOut;
#endif  
  

  //private functions
  void ResetOutput();
  void ResetMap();
  void GenerateProcessedStrips();
  bool AboveNoiseTreshold(UInt_t, double);
  bool CheckNeaboringStrips(UInt_t, UInt_t);

  void SearchClusterWithSpectrum(TH1F*, const bool);
  void SearchClusterWW(TH1F*, const bool);
  TF1* FitStripsHisto(TH1F*, double, UInt_t, UInt_t);
  
   
 }; //end class micromegasplane

} //end MM namespace
