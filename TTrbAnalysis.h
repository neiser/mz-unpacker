#ifndef _T_TRB_ANALYSIS_H
#define _T_TRB_ANALYSIS_H
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

// +++ define constants +++
#define N_TDC_CHAN 65 // number of TDC channels
#define TDC_CHAN_OFFSET 1 // TDC channel index offset (first two channels are reserved for reference time)
#define TDC_SWAP_RISING_FALLING 1
#define HIT_TIME_MIN -70000.0 // lower boundary for hit time histograms (in ns); change as required
#define HIT_TIME_MAX 10000.0 // upper boundary for hit time histograms (in ns); change as required

// +++ class definition +++
class TTrbAnalysis : public TObject{
private:
	std::map< Int_t,Int_t > PixelHits; // map containing indices of leading and trailing timestamp information
	std::multimap< UInt_t,Int_t > TdcHits; //multimap containing all TDC hits, unique channel number is used as key and array index is the value, will be sorted according to channel number
	std::map< UInt_t,UInt_t > TrbAddresses; // map containing the TRB address as key and its index
	std::map< UInt_t,Int_t > TdcRefTimes; // map containing the TDC address and its reference time
	std::map< Int_t,Double_t > TdcLeadingEdges; // map containing unique TDC channel ID and leading edge timestamp corrected for reference time, excluding multiple hits
	std::map< Int_t,Double_t > TimeOverThreshold; // map containing unique TDC channel ID and pulse width information
	std::vector< std::pair< UInt_t,UInt_t > > ExcludedChannels; // list of channels excluded from calibration(needs to be provided by user)
	Bool_t CheckRandomBits(); // check if hits' random bits are the same in an event
	void ClearEventMaps(); // clear event-related maps
	Int_t ComputeTdcChanId(UInt_t nTrbAddress, UInt_t nTdcChannel);
	std::vector< std::pair< Double_t,Int_t > > ComputeEventTiming(); // compute timing differences between hits in one event
	void FillTdcHits();
	void FillTdcLeadingEdge(); // fill TDC leading edge timestamp map (removing multi-hit channels)
	void FillTimeOverThreshold(); // fill pulse width into Time-over-Threshold map
	Int_t HitMatching(Bool_t bSkipMultiHits=kTRUE); // match leading and trailing edge timestamps, returning the number of channels with multiple hits
	void Init();
	Bool_t OpenTrbTree(string cUserDataFilename);
	Bool_t SetRefTimestamps();
	Int_t SetTrbAddresses(string cUserTdcAddressesFile); // set TRB addresses using text file as input
protected:
	TTrbDataTree *TrbData; // pointer to TRB data tree
	// run control level variables
	Bool_t bVerboseMode; // flag controlling verbose mode
	// data and setup level variables
	Int_t nEventsMax; // max number of events in data file
	Int_t nMaxTdcChannel; // maximum TDC channel ID
	Int_t nTrbEndpoints; // number of TRB board addresses defined by user input
	Int_t nTdcRefChannel;
	// event level information for analysis
	Bool_t bAllRefChanValid; // flag indicating that all reference channel signals are found
	Bool_t bRefChanIsSet; // flag indicating user specified reference channel is valid
	Int_t nEvtMultHits; // number of channels with multiple hits in event
public:
	TTrbAnalysis(string cUserDataFilename, string cUserTdcAddressesFile, Bool_t bUserVerboseMode=kFALSE); // constructor
	//string cUserTdcAddressesFile
	~TTrbAnalysis(); // destructor
	void Analyse(string cUserAnalysisFilename); // analysis routine
	void Analyse(string cUserAnalysisFilename, UInt_t nUserTrbAddress, UInt_t nUserTdcChannel); // analysis routine
	Bool_t ExcludeChannel(UInt_t nUserTrbAddress, UInt_t nUserTdcChannel); // exclude channel from calibration
	UInt_t ExcludeChannels(string UserFilename); // exclude channels stored in text file (first column is the FPGA address (hex) and second column is TDC channel)
	Int_t GetEntry(Long64_t nEntryIndex);
	std::map< Int_t,Double_t >* GetLeadingEdge() { return (&TdcLeadingEdges); }; // get timestamps of leading edges, no multiple hits!
	Int_t GetNEvents() const { return (nEventsMax); };
	Int_t GetNExclChannels() const { return ((Int_t)ExcludedChannels.size()); };
	Int_t GetNTrb() const { return (nTrbEndpoints); };
	std::map< Int_t,Double_t >* GetToT() { return (&TimeOverThreshold); };
	void PrintExcludedChannels() const;
	void PrintRefTimestamps() const;
	void PrintTdcHits() const;
	void PrintTdcLeadingEdges() const;
	void PrintTrbAddresses() const;
	void SetRefChannel(UInt_t nTrbAddress, UInt_t nTdcChannel); // set TDC reference channel (not for synchronisation!)
	void WriteTdcMapping(string cUserMappingFile);
	/* some magic ROOT stuff... */
	ClassDef(TTrbAnalysis,1);
};

#endif
