#ifndef _MY_UTILITIES_H
#define _MY_UTILITIES_H
// +++ include header files +++
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <cfloat>
#include <climits>
#include <limits>
#include <stdexcept>
#include <string>
#include <ctime>
#include <cctype>
#include <algorithm>
#include <cmath>

#define TOKEN_SIZE 10

;
Int_t Combinations(UInt_t nElements, UInt_t nSampleSize); // compute number of possible combination w/o ordering limitations
UInt_t HexStringToInt(string cUserString);
std::vector<string> LineParser(string cUserLine, char cUserDelimiter=' ', Bool_t bVerboseMode=kFALSE);
UInt_t SwapBigEndian(UInt_t nBigEndianNumber);
string RandomString(Int_t nStringLength); // create a random string of length l


#endif