//root
#include"TRandom3.h"
#include"TRandom2.h"
//std
#include<fstream>
//user
#include"ToyDataCreator.hh"
#include"Minimizationfunctions.hh"
#include"utility.hh"

namespace Micromega
{
 
 ToyDataCreator::ToyDataCreator(const UInt_t NumberOfChannels_,
                                const UInt_t MultiplexFactor_,
                                const UInt_t MPVCharge_,
                                const Double_t Sigma_,
                                const MapMethod mapmethod
                                )
  :
  NumberOfChannels(NumberOfChannels_),
  MultiplexFactor(MultiplexFactor_),
  MPVCharge(MPVCharge_),
  ChargeSigma(20.), //from Landau fit of run 4238 MM3X
  Sigma(Sigma_),
  verbose(0),
  LambdaMin(1.),
  noisemethod(NoiseMethod::NONE),
  clustermethod(ClusterMethod::GAUS)  
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

  //reserve noise
  Noise.reserve(NumberOfChannels);
  for(UInt_t n(0); n < NumberOfChannels; ++n)Noise[n] = 0;

  //initialize maps
  FillRegMatrix();
  FillMultiplexingMatrix(mapmethod);                  
 }
 
 ToyDataCreator::ToyDataCreator(const UInt_t NumberOfChannels_,
                                const UInt_t MultiplexFactor_,
                                const UInt_t MPVCharge_,
                                const Double_t Sigma_,
                                const TString filename
                                )
  :
  NumberOfChannels(NumberOfChannels_),
  MultiplexFactor(MultiplexFactor_),
  MPVCharge(MPVCharge_),
  ChargeSigma(20.), //from Landau fit of run 4238 MM3X  
  Sigma(Sigma_),
  verbose(0),
  LambdaMin(1.),  
  noisemethod(NoiseMethod::NONE),
  clustermethod(ClusterMethod::GAUS)
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

  //reserve noise
  Noise.reserve(NumberOfChannels);
  for(UInt_t n(0); n < NumberOfChannels; ++n)Noise[n] = 0;

  //initialize maps
  FillRegMatrix();
  //decide what kind of file it is
  const bool ItExist = Utils::FileExist(filename);
  if(ItExist)
   {
    if(filename.EndsWith(".root"))
     {
      BuildMultiplexingFromROOTFile(filename);
     }
    else if(filename.EndsWith(".txt"))
     {
      BuildMultiplexingFromTXTFile(filename);
     }
    else
     {
      std::cout << "\033[1;34m WARNING: \033[0m" << filename << " has an unknown format, using algorithm for multiplexing map \n";
      BuildMultiplexingFromAlgorithm();
     }
   }

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
                                    const Double_t clusterposition,
                                    const ClusterMethod clusmethod
                                    )
 {

  Double_t clustertotalcharge(1e5);
  Double_t clustersigma(1e5);
  
  //create actual cluster
  switch(clusmethod)
   {    
   case GAUS:
    clustertotalcharge = gRandom->Landau(MPVCharge, ChargeSigma); //SSigma currently based on MM3 x plane
    clustersigma       = gRandom->Gaus(Sigma, 1);
    while(clustersigma < 0.5)clustersigma       = gRandom->Gaus(Sigma, 1);
    CreateClusterWithGaus(clusterposition, clustersigma, (UInt_t)clustertotalcharge, StripsOutput);
    break;
   case GAUSFROMFILE:
    //while(clustertotalcharge > 500)   //hard coded limit to cluster charge
    ChargeVsSigma->GetRandom2(clustertotalcharge, clustersigma);
    //correction
    clustersigma /= 2; //convert amplitude to gaus sigma WARNING: factor to check
    //create gaus
    CreateClusterWithGaus(clusterposition, clustersigma, (UInt_t)clustertotalcharge, StripsOutput);
   default:
    clustertotalcharge = gRandom->Landau(MPVCharge, ChargeSigma); //SSigma currently based on MM3 x plane
    clustersigma       = gRandom->Gaus(Sigma, 1);
    while(clustersigma < 0.5)clustersigma       = gRandom->Gaus(Sigma, 1);    
    CreateClusterWithGaus(clusterposition, clustersigma, (UInt_t)clustertotalcharge, StripsOutput);
    break;     
   }
  
  //save cluster main values
  positions.push_back(clusterposition);
  amplitudes.push_back(clustersigma);
  charges.push_back(clustertotalcharge);
  
 }

 void ToyDataCreator::CreateClusterWithGaus(const Double_t clusterposition,
                                            const Double_t clustersigma,
                                            const UInt_t clustertotalcharge,
                                            UInt_t* StripsOutput) const
 {

  TH1I* histo = new TH1I("dummy", "dummy", NumberOfStrips, -0.5, NumberOfStrips - 0.5);
  histo->SetDirectory(0);
  for(UInt_t i(0); i < clustertotalcharge; ++i)histo->Fill(gRandom->Gaus(clusterposition, clustersigma));
  for(UInt_t i(0); i < NumberOfStrips; ++i)StripsOutput[i] += histo->GetBinContent(i);
  //cleaning memory
  delete histo;
 }

 bool ToyDataCreator::GenerateToy(UInt_t* Chan,
                                  UInt_t* Strips_Physical,
                                  Double_t* Strips_Processed,
                                  const UInt_t NumberOfClusters,
                                  const bool DoMinimization,
                                  UInt_t minimaldistance,
                                  UInt_t maximaldistance                                  
                                  )
 {

  //reset variables
  for(UInt_t chan(0); chan < NumberOfChannels; ++chan)Chan[chan] = 0;
  for(UInt_t strip(0); strip < NumberOfStrips; ++strip){Strips_Physical[strip] = 0;Strips_Processed[strip] = 0.;}

  //redefine distance as needed
  maximaldistance = maximaldistance > NumberOfStrips ? NumberOfStrips : maximaldistance;
  if(minimaldistance >= maximaldistance)
   {
    std::cerr << "ERROR: minimal distance larger than maximal distance in @GenerateToy \n";
    return false;
   }
   

  //clear vectors
  Clear();

  //create desidered number of gausian
  Double_t clusterposition(-1);
  for(UInt_t n(0); n < NumberOfClusters; ++n)
   {
    //throw random values for all relevant output of a cluster    
    if(clusterposition < 0 || (minimaldistance == 0 && maximaldistance == NumberOfStrips))
     {
      clusterposition = gRandom->Uniform(0, NumberOfStrips);
     }
    else
     {
      Double_t distance    = (1. - 2. * gRandom->Integer(2) ) * gRandom->Uniform(minimaldistance, maximaldistance);
      UInt_t counter(1);
      while(((clusterposition+distance) < 0 || (clusterposition+distance) > NumberOfStrips) && counter < 5)
       {
        distance    = (1. - 2. * gRandom->Integer(2) ) * gRandom->Uniform(minimaldistance, maximaldistance);
        ++counter;
       }
      //assign it
      if(counter > 5)
       clusterposition = gRandom->Uniform(0, NumberOfStrips); //roll back to uniform (add a warning?)
      else       
       clusterposition += distance; //add distance      
     }
    
    CreateCluster(Strips_Physical,
                  clusterposition,
                  clustermethod
                  );
   }



  //define multiplexing output
  UInt_t chan(0);
  for(UInt_t strip(0); strip < NumberOfStrips; ++strip)
   {
    Chan[ReverseMultiplexMAP[strip]] += Strips_Physical[strip];
   }

  
  
  //add the entry to the strip after a smearing,
  //NOTA: nbins = nstriips
  //NOTA2: Poisson used for tyhe smearing
  for(UInt_t chan(0); chan < NumberOfChannels; ++chan)
   {
    Double_t value(0.),noise(0.);
    value = Chan[chan];
  
    //add noise tot the strips
    switch(noisemethod)
     {
     case NONE:
      break;
     case FROMFILE:
      //use saved noise
      noise = gRandom->Gaus(0, Noise[chan]);
      break;
     default:
      break;
     }

    if(noise + value > 0. )
     Chan[chan] = value + noise;
    else
     Chan[chan] = 0;
 }
  
  //Apply multiplexing, remove noise and remove single strips
  for(UInt_t strip(0); strip < NumberOfStrips; ++strip)
   {
    if(IsStripValid(Chan, strip))
     {
      Strips_Processed[strip] = Chan[ReverseMultiplexMAP[strip]];
     }
    else
     {
      Strips_Processed[strip] = 0;
     }
   }

  //do minimization if required
  if(DoMinimization)
   {
    //Solve the numericalproblem
    for(UInt_t chan(0); chan < NumberOfChannels; ++chan)data_v.push_back(Chan[chan]);
    NumericalMinimization(Strips_Processed, verbose, LambdaMin);
   }
    
  return true;
  
  
  
 }

 void ToyDataCreator::FillMultiplexingMatrix(const MapMethod mapmethod)
 {
  // populate the matrix wit the multiplexing algorithm values
  
  mmultiplex.reserve(VectorXd::Constant(NumberOfStrips, MultiplexFactor));

  //prepare string
  TString mapname;

  switch(mapmethod)
   {
   case ALGO:
    BuildMultiplexingFromAlgorithm();
    break;
   case TXT:
    //search if such map exist
    mapname.Form("multiplexingmaps/multiplexing_p%i_m%i.txt", NumberOfChannels, MultiplexFactor);
    //check if it exist
    if(Utils::FileExist(mapname))
     {
      BuildMultiplexingFromTXTFile(mapname);
     }
    else
     {
      std::cout << "\033[1;34 WARNING \033[0m File: " << mapname << "does not exist, building standard multiplex map \n";
      BuildMultiplexingFromAlgorithm();
     }
    break;
   case ROOT:
    BuildMultiplexingFromROOTFile(mapname);
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
    Double_t flag;
    while(reader >> channel >> strip >> flag)
     {      
      if(flag > 0)
       {
        mmultiplex.insert(channel, strip) = 1;
        ReverseMultiplexMAP[strip] = channel;
       }
     }
   }
  mmultiplex.makeCompressed();  
  std::cout << "Map successfully red from file: " << filename << "\n";
  
 } // from txt finish function

 void ToyDataCreator::BuildMultiplexingFromROOTFile(const char* filename)
 {
  //check if it exist
  const bool ItExist = Utils::FileExist(filename);
  if(!ItExist)
   {
    std::cout << "WARNING: " << " no file with name: " << filename << " building map from algorithm \n";
    BuildMultiplexingFromAlgorithm();
    return;
   }
  //create root file
  TFile* file = new TFile(filename, "READ");
  TTree* tree = (TTree*)file->Get("InfoTree"); //search foir standard tree
  if(!tree)
   {
    std::cout << "WARNING: " << " no tree called InfoTree in file: " << filename << " building map from algorithm \n";
   }

  //build maps
  UInt_t MAP[NumberOfChannels][MultiplexFactor];
  tree->SetBranchAddress("MicromegasMap", MAP);
  tree->GetEntry(0);

  //import it in object
  for(UInt_t chan(0); chan < NumberOfChannels; ++chan)
   for(UInt_t mfac(0); mfac < MultiplexFactor; ++mfac)
    {
     const UInt_t strip = MAP[chan][mfac];
     ReverseMultiplexMAP[strip] = chan;
     mmultiplex.insert(chan, strip) = 1;
    }

  mmultiplex.makeCompressed();
  file->Close();
  
 }

 void ToyDataCreator::InitializeNoiseFromROOTFile(const char* filename, const char* branchname)
 {
  //check if it exist
  const bool ItExist = Utils::FileExist(filename);
  if(!ItExist)
   {
    std::cout << "WARNING: " << " no file with name: " << filename << " building map from algorithm \n";
    BuildMultiplexingFromAlgorithm();
    return;
   }
  //create root file
  TFile* file = new TFile(filename, "READ");
  TTree* tree = (TTree*)file->Get("InfoTree"); //search foir standard tree
  if(!tree)
   {
    std::cout << "WARNING: " << " no tree called InfoTree in file: " << filename << " building map from algorithm \n";
   }

  //build maps
  UInt_t NOISE[NumberOfChannels];
  tree->SetBranchAddress(branchname, NOISE);
  tree->GetEntry(0);

  //import it in object
  for(UInt_t chan(0); chan < NumberOfChannels; ++chan)
   {
    Noise[chan] = NOISE[chan];
   }
  //exit(1);
  file->Close();  
 }

 std::vector<UInt_t> ToyDataCreator::CreateMultiplexedStrips(UInt_t* Chan) const
 {
  std::vector<UInt_t> STRIPS;
  STRIPS.reserve(NumberOfStrips);
  for(UInt_t strip(0); strip < NumberOfStrips; ++strip)STRIPS[strip] = Chan[ReverseMultiplexMAP[strip]];

  return STRIPS;
                                                                            
 }

 bool ToyDataCreator::IsStripValid(const UInt_t* Chan, const UInt_t strip, const UInt_t noisesigma) const
 {
  const bool prestripvalid =  strip == 0 ? false : (Chan[ReverseMultiplexMAP[strip - 1]] > noisesigma*Noise[ReverseMultiplexMAP[strip]]);
  const bool poststripvalid =  strip == (NumberOfStrips-1) ? false : (Chan[ReverseMultiplexMAP[strip + 1]] > noisesigma*Noise[ReverseMultiplexMAP[strip]]);
  const bool stripvalid = Chan[ReverseMultiplexMAP[strip]] > Noise[ReverseMultiplexMAP[strip]];

  const bool isvalid = stripvalid && (prestripvalid || poststripvalid);
  

  return isvalid;
 }

 //DoLambdaScan
 void ToyDataCreator::DoLambdaScan(UInt_t* Chan,
                                   std::vector<Double_t>& minima,
                                   std::vector<Double_t>& scanpoint)                   
 {
  //Clear vector
  Clear();
  //Define solutions
  Double_t solutions[NumberOfStrips];  
  //load info into data
  for(UInt_t chan(0); chan < NumberOfChannels; ++chan)data_v.push_back(Chan[chan]);

  //apply minimization for each scan point
  for(UInt_t i(0); i < 10; ++i)
   {
    const Double_t lambda = scanpoint[i];
    NumericalMinimization(solutions, verbose, lambda);
    //get minimum
    const Double_t min = LogLikelihood(solutions);
    //load it into the histogram
    minima[i] = min;
    std::cout << "lambda: " << lambda << " min: " << min << " \n";
   }
 }

 void ToyDataCreator::InitializeChargeVsSigmaHisto(const char* filename, const char* histoname)
 {
  TFile* file = new TFile(filename, "READ");
  //histo
  ChargeVsSigma = (TH2F*)file->Get(histoname);
  if(ChargeVsSigma)
   {
    std::cout << "histogram with name: " << histoname << " successfuly loaded from file: " << filename << " \n";
    clustermethod = GAUSFROMFILE;
   }
  else
   {
    std::cout << "histogram with name: " << histoname << " was not found in file: " << filename << " \n";
   }
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

 void ToyDataCreator::SaveParameterInTree(TTree* tree) const
 {
  UInt_t MicromegasMap[NumberOfChannels][MultiplexFactor];
  UInt_t NOISE[NumberOfChannels];
  tree->Branch("MicromegasMap", &MicromegasMap, TString::Format("MicromegasMapint[%i][%i]/i", NumberOfChannels, MultiplexFactor));
  tree->Branch("Noise", &NOISE, TString::Format("Noise[%i]/i", NumberOfChannels));  
  for(UInt_t chan(0); chan < NumberOfChannels; ++chan)
   {
    NOISE[chan] = Noise[chan];
    for(UInt_t mfac(0); mfac < MultiplexFactor; ++mfac)
     {
      //find strip, (very inefficient)
      UInt_t strip(0);
      while(ReverseMultiplexMAP[strip] != chan)++strip;
      MicromegasMap[chan][mfac] = strip;
     }
   }

  //fill it
  tree->Fill();
   

 }


} // END NAMESPACE MICROMEGAS
