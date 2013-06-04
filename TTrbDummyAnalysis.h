#ifndef _T_TRB_DUMMY_ANALYSIS_H
#define _T_TRB_DUMMY_ANALYSIS_H
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

// +++ class definition +++
struct TrbPixelHit{
		UInt_t nChannelA;
		UInt_t nChannelB;
		Double_t fTimeOverThreshold;
		Int_t nSyncIndex;
		Bool_t bHasSyncTime;
	};

class TTrbDummyAnalysis : public TTrbAnalysisBase {
private:
	void Init();
	Bool_t bSkipMultiHits;
protected:
	std::map< std::pair< Int_t,Int_t >,TrbPixelHit > MatchedHits; // map containing indices of leading and trailing timestamp information
	Int_t HitMatching(); // match leading and trailing edge timestamps, returning the number of channels with multiple hits

public:
	TTrbDummyAnalysis(string cUserDataFilename); // constructor
	TTrbDummyAnalysis(string cUserDataFilename, string cUserTdcAddressFile, UInt_t nUserTdcWidth=32, UInt_t nUserTdcOffset=2); // constructor
	~TTrbDummyAnalysis();
	void KeepMultiHits() { bSkipMultiHits = kFALSE; };
	UInt_t GetNMatchedHits() const { return ((UInt_t)MatchedHits.size()); }; // get number of matched TDC hits, i.e. leading and trailing edge
	void PrintMatchedHits() const; // print list of matched hit to screen
	void ScanEvent() { TTrbAnalysisBase::ScanEvent(); HitMatching(); };
	void Show();
	void SkipMultiHits() { bSkipMultiHits = kTRUE; };
	void Analyse(string cUserAnalysisFilename); // user analysis code goes here...
	/* some magic ROOT stuff... */
	ClassDef(TTrbDummyAnalysis,1);
};

#endif _T_TRB_DUMMY_ANALYSIS_H