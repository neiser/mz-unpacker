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
#include <list>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "TChain.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TObject.h"
#include "TTree.h"

#include "myUtilities.h"
#include "TTrbDataTree.h"

// struct definitions
struct TrbTdcSetupModel{
	UInt_t nTdcSize; // number of TDC channels available (excludes synchronisation channels)
	UInt_t nTdcOffset; // number of channels reserved for synchronisation signals, expects synchronisation signal in the lowest channels
};


// +++ class definition +++
class TTrbAnalysisBase : public TObject{
private:
	Bool_t bCanAnalyse; // flag indicating if all necessary information is available to proceed with analysis
	Bool_t bIsChain; // flag indicating using a TChain as data source
	Bool_t bTreeIsOpen; // flag indicating tree pointer is valid
	Bool_t bVerboseMode;
	string cDataFilename; // name of data file
	string cTreeName; // name of tree, standard is "T"
	TFile *RawData; // pointer to RooT file containing Tree with raw data
	void OpenTree();
	void OpenTree(TTree *UserTree);
protected:
	// flags indicating status of base class
	Bool_t bDoHitMatching; // flag to control event treatment
	
	// variables used for description of setup
	UInt_t nTdcDefaultSize; // number of channels per TDC
	UInt_t nTdcDefaultOffset; // channel offset in TDC to mask reference channels
	TTrbDataTree *TrbData; // pointer to TRB data tree
	// maps used in analysis to access Tree entries
	std::map< UInt_t,TrbTdcSetupModel > TdcAddresses; // map containing the TDC address as key and its number of channels as value
	std::map< std::pair< UInt_t,UInt_t >,UInt_t > MappingTable; // look-up table for channel mapping
	std::map< UInt_t,UInt_t > EvtSyncTimestamps; // map containing TDC address and array index where sync timestamp can be found for this TDC in the current event
	std::multimap< UInt_t,UInt_t > EvtTdcHits; // multimap containing unique channel number as key and array index as value
	std::set< std::pair< UInt_t,UInt_t > > ExcludedChannels; // list of channels excluded from calibration(needs to be provided by user)
	virtual void ComputeMappingTable(); // compute mapping table
	Bool_t GetTreeStatus() const { return (bTreeIsOpen); };
	void Init();
	void UpdateStatus(); // this function checks if all necessary information is available to perform an analysis
public:
	TTrbAnalysisBase(string cUserDataFilename, Bool_t bUserVerboseMode=kFALSE, string cUserTreeName="T"); // constructor
	TTrbAnalysisBase(TChain &UserChain, Bool_t bUserVerboseMode=kFALSE, string cUserTreeName="T"); // constructor
	//string cUserTdcAddressesFile
	virtual ~TTrbAnalysisBase(); // destructor
	//void Analyse(string cUserAnalysisFilename); // analysis routine
	virtual void Analyse(string cUserAnalysisFilename) = 0; // pure virtual analysis function, needs to be implemented in derived class
	virtual Bool_t CheckRandomBits(); // check if hits' random bits are the same in an event
	virtual Bool_t CheckDecodingStatus(UInt_t nUserStatus=0) const;
	void DisableHitMatching() { bDoHitMatching = kFALSE; }; // switch off hit matching
	void EnableHitMatching() { bDoHitMatching = kTRUE; }; // switch on hit matching
	Bool_t ExcludeChannel(UInt_t nUserTrbAddress, UInt_t nUserTdcChannel); // exclude channel from analysis
	UInt_t ExcludeChannels(string UserFilename); // exclude channels stored in text file (first column is the FPGA address (hex) and second column is TDC channel) from analysis
	Bool_t GetChannelAddress(Int_t nUserSeqId, Int_t &nTdcAdress, Int_t &nTdcChan) const; // return channel TDC address and number given a seq ID
	virtual Int_t GetEntry(Long64_t nEntryIndex); // get entry from ROOT Tree and store event values in local variables
	Bool_t GetHitMatchingFlag() const { return (bDoHitMatching); };
	Int_t GetNEvents() const { return ((Int_t)TrbData->fChain->GetEntries()); }; // get number of events in RooT tree
	UInt_t GetNSyncTimestamps() const { return ((UInt_t)EvtSyncTimestamps.size()); }; // get number of TDC sync timestamps found in event
	UInt_t GetNTdcs() const { return ((UInt_t)TdcAddresses.size()); }; // number of TDCs in setup
	UInt_t GetNTdcHits() const { return ((UInt_t)EvtTdcHits.size()); }; // get number of TDC hits per event
	Int_t GetSeqId(UInt_t nUserTdcAddress, UInt_t nUserTdcChannel) const; // get sequential channel number
	UInt_t GetSizeOfMapTable() const { return ((UInt_t)MappingTable.size()); }; // get size of mapping table, corresponds to number of channels
	Bool_t GetStatus() const { return (bCanAnalyse); }; // get status of base analysis class, only proceed with analysis if true!
	Int_t GetTdcAddress(UInt_t nArrayIndex) const; // get TDC address of entry at index
	Int_t GetTdcOffset(UInt_t nUserTdcAddress) const; // get TDC offset (channels reserved for synchronisation signal)
	Int_t GetTdcSize(UInt_t nUserTdcAddress) const; // get number of channels of TDC, -1 if TDC address does not exist
	Int_t GetTdcSyncIndex(UInt_t nTdcAddress) const; // get position of sync information in array based on TDC address
	Double_t GetTdcSyncTimestamp(UInt_t nTdcAddress) const;
	Double_t GetTime(UInt_t nArrayIndex) const; // get calibrated time of hit at index
	string GetTreeName() const { return (cTreeName); }; // get name of tree
	void PrintExcludedChannels() const; // print list of excluded channels to screen
	void PrintSyncTimestamps() const; // print list of sync timestamps to screen
	virtual void PrintTdcAddresses() const; // print list of TDC addresses to screen
	void PrintTdcHits() const; // print list of TDC hits to screen
	virtual void PrintTdcMapping() const; // print TDC mapping table to screen
	virtual void ScanEvent(); // scan event data and fill sync timestamp map and hit index multimap
	virtual Int_t SetTdcAddresses(string cUserTdcAddressesFile); // set TRB addresses using text file as input
	virtual Int_t SetTdcAddresses(string cUserTdcAddressesFile, UInt_t nUserChanPerTdc, UInt_t nUserTdcOffset) { SetTdcSize(nUserChanPerTdc); SetTdcOffset(nUserTdcOffset); return (SetTdcAddresses(cUserTdcAddressesFile)); }; // set TDC addresses with fixed size and offset
	void SetTdcSize(UInt_t nUserChanPerTdc) { nTdcDefaultSize = nUserChanPerTdc; ComputeMappingTable(); }; // set number of TDC channels per TDC, excluding reference channels, needs to trigger a rebuild of mapping table
	void SetTdcOffset(UInt_t nUserTdcOffset) { nTdcDefaultOffset = nUserTdcOffset; ComputeMappingTable(); }; // set TDC channel offset to mask reference channels, needs to trigger a rebuild of mapping table
//	void SetTreeName( string cUserTreeName) { if(!cUserTreeName.empty()) cTreeName = cUserTreeName; }; // set name of data tree, standard is "T"
	void Show(Int_t nEventIndex);
	void WriteTdcMapping(string cUserMappingFile); // write mapping table to text file
	/* some magic ROOT stuff... */
	ClassDef(TTrbAnalysisBase,1);
};

#endif