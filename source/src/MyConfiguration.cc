//user
#include"MyConfiguration.hh"
//root
#include"TList.h"

namespace Config
{
 
 MyConfiguration::MyConfiguration(int nargs, char** argvs)
  : configuration(nargs, argvs)
    
 {
  //CONFIGURATION
  Nevent                    = 10000;
  Verbose                   = 0;
  NClusters                 = 2;
  NumberOfChannels          = 64;
  MultiplexFactor           = 5;
  NumberOfStrips            = MultiplexFactor * NumberOfChannels;
  MPVCharge                 = 100;
  Sigma                     = 3;
  MultiplexFromFile         = false;
  SaveWaveforms             = false;
  ApplyMinimization         = false;
  outname.Form("ToyCluster_Chan%i_Mult%i_MPVCharge%i_Sigma%0.0f",
               NumberOfChannels,
               MultiplexFactor,
               MPVCharge,
               Sigma);
  outdir = "./";
 }

 bool MyConfiguration::ApplyConfigurationToy()
 {
  if(!configuration.Initialize())
   {
    return false;
   }
  string spec;
  configuration.GetOption(compression,           "compression"           );   
  configuration.GetOption(Nevent,                "events"                );   
  configuration.GetOption(Verbose,               "verbose"               );   
  configuration.GetOption(NumberOfChannels,      "channels"              );   
  configuration.GetOption(MultiplexFactor,       "mfactor"               );
  configuration.GetOption(NClusters,             "nclusters"             );   
  configuration.GetOption(Sigma,                 "amplitude"             );
  configuration.GetOption(MPVCharge,             "charge"                );
  configuration.GetOption(outdir,                "outdir"                );
  //switches
  configuration.GetOption(SaveWaveforms,         "save-waveforms"        );
  configuration.GetOption(MultiplexFromFile,     "from-file"             );
  configuration.GetOption(ApplyMinimization,     "apply-minimization"    );
  
  //Calculate rest
  NumberOfStrips = MultiplexFactor * NumberOfChannels;


  //SUMMARY
  std::cout << "\033[1;34 --> Creating / processing events: \033[0m \033[1;31m "                       << Nevent << "\033[0m \n";
  std::cout << "\033[1;34 --> Channels: \033[0m \033[1;31m "                 << NumberOfChannels << "\033[0m \n";
  std::cout << "\033[1;34 --> Multiplex Factor: \033[0m \033[1;31m "         << MultiplexFactor << "\033[0m \n";
  std::cout << "\033[1;34 --> Strips: \033[0m \033[1;31m "                   << NumberOfStrips << "\033[0m \n";
  std::cout << "\033[1;34 --> Simulating Clusters: \033[0m \033[1;31m "      << NClusters << "\033[0m \n";  
  std::cout << "\033[1;34 --> MPV Charge: \033[0m \033[1;31m "               << MPVCharge << "\033[0m \n";
  std::cout << "\033[1;34 --> Sigma: \033[0m \033[1;31m "                    << Sigma << "\033[0m \n";
  std::cout << "\033[1;34 --> Simulating Number Of Clusters: \033[0m \033[1;31m " << NClusters << "\033[0m \n";
  std::cout << "\033[1;34 --> Output set at compression level: \033[0m \033[1;31m " << compression << "\033[0m \n";

  if(!MultiplexFromFile)
   {
    std::cout << "\033[1;34 --> Multiplex map will be built using prime number algorithm: \033[0m \n";
   }
  else
   {
    std::cout << "\033[1;34 --> Multiplex map will be built from file: \033[0m \n";
   }
  if(SaveWaveforms)
   {
    std::cout << "\033[1;34 --> All waveforms will be saved in histograms: \033[0m \n";
   }
  if(ApplyMinimization)
   {
    std::cout << "\033[1;34 --> All will be minimized before storing: \033[0m \n";
   }
    

  //CreateOutDirectory();
  std::cout << "\033[1;34 --> Result will be saved in directory: \033[0m \033[1;31m " << outdir << "\033[0m \n";

  return true;
 
 }



 bool MyConfiguration::CreateOutDirectory()
 {
  boost::filesystem::create_directories(outdir.c_str());
  return true;
 }

 bool MyConfiguration::SaveParamterInTree(TTree* tree)
 {
  TList* list = new TList();

  Utils::AddParameterToList(list, "NumberOfChannels", NumberOfChannels);
  Utils::AddParameterToList(list, "NumberOfStrips",   NumberOfStrips);
  Utils::AddParameterToList(list, "MultiplexFactor",  MultiplexFactor);
  Utils::AddParameterToList(list, "MPVCharge",        MPVCharge);
  Utils::AddParameterToList(list, "Sigma",            Sigma);
  Utils::AddParameterToList(list, "NClusters",        NClusters);
  Utils::AddParameterToList(list, "Minimization",     ApplyMinimization);

  //add to user info
  Utils::AddInfoParameters(tree, list);

  return true;
  
 }
 

}//END NAMESPACE CONFIG
