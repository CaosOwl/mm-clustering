//root
#include"TRandom3.h"
#include"TRandom2.h"
//std
#include<fstream>
//user
#include"ToyDataCreator.hh"
#include"Minimizationfunctions.hh"

namespace Micromega
{
 
 ToyDataCreator::ToyDataCreator(const UInt_t NumberOfChannels_,
                                const UInt_t MultiplexFactor_,
                                const UInt_t MPVCharge_,
                                const Double_t Sigma_
                                )
  :
  NumberOfChannels(NumberOfChannels_),
  MultiplexFactor(MultiplexFactor_),
  MPVCharge(MPVCharge_),
  Sigma(Sigma_)
 {
  //calcualte number of strips
  NumberOfStrips = NumberOfChannels * MultiplexFactor;
  //Initiliaze Balint Matrices (defined at global level)
  mmultiplex = SparseMatrix<double>(NumberOfChannels, NumberOfStrips);
  vstrips = MatrixXd(NumberOfStrips, 1);
  vmultiplex = MatrixXd(NumberOfChannels, 1);
  mQ2 = MatrixXd(NumberOfStrips, 1);
  mQ2T = MatrixXd(1, NumberOfStrips);
  ms = MatrixXd(1,1);
  mreg = SparseMatrix<double>(NumberOfStrips, NumberOfStrips);
  //other globals
  NStrips = NumberOfStrips;
  NChan   = NumberOfChannels;
  MFac    = MultiplexFactor;
  //reverse multiplex map
  ReverseMultiplexMAP = std::vector<UInt_t>(NumberOfStrips, 0);

  //initialize maps
  FillRegMatrix();
  FillMultiplexingMatrix(MapMethod::ALGO);
                
  
 }

 void ToyDataCreator::Clear()
 {
  //Clear all vectors and Matrices
  positions.clear();
  amplitudes.clear();
  charges.clear();
  data_v.clear();
 }


 void ToyDataCreator::CreateCluster(UInt_t* StripsOutput,
                                    const ClusterMethod clusmethod,
                                    const NoiseMethod noisemethod)
 {
  //throw random values for all relevant output of a cluster
  const Double_t clusterposition    = gRandom->Uniform(0, NumberOfStrips);
  const Double_t clustersigma       = gRandom->Poisson(Sigma);
  const UInt_t   clustertotalcharge = gRandom->Landau(MPVCharge, 20); //SSigma currently based on MM3 x plane

  //create actual gaussian
  switch(clusmethod)
   {
   case GAUS:
    for(UInt_t i(0); i < clustertotalcharge; ++i)
     {
      //select strips
      UInt_t strip(NumberOfStrips + 1);
      while( strip > NumberOfStrips)
       strip = (UInt_t)(gRandom->Gaus(clusterposition, clustersigma));
      StripsOutput[strip] += 1;
     }
    break;
   default:
    for(UInt_t i(0); i < clustertotalcharge; ++i)
     {
      //select strips
      UInt_t strip(NumberOfStrips + 1);
      while( strip > NumberOfStrips)
       strip = (UInt_t)(gRandom->Gaus(clusterposition, clustersigma));
      StripsOutput[strip] += 1;
     }    
    break;    
    
   }

  //add the entry to the strip after a smearing,
  //NOTA: nbins = nstriips
  //NOTA2: Poisson used for tyhe smearing
  for(UInt_t strip(0); strip < NumberOfStrips; ++strip)
   {
    StripsOutput[strip] = gRandom->Poisson(StripsOutput[strip]);

    //add noise tot the strips
    switch(noisemethod)
     {
     case NONE:
      break;
     default:
      break;
     }
   }

  //save cluster main values
  positions.push_back(clusterposition);
  amplitudes.push_back(clustersigma);
  charges.push_back(clustertotalcharge);
  
 }

 bool ToyDataCreator::GenerateToy(UInt_t* Chan,
                                  UInt_t* Strips_Physical,
                                  UInt_t* Strips_Processed,
                                  const UInt_t NumberOfClusters,
                                  const bool DoMinimization)
 {

  //reset variables
  for(UInt_t chan(0); chan < NumberOfChannels; ++chan)Chan[chan] = 0;
  for(UInt_t strip(0); strip < NumberOfStrips; ++strip){Strips_Physical[strip] = 0;Strips_Processed[strip] = 0;}


  //clear vectors
  Clear();

  //create desidered number of gausian
  for(UInt_t n(0); n < NumberOfClusters; ++n)
   {
    CreateCluster(Strips_Physical,
                  ClusterMethod::GAUS,
                  NoiseMethod::NONE);                  
   }

  //define multiplexing output
  UInt_t chan(0);
  for(UInt_t strip(0); strip < NumberOfStrips; ++strip)
   {
    //std::cout << strip << " " << ReverseMultiplexMAP[strip] << " \n";
    Chan[ReverseMultiplexMAP[strip]] += Strips_Physical[strip];
   } 


  if(DoMinimization)
   {
    //Solve the numericalproblem
    for(UInt_t chan(0); chan < NumberOfChannels; ++chan)data_v.push_back(Chan[chan]);
    std::cout << "before minimization \n";
    Strips_Processed = NumericalMinimization(2);
    std::cout << "after minimization \n";
   }
  else
   {
    //simply apply multiplexing
    for(UInt_t strip(0); strip < NumberOfStrips; ++strip)
     Strips_Processed[strip] = Chan[ReverseMultiplexMAP[strip]];
   }

  return true;
  
  
  
 }

 void ToyDataCreator::FillMultiplexingMatrix(const MapMethod mapmethod)
 {
  // populate the matrix wit the multiplexing algorithm values
  
  mmultiplex.reserve(VectorXd::Constant(NumberOfStrips, MultiplexFactor));

  switch(mapmethod)
   {
   case ALGO:
    BuildMultiplexingFromAlgorithm();
    break;
   case TXT:
    BuildMultiplexingFromTXTFile(targetfile);
    break;
   case ROOT:
    BuildMultiplexingFromROOTFile(targetfile);
    break;
   default:
    BuildMultiplexingFromAlgorithm();
   }
 }

 void ToyDataCreator::FillRegMatrix()
 {
  mreg.reserve(VectorXd::Constant(NumberOfStrips, 3));
  for(UInt_t i = 0; i < NumberOfStrips; i++){
   for(UInt_t j = 0; j < NumberOfStrips; j++){
    // diagonal
    if(i == j){
     mreg.insert(i,j) = -2.0;
    }
    if(i+1 == j || i-1 == j){       
     mreg.insert(i,j) = 1.0;
    }
   }
  }
  mreg.makeCompressed();
 }

 //FUNCTION TO CREATE MULTIPLEX MAP
 void ToyDataCreator::BuildMultiplexingFromAlgorithm()
 {
  UInt_t strip(0);
  for(UInt_t s(1); s < (MultiplexFactor+1); ++s)
   {
    for(UInt_t i(0); i < NumberOfChannels; ++i)
     {
      const UInt_t chan = ((i*s) % NumberOfChannels);
      ReverseMultiplexMAP[strip] = chan;
      mmultiplex.insert(chan, strip) = 1;      
      ++strip;
     }
   }
  mmultiplex.makeCompressed();
 }

 void ToyDataCreator::BuildMultiplexingFromTXTFile(const char* filename)
 {
  std::ifstream reader;
  reader.open(filename);
  if(!reader.is_open())
   {
    std::cerr << "file: " << filename << " not found, building map with the algorithm \n";
    BuildMultiplexingFromAlgorithm();
    return;
   }
  else
   {
    UInt_t strip, channel;
    UInt_t flag;
    while(true)
     {
      reader >> strip >> channel >> flag;
      if(!reader.good())break;
      if(flag > 0)
       {
        mmultiplex.insert(channel, strip) = flag;
        ReverseMultiplexMAP[strip] = channel;
       }
     }
   }
  std::cout << "Map successfully red from file: " << filename << "\n";
  
 } // from txt finish function

 void ToyDataCreator::BuildMultiplexingFromROOTFile(const char* filename)
 {
  filename = "";
  return;
 }


 //Get Function
 Double_t ToyDataCreator::GetPosition(UInt_t index)
 {
  if(index < positions.size())
   {
    return positions[index];
   }
  else
   {
    std::cout << index << " is larger than the number of clusters \n";
    return 999;
   }
 }

 Double_t ToyDataCreator::GetSigma(UInt_t index)
 {
  if(index < amplitudes.size())
   {
    return amplitudes[index];
   }
  else
   {
    std::cout << index << " is larger than the number of clusters \n";
    return 999;
   }
 }

 UInt_t ToyDataCreator::GetCharge(UInt_t index)
 {
  if(index < charges.size())
   {
    return charges[index];
   }
  else
   {
    std::cout << index << " is larger than the number of clusters \n";
    return 999;
   }
 }


} // END NAMESPACE MICROMEGAS
