#pragma once
//root
#include"TH1.h"
#include"TTree.h"
//std
#include<vector>

namespace Micromega
{
 enum ClusterMethod{GAUS};
 enum NoiseMethod{NONE, FROMFILE};
 enum MapMethod{ALGO, TXT, ROOT};
 
 class ToyDataCreator
 {

 public:
  //CONSTRUCTOR
  ToyDataCreator(const UInt_t,const UInt_t,const UInt_t,const Double_t, const MapMethod mapmethod = ALGO);
  ToyDataCreator(const UInt_t,const UInt_t,const UInt_t,const Double_t, const TString filename);
 
  bool GenerateToy(UInt_t*, UInt_t*, UInt_t*, const UInt_t NumberOfClusters = 1, const bool DoMinimization = true);

  //set functions

  //get functions
  Double_t GetPosition(UInt_t index);
  Double_t GetSigma(UInt_t index);
  UInt_t   GetCharge(UInt_t index);

  //set functions
  void SetNoiseMethod  (const NoiseMethod method){noisemethod = method;}
  void SetClusterMethod(const ClusterMethod method){clustermethod = method;}

  //utilities
  void SaveMultiplexMapToTree(TTree*) const;
  void InitializeNoiseFromROOTFile(const char*, const char* branchname = "MM3X_sigma");

 private:
  //Fundamental parameters
  UInt_t NumberOfChannels;
  UInt_t MultiplexFactor;
  UInt_t NumberOfStrips;
  UInt_t MPVCharge;
  double Sigma;
  NoiseMethod   noisemethod;
  ClusterMethod clustermethod;
  //mapping
  std::vector<UInt_t> ReverseMultiplexMAP;
  std::vector<UInt_t> Noise;

  //Inside the event
  std::vector<Double_t> positions;
  std::vector<Double_t> amplitudes;
  std::vector<UInt_t> charges;

  //PRIVATE FUNCTIONS
  void FillRegMatrix();
  void FillMultiplexingMatrix(const MapMethod);
  void FillData(const UInt_t);
  void CreateCluster(UInt_t*, const ClusterMethod = GAUS, const NoiseMethod = NONE);
  void Clear();

  //building multiplexing mapping
  void BuildMultiplexingFromAlgorithm();
  void BuildMultiplexingFromTXTFile(const char* filename);
  void BuildMultiplexingFromROOTFile(const char* filename);
  TString BuildMultiplexFileName() const;
 };

} //END NAMESPACE MICROMEGA
