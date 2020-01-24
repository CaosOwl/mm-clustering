//user
#include"MyConfiguration.hh"


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
  configuration.GetOption(Nevent,                "events"          );   
  configuration.GetOption(Verbose,               "verbose"         );   
  configuration.GetOption(NumberOfChannels,      "channels"        );   
  configuration.GetOption(MultiplexFactor,       "mfactor"         );
  configuration.GetOption(NClusters,             "nclusters"       );   
  configuration.GetOption(Sigma,                 "amplitude"       );
  configuration.GetOption(MPVCharge,             "charge"          );
  configuration.GetOption(outdir,                "outdir"          );

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

  if(!MultiplexFromFile)
   {
    std::cout << "\033[1;34 --> Multiplex map will be built using prime number algorithm: \033[0m \n";
   }
  else
   {
    std::cout << "\033[1;34 --> Multiplex map will be built from file: \033[0m \n";
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
 

}//END NAMESPACE CONFIG
