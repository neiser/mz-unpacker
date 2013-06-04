#ifndef _T_LASERSPOT_ANALYSIS_H
#define _T_LASERSPOT_ANALYSIS_H
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

class TLaserspotAnalysis : public TTrbAnalysisBase {
private:
	void Init();
	Bool_t bSkipMultiHits;
	Bool_t bApplyTimingCut;
	std::pair< UInt_t,UInt_t > LaserTriggerAddress; // laser trigger address, first->TDC address, second->TDC channel
	std::pair< UInt_t,UInt_t > McpSpotAddress; // illumintad MCP-PMT pixel address, first->TDC address, second->TDC channel
	std::pair< Double_t,Double_t> TimingWindow; // lower and upper time value for timing cut

protected:
	std::map< std::pair< Int_t,Int_t >,TrbPixelHit > MatchedHits; // map containing indices of leading and trailing timestamp information
	Int_t HitMatching(); // match leading and trailing edge timestamps, returning the number of channels with multiple hits
	std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator FindHitByValue(UInt_t nUserSeqId) const;

public:
	TLaserspotAnalysis(string cUserDataFilename); // constructor
	TLaserspotAnalysis(string cUserDataFilename, string cUserTdcAddressFile, UInt_t nUserLaserTrbAddress, UInt_t nUserLaserTdcChan, UInt_t nUserMcpSpotTrbAddress, UInt_t nUserMcpSpotTdcChan, UInt_t nUserTdcOffset=2, UInt_t nUserTdcWidth=32); // constructor
	~TLaserspotAnalysis();
	void ClearTimingWindow() { TimingWindow=std::make_pair(0,0); bApplyTimingCut=kFALSE; };
	void KeepMultiHits() { bSkipMultiHits = kFALSE; };
	UInt_t GetNMatchedHits() const { return ((UInt_t)MatchedHits.size()); }; // get number of matched TDC hits, i.e. leading and trailing edge
	void PrintLaserTriggerAddress() const;
	void PrintMatchedHits() const; // print list of matched hit to screen
	void PrintMcpSpotAddress() const;
	void PrintTimingWindow() const;
	void ScanEvent() { TTrbAnalysisBase::ScanEvent(); HitMatching(); };
	void SetLaserTriggerAddress(UInt_t nUserLaserTrbAddress, UInt_t nUserLaserTdcChan) { LaserTriggerAddress = std::make_pair(nUserLaserTrbAddress,nUserLaserTdcChan); };
	void SetMcpSpotAddress(UInt_t nUserMcpSpotTrbAddress, UInt_t nUserMcpSpotTdcChan) { McpSpotAddress = std::make_pair(nUserMcpSpotTrbAddress,nUserMcpSpotTdcChan); };
	void SetTimingWindow(Double_t fUserLow, Double_t fUserHigh) { TimingWindow = (fUserLow<fUserHigh)? std::make_pair(fUserLow,fUserHigh) : std::make_pair(fUserHigh,fUserLow); bApplyTimingCut=kTRUE; };
	void Show();
	void SkipMultiHits() { bSkipMultiHits = kTRUE; };
	void Analyse(string cUserAnalysisFilename); // user analysis code goes here...
	/* some magic ROOT stuff... */
	ClassDef(TLaserspotAnalysis,1);
};

#endif _T_LASERSPOT_ANALYSIS_H