//user
#include"TimeCheck.hh"

using namespace std;

namespace Time
{
		
		
 TimeCheck::TimeCheck(const unsigned int max)
 {
  count = 0;
  checkpoints = vector<clock_t>(max,0);
  names = vector<string>(max,"nan");
  start = 0;
  end = 0;
 }

 void TimeCheck::Start()
 {
  start = clock();
 }

 void TimeCheck::End()
 {
  end = clock();
 }
		
 void TimeCheck::AddCheckPoint(string name)
 {
  if(count == names.size()-1)
   {
    cout << "Maximum Size reached, impossible to add the cut" <<endl;
    return;
   }
  if(names[count] == "nan")
   {
    names[count] = name;
    checkpoints[count] = clock();
   }
  else if (names[count] != name)
   cout << "name of this checkpoint was already assigned" << endl;
  count++;
 }

 void TimeCheck::Print(ostream& stream)
 {
  unsigned int counter(0);
  stream << "|Cut Name|time in clocks| time in seconds| difference from previous step in seconds" << endl;
  while(names[counter] != "nan" && counter != names.size() - 1)
   {
    clock_t prestep= counter==0 ? start : checkpoints[counter-1];
    stream << "|  " << names[counter] << "|" << double(checkpoints[counter] - start)  << "|" << double(checkpoints[counter] - start) / CLOCKS_PER_SEC << "|" << double(checkpoints[counter] - prestep) / CLOCKS_PER_SEC <<"|" <<  endl;
    ++counter;
   }
 }//finish Print function

 void TimeCheck::SaveInTree(TTree* t)
 {
  TList* timelist = new TList();
  unsigned int counter(0);
  while(names[counter] != "nan" && counter != names.size() - 1)
   {
    clock_t prestep = counter==0 ? start : checkpoints[counter-1];
    const double thistime =  double(checkpoints[counter] - prestep) / CLOCKS_PER_SEC;
    TParameter<double>* newpar = new TParameter<double>(names[counter].c_str(),thistime);
    timelist->Add(newpar);
    ++counter;
   }
  //save list in the userinfo
  Utils::AddInfoParameters(t, timelist);

 }// end of the function SaveInTree

}//end timecheck namespace
