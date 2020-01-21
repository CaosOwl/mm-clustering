#include "MicromegasPlane.hh"
//root
#include "TStyle.h"
#include "TFitResultPtr.h"

namespace Micromega

{
 MicromegasPlane::MicromegasPlane()
  :
  name("dummy"),
  tree(nullptr),
  PeakFinder(nullptr)
 {
  ResetOutput();
  ResetMap();  
 }

 MicromegasPlane::MicromegasPlane(const char* branchname, TTree* tree_, TTree* infotree_)
  :
  name(branchname),
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
  Utils::SafeSetBranchAddress(tree, branchname, ChanOutput, 1);

  //Initialize peak finder
  PeakFinder = new TSpectrum();

  //try to initialize noise using standard
  InitializeNoise(infotree_, TString::Format("%s_sigma", branchname));

  //try to initialize multiplex map from standard
  InitializeMultiplexingMap(infotree_);
                  
  
 }

 void MicromegasPlane::ResetOutput()
 {
  for(UInt_t i(0); i < NCHAN; ++i)
   {
    ChanOutput[i] = 0;
    ChanNoise[i] = 0;
    ChanSmear[i] = 0;
    NoiseMap[i] = 0;
   }
  for(UInt_t i(0); i < NCHAN*MFACTOR; ++i)
   {
    StripsOutput[i] = 0;
    StripsOutputProcessed[i] = 0;
   }
 }

 void MicromegasPlane::ResetMap()
 {
  for(UInt_t i(0); i < NCHAN; ++i)
   for(UInt_t j(0); j < MFACTOR; ++j)
    {
     MultiplexMAP[i][j] = 0;
    }
  
  for(UInt_t i(0); i < NCHAN*MFACTOR; ++i)
   {
    ReverseMultiplexMAP[i] = 0;    
   }
  
 }

 void MicromegasPlane::InitializeMultiplexingMap(TTree* infotree, const char* branchname)
 {
  if(Utils::SafeSetBranchAddress(infotree, branchname, MultiplexMAP, 1))
   {
    infotree->GetEntry(0);
    //save reverse map
    for(UInt_t i(0); i < NCHAN; ++i)
     for(UInt_t j(0); j < MFACTOR; ++j)
      {
       ReverseMultiplexMAP[MultiplexMAP[i][j]] = i;
      }
   }

  
 }

 void MicromegasPlane::InitializeNoise(TTree* infotree, const char* branchname)
 {
  if(Utils::SafeSetBranchAddress(infotree, branchname, NoiseMap, 1))
   {
    infotree->GetEntry(0);
   }
 }

 bool MicromegasPlane::AboveNoiseTreshold(UInt_t strip, double sigma)
 {
  const UInt_t allowednoise = sigma * NoiseMap[ReverseMultiplexMAP[strip]];
  return StripsOutputProcessed[strip] > allowednoise;
 }

 bool MicromegasPlane::CheckNeaboringStrips(UInt_t strip, UInt_t counter)
 {
  //one is for sure activated (central one)
  UInt_t activatestrip(-1);
  for(UInt_t n(strip - counter); n < (strip + counter); ++n)
   {
    if(StripsOutputProcessed[n] > 0)
     ++activatestrip;
   }
  
  return activatestrip >= counter;
 }

 void MicromegasPlane::GenerateSmearing()
 {
   
  for(unsigned int chan(0); chan < NCHAN; ++chan)
   {
    //use a simple poisson distribution for now
    const UInt_t signal = ChanOutput[chan];
    const UInt_t smear = gRandom->Poisson(signal);
    ChanSmear[chan] = smear;
    if(signal < smear)
     ChanOutput[chan] = 0;
    else
     ChanOutput[chan] += smear;
   }
   
 }

 void MicromegasPlane::GenerateNoise()
 {
  for(unsigned int chan(0); chan < NCHAN; ++chan)
   {
    const UInt_t signal = ChanOutput[chan];
    const UInt_t noise = gRandom->Gaus(0, NoiseMap[chan]);
    ChanNoise[chan] = noise;
    if(signal < noise)
     {
      //nothing to be done, signal sums to zero
      ChanOutput[chan] = 0;
     }
    else
     {
      //sum noise to signal in channel
      ChanOutput[chan] += noise;
     }
   }
    
 }


 void MicromegasPlane::GenerateStrips(
                                      const bool UseSmearing,
                                      const bool UseNoise,
                                      const bool ProcessStrip
                                      )  
 {
  
  if(UseSmearing)
   {
    GenerateSmearing();
   }
  
  if(UseNoise)
   {
    GenerateNoise();
   }
  
  //Generate strip using channel and multiplexed map
  for(unsigned int chan(0); chan < NCHAN; ++chan)
   for(unsigned int mfac(0); mfac < MFACTOR; ++mfac)
    {
     const unsigned int strip = MultiplexMAP[chan][mfac];
     StripsOutput[strip] = ChanOutput[chan];
     StripsOutputProcessed[strip] = ChanOutput[chan];
    }

  //automatically generate also the processed one
  if(ProcessStrip)
   GenerateProcessedStrips();
 }

 void MicromegasPlane::GenerateProcessedStrips()  
 {  
  
  //Generate strip using channel and multiplexed map
  for(unsigned int strip(0); strip < NCHAN*MFACTOR; ++strip)
   {

    //check for noise treshold
     
    if(!AboveNoiseTreshold(strip, 2))
     {
      StripsOutputProcessed[strip] = 0;
      continue;
     }

    if(strip == 0 || strip == (NCHAN*MFACTOR - 1))
     continue;

    //isolated strip
    //if(!CheckNeaboringStrips(strip, 1))
    if(StripsOutputProcessed[strip - 1] == 0 && StripsOutputProcessed[strip + 1] == 0)
     {
      StripsOutputProcessed[strip] = 0;
     }
   }
 }

 
 TH1F* MicromegasPlane::GenerateStripsHistogram(const ClusterSearchType searchtype,
                                                const int event,
                                                const bool FitHisto
                                                )
 {
  TH1F* result = new TH1F(TString::Format("%s-%i-waveform", name.Data(), event),
                          TString::Format("waveform of plane %s in event %i", name.Data(), event),
                          NCHAN*MFACTOR,
                          -0.5,
                          NCHAN*MFACTOR - 0.5
                          );

  //fill the histogram
  for(Int_t strip(0); strip < result->GetNbinsX(); ++strip)
   {
    result->SetBinContent(strip, StripsOutputProcessed[strip]);
   }

  //Search for Clusters
  switch(searchtype)
   {

   case PEAKSEARCH:
    SearchClusterWithSpectrum(result, FitHisto);
    break;

   case WW:
    SearchClusterWW(result, FitHisto);
    break;

   case NONE:
    break;

   default:
    SearchClusterWW(result, FitHisto);
       
   }         


  return result;
                          
 }

 void MicromegasPlane::PrintChannels(std::ostream& out)
 {
  
  out << "Channel output for this event was: \n";
  for(unsigned int chan(0); chan < NCHAN; ++chan)
   {
    out << ChanOutput[chan] << " "; 
   }
  out << "\n";
  
 }

 void MicromegasPlane::PrintNoises(std::ostream& out)
 {
  
  out << "Channel noise for this event was: \n";
  for(unsigned int chan(0); chan < NCHAN; ++chan)
   {
    out << ChanNoise[chan] << " ";
   }
  out << "\n";
  
 } 

 void MicromegasPlane::PrintEvent(std::ostream& out)
 {
  //event:
  PrintChannels(out);
  PrintNoises(out);
  //PrintStrips(out);
  //PrintMaps(out);

  
 }



 //CLUSTER SEARCH
 void MicromegasPlane::SearchClusterWW(TH1F* result,
                                       const bool FitHisto)
 {

  clusvec.clear();  

  //use threshold,
  //WARNING: THIS ASSUME THAT NOISE WAS ALREADY REMOVED
  bool clusterup = false;
  Micromega::Cluster clus;
  for(UInt_t strip = 0; strip < NCHAN*MFACTOR; ++strip)
   {
    const double charge = StripsOutputProcessed[strip];
    if(clusterup)
     {
      if(charge > 0)
       {
        clus.add_strip(strip, charge);
       }
      else
       {
        //finalize cluster
        clusvec.push_back(clus);
        clus.clear();
        clusterup = false;
       }
     }
    else
     {
      if(charge > 0)
       {
        clus.add_strip(strip, charge);
        clusterup = true;
       }
     }
   }

  if(clusvec.size() == 0)
   {
    //std::cout << "No Cluster in the event \n";
    return;
   }

  //find best cluster by integrated charge
  auto bestcluster = std::max_element(clusvec.begin(), clusvec.end(), cmpByTotalCharge);
  
  //set it
  xposition = bestcluster->position();  
  sigma     = bestcluster->size;
  amplitude = bestcluster->charge_total;

  //fit function in that point
  TF1* mygaus = FitStripsHisto(result, xposition, bestcluster->peakcharge(), sigma);

  chi2 = mygaus->GetChisquare();
 }

 
 void MicromegasPlane::SearchClusterWithSpectrum(TH1F* result,
                                                 const bool FitHisto)
 {
  delete PeakFinder;
  PeakFinder = new TSpectrum();     
  PeakFinder->Search(result,
                     10,        //sigma of the peak
                     "nodraw", //graphic option
                     0.1       //threshold of the peak
                     );
    
    
  if(FitHisto && PeakFinder->GetNPeaks() > 0)
   {
    double xpos(-999);
    UInt_t maxamplitude(0.);

    //Find Best peak
    Double_t* Xpositions = PeakFinder->GetPositionX();
    //find main peak
    for(Int_t peak(0); peak < PeakFinder->GetNPeaks(); ++peak)
     {
      const double thisxpos = Xpositions[peak];
      const double thisamplitude = result->GetBinContent(thisxpos);
      if(thisamplitude > maxamplitude)
       {
        maxamplitude = thisamplitude;
        xpos = thisxpos;
       }
     }


    TF1* mygaus = FitStripsHisto(result, xpos, maxamplitude, 5);

    //save result
    amplitude = mygaus->GetParameter("Norm");
    xposition = mygaus->GetParameter("Mean");
    sigma     = mygaus->GetParameter("Sigma");
    chi2      = mygaus->GetChisquare();
       
      
   }    
 }

 TF1* MicromegasPlane::FitStripsHisto(TH1F* result,
                                              double xpos,
                                              UInt_t maxamplitude,
                                              UInt_t SigmaInit)
 {

  //update style
  gStyle->SetOptFit(1);
      
  //fit main peak with a gaussian
      
  //choose range of the function
  double lowrange  = xpos - SigmaInit > 0 ? xpos - SigmaInit : 0;      
  double highrange = xpos + SigmaInit < result->GetNbinsX() ? xpos + SigmaInit : result->GetNbinsX();

  //correct range in case some void inbetween is present
#if 0      
  while(result->GetBinContent(lowrange) < 0.1)
   lowrange += 1;

  while(result->GetBinContent(highrange) < 0.1)
   highrange -= 1;
#endif
        
  TF1* mygaus = new TF1("mygaus", "[0]*TMath::Gaus(x, [1], [2])", lowrange, highrange);

  //Set initial values
  mygaus->SetParNames("Norm", "Mean", "Sigma");
  mygaus->SetParameters(maxamplitude,  xpos, SigmaInit);

  //only fit if histogram is not empty
  if(result->GetBinContent(xpos) < 0.1)
   return mygaus;  
      
  //fit
  result->Fit("mygaus", "QR+M");

  return mygaus;
 }

} //END NAMESPACE MICROMEGA
