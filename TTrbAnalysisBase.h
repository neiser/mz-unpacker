#ifndef _T_TRB_ANALYSIS_BASE_H
#define _T_TRB_ANALYSIS_BASE_H
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

// +++ class definition +++
class TTrbAnalysisBase : public TObject{
private:
	// containers describing TRB setup 
	std::map< UInt_t,UInt_t > TdcAddresses; // map containing the TDC address as key and its number of channels as value
	std::map< std::pair< UInt_t,UInt_t >,UInt_t > MappingTable; // look-up table for channel mapping
	std::vector< std::pair< UInt_t,UInt_t > > ExcludedChannels; // list of channels excluded from calibration(needs to be provided by user)
	// event maps
	std::multimap< UInt_t,UInt_t > EvtChanEntries; // multimap containing unique channel number as key and array index as value
	std::map< UInt_t,UInt_t > EvtSyncTimeStamps; // map containing TDC address and array index where sync timestamp can be found for this TDC in the current event
protected:
	TTrbDataTree *TrbData; // pointer to TRB data tree
	UInt_t nChanPerTdc; // number of channels per TDC
	UInt_t nTdcOffset; // channel offset in TDC to mask reference channels
	Int_t nEventsMax; // max number of events stored in ROOT Tree
	// run control level variables
	Bool_t bVerboseMode; // flag controlling verbose mode
	Bool_t CheckRandomBits(); // check if hits' random bits are the same in an event
	void ClearEventMaps(); // clear event-related maps
	void ComputeMappingTable(); // compute mapping table
	void FillSyncTimeStamps(); // find sync timestamps in an event and write to EvtSyncTimeStamps map, return value
	void Init();
	Bool_t OpenTrbTree(string cUserDataFilename); // open ROOT file containing Tree with calibrated TRB data
	void SetTdcAddresses(string cUserTdcAddressesFile); // set TRB addresses using text file as input
public:
	TTrbAnalysisBase(string cUserDataFilename, string cUserTdcAddressesFile, UInt_t nUserTdcChannels=32, UInt_t nUserTdcOffset=2, Bool_t bUserVerboseMode=kFALSE); // constructor
	//string cUserTdcAddressesFile
	virtual ~TTrbAnalysisBase(); // destructor
	//void Analyse(string cUserAnalysisFilename); // analysis routine
	Bool_t ExcludeChannel(UInt_t nUserTrbAddress, UInt_t nUserTdcChannel); // exclude channel from analysis
	UInt_t ExcludeChannels(string UserFilename); // exclude channels stored in text file (first column is the FPGA address (hex) and second column is TDC channel) from analysis
	Int_t GetEntry(Long64_t nEntryIndex); // get entry from ROOT Tree and store event values in local variables
	UInt_t GetSizeOfMapTable() const { return ((UInt_t)MappingTable.size()); };
	UInt_t GetSizeOfSyncMap() const { return ((UInt_t)EvtSyncTimeStamps.size()); };
	Int_t GetSeqId(UInt_t nUserTdcAddress, UInt_t nUserTdcChannel); // get sequential channel number
	UInt_t GetTdcN() const { return ((UInt_t)TdcAddresses.size()); };
	UInt_t GetTdcOffset() const { return (nTdcOffset); };
	void PrintSyncTimeStamps() const;
	void PrintTdcAddresses() const;
	void SetChanPerTdc(UInt_t nUserChanPerTdc) { nChanPerTdc = nUserChanPerTdc; }; // set number of TDC channels per TDC, excluding reference channels
	void SetTdcOffset(UInt_t nUserTdcOffset) { nTdcOffset = nUserTdcOffset; }; // set TDC channel offset to mask reference channels
	void WriteTdcMapping(string cUserMappingFile); // write mapping table to text file
	/* some magic ROOT stuff... */
	ClassDef(TTrbAnalysisBase,1);
};

#endif _T_TRB_ANALYSIS_BASE_H