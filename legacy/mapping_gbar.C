#include <iostream>

#include <fstream>
using namespace std;


void mapping_gbar(){

  int p=61; int Nstrip_MG=1037;
  int MultiplexSeries[]={30,10,15,19,5,20,27,22,11,24,18,12,9,6,3,4,1,16,8,2,23,21,7,25,14,28,17,26,13,29};
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
  myfile.open ("multiplexing_p61_m17_pro.txt");
  double entry=0.0;
  for(int ch=0; ch<61; ch++){ 
	for(int strip=0; strip<1037; strip++){
		if(Detector[strip] == ch){entry = 1.0;}
		else{entry = 0.0;}
		myfile << ch << "\t" << strip << "\t" << entry <<"\n";
	}
  }
  myfile.close();
}
