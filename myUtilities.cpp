#include "myUtilities.h"


Int_t Combinations(UInt_t nElements, UInt_t nSampleSize){
	if(nElements<1 || nSampleSize<1){
		return (-1);
	}
	if(nSampleSize>nElements){
		return (-1);
	}
	UInt_t nNominator = 1;
	UInt_t nDenominator = 1;
	for(UInt_t i=nElements; i>0; --i){
		if(i>(nElements-nSampleSize))
			nNominator *= i;
		if(i<(nSampleSize+1))
			nDenominator*= i;
	}
	return (nNominator/nDenominator);
}

UInt_t HexStringToInt(string cUserString){
	if(cUserString.empty())
		return (0);
	stringstream cConverter(cUserString);
	UInt_t nValue;
	cConverter >> hex >> nValue;
	return (nValue);
}


std::vector<string> LineParser(string cUserLine, char cUserDelimiter, Bool_t bVerboseMode){
	// parse line provided by user and return a vector of strings containing individual tokens
	// user needs to provide column separator


	std::vector<string> cTokens; // vector for storing individual tokens from lime
	cTokens.reserve(TOKEN_SIZE);

	if(cUserLine.empty()) // return if user provided line is empty
		return (cTokens);

	string cBuffer; // buffer for storing temorary token
	stringstream cParsingLine(cUserLine);
	//while(!cParsingLine.eof()){ // parse string containing line
	while(std::getline(cParsingLine,cBuffer,cUserDelimiter)){ 
		//cParsingLine >> cBuffer; // extract token from line and store in temporary variable
		if(!cBuffer.empty()) // if token is not empty add to list of tokens
			cTokens.push_back(cBuffer);
		cBuffer.clear(); // empty temporary variable 
	}
	if(bVerboseMode){
		for(std::vector<string>::const_iterator ShowTokens=cTokens.begin(); ShowTokens!=cTokens.end(); ShowTokens++){
			cout << *ShowTokens << endl;
		}
	}

	return (cTokens);
}

UInt_t SwapBigEndian(UInt_t nBigEndianNumber){
	return ( ((nBigEndianNumber & 0x000000FF)<<24) // move last byte to front
		+ ((nBigEndianNumber & 0x0000FF00)<<8) 
		+ ((nBigEndianNumber & 0x00FF0000)>>8)
		+ ((nBigEndianNumber & 0xFF000000)>>24)
		);
}

string RandomString(Int_t nStringLength){
	string cRandomString;
	static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < nStringLength; ++i) {
        cRandomString += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
	return (cRandomString);
}