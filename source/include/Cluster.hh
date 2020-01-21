#pragma once

#include"Globals.hh"
//std
#include<iostream>

namespace Micromega
{
class Cluster {





public:
 int bin[NCHAN*MFACTOR];       // strips index
 double charge[NCHAN*MFACTOR]; // strips charge
 double charge_total;    // total charge
 size_t size;               // total strips


 //CONSTRUCTOR
 Cluster();


 //FUNCTION
 double position() const;
 double peakcharge() const;
 void   add_strip(const int, const double);
 void   clear();

};

//sorting function
 bool cmpByTotalCharge(const Cluster& a, const Cluster& b);
 bool cmpBySize(const Cluster& a, const Cluster& b);
 bool isClusterDeleted(const Cluster& a);

} //END NAMESPACE MICROMEGA
