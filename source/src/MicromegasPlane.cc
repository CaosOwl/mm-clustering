#include "MicromegasPlane.hh"


namespace Micromega

{
 MicromegasPlane::MicromegasPlane()
  :
  tree(nullptr),
  PeakFinder(nullptr)
 {
  ResetOutput();
  ResetMap();  
 }

  MicromegasPlane::MicromegasPlane(, TTree* tree_, const char* branchname)
  :
  tree(tree_),
  PeakFinder(nullptr)
 {
  ResetOutput();
  ResetMap();
  if(!tree)
   {
    std::cerr << "WARNING: TREE WAS NOT FOUND IN INITIALIZATION OF @MICROMEGASPLANE \n";
   }
  
  //Initialize branch
  SafeSetBranchAddress(tree, branchname, ChanOutput, 1);

  //Initialize peak finder
  PeakFinder = new TSpectrum(5, 1);
  
 }

 MicromegasPlane::ResetOutput()
 {
  for(UInt_t i(0); i < NCHAN; ++i)
   {
    ChanOutput[i] = 0;
   }
  for(UInt_t i(0); i < NCHAN*MFACTOR; ++i)
   {
    StripsOutput[i] = 0;
   }
 }

 MicromegasPlane::ResetMap()
 {
  for(UInt_t i(0); i < NCHAN; ++i)
   for(UInt_t j(0); j < MFACTOR; ++j)
   {
    MultiplexMAP[i][j] = 0;
   }
 }


 MicromegasPlane::GenerateStrips(double MultiplexMAP[NCHAN][MFACTOR])
 {

 }

 
 

}
