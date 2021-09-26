#include<fstream>


void test(const char* filename = "test.root")
{
 
 TFile* file = new TFile(filename);  

 if (file->IsZombie()) {
   std::cout << "Error opening file" << std::endl;
   return 1;
}
 else
   {
     std::cout << "File passes consistency check" << std::endl;
   }

}
