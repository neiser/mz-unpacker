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
#include <list>
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
struct PixelHitModel{
		UInt_t nChannelA; // unique channel ID of leading edge
		UInt_t nChannelB; // unique channel ID of trailing edge
		Double_t fSyncLETime; // synchronised leading edge time
		Double_t fTimeOverThreshold; // time-over-threshold
		Int_t nSyncIndex; // array index of synchronisation timestamp for this hit
		Bool_t bHasSyncTime; // flag indicating correct sync timestamp
		UInt_t nChannelAIndex; // array index of data entry in tree
		UInt_t nChannelBIndex; // array index of data entry in tree

		PixelHitModel() {};
		//bool operator < (const PixelHitModel &b) const { return fSyncLETime < b.fSyncLETime; };
		friend ostream& operator << (ostream &s, const PixelHitModel &a) {
			s << a.nChannelA << "\t" << a.nChannelB << "\t" << a.fSyncLETime << "\t" << a.fTimeOverThreshold << "\t" << a.nSyncIndex;
			return s;
		};
		UInt_t GetLeadEdgeChan() const { return nChannelA; };
		Double_t GetLeadEdgeTime() const { return fSyncLETime; };
		Double_t GetToT() const { return fTimeOverThreshold; };
		UInt_t GetTrailEdgeChan() const { return nChannelB; };
	};

// MISSING ITEMS
// - reference channel, e.g. trigger signal
// - trigger timing window?
// - multiplicity per channel

class TDircAnalysisBase : public TTrbAnalysisBase {
private:
	Bool_t bSkipMultiHits; // flag indicating treatment of multiple hits in a channel, standard setting is true, i.e. skipping channels with multiple hits
	Bool_t bApplyTimingCut; // flag indicating user set timing window for hit matching
	Bool_t bApplyTrigCut; // flag indicating user set trigger timing window
	Bool_t bTrigChanIsSet; // flag indicating trigger channel has been set
	Bool_t bVerboseMode; // flag setting verbose mode
	Int_t nMultiHitChan; // number of channels with multiple hits per event
	Int_t nTriggerSeqId; // sequential channel ID of trigger channel
	std::pair< Double_t,Double_t > TimingWindow; // lower and upper time value for timing cut
	std::pair< UInt_t,UInt_t > TriggerChannelAddress; // address of trigger channel, first TDC address, second TDC channel
	std::pair< Double_t,Double_t > TriggerWindow; // timing window for trigger channel
	void Init();
	Bool_t IsChannel(const PixelHitModel &CurrentHit, UInt_t nSeqChanId) const;
protected:
	Bool_t bHitMatchingError; // indicate error at hit matching stage, i.e. number of leading and trailing edges do not match
	std::set< UInt_t > SwapList; // list of TDC addresses where we need to swap leading and trailing edges
	std::map< UInt_t,std::list<PixelHitModel> > EvtReconHits; // map storing reconstructed hits
	std::list<PixelHitModel> EvtTriggerHits; // map storing reconstructed trigger hits
	std::list<UInt_t> EvtHitMatchErrChan; // list containing channel IDs of the channels where hit matching failed
	void HitMatching(); // match leading and trailing edge timestamps
public:
	TDircAnalysisBase(string cUserDataFilename); // constructor
	TDircAnalysisBase(TChain &UserChain); // constructor
	TDircAnalysisBase(string cUserDataFilename, string cUserTdcAddressFile); // constructor
	TDircAnalysisBase(string cUserDataFilename, string cUserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth); // constructor
	~TDircAnalysisBase();
	//virtual void Analyse(string cUserAnalysisFilename); // user analysis code goes here...
	virtual void Analyse(string cUserAnalysisFilename) = 0;
	void ClearTimingWindow() { TimingWindow=std::make_pair(0,0); bApplyTimingCut=kFALSE; }; // clear timing value limits
	void ClearTriggerChannel() { nTriggerSeqId=-1; TriggerChannelAddress=std::make_pair(0,0); bTrigChanIsSet=kFALSE; }; // clear trigger channel address
	void ClearTriggerWindow() { TriggerWindow=std::make_pair(0,0); bApplyTrigCut=kFALSE; }; // clear trigger timing window
	void ClearSwapList() { SwapList.clear(); }; // clear list of TDC addresses marked to swap edges
	void ClearVerboseMode() { bSkipMultiHits = kFALSE; }; // switch off verbose mode
	void KeepMultiHits() { bSkipMultiHits = kFALSE; }; // enables hit matching with multi-hits
	UInt_t GetChanMultiplicity(UInt_t nSeqChanId) const; // count hits in given channel
	UInt_t GetNMatchedHits() const; // get number of matched TDC hits, i.e. leading and trailing edge
	UInt_t GetNMultiHits() const { return (nMultiHitChan); }; // get number of channels with multiple hits in event
	Int_t GetTriggerChannel() const { return (nTriggerSeqId); }; // return seq ID of trigger channel, -1 if it is not set
	Int_t GetTriggerMultiplicity() const; // return trigger channel hit multiplicity
	Bool_t GetTriggerStatus() const { return (bTrigChanIsSet); }; // return trigger channel status flag
	Bool_t GetTriggerTime(Double_t &fTriggerTime) const; // return calibrated time of trigger channel
	void PrintChanMultiplicity() const; // print channel multiplicity for this event
	void PrintReconHits() const; // print list of matched hit to screen
	void PrintReconTriggerHits() const; // print list of matched trigger hits
	void PrintSwapList() const; // print list of TDC addresses that are in the swap edges list
	void PrintStatus() const; // print user settings to terminal
	void PrintTimingWindow() const; // print timing window values to terminal
	void PrintTriggerAddress() const; // print trigger channel address to terminal
	void PrintTriggerWindow() const; // print trigger timing window to terminal
	void ScanEvent() { TTrbAnalysisBase::ScanEvent(); HitMatching(); };
	void SetTimingWindow(Double_t fUserLow, Double_t fUserHigh) { TimingWindow = (fUserLow<fUserHigh)? std::make_pair(fUserLow,fUserHigh) : std::make_pair(fUserHigh,fUserLow); bApplyTimingCut=kTRUE; };
	Bool_t SetTriggerChannel(UInt_t nUserTdcAddress, UInt_t nUserTdcChannel); // set trigger channel address
	void SetTriggerWindow(Double_t fUserLow, Double_t fUserHigh) { TriggerWindow = (fUserLow<fUserHigh)? std::make_pair(fUserLow,fUserHigh) : std::make_pair(fUserHigh,fUserLow); bApplyTrigCut=kTRUE; };
	void SetVerboseMode() { bVerboseMode=kTRUE; }; // switch on verbose mode
	void Show(); // print data of event 0 to terminal
	void SkipMultiHits() { bSkipMultiHits = kTRUE; }; // set flag to skip channels with multiple hits
	Bool_t SwapEdges(UInt_t nUserTdcAddress); // enter TDC address in swap list, leading and trailing edges will be swapped when matching the hits
	/* some magic ROOT stuff... */
	ClassDef(TDircAnalysisBase,1);
};

#endif