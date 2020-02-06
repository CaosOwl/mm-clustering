#include"utility.hh"
//root
#include"TSystemFile.h"

using namespace std;

//piece of code to check chisq2 of selected tracks

double GetChi2Track(const double mom,
																				std::vector<double>* reco_mom,
																				std::vector<double>* reco_chi2
																				)
{
		//cout << "momentum to search: " << mom << endl;
		const double tol = 0.1; //tolerance over momentum
		for(unsigned int i = 0; i < reco_mom->size(); i++){
				//cout << " MOM: " << (*reco_mom)[i] << "Chi2: " << (*reco_chi2)[i] << endl;
				if(fabs((*reco_mom)[i] - mom) < tol)
						{
								//cout << "SELECTED MOM: " << (*reco_mom)[i] << "Chi2: " << (*reco_chi2)[i] << endl;
								return (*reco_chi2)[i];
						}
				
		}
		//in case no candidate is found
		cout << "Error: no candidate found" << endl;
		return 9999;
}


//simple shower check
double EcalRatio(vector<double>* Ee,vector<double>* Ex,vector<double>* Ey)
{
		double Eouter(0.),Etot(0.);
		for(unsigned int i=0; i < Ee->size(); i++) { 
				if(Ey->at(i) != 3)continue;
				if(Ex->at(i) != 2 && Ex->at(i) != 3)
						{
								Eouter += Ee->at(i);
								Etot += Ee->at(i);
						}
				else
						{
								Etot   += Ee->at(i);
						}
		} //end loop
				
		if(Etot == 0)
				return 1.;
		else
				return Eouter / Etot;
}//EcalRatio


///CLASS TO REDIRECT OUTPUT TO A FILE
redirect_outputs::redirect_outputs ( std::ostream& lhs, std::ostream& rhs=std::cout )
 : myStream(rhs), myBuffer(myStream.rdbuf())
{
 myStream.rdbuf(lhs.rdbuf());
}

redirect_outputs::~redirect_outputs () {
 myStream.rdbuf(myBuffer);
}

// redirect output stream to a string.
capture_outputs::capture_outputs ( std::ostream& stream=std::cout )
 : myContents(), myRedirect(myContents, stream)
{}

std::string capture_outputs::contents () const
{
 return (myContents.str());
}


//calculate predicted angle in Lab System using assumed decay and energy detected downstream
//NOTE coded for two body decays
/*
- Masses in GeV
- Energy in GeV
- Angle is returned in radiant
 */
double PredictedAngle(const double M,const double m1,const double m2,const double E,const TVector3& mom1,const TVector3& mom2)
{
 const double gamma = E / M;
 TVector3 boost(0.,0.,gamma);
 //build original lorentz vector alligned in Z direction
 const TVector3 summom = mom1 + mom2;
 TLorentzVector vec;
 vec.SetXYZM(summom.X(),summom.Y(),summom.Z(),M);
 vec.SetE(E);
 //Use the class to generated phase space
 double masses[] = {m1,m2};
 TGenPhaseSpace decay;
 decay.SetDecay(vec,2,masses);
 //generate decay
 decay.Generate();
 //get lorentz vector of the decay
 TLorentzVector* vec1 = decay.GetDecay(0);
 TLorentzVector* vec2 = decay.GetDecay(1);
 vec1->Boost(boost);
 vec2->Boost(boost);
 //return angle between the two
 return vec1->Angle(vec2->Vect());
}


namespace Utils
{

string GetBaseName(const char* input,const char* escape, const char* extension)
{
		string filename(input);
		// Remove directory if present.
		// Do this before extension removal incase directory has a period character.
		const size_t last_slash_idx = filename.find_last_of(escape);
		if (std::string::npos != last_slash_idx)
				{
						filename.erase(0, last_slash_idx + 1);
				}
		
		// Remove extension if present.
		const size_t period_idx = filename.rfind(extension);
		if (std::string::npos != period_idx)
				{
						filename.erase(period_idx);
				}
		return filename;
} //end of GetBase Name


 string GetExtension(const char* input,const char* escape)
{
		string filename(input);
		// Remove directory if present.
		// Do this before extension removal incase directory has a period character.
		const size_t last_slash_idx = filename.find_last_of(escape);
		if (std::string::npos != last_slash_idx)
				{
						filename.erase(0, last_slash_idx + 1);
				}
  
		return filename;
} //end of GetBase Name


string GetDir(const char* input,const char* escape)
{
		string filename(input);
		// Remove directory if present.
		// Do this before extension removal incase directory has a period character.
		const size_t last_slash_idx = filename.find_last_of(escape);
		if (std::string::npos != last_slash_idx)
				{
     filename.erase(last_slash_idx + 1, filename.size());
				}
		return filename;
} //end of GetDir Name


 string RemoveExtension(const char* input,const char* escape)
 {
		std::string filename(input);
  const std::string ext(escape);
  if ( filename != ext &&
       filename.size() > ext.size() &&
       filename.substr(filename.size() - ext.size()) == ext )
   {
    // if so then strip them off
    filename = filename.substr(0, filename.size() - ext.size());
   }  
		return filename;
} //end of GetDir Name

 bool FileExist(TString FileName)
 {
  const char* filename  = FileName.Data();
  const char* directory = (GetDir(filename)).c_str();
  TFile* thisfile = new TFile(filename, "READ");
  
  if(thisfile)
   {
    thisfile->Close();
    return true;
   }
  else
   {
    return false;
   }
  
 }

  //moved in the header to keep template implementation
 #if 0
 template<typename temp> void SafeSetBranchAddress(TTree* chain,const char* BranchName,temp& var, const uint Verbose)
 {
		TBranch* br = (TBranch*)chain->GetListOfBranches()->FindObject(BranchName);
		if(br)
   {
    chain->SetBranchAddress(BranchName,&var);
   }
		else if(Verbose > 0)
   {
    cout << "Branch with name: " << BranchName << " does not exist, variable will be not assigned" << endl;
   }				
 }
 #endif
 
//function to check if branch exist before set the branch address
bool BranchExist(TTree* chain,TString BranchName)
{ 
		TBranch* br = (TBranch*)chain->GetListOfBranches()->FindObject(BranchName);
		if(br)
				{
						return true;
				}
		else
				{
						return false;
				}				
}

 
 } //end namespace Utils



void Utils::AddInfoParameters(TTree* tree, TList* list)
{
		if(!tree || !list)
				{
						cout << "ERROR: one of the pointers is empty, " << endl;
				}
		TIter next(list);
		TObject* object = NULL;
		while ((object = next()))
				{
						tree->GetUserInfo()->Add(object);
				}
}

namespace OS
{ 
 //function to interrupt program
 void interrupt_loop(int sig)
 {
  //programflag = INTERRUPTLOOP;
 }

 void SetStandardInterruptOption()
 {
  //myaction.sa_handler = interrupt_loop;
  //OS::sigaction(SIGINT, &myaction, NULL);
 }
}//end Namespace OS

namespace LOG
{
 // redirect outputs to another output stream.
 redirect_outputs::redirect_outputs ( std::ostream& lhs, std::ostream& rhs=std::cout )
  : myStream(rhs), myBuffer(myStream.rdbuf())
 {
  myStream.rdbuf(lhs.rdbuf());
 }

 redirect_outputs::~redirect_outputs () {
  myStream.rdbuf(myBuffer);
 }

// redirect output stream to a string.
capture_outputs::capture_outputs ( std::ostream& stream=std::cout )
 : myContents(), myRedirect(myContents, stream)
{}

std::string capture_outputs::contents () const
{
 return (myContents.str());
}

}//end namespace log

//Histogram utils
namespace HistogramUtils
{
 HistoPar::HistoPar(unsigned int _nbin, double _lr, double _hr)
  :
  nbin(_nbin), lr(_lr), hr(_hr) {}  
}
