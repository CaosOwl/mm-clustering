#pragma once
//root
#include <TFile.h>
#include <TTree.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TProfile2D.h>
#include <TChain.h>
#include <TList.h>
#include <TGenPhaseSpace.h>
#include <TLorentzVector.h>
#include <TParameter.h>
#include <TString.h>

//std library
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <cstdio>
#include <memory>

namespace OS
{
#include <signal.h>
}

using namespace std;

//piece of code to check chisq2 of selected tracks

double GetChi2Track(const double mom,
																				std::vector<double>* reco_mom,
																				std::vector<double>* reco_chi2
																				);


//simple shower check
double EcalRatio(vector<double>* Ee,vector<double>* Ex,vector<double>* Ey);


///CLASS TO REDIRECT OUTPUT TO A FILE
class redirect_outputs
{
 std::ostream& myStream;
 std::streambuf *const myBuffer;
 
public:
 redirect_outputs ( std::ostream& lhs, std::ostream& rhs);

 ~redirect_outputs ();
};

// redirect output stream to a string.
class capture_outputs
{
 std::ostringstream myContents;
 const redirect_outputs myRedirect;
public:
 capture_outputs ( std::ostream& stream);
 std::string contents () const;
};


//calculate predicted angle in Lab System using assumed decay and energy detected downstream
//NOTE coded for two body decays
/*
  - Masses in GeV
  - Energy in GeV
  - Angle is returned in radiant
*/
double PredictedAngle(const double M,const double m1,const double m2,const double E,const TVector3& mom1,const TVector3& mom2);


namespace Utils
{

 string GetBaseName(const char* input,const char* escape = "/",const char* extension = ".");


 string GetExtension(const char* input,const char* escape = ".");


 string GetDir(const char* input,const char* escape = "/");

 string RemoveExtension(const char* input,const char* escape);

 bool FileExist(TString);

 
 template<typename temp> bool SafeSetBranchAddress(TTree* chain,const char* BranchName,temp& var, const uint Verbose = 0)
 {
		TBranch* br = (TBranch*)chain->GetListOfBranches()->FindObject(BranchName);
		if(br)
   {
    chain->SetBranchAddress(BranchName, &var);
    if(Verbose > 0)
     cout << "Branch with name: " << BranchName << " Exist! Variable was assigned \n";  
    return true;
   }
		else if(Verbose > 0)
   {
    cout << "Branch with name: " << BranchName << " does not exist, variable will be not assigned" << endl;
    chain->SetBranchStatus(BranchName, 0);    
   }
  return false;
 }
 
 inline double GetParameter(TList* list,const char* parname, const uint Verbose = 0)
 {
  TParameter<double>* par = (TParameter<double>*)list->FindObject(parname);
  if(!par)
   {
    std::cerr << "parameter with name: " << parname << " not found in the list \n";
    return -9999.;
   }
  else
   {
    return par->GetVal();
   }
 }

 inline UInt_t GetIntParameter(TList* list,const char* parname, const uint Verbose = 0)
 {
  TParameter<UInt_t>* par = (TParameter<UInt_t>*)list->FindObject(parname);
  if(!par)
   {
    std::cerr << "parameter with name: " << parname << " not found in the list \n";
    return 9999;
   }
  else
   {
    return par->GetVal();
   }
 }

 //check if branch exist in chain
 bool BranchExist(TTree* chain,TString BranchName);

 void AddInfoParameters(TTree* tree, TList* list);

 template<class var>
 bool AddParameterToList(TList* list, const char* name, var target)
 {
  TParameter<var> *par = new TParameter<var>(name, target);
  list->Add(par);
  return true;
 }

 
inline void ImportReverseMultiplexMap(UInt_t* map, TTree* tree)
{
 //map
 UInt_t OriginalMap[64][5];
 
 //set branch
 Utils::SafeSetBranchAddress(tree, "MicromegasMap", OriginalMap, 1);
 tree->GetEntry(0);

 //assign it
 for(UInt_t chan(0); chan < 64; ++chan)
  for(UInt_t mfac(0); mfac < 5; ++mfac)
   {
    map[OriginalMap[chan][mfac]] = chan;
   }

}

 inline std::string GetCurrentGitHash()
 {
  
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("git rev-parse HEAD", "r"), pclose);
  if (!pipe) {
   throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
   result += buffer.data();
  }
  return result;

 }


 
 inline void SaveGitHashInTree(TTree* tree)
 {

  std::string result = GetCurrentGitHash();
  
  TString TheHash;

  tree->Branch("GitHash", &TheHash);

  TheHash = (result.c_str());

  tree->Fill();

  TParameter<int>* hash = new TParameter<int>(result.c_str(),10);
  
  tree->GetUserInfo()->Add(hash);

 }
} //end namespace Utils

namespace OS
{ 
 enum MyCommands {NORMAL, INTERRUPTLOOP};

 //TODO: IMPLEMENT UTILITY TO INTERFACE WITH SIGNAL CATCHING 
 
 //struct OS::sigaction myaction;
  
 void interrupt_loop(int sig);
 
 void SetStandardInterruptOption();  

}//end Namespace OS

namespace LOG
{
 // redirect outputs to another output stream.
 class redirect_outputs
 {
  std::ostream& myStream;
  std::streambuf *const myBuffer;
 public:
  redirect_outputs ( std::ostream& lhs, std::ostream& rhs);
  ~redirect_outputs ();
 };
 
 // redirect output stream to a string.
 class capture_outputs
 {
  std::ostringstream myContents;
  const redirect_outputs myRedirect;
 public:
  capture_outputs ( std::ostream& stream);
  std::string contents () const;
 };
 
}//end namespace log

//Utility for the histograms

namespace HistogramUtils
{
 struct HistoPar
 {
  unsigned int nbin; //number of bins  
  double lr; //low range
  double hr; //high range

  HistoPar();
  HistoPar(unsigned int, double, double);
 };

 //Fill all object in a container
 //works as long as the type has the fill option
template<class mycontainer>
void FillAll(std::vector<mycontainer>& vec,const bool IsSignal)
{
 for(auto p = vec.begin();p!=vec.end();++p)
  {
   (*p)->Fill(IsSignal);
  }
}
}
