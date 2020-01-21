#include"Cluster.hh"

namespace Micromega
{


 Cluster::Cluster() : charge_total(0), size(0) {}




 // return cluster center-of-mass
 double Cluster::position() const
 {
  double num = 0;
  double den = 0;
    
  for (size_t i = 0; i < size; i++) {
   num += bin[i] * charge[i];
   den += charge[i];
  }
    
  return (den != 0) ? num/den : 0;
 }

 double Cluster::peakcharge() const
 {
  double maxcharge = 0;
    
  for (size_t i = 0; i < size; i++)
   {
    if(charge[i] > maxcharge)
     maxcharge = charge[i];
   }
    
  return maxcharge;
 }



 void Cluster::add_strip(const int bin_, const double charge_)
 {
  bin[size] = bin_;
  charge[size] = charge_;
  charge_total += charge_;
  size++;
 }

 void Cluster::clear()
 {
  size = 0;
 }

 bool cmpByTotalCharge(const Cluster& a, const Cluster& b)
 {
  return (a.charge_total < b.charge_total);
 }

 bool cmpBySize(const Cluster& a, const Cluster& b)
 {
  return (a.size < b.size);
 }

 bool isClusterDeleted(const Cluster& a)
 {
  return (a.size == 0);
 }


} //END NAMESPACE MICROMEGA
