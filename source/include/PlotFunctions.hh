#pragma once
//root
#include <TFile.h>
#include <TTree.h>
#include <TH2F.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TProfile2D.h>
#include <THStack.h>
#include <TStyle.h>
#include <TLegend.h>
#include <TList.h>
#include <TParameter.h>
#include <TLatex.h>
#include <TKey.h>
#include <TDirectory.h>
#include <TF1.h>
//user
#include "utility.hh"
#include "definitions.hh"

//SOME FUNCTION USEFUL FOR PLOTTING


template<class histo>
bool ConditionHistogram1D(histo* hist,const int color,const double NormPar = 1.)
{
 if(!hist)
  {
   std::cerr << "Histogram pointer not found in function @ConditionHistogram1D \n";
   return false;
  }
 if(hist->Integral() < 0.00001)
  {
   //std::cout << "Integral too small, normalization impossible \n";
   return false;
  }
 hist->Scale(NormPar / hist->Integral() );
 hist->SetLineColor(color);
 hist->SetLineWidth(2);
 return true;
}

template<class GRAPH>
bool ConditionGraph1D(GRAPH* graph, const int color=kBlue, const int style=22)
{
 //Marker
 graph->SetMarkerColor(color);
 graph->SetMarkerStyle(style);
 graph->SetMarkerSize(1.5);
 //line
 graph->SetLineColor(color);
 graph->SetLineWidth(2);
 //sort graph
 graph->Sort();
 return true;
}

template<class histo>
bool CreateResidualHistogram(histo* residual, histo* reference)
{
 //loop over
 const int N = residual->GetNbinsX();
 for(int bin(0); bin < N; ++bin)
  {
   //continue if one of the two is too small
   if( residual->GetBinContent(bin) < 0.001
       ||
       reference->GetBinContent(bin) < 0.001)
    {
     residual->SetBinContent(bin, 0.);
     residual->SetBinError(bin, 0.);     
     continue;     
    }
   //bin in percentage
   const double bincontent = 100. * (residual->GetBinContent(bin) - reference->GetBinContent(bin)) / residual->GetBinContent(bin);
   const double binerror = 10. * TMath::Sqrt(
                                              TMath::Power(residual->GetBinError(bin)*reference->GetBinContent(bin) / TMath::Power(residual->GetBinContent(bin), 2) ,2) +
                                              TMath::Power(reference->GetBinError(bin) / residual->GetBinContent(bin),2)
                                              );   
   //setting the bin
   residual->SetBinContent(bin, bincontent);
   residual->SetBinError(bin, binerror);
  }
 //cosmetics
 residual->SetMarkerStyle(3);
 residual->SetMarkerSize(2);
 residual->SetFillColor(kRed); 
 return true;
}

inline void WriteDirectory(TDirectory* mydir)
{
 TObject *obj;
 TKey *key;
 //define iterator
 TIter next( mydir->GetListOfKeys());
 while((key = (TKey *) next())) {
  obj = key->ReadObj(); // copy object to memory
  obj->Write();
 }
}


inline TCanvas* PlotStack(const std::string name,THStack* hs,TLegend* leg)
{
 TCanvas* c = new TCanvas(name.c_str());
 hs->Draw("NOSTAKC,HIST");
 leg->Draw("same");
 return c;
}

template<class histo>
void AddTag(histo* h,const int tag)
{
 TString oldname = h->GetName();
 TString stringtag;
 stringtag.Form("_%i",tag);
 TString newname = oldname + stringtag;
 h->SetName(newname);
}

template<class histo>
inline void PlotAnalysisCanvas(histo* h,
                               const double& tot,
                               const char* drawopt = "COLZ",
                               TString option = "",
                               const uint precision = 2)
{
 stringstream ss;
 TLatex Tl;
 Tl.SetTextSize(0.05);		
 double effi = h->GetEntries() / tot;
 ss.precision(precision);
 ss << "efficiency: " ;
 ss << effi;
 if(option.Contains("ERR") && effi < 0.99)
  {
   ss << " #pm ";
   ss << effi / sqrt(h->GetEntries());
  }
 h->Draw(drawopt);
 //h->Draw("COLZ");
 Tl.DrawLatex(10,150,ss.str().c_str());  
}

template<class histo>
void PlotHisto1D(histo* h1,
                 histo* h2,
                 std::string option = "HIST",
                 std::string legendoption = "l",
                 const int color0 = kBlack,
                 const int color1 = kRed)
{
 //check if histogram are present
 if(!(h1 && h2))
  {
   std::cout << "one of the histogram is a null pointer in @PlotHisto1D, returning without plotting \n";
   return;
  }
 
 //condition both histogram
 ConditionHistogram1D(h1,color0);
 ConditionHistogram1D(h2,color1);
 //create stack
 THStack* hs = new THStack("hs","");
 //set hs basics
 hs->SetTitle((h1->GetTitle() + (string)" comparison").c_str());
 hs->SetName((h1->GetName() +   (string)"_comp").c_str());
 hs->Add(h1);h2->Add(h2);
 TLegend *leg = new TLegend(0.46,0.66,0.77,0.85);
 leg->AddEntry(h1, h1->GetName(), legendoption.c_str());
 leg->AddEntry(h2, h2->GetName(), legendoption.c_str());
 //hs->DrawClone(("NOSTACK" + option1d).c_str());
 //CHECK WHICH HISTOGRAM HAS THE MAXIMUM
 const double max1 = h1->GetBinContent(h1->GetMaximumBin());
 const double max2 = h2->GetBinContent(h2->GetMaximumBin());
 if(max1 > max2)
  {
   h1->DrawCopy(option.c_str());
   h2->DrawCopy((option + "same").c_str());
  }
 else
  {
   h2->DrawCopy(option.c_str());
   h1->DrawCopy((option + "same").c_str());
  }
 leg->Draw("same");
}

template<class histo>
bool HistoFitDoubleGauss(histo* hist)
{
 
 if(!hist)
  {
   std::cerr << "Invalid histogram pointer in function @PlotHistogramFit \n";
   return false;
  }

 //Estimate the ranges
 //First gaussian
 //const double gaus_center = hist->GetMean();
 const int maxbin = hist->GetMaximumBin();
 const double gaus_center = hist->GetBinCenter(maxbin);
 const double gaus_spread = 4*hist->GetStdDev();

 //Create the two functions,
 //narrow gaussian for the center
 //larger gaussian for the tails
 TF1* doublegaus = new TF1("doublegaus", "[0]*TMath::Gaus(x, [1], [2]) + [3]*TMath::Gaus(x, [1], [4])", gaus_center - gaus_spread, gaus_center - gaus_spread);

 //setparameters name
 doublegaus->SetParNames("Normcenter",
                         "Mean",
                         "SigmaCenter",
                         "NormWide",
                         "SigmaWide");
 //Set parameters limit
 //doublegaus->SetParLimits(0, 0., 4 * hist->GetBinContent(maxbin)); //normcenter
 //doublegaus->SetParLimits(1, gaus_center - gaus_spread / 4, gaus_center + gaus_spread / 4); //mean
 doublegaus->SetParLimits(2, 0., gaus_spread / 4); //sigmacenter
 //doublegaus->SetParLimits(3, 0., hist->GetBinContent(maxbin) ); //normwide
 doublegaus->SetParLimits(4, gaus_spread / 4, 4 * gaus_spread); //sigmawide
 
 //Set Parameters first guess
 doublegaus->SetParameters(hist->GetBinContent(maxbin) / 2,
                           //hist->GetBinCenter(hist->GetMaximumBin()),
                           gaus_center,
                           gaus_spread / 4,
                           hist->GetBinContent(maxbin) / 4,
                           gaus_spread / 2);
 
 //Set graphic options
 doublegaus->SetLineColor(kBlue);
 doublegaus->SetLineWidth(3); 
                           

 //Fit function to get first estimate of parameters
 hist->Fit("doublegaus", "QR+");
 //showfit parameters
 gStyle->SetOptFit(1); 

 //set parameters nam


 return true;
 
}

template<class histo>
bool HistoFitDoubleGaussAsymmetric(histo* hist)
{
 
 if(!hist)
  {
   std::cerr << "Invalid histogram pointer in function @PlotHistogramFit \n";
   return false;
  }

 //Estimate the ranges
 //First gaussian
 //const double gaus_center = hist->GetMean();
 const double gaus_center = hist->GetBinCenter(hist->GetMaximumBin());
 const double gaus1_range  = 4 * hist->GetBinWidth(hist->GetMaximumBin());
 //Second gausian
 const double gaus2_range  = 2 * hist->GetStdDev();

 //Create the two functions
 TF1* gausleft = new TF1("gausleft", "gaus", gaus_center - gaus2_range, gaus_center - gaus1_range); 
 TF1* gauscenter = new TF1("gauscenter", "gaus", gaus_center - gaus1_range, gaus_center + gaus1_range);
 TF1* gausright = new TF1("gausright", "gaus", gaus_center + gaus1_range, gaus_center + gaus2_range);
 TF1* total = new TF1("gaustotal", "gaus(0) + gaus(3) + gaus(6)", gaus_center - gaus2_range, gaus_center + gaus2_range);

 //Fit function to get first estimate of parameters
 hist->Fit("gausleft",   "Q0R");
 hist->Fit("gauscenter", "Q0R+");
 hist->Fit("gausright",  "Q0R+");

 //Collect and set initiali parameters
 double par[9];
 gausleft->GetParameters(  &par[0]);
 gauscenter->GetParameters(&par[3]);
 gausright->GetParameters( &par[6]);
 total->SetParameters(par);

 //set parameters name

 //Set graphic options
 total->SetLineColor(kBlue);
 total->SetLineWidth(2);

 //final fit
 hist->Fit("gaustotal", "QR+");

 return true;
 
}

template<class histo>
TGraph* CreatePrecisionVSEfficiencyGraph(histo* hist, const double parameter)
{
 //guard
 if(!hist)
  {
   std::cerr << "histogram does not exist in function @CreatePrecisionVSEfficiencyGraph, exiting";
   return NULL;
  }
 //Calculate number of reasonable point
 const int thebin = hist->FindBin(parameter);
 const double binwidth = hist->GetBinWidth(thebin);

 const int NPoint = ceil(parameter / binwidth); //enough to cover relative error of 100%

 TGraph* result = new TGraph(NPoint);

 //set points
 for(int i(0); i<NPoint; ++i)
  {
   //Calculate efficiency
   const double precision = 100 * ( (hist->GetBinCenter(thebin) - hist->GetBinCenter(thebin - i) - binwidth/2) / parameter ) ;
   const double eff = 100.*(hist->Integral(thebin - i, thebin + i) / hist->Integral() );
   result->SetPoint(i, precision, eff);
  }

 //style
 result->GetXaxis()->SetTitle("Precision of measurements #delta m / m [%]");
 result->GetYaxis()->SetTitle("Number of events inside the area [%]");

 return result;
}
