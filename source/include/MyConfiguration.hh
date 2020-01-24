#pragma once
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

  //GET FUNCTIONS
  myint          GetNEvent() const                  { return Nevent;}
  myint          GetVerbose() const                 { return Verbose;}
  myint          GetNClusters() const               { return NClusters;}
  myint          GetNumberOfChannels() const        { return NumberOfChannels;}
  myint          GetMultiplexFactor() const         { return MultiplexFactor;}
  myint          GetNumberOfStrips() const          { return NumberOfStrips;}
  myint          GetMPVCharge() const               { return MPVCharge;}
  myfloat        GetSigma() const                   { return Sigma;}
  bool           MultiplexFromFileSwitch()          { return MultiplexFromFile;}
  std::string    GetOutDir() const                  { return outdir;}
  TString        GetFileNamePath() const            { return outname;}

 private:
  //CONFIGURATION
  ConfigManager configuration;
  //configuration variable
  myint     Nevent;
  myint     Verbose;
  myint     NClusters;
  myint     NumberOfChannels;
  myint     MultiplexFactor;
  myint     NumberOfStrips;
  myint     MPVCharge;
  myfloat   Sigma;
  bool      MultiplexFromFile;
  
  TString     outname;
  std::string outdir;
  
 };

} //END NAMESPACE CONFIGURATION


