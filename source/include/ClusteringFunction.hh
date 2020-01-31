//root
#include"TSpectrum.h"
#include"TF1.h"
#include"TStyle.h"
#include"TCanvas.h"
#include "TMarker.h"
#include "TLine.h"
#include "TText.h"
#include "TBox.h"
#include "TPaveText.h"
//std
#include<fstream>

enum FitMethod{SPECTRUM};

const UInt_t mindistance = 1;

struct FitClus
{
 Double_t position;
 Double_t sigma;
 UInt_t   charge;

 FitClus()
  :
  position(0.),sigma(0.),charge(0){}

 FitClus(Double_t p, Double_t s, UInt_t c)
  :
  position(p),sigma(s),charge(c){}
};


struct FitPlane
{
 TH1F* histo;
 FitClus true1;
 FitClus true2;
 FitClus sol1;
 FitClus sol2;
 UInt_t peaksfound;

 Double_t distance() const
 {
  return true1.position - true2.position;
 }

 Double_t residual1() const
 {
  return fabs(true1.position - sol1.position);
 }

 Double_t residual2() const
 {
  return fabs(true2.position - sol2.position);
 }

 void matchsolutions(FitClus c1, FitClus c2)
 {
  const Double_t& s1 = c1.position;
  const Double_t& s2 = c2.position;
  if(fabs(s1 - true1.position) < fabs(s1 - true2.position))
   {
    if(fabs(s2 - true1.position) < fabs(s2 - true2.position))
     {
      if(fabs(s1 - true1.position) < fabs(s2 - true1.position))
       {
         sol1 = c1;
         sol2 = c2;
       }
       else
        {
         sol1 = c2;
         sol2 = c1;
        }
     }
     else
      {
       sol1 = c1;
       sol2 = c2;         
      }
   }
  else
   {
    if(fabs(s2 - true2.position) < fabs(s2 - true1.position))
     {
      if(fabs(s2 - true2.position) < fabs(s1 - true2.position))
       {
        sol1 = c1;
        sol2 = c2;
       }
      else
       {
        sol1 = c2;
        sol2 = c1;
       }
     }
    else
     {
      sol1 = c2;
      sol2 = c1;
     }
   }
 }

 void Print(ostream& stream)
 {
  stream << "solution 1 was: " << sol1.position << " with true solution: " << true1.position <<"\n";
  stream << "solution 2 was: " << sol2.position << " with true solution: " << true2.position <<"\n";
 }
 
};

TF1* CreateClusFunction(Double_t range1,
                        Double_t range2,
                        const UInt_t tag = 1)
{
 TString name;
 name.Form("mygaus%i", tag);
 TF1* res = new TF1(name,
                    "[0] * TMath::Gaus(x, [1], [2])",
                    range1,
                    range2);

 //set parameter name
 res->SetParNames("Norm", "Posi", "Sigma");

 //width
 res->SetLineWidth(2);

 return res;
}

TF1* CreateDoubleClusFunction(const char* name,
                              Double_t range1,
                              Double_t range2)

{

 TF1* totalfit = new TF1(name, "[0] * TMath::Gaus(x, [2], [4]) + [1] * TMath::Gaus(x, [3], [5])", range1, range2);
 totalfit->SetLineWidth(2);
 //set parameters name
 totalfit->SetParName(0, "Norm1");
 totalfit->SetParName(1, "Norm2");
 totalfit->SetParName(2, "Posi1");
 totalfit->SetParName(3, "Posi2");
 totalfit->SetParName(4, "Sigm1");
 totalfit->SetParName(5, "Sigm2");

 return totalfit;
}


TCanvas* FitPeaks(UInt_t* Strips,
                  UInt_t nstrips,
                  FitPlane& plane,
                  FitMethod method)
{
 //create canvas
 TCanvas* canv = new TCanvas();

 //build histogram
 plane.histo = new TH1F("histo", "true histo", nstrips, -0.5, nstrips - 0.5);
 for(UInt_t n(0); n < nstrips; ++n)plane.histo->SetBinContent(n, Strips[n]);

 //range of the fit

#if 0
 UInt_t range1(0);
 while(plane.histo->GetBinContent(range1) == 0) ++range1;
 UInt_t range2(range1);
 while(plane.histo->GetBinContent(range2) != 0) ++range2;
#endif
 UInt_t range1(0), range2(nstrips - 1);
 const Double_t center = ((Double_t)range1 - range2) / 2.;
 Double_t cmass(0.), norm(0.);
 for(UInt_t n(range1); n < range2; ++n)
  {
   norm = plane.histo->GetBinContent(n);
   cmass += n * norm;
  }
 cmass /= norm;
 
 //find intitial values with TSpectrum
 TSpectrum* peakfinder = new TSpectrum();
 peakfinder->Search(plane.histo,
                    1,     //sigma of the peaks
                    "nodraw",
                    0.05
                    );
 std::cout << "peak found: " << peakfinder->GetNPeaks() << "\n";
 Double_t* Peaks = peakfinder->GetPositionX();
 UInt_t npeaks = peakfinder->GetNPeaks();
 for(UInt_t peak(0); peak < npeaks; ++peak)
  {
   std::cout << "position: " << Peaks[peak] << "\n";
  }

 if(npeaks == 0)
  {
   std::cout << " no peaks found \n";
   return canv;
  }
 #if 0
 else
  {
   plane.matchsolutions(Peaks[0], Peaks[1]);
   return;
  }
 #endif

 //starting point for the fit
 TF1* mygaus1 = CreateClusFunction(0, nstrips - 1, 1);
 mygaus1->SetLineColor(kBlue);
 TF1* mygaus2 = CreateClusFunction(0, nstrips - 1, 2);
 mygaus2->SetLineColor(kGreen);
 TF1* totalfit = CreateDoubleClusFunction("totalfit", 0, nstrips -1);

 //initial fit
 if(npeaks > 1)
  {
   //gaus1
   mygaus1->SetParameter(0, plane.histo->GetBinContent(Peaks[0]));
   mygaus1->SetParameter(1, Peaks[0]);
   mygaus1->SetParLimits(1, Peaks[0] - 1, Peaks[0] + 1);
   mygaus1->SetParameter(2, plane.true1.sigma);//mygaus1->SetParameter(2, (Peaks[1] - Peaks[0]) / 2);
   //gaus2
   mygaus2->SetParameter(0, plane.histo->GetBinContent(Peaks[1]));
   mygaus2->SetParameter(1, Peaks[1]);
   mygaus2->SetParLimits(1, Peaks[1] - 1, Peaks[1] + 1);
   mygaus2->SetParameter(2, plane.true2.sigma);//mygaus2->SetParameter(2, (Peaks[1] - Peaks[0]) / 2);
  }
 else
  {
   mygaus1->SetParameter(0, plane.histo->GetBinContent(Peaks[0] - 2));
   mygaus1->SetParameter(1, Peaks[0] - 2);
   mygaus2->SetParameter(2, plane.true1.sigma);//mygaus1->SetParameter(2, center / 2);
   mygaus2->SetParameter(0, plane.histo->GetBinContent(Peaks[1] + 2));
   mygaus2->SetParameter(1, Peaks[1] + 2);
   mygaus2->SetParameter(2, plane.true2.sigma);//mygaus2->SetParameter(2, center / 2);
  }

 //set par limits
 mygaus1->SetParLimits(2, plane.true1.sigma -1, plane.true1.sigma + 1);
 mygaus2->SetParLimits(2, plane.true2.sigma -1, plane.true2.sigma + 1);

 //do first first fit
 plane.histo->Fit("mygaus1", "QR+");//, "", Peaks[0] - plane.true1.sigma, Peaks[0] + plane.true1.sigma);
 plane.histo->Fit("mygaus2", "QR+");//, "", Peaks[1] - plane.true2.sigma, Peaks[1] + plane.true2.sigma);

 //set parameter for the second fit
 totalfit->SetParameters(mygaus1->GetParameter(0),
                         mygaus2->GetParameter(0),
                         mygaus1->GetParameter(1),
                         mygaus2->GetParameter(1),
                         mygaus1->GetParameter(2),
                         mygaus2->GetParameter(2)
                         );

 //fix parameters
 //totalfit->FixParameter(4, plane.true1.sigma);
 //totalfit->FixParameter(5, plane.true2.sigma);

 //final combined fit
 gStyle->SetOptFit(1);
 plane.histo->Fit("totalfit", "MR+", "", range1, range2);

 //save results
 FitClus sol1(totalfit->GetParameter("Posi1"),totalfit->GetParameter("Sigm1"),totalfit->GetParameter("Norm1"));
 FitClus sol2(totalfit->GetParameter("Posi2"),totalfit->GetParameter("Sigm2"),totalfit->GetParameter("Norm2"));
 //matching solutions
 plane.matchsolutions(sol1, sol2);

 TString name(plane.histo->GetName());
 canv->SetName(name + "-canv");
 canv->SetTitle(plane.histo->GetTitle());

 //Plot histo
 plane.histo->SetLineWidth(2);
 plane.histo->Draw();

 //build legend
 //canv->BuildLegend();

 //draw solution
 TLine line;
 line.SetLineColor(kBlack);
 line.SetLineWidth(3);
 line.DrawLine(plane.true1.position, 0, plane.true1.position, 100);
 line.DrawLine(plane.true2.position, 0, plane.true2.position, 100);

 return canv;
 
}