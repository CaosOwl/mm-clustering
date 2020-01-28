#pragma once
//root
#include"TH1.h"
#include <Math/GSLSimAnMinimizer.h>
#include "Math/GSLMinimizer.h"
#include <Math/Functor.h>
#include "Math/Minimizer.h"
#include "Math/Factory.h"
//eigen
#include <Eigen/Sparse>
#include <Eigen/Dense>
using namespace Eigen;

//TODO:
/*
Many variable are defined globaly to mantain previous 
code structure used by Balint, ideally this has to be 
reworked
 */


//Henerited from Balint
std::vector<double> data_v;// channel values data

//parameter
UInt_t NStrips; //strips number
UInt_t NChan;   // channel number
UInt_t MFac;    //multiplexing factor


//Matricesx
SparseMatrix<double> mmultiplex;
MatrixXd vstrips;
MatrixXd vmultiplex;
MatrixXd mQ2;
MatrixXd mQ2T;
MatrixXd ms;
SparseMatrix<double> mreg;

double LogLikelihood(const double *pars )
{

 // evaluate model
 for(UInt_t i = 0; i < NStrips; i++){   
  vstrips(i, 0) = pars[i];
 }

 vmultiplex = mmultiplex * vstrips;

  
 Double_t logl = 0;
  
 for(UInt_t i = 0; i < data_v.size(); i++){
  if(data_v[i] > 0){
   logl += ( vmultiplex(i, 0) - data_v[i] )*( vmultiplex(i, 0) - data_v[i] );
  }     
 }
  
 // Additional constraint
 mQ2 = mreg * vstrips;
 mQ2T = mQ2.transpose();
 ms = mQ2T * mQ2;
 logl += ms(0,0);
  
 return logl;
}


inline Double_t* NumericalMinimization(UInt_t mode, UInt_t verbose = 0)
{
 // GSL Simulated Annealing minimizer
 //   ROOT::Math::GSLSimAnMinimizer min;

 // GSL Minimizer
 // Choose method upon creation between:
 // kConjugateFR, kConjugatePR, kVectorBFGS,
 // kVectorBFGS2, kSteepestDescent
 //  ROOT::Math::GSLMinimizer min( ROOT::Math::kVectorBFGS );

 // Minuit
 // Choose method upon creation between:
 // kMigrad, kSimplex, kCombined, 
 // kScan, kFumili
 Eigen::setNbThreads(8);

 UInt_t n = Eigen::nbThreads();
 std::cout << "Eigen threads: " << n << std::endl;  

 ROOT::Math::Minimizer* min = NULL;
 if(mode == 1){
  //        min =   ROOT::Math::Factory::CreateMinimizer("GSLMultiMin", "SteepestDescent");
  //            min =   ROOT::Math::Factory::CreateMinimizer("GSLSimAn", "");
  //      min =        ROOT::Math::Factory::CreateMinimizer("Minuit2", "kSimplex");
  min->SetPrintLevel(1);
  min->SetMaxIterations(1e+03);
  min->SetTolerance(30000);
 }else{

  min = ROOT::Math::Factory::CreateMinimizer("Minuit2", "Migrad");
  //gErrorIgnoreLevel = 1001; 
  min->SetPrintLevel(verbose);
  min->SetMaxIterations(50);
  //   min->SetTolerance(30000);
  min->SetMaxFunctionCalls(8e+05);
 }
  

 std::cout << "Before functor: " << n << std::endl;  
 ROOT::Math::Functor f(&LogLikelihood,NStrips); 
 double step[NStrips];
 for(UInt_t i = 0; i < NStrips; i++){
  step[i] = 100;
 }
 min->SetFunction(f);

 Double_t variable[NStrips];
 for(UInt_t i = 0; i < NStrips; i++){
  variable[i] = 100; //starting value
 }
 
 // Set the free variables to be minimized!
 for(UInt_t i = 0; i < NStrips; i++){
  std::string name = "x" + std::to_string(i);
  min->SetVariable(i,name.c_str(), variable[i], step[i]);
  min->SetVariableLimits(i, 0, 1e+04);
 } 

 std::cout << "Before minimization: " << n << std::endl;  
 min->Minimize(); 
 Double_t *solution = (Double_t*)min->X();
 std::cout << "After minimization: " << n << std::endl;  
 
 delete min;

 return solution;	
}
