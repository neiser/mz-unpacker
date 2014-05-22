#ifndef _T_GSI_TREE_CONVERTER_H
#define _T_GSI_TREE_CONVERTER_H
// +++ include header files +++
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TObject.h"
#include "TTree.h"

#include "myUtilities.h"
#include "TTrbDataTree.h"
#include "TTrbAnalysisBase.h"
#include "TDircAnalysisBase.h"

// +++ class definition +++
struct GsiDataModel{
	Double_t *fLeadingEdge; // leading edge timing information (D)
	Double_t *fTot; // time over threshold information (D)
	UInt_t *nTdcId; // TDC address (i)
	UInt_t *nTdcChan; // TDC channel (i)
	Int_t nMcpId; // MCP ID (user mapping required) (I)
	Bool_t *bIsValid; // indicates if there was a valid entry for this channel  (O)
};


class TGsiTreeConverter : public TDircAnalysisBase {
private:
	TFile *OutputFile; // file for storing converted tree
	TTree *OnlineTree; // this tree will be used to store the online data for fast analysis
	//Double_t *fLeadingEdges; // array for leading edge values
	//Double_t *fTots; // array for time-over-threshold values
	GsiDataModel EventData;
	void Init();
protected:
	enum DQCuts {NO_CUTS,DECODE_ERR,RNDM_BIT_ERR,SYNC_ERR,NO_HITS_ERR,NO_MATCH_ERR,NO_TRIG_ERR}; //empty TDC cut,missing reference signal cut,laser trig missing,};
public:
	TGsiTreeConverter(string cUserDataFilename, string UserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth); // standard constructor
	TGsiTreeConverter(string cUserDataFilename, string UserTdcAddressFile); // constructor, all TDC defintions in text file
	virtual ~TGsiTreeConverter(); // standard destructor
	void Analyse(string cUserAnalysisFilename); // analysis routine goes here, this method is needed!

	void ConvertTree(string cUserAnalysisFilename); // extract relevant data and write into GSI online tree

	/* some magic ROOT stuff... */
	ClassDef(TGsiTreeConverter,1);
};

#endif