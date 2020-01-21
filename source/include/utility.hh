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

//std library
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>

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

 //check if branch exist in chain
 bool BranchExist(TChain* chain,TString BranchName);

 void AddInfoParameters(TTree* tree, TList* list);
 
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
