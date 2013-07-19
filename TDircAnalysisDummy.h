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

public:
	TDircAnalysisDummy(string cUserDataFilename, string UserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth); // standard constructor
	virtual ~TDircAnalysisDummy(); // standard destructor
	void Analyse(string cUserAnalysisFilename); // analysis routine goes here, this method is needed!


	/* some magic ROOT stuff... */
	ClassDef(TDircAnalysisDummy,1);
};

#endif