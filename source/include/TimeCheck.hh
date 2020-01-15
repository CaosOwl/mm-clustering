#pragma once
//std library
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream> 
#include <ctime>

//root
#include "TTree.h"
#include "TParameter.h"
#include "TList.h"

//user
#include"utility.hh"

using namespace std;

namespace Time
{
 struct TimeCheck
 {
		unsigned int count;
		clock_t start;
		clock_t end;
		vector<clock_t> checkpoints;
		vector<string> names;
		
		
  TimeCheck(const unsigned int);
  
  void Start();
  void End();
		
  void AddCheckPoint(string);
  void Print(ostream&);
  void SaveInTree(TTree*);
   
   }; //end struct timecheck

}//end timecheck namespace
