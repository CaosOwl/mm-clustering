#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>

// include ROOT libraries
#include "TTree.h"
#include "TMath.h"
#include "TFile.h"

using namespace std;

void mapping_NA64_legacy() {
    
    
  
    int p = 64;int Nstrip_MG=320;
    int strip=0;
    int channel=0;
    int Detector[Nstrip_MG];
    
    ofstream myfile ("Mapping_1_.txt");
    
    TFile* f = new TFile("Mapping_Tree_new.root","RECREATE");
    TTree* t1 = new TTree("t1","Tree with the Mapping");
    int MM_Ch, MM_strip,MM_Layer;
    
    t1->Branch("MM_Ch",&MM_Ch,"MM_Ch/i");
    t1->Branch("MM_strip",&MM_strip,"MM_strip/i");
    t1->Branch("MM_Layer",&MM_Layer,"MM_Layer/i");


    //NOTE:
    //REMOVED PIECE OF CODE DUMPED ON NOT UNDERSTOOD, OLD CODE LIES COMMENTED BELOW


    for(int s=1;s<(p-1)/2;s=s+6)
    {
        for(int i=0;i<=(p-1);i++)
        {
            channel=1+((i*s)%p);
            strip++;
            MM_Ch=channel-1;
            MM_strip=strip-1;
            MM_Layer=1;
            //channel=channel*2;
            //channel=128-((channel-1)*2);
            
            t1->Fill();
            cout<<"Channel "<<channel<<" --Strip "<<strip<<endl;
            
        }
    }
    

    #if 0 
    for(int s=1;s<(p-1)/2;s=s+6)
    {
        for(int i=0;i<=(p-1);i++)
        {
            channel=1+((i*s)%p);
            strip++;
            MM_Ch=channel-1;
            MM_strip=strip-1;
            MM_Layer=1;
            //channel=channel*2;
            //channel=128-((channel-1)*2);
            
            t1->Fill();
            cout<<"Channel "<<channel<<" --Strip "<<strip<<endl;
            myfile<<((MM_Ch))<<std::setw(5)<<std::setfill(' ')<<0<<std::setw(5)<<std::setfill(' ')<<0<<std::setw(5)<<std::setfill(' ')<<2<<std::setw(10)<<std::setfill(' ')<<MM_strip<<'\n';
            
        }
    }
    #endif
    /*strip=0;
    
    for(int s=1;s<(p-1)/2;s=s+6)
    {
        for(int i=0;i<=(p-1);i++)
        {
            channel=1+((i*s)%p);
            strip++;
            MM_Ch=(channel*2)+1;
            MM_strip=strip;
            MM_Layer=2;
            
            t1->Fill();
            cout<<"Channel "<<channel<<" --Strip "<<strip<<endl;
            myfile<<((channel*2)+1)<<std::setw(5)<<std::setfill(' ')<<0<<std::setw(5)<<std::setfill(' ')<<0<<std::setw(5)<<std::setfill(' ')<<2<<std::setw(10)<<std::setfill(' ')<<strip<<'\n';
            
        }
    }*/
    t1->Write();
    f->Close();
    myfile.close();
}

