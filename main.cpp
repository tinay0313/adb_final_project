/** @file main.cpp
    @author Ann Tsai mt4050
 */

#include <iostream>
#include <fstream>
#include <getopt.h>
#include <cstring>
#include <array>


#include "Parser.h"


  
int main(int argc, char** argv) {
  
   // inputfile
   std::string inputFile;
   inputFile = argv[1];
   std::cout << "inputFile: " << inputFile << std::endl;

   Parser parser = Parser(&inputFile);
  
   return 0;
 
};
