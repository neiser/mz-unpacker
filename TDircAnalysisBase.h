#ifndef _T_DIRC_ANALYSIS_BASE_H
#define _T_DIRC_ANALYSIS_BASE_H
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
		UInt_t nChannelA; // unique channel ID of leading edge
		UInt_t nChannelB; // unique channel ID of trailing edge
		Double_t fSyncLETime; // synchronised leading edge time
		Double_t fTimeOverThreshold; // time-over-threshold
		Int_t nSyncIndex; // array index of synchronisation timestamp for this hit
		Bool_t bHasSyncTime; // flag indicating correct sync timestamp
	};

class TDircAnalysisBase : public TTrbAnalysisBase {
private:
	void Init();
	Bool_t bSkipMultiHits; // flag indicating treatment of multiple hits in a channel, standard setting is true, i.e. skipping channels with multiple hits
	Bool_t bApplyTimingCut;
	Bool_t bVerboseMode; // flag setting verbose mode
	Int_t nMultiHitChan; // number of channels with multiple hits per event
	std::pair< Double_t,Double_t> TimingWindow; // lower and upper time value for timing cut

protected:
	std::set< UInt_t > SwapList; // list of TDC addresses where we need to swap leading and trailing edges
	std::map< std::pair< Int_t,Int_t >,TrbPixelHit > MatchedHits; // map containing indices of leading and trailing timestamp information
	std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator FindHitByValue(UInt_t nUserSeqId) const;
	void HitMatching(); // match leading and trailing edge timestamps
public:
	TDircAnalysisBase(string cUserDataFilename); // constructor
	TDircAnalysisBase(string cUserDataFilename, string cUserTdcAddressFile); // constructor
	TDircAnalysisBase(string cUserDataFilename, string cUserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth); // constructor
	~TDircAnalysisBase();
	//virtual void Analyse(string cUserAnalysisFilename); // user analysis code goes here...
	virtual void Analyse(string cUserAnalysisFilename) = 0;
	void ClearTimingWindow() { TimingWindow=std::make_pair(0,0); bApplyTimingCut=kFALSE; };
	void ClearSwapList() { SwapList.clear(); }; // clear list of TDC addresses marked to swap edges
	void ClearVerboseMode() { bSkipMultiHits = kFALSE; }; // switch off verbose mode
	void KeepMultiHits() { bSkipMultiHits = kFALSE; };
	UInt_t GetNMatchedHits() const { return ((UInt_t)MatchedHits.size()); }; // get number of matched TDC hits, i.e. leading and trailing edge
	UInt_t GetNMultiHits() const { return (nMultiHitChan); }; // get number of channels with multiple hits in event
	void PrintMatchedHits() const; // print list of matched hit to screen
	void PrintSwapList() const; // print list of TDC addresses that are in the swap edges list
	void PrintTimingWindow() const;
	void ScanEvent() { TTrbAnalysisBase::ScanEvent(); HitMatching(); };
	void SetTimingWindow(Double_t fUserLow, Double_t fUserHigh) { TimingWindow = (fUserLow<fUserHigh)? std::make_pair(fUserLow,fUserHigh) : std::make_pair(fUserHigh,fUserLow); bApplyTimingCut=kTRUE; };
	void SetVerboseMode() { bVerboseMode=kTRUE; }; // switch on verbose mode
	void Show();
	void SkipMultiHits() { bSkipMultiHits = kTRUE; }; // set flag to skip channels with multiple hits
	void SwapEdges(UInt_t nUserTdcAddress); // enter TDC address in swap list, leading and trailing edges will be swapped when matching the hits
	/* some magic ROOT stuff... */
	ClassDef(TDircAnalysisBase,1);
};

#endif