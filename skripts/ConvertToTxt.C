#include<fstream>


void ConvertToTxt(const char* filename)
{
 const UInt_t nchan = 64;
 const UInt_t nstrip = 320;
 const UInt_t nsol = 2;
 
 TFile* file = new TFile(filename);
 TTree* tree = (TTree*)file->Get("ClusterTree");

 UInt_t chan[nchan];
 UInt_t strips[nstrip];
 Double_t sol[nsol];


 //output
 TString outname(filename);
 std::ofstream out((outname + ".txt").Data());


 tree->SetBranchAddress("ChanOutput", chan);
 tree->SetBranchAddress("Strips_Physical", strips);
 tree->SetBranchAddress("TruePosition", sol);

 for(UInt_t i(0); i < 100; ++i)
  {
   tree->GetEntry(i);
   for(UInt_t n(0); n < nchan; ++n)out << chan[n] << ",";
   for(UInt_t n(0); n < nstrip; ++n)out << strips[n] << ",";
   for(UInt_t n(0); n < nsol; ++n)out << sol[n] << ",";
   out << "\n";    
  }

 out.close();

 return;

}
