#ifndef _T_FLASH_ANALYSIS_H
#define _T_FLASH_ANALYSIS_H
// +++ include header files +++
#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdlib>
#include <ctime>
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
//#include "TFlashAnalysis_LinkDef.h"


struct PairResults{ // store histogram pointers for results
	std::bitset<2> RegisteredHistograms; 
	TH1D* hTimeDifference; // time difference between two pixels;
	TH2D* hToTCorrelation; // Time-over-Threshold correaltion plot
	void Init(){
		RegisteredHistograms.reset(); // reset bitset to false
		hTimeDifference = NULL;
		hToTCorrelation = NULL;
	};
};

// +++ class definition +++
class TFlashAnalysis : public TDircAnalysisBase {
	typedef std::map< std::pair<UInt_t,UInt_t>, std::pair<std::list<PixelHitModel>::const_iterator,std::list<PixelHitModel>::const_iterator> > PIXELPAIR;
private:
	enum TriggerChannels {TRIGGER_1=256, TRIGGER_2=258, TRIGGER_3=260, TRIGGER_4=262, COIN=264};
	Bool_t bApplyOffset;
	Bool_t bIsSortedListOfPairs;
	UInt_t nNumberOfHitPixels; // number of pixels hit in one event
	UInt_t nNumberOfFiredTriggers; // number of trigger channels that have fired
	// need some object to store cuts and time offsets
	std::map< UInt_t, std::pair<Double_t, Double_t> > PixelLECuts;
	std::map< UInt_t, std::pair<Double_t, Double_t> > PixelTotCuts;
	std::map< UInt_t, Double_t > PixelTimeOffsets; // map for storing pixel time offset constants
	std::map< std::pair<UInt_t,UInt_t>, PairResults > PairHistograms;
	std::set< std::pair<UInt_t,UInt_t> > PixelPairs; // store combination of pixels the user wants to compare
	std::set<UInt_t> TriggerChannels; // store channel belonging to trigger system that need to be present
	PIXELPAIR DetectedPixelPairs;
	std::ofstream LogFileBuffer; // output stream handle connecting to log file
	time_t RawTime; // time structure
	void ApplyPixelCuts(); // apply pixel cuts, any hits not passing the cuts will be removed
	void Clear(); // clear results of event analysis
	void FillHistograms(PIXELPAIR::const_iterator it); // fill standard analysis histograms
	void Init(); // initialise FLASH analysis class values
protected:
	Double_t ComputePixelTimeDiff(PIXELPAIR::const_iterator UserPair) const; // compute time difference between two pixels
	void WriteLogfileHeader(); // write status information to log file
public:
	//TFlashAnalysis(TChain &UserChain);
	TFlashAnalysis(string cUserDataFilename, string UserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth); // standard constructor
	TFlashAnalysis(string cUserDataFilename, string UserTdcAddressFile); // constructor, all TDC defintions in text file
	virtual ~TFlashAnalysis(); // standard destructor
	Bool_t AddPixelLECut(UInt_t nUserChan, Double_t fLow, Double_t fHigh); // add pixel cut definitions
	Bool_t AddPixelToTCut(UInt_t nUserChan, Double_t fLow, Double_t fHigh); // add pixel cut definitions
	Bool_t AddPixelPair(UInt_t nUserChanA, UInt_t nUserChanB); // register a new pixel correlation pair
	UInt_t AddPixelPairs(string cUserPairList); // register new pixel pairs
	Bool_t AddTriggerChannel(UInt_t nUserChannel);
	void Analyse(Long64_t nEntryIndex); // analysis routine goes here, this method is needed!
	void AnalyseTrigger(string cUserAnalysisFilename); // analysis of trigger signal only
	void ApplyOffsets() { bApplyOffset=kTRUE; };
	void ClearPixelTimeOffset(UInt_t nUserSeqId); // clear pixel time offset from map
	void ClearPixelPairs() { PixelPairs.clear(); }; 
	void FillTimingHistogram(TH1D& hTimingHist); // plot all timestamps into one histogram
	void FillTimingHistogram(UInt_t nUserChan, TH1D& hTimingHist); // plot timestamps of one channel into histogram
	void FillTimingHistogram(TH2D& hTimingHist) const { FillTimingHistogram(hTimingHist,0,GetSizeOfMapTable()-1); }; // plot timestamps as a function of the seq channel ID
	void FillTimingHistogram(TH2D& hTimingHist, UInt_t nSeqIdLow, UInt_t nSeqIdHigh) const; // plot timestamps as a function of the seq channel ID
	void FillToTHistogram(TH2D& hTimingHist) const; 
	void FillToTCorrelation(UInt_t nChanA, UInt_t nChanB, TH2D& hUserHist) const;
	void FillWalkHistogram(UInt_t nRefChan, Double_t fToTLow, Double_t fTotHigh, UInt_t nUserChan, TH2D& hUserHist) const;
	UInt_t GetNumberOfHitPixels() const { return (nNumberOfHitPixels); }; // return number of hit pixels in this event
	UInt_t GetNumberOfCorrelations() const { return((UInt_t)DetectedPixelPairs.size()); }; // return size of correlation map
	Bool_t GetPairTimeDiff(UInt_t nUserChanA, UInt_t nUserChanB, Double_t& fDelta) const { return (GetPairTimeDiff(std::make_pair(nUserChanA,nUserChanB),fDelta)); } ; // get leading edge time difference between two pixels
	Bool_t GetPairTimeDiff(std::pair<UInt_t,UInt_t> UserPair, Double_t& fDelta) const; // get leading edge time difference between two pixels
	Bool_t GetPairTimeDiff(UInt_t nUserChanA, Double_t fChanATotLow, Double_t fChanATotHigh, UInt_t nUserChanB, Double_t fChanBTotLow, Double_t fChanBTotHigh, Double_t& fDelta) const; // get leading edge time difference between two pixels wit ToT cuts
	Double_t GetPixelOffset(UInt_t nSeqId) const;
	const std::set< std::pair<UInt_t,UInt_t> >* GetListOfPixelPairs();
	void IgnoreOffsets() { bApplyOffset=kFALSE; };
	TH2D* MakePixelCorrelationMap(); // create 2D histogram showing pixel correlations
	void PrintListOfPixelPairs() const;
	Bool_t RegisterTimeDiffHist(UInt_t nChanA, UInt_t nChanB, TH1D* hUserHist); // register time difference histogram
	Bool_t RegisterTotCorrHist(UInt_t nChanA, UInt_t nChanB, TH2D* hUserHist); // register time difference histogram
	Bool_t SetPixelTimeOffset(UInt_t nUserSeqId, Double_t fUserOffset=0.0); // set timing offset for single pixel
	/* some magic ROOT stuff... */
	ClassDef(TFlashAnalysis,1);
};

#endif