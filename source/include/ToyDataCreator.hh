#pragma once
//root
#include"TH1.h"
#include"TH2.h"
#include"TTree.h"
//std
#include<vector>

namespace Micromega
{
 enum ClusterMethod{GAUS, GAUSFROMFILE};
 enum NoiseMethod{NONE, FROMFILE};
 enum MapMethod{ALGO, TXT, ROOT};
 
 class ToyDataCreator
 {

 public:
  //CONSTRUCTOR
  ToyDataCreator(const UInt_t,const UInt_t,const UInt_t,const Double_t, const MapMethod mapmethod = ALGO);
  ToyDataCreator(const UInt_t,const UInt_t,const UInt_t,const Double_t, const TString filename);
 
  bool GenerateToy(UInt_t*, UInt_t*, Double_t*, const UInt_t NumberOfClusters = 1, const bool DoMinimization = true, UInt_t minimaldistance = 0, UInt_t maximaldistance = 9999);
  bool GenerateToy(UInt_t*, UInt_t*, Double_t*, const Double_t, const Double_t, const bool DoMinimization = true);
  bool ProcessPlane(UInt_t*, UInt_t*, Double_t*, const bool DoMinimization = true);

  //get functions
  Double_t GetPosition(UInt_t index);
  Double_t GetSigma(UInt_t index);
  UInt_t   GetCharge(UInt_t index);

  //set functions
  void SetNoiseMethod  (const NoiseMethod method){noisemethod = method;}
  void SetClusterMethod(const ClusterMethod method){clustermethod = method;}
  void SetVerbose(const UInt_t v){verbose = v;}
  void SetLambda(const UInt_t l){LambdaMin = l;}
  void SetChargeSigma(const Double_t c){ChargeSigma = c;}

  //utilities
  void SaveParameterInTree(TTree*) const;
  void InitializeNoiseFromROOTFile(const char*, const char* branchname = "MM3X_sigma");
  std::vector<UInt_t> CreateMultiplexedStrips(UInt_t*) const;
  bool IsStripValid(const UInt_t*, const UInt_t,const UInt_t noisesigma = 2) const;
  void DoLambdaScan(UInt_t*, std::vector<Double_t>&, std::vector<Double_t>&);
  void InitializeChargeVsSigmaHisto(const char*, const char*);

 private:
  //Fundamental parameters
  UInt_t NumberOfChannels;
  UInt_t MultiplexFactor;
  UInt_t NumberOfStrips;
  UInt_t MPVCharge;
  Double_t ChargeSigma;
  double Sigma;
  UInt_t verbose;
  Double_t LambdaMin;
  NoiseMethod   noisemethod;
  ClusterMethod clustermethod;
  //histogram to define a plane
  TH2F* ChargeVsSigma;
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
  void Clear();
  void ResetInput(UInt_t*, UInt_t*, Double_t*) const;

  //Cluster functions
  void CreateCluster(UInt_t*, const Double_t, const ClusterMethod = GAUS);
  void CreateClusterWithGaus(const Double_t, const Double_t, const UInt_t, UInt_t*) const;

  //building multiplexing mapping
  void BuildMultiplexingFromAlgorithm();
  void BuildMultiplexingFromTXTFile(const char*);
  void BuildMultiplexingFromROOTFile(const char*);
  TString BuildMultiplexFileName() const;
 };

} //END NAMESPACE MICROMEGA
