#pragma once
//root
#include "TTree.h"
//user
#include "ConfigManager.hh"
#include "utility.hh"
#include <boost/filesystem.hpp>

typedef int    myint;
typedef double myfloat;

namespace Config

{

 class MyConfiguration
 {  
 public:
  //CONSTRUCTOR  
  MyConfiguration(int, char**);
  //APPLY CONFIGURATION
  bool ApplyConfigurationToy();
  
  //ACCESSORIES
  bool CreateOutDirectory();
  TString CreateOutputName(const char*);
  bool SaveParamterInTree(TTree*);

  //GET FUNCTIONS
  myint          GetNEvent() const                  { return Nevent;}
  myint          GetCompressionLevel() const        { return compression;}  
  myint          GetVerbose() const                 { return Verbose;}
  myint          GetNClusters() const               { return NClusters;}
  myint          GetNumberOfChannels() const        { return NumberOfChannels;}
  myint          GetMultiplexFactor() const         { return MultiplexFactor;}
  myint          GetNumberOfStrips() const          { return NumberOfStrips;}
  myint          GetMPVCharge() const               { return MPVCharge;}
  myfloat        GetSigma() const                   { return Sigma;}
  myint          GetMinimalDistance() const         { return MinimalDistance;}
  myint          GetMaximalDistance() const         { return MaximalDistance;}  
  myfloat        GetLambdaMin() const               { return LambdaMin;}
  std::string    GetTargetFile() const              { return targetfile;}
  TString        GetFileNamePath() const            { return outname;}
  //Switches
  bool           MultiplexFromFileSwitch()          { return MultiplexFromFile;}
  bool           SaveWaveformsSwitch()              { return SaveWaveforms;}
  bool           MinimizationSwitch()               { return ApplyMinimization;}
  bool           UserFileSwitch()                   { return UseUserFile;}

 private:
  //CONFIGURATION
  ConfigManager configuration;
  //configuration variable
  myint     Nevent;
  myint     compression;
  myint     Verbose;
  myint     NClusters;
  myint     NumberOfChannels;
  myint     MultiplexFactor;
  myint     NumberOfStrips;
  myint     MPVCharge;
  myfloat   Sigma;
  myint     MinimalDistance;
  myint     MaximalDistance;
  myfloat   LambdaMin;
  //switches
  bool      MultiplexFromFile;
  bool      UseUserFile;
  bool      SaveWaveforms;
  bool      ApplyMinimization;
  
  TString     outname;
  std::string targetfile;
  std::string outdir;
  std::string outname_;
  
 };

} //END NAMESPACE CONFIGURATION


