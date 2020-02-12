#include <iostream>

#include <fstream>
using namespace std;

void mapping_NA64(){

 int p=64; int Nstrip_MG=320;
 int MultiplexSeries[]={1, 7, 13, 19, 25};   //Multiplex series used in na64 
 int Detector[Nstrip_MG]; // strip to channel correspondance
 for(int i=0;i<(p-1)/2;i++)
  {
   for(int j=0;j<p;j++)
    {
     if(i*p+j<Nstrip_MG)
      {
       Detector[i*p+j]=(0+MultiplexSeries[i]*j)%p;
      }
    }
  }

 // create map
 ofstream myfile;
 myfile.open ("multiplexing_p64_m5_na64.txt");
 double entry=0.0;
 for(int ch=0; ch < p; ch++){ 
  for(int strip=0; strip < Nstrip_MG; strip++){
   if(Detector[strip] == ch){entry = 1.0;}
   else{entry = 0.0;}
   myfile << ch << "\t" << strip << "\t" << entry <<"\n";
  }
 }
 myfile.close();  
}
