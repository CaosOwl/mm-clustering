#pragma once
//std library
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream> 
#include <ctime>

//root
#include "TTree.h"
#include "TParameter.h"
#include "TList.h"
#include "TSpectrum.h"
#include "TF1.h"

//user
#include"utility.hh"

#define NCHAN 64
#define MFACTOR 5


using namespace std;

namespace Micromega
{
 Class MicromegasPlane
 {

 public:

 //CONSTRUCTOR
 MicromegasPlane();
 MicromegasPlane(const char*, TTree*);

 void GenerateStrips(double MultiplexMAP[NCHAN][MFACTOR]);
 //bool InitializeTree(TTree*);
 //bool InitializeTree(const char*);
 TH1F* GenerateStripsHistogram();
 void GenerateDoubleClusterHistogram(TH1F*, TH1F*, TH1F*);
 //void InitializeMultiplexingMap(double MultiplexMAP[NCHAN][MFACTOR]);
 void InitializeMultiplexingMap(TTree*);


 private:

 TTree* tree;

 TSpectrum* PeakFinder;
 
 Uint_t ChanOutput[NCHAN];
 Uint_t StripsOutput[NCHAN*MFACTOR];
 Uint_t MultiplexMAP[NCHAN][MFACTOR];

 //private functions
 void ResetOutput();
 void ResetMap();
  
   
 }; //end class micromegasplane

} //end MM namespace
