#ifndef _T_DIRC_ANALYSIS_DUMMY_H
#define _T_DIRC_ANALYSIS_DUMMY_H
// +++ include header files +++
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
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
class TDircAnalysisDummy : public TDircAnalysisBase {
private:
	void Init();
protected:
	enum DQCuts {NO_CUTS,DECODE_ERR,RNDM_BIT_ERR,SYNC_ERR,NO_HITS_ERR,NO_MATCH_ERR,NO_TRIG_ERR}; //empty TDC cut,missing reference signal cut,laser trig missing,};
public:
	TDircAnalysisDummy(string cUserDataFilename, string UserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth); // standard constructor
	TDircAnalysisDummy(string cUserDataFilename, string UserTdcAddressFile); // constructor, all TDC defintions in text file
	virtual ~TDircAnalysisDummy(); // standard destructor
	void Analyse(string cUserAnalysisFilename); // analysis routine goes here, this method is needed!


	/* some magic ROOT stuff... */
	ClassDef(TDircAnalysisDummy,1);
};

#endif