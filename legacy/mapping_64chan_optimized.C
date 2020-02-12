#include <iostream>

#include <fstream>
using namespace std;

///////////////////////////////////////////////////////////
/*
THIS MACRO WAS WRITEN TO PRODUCE A MULTIPLEX MAP THAT OPTIMALLY MAP
64 CHANNEL TO 320 STRIPS FOLLOWING THE ADVISE OF SEBASTIEN PROCUREUR.
HERE IS THE MAIL THAT WAS SENT BY HIM:


Dear Emilio,
Thanks for all the interesting info. I had a quick look at the mapping, so the multiplexing sequence is {1,7,13,19,25}. That sounds good a priori, as all these numbers are prime with 64, however there is indeed a repetition between the group 13 and 19, where the sequence reads:
50 63 12 25 38 51 0 19 38 57 12 31 50
This sequence is doubly bad: first, there is a very quick repetition of channel 38, which is connected to 2 strips (n° 190 and 194) separated by only 3 intermediate strips (distance =4). Second (and less important), other channels are repeated every other 2 strips, i.e. channels 12, 50, etc.  
I guess it is at this position you have ambiguities, right?

If you want to keep the same number of channels and strips, I propose you the following multiplexing sequence: {1,13,29,17,21}. In this case you should not see any pair of strips connected to the same channel and distant by less than 16 strips (you can check :) ). More specifically:
- the closest repetition from groupe 1 and 13 has distance 16 (channel 52)
- the closest repetition from groupe 13 and 29 has distance 16 (channel 11)
- the closest repetition from groupe 29 and 17 has distance 16 (channel 12)
- the closest repetition from groupe 17 and 21 has distance 16 (channel 60)
From what I see, it seems to be the best possible sequence for the configuration with 64 channels and multiplexing by 5.

Cheers,
Se
 */
/////////////////////////////////////////////////////////



void mapping_64chan_optimized(){

 int p=64; int Nstrip_MG=320;
 int MultiplexSeries[]={1, 13, 29, 17, 21};   //Multiplex series as proposed by Procureur 
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
 myfile.open ("multiplexing_p64_m5_optimized.txt");
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
