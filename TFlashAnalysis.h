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

//#pragma link C++ class std::map< UInt_t,std::list<PixelHitModel> >+;

struct PixelCuts{ // store cuts for each pixel here
	UInt_t nChanId;
	Double_t fLeadingEdgeLow; // lower value for leading edge timestamp
	Double_t fLeadingEdgeHigh; // upper value for leading edge timestamp
	Double_t fToTLow;
	Double_t fToTHigh;
};

// +++ class definition +++
class TFlashAnalysis : public TDircAnalysisBase {
private:
	enum TriggerChannels {TRIGGER_1=256, TRIGGER_2=258, TRIGGER_3=260, TRIGGER_4=262, COIN=264};
	UInt_t nNumberOfHitPixels; // number of pixels hit in one event
	// need some object to store cuts and time offsets
	std::map< UInt_t, Double_t > PixelTimeOffsets; // map for storing pixel time offset constants
	std::set< std::pair<UInt_t,UInt_t> > PixelPairs; // store combination of pixels the user wants to compare
	std::map< std::pair<UInt_t,UInt_t>,Double_t > PixelCorrelations;
	std::ofstream LogFileBuffer; // output stream handle connecting to log file
	time_t RawTime; // time structure
	void Clear(); // clear results of event analysis
	void Init(); // initialise FLASH analysis class values
protected:
	Double_t ComputeChannelCorrelation(UInt_t nChanA, UInt_t nChanB, Bool_t &IsValid);
	Double_t ComputeChannelCorrelation(UInt_t nChanA, Double_t fChanATotLow, Double_t fChanATotHigh, UInt_t nChanB, Double_t fChanBTotLow, Double_t fChanBTotHigh, Bool_t &IsValid);
	void WriteLogfileHeader(); // write status information to log file
public:
	//TFlashAnalysis(TChain &UserChain);
	TFlashAnalysis(string cUserDataFilename, string UserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth); // standard constructor
	TFlashAnalysis(string cUserDataFilename, string UserTdcAddressFile); // constructor, all TDC defintions in text file
	virtual ~TFlashAnalysis(); // standard destructor
	Bool_t AddPixelPair(UInt_t nUserChanA, UInt_t nUserChanB); // register a new pixel correlation pair
	UInt_t AddPixelPairs(string cUserPairList); // register new pixel pairs
	void Analyse(Long64_t nEntryIndex); // analysis routine goes here, this method is needed!
	void AnalyseTrigger(string cUserAnalysisFilename); // analysis of trigger signal only
	void ClearPixelTimeOffset(UInt_t nUserSeqId); // clear pixel time offset from map
	void ClearPixelPairs() { PixelPairs.clear(); }; 
	void FillTimingHistogram(TH1D& hTimingHist); // plot all timestamps into one histogram
	void FillTimingHistogram(UInt_t nUserChan, TH1D& hTimingHist); // plot timestamps of one channel into histogram
	void FillTimingHistogram(TH2D& hTimingHist) const { FillTimingHistogram(hTimingHist,0,GetSizeOfMapTable()-1); }; // plot timestamps as a function of the seq channel ID
	void FillTimingHistogram(TH2D& hTimingHist, UInt_t nSeqIdLow, UInt_t nSeqIdHigh) const; // plot timestamps as a function of the seq channel ID
	void FillToTHistogram(TH2D& hTimingHist) const; 
	UInt_t GetNumberOfHitPixels() const { return (nNumberOfHitPixels); }; // return number of hit pixels in this event
	UInt_t GetNumberOfCorrelations() const { return((UInt_t)PixelCorrelations.size()); }; // return size of correlation map
	Bool_t GetPixelCorrelation(UInt_t nUserChanA, UInt_t nUserChanB, Double_t& fDelta) const { return(GetPixelCorrelation(std::make_pair(nUserChanA,nUserChanB),fDelta)); };
	Bool_t GetPixelCorrelation(std::pair<UInt_t,UInt_t> UserPair, Double_t& fDelta) const;
	Bool_t GetPixelCorrelation(UInt_t nChanA, Double_t fChanATotLow, Double_t fChanATotHigh, UInt_t nChanB, Double_t fChanBTotLow, Double_t fChanBTotHigh, Double_t& fDelta);
	TH2D* MakePixelCorrelationMap(); // create 2D histogram showing pixel correlations
	Bool_t SetPixelTimeOffset(UInt_t nUserSeqId, Double_t fUserOffset=0.0); // set timing offset for single pixel
	//const std::map< UInt_t,std::list<PixelHitModel> > & GetReconHits() const { return EvtReconHits; };
	//const std::list<PixelHitModel> & GetPixelHits(UInt_t nUserSeqId) const;
	/* some magic ROOT stuff... */
	ClassDef(TFlashAnalysis,1);
};

#endif