#ifndef _T_TRB_CALIBRATION_H
#define _T_TRB_CALIBRATION_H
// +++ include header files +++
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "TBranch.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TIterator.h"
#include "TList.h"
#include "TObject.h"
#include "TTree.h"

#include "myUtilities.h"
#include "TTrbDataTree.h"
#include "TTrbEventData.h"
#include "TTrbFineTime.h"
#include "TTrbHit.h"
#include "TrbStructs.h"

// +++ define constants +++
#define FINE_TIME_BINS 601 // number of bins in fine time histogram

#define CLOCK_CYCLE_LENGTH 5.0 // clock cycle length in ns

#define COARSE_TIME_BITS 11 // number of bits of coarse time

#define TREE_AUTOSAVE 300000000 // set to 300 MBytes

#define MIN_STATS 10000 // minimum number of entries in fine time histogram for a valid calibration (this is a guide only)

// +++ class definition +++
class TTrbCalibration : public TObject{
private:
	TFile *CalibrationOutfile; // file for storing calibration plots, i.e. fine time distributions, calibration maps etc.
	std::ofstream LogFile; // output file stream object for log file
	TTrbDataTree *TrbData;
	map<pair<UInt_t, UInt_t>, TTrbFineTime> ChannelCalibrations;
	void ApplyTdcCalibration(); // apply TDC calibration to data
	Bool_t CreateTree(); // create ROOT tree
	
	std::pair<UInt_t,UInt_t> DecodeChannelId(string cGraphName); // decode channel ID from graph name
	void CloseLogFile() { if(LogFile.is_open()) LogFile.close(); }; // close log file
	void FillCalibrationTable(); // compute calibration look-up table
	void FillFineTimeHistograms(); // fill fine time histograms
	void Init(); // initialise calibration object
	Bool_t OpenLogFile(); // open logfile
	Bool_t OpenRootFile();
	Bool_t OpenTrbTree(string cUserDataFilename); // open HLD data TTree
	static void PrintStatus(std::pair< std::pair< UInt_t,UInt_t >,TTrbFineTime > CurrentEntry){
		if(CurrentEntry.second.IsCalibrated()){
			CurrentEntry.second.PrintSettings();
			CurrentEntry.second.PrintStatus();
		}
	}
	void WriteSettingsToLog(); // write calibration settings to log file
	void WriteToFile(); // write calibrations histograms to file
protected:
	std::vector< pair< UInt_t,UInt_t > > ExcludedChannels; // list of channels excluded from calibration(needs to be provided by user)
	std::list<std::pair<UInt_t,UInt_t>> MissingChannels; // list of channels missing in calibration map 
	Bool_t bVerboseMode; // flag controlling verbose mode
	Double_t fBinThreshold; // threshold for bin entries in TDC fine time histogram
	Long64_t nEventsMax; // maximum number of events in TTree
	UInt_t nEntriesMin; // minimum entries required for valid calibration
	string cCalibrationFilename;
	Int_t nCalibrationType;
	string cInputFilename; // file name of input data file
	string cRootFilename; //
	string cLogFilename; // file name for logfile
	TFile* OutputRootFile; // pointer to ROOT file for storing decoded data
	TTree* OutputTree; // pointer to ROOT TTree object for data storage
public:
	TTrbCalibration(string cUserDataFilename, Int_t nUserCalibrationType=0, UInt_t nUserLimit=MIN_STATS, Bool_t bUserVerboseMode=kFALSE); // constructor
	~TTrbCalibration(); // destructor

	void DoTdcCalibration(); // run TDC fine time calibration
	void DoTdcCalibration(string cCalibrationFile); // run TDC fine time calibration using calibration constants from file
	Bool_t ExcludeChannel(UInt_t nUserTrbAddress, UInt_t nUserTdcChannel); // exclude channel from calibration
	UInt_t ExcludeChannels(string UserFilename); // exclude channels stored in text file (first column is the FPGA address (hex) and second column is TDC channel)
	UInt_t GetNChannels() const { return ((UInt_t)ChannelCalibrations.size()); }; // get number of TDC channels 
	UInt_t GetNExclChannels() const { return ((UInt_t)ExcludedChannels.size()); };
	void PrintExcludedChannels() const;
	void PrintMissingChannels() const;
	void PrintSettings() const;
	void SetCalibrationMethod(Int_t nUserCalibrationType) { nCalibrationType=nUserCalibrationType; };
	void SetLogfileName(string cUserFilename) { cLogFilename=cUserFilename; }; // set name of logfile
	void SetStatsLimit(UInt_t nUserLimit) { nEntriesMin=nUserLimit; };
	void VerboseModeOn() { bVerboseMode=kTRUE; };
	static void WriteHistogram(std::pair< std::pair< UInt_t,UInt_t >,TTrbFineTime > CurrentEntry) { 
		CurrentEntry.second.WriteHistograms(); // write fine time calibration histograms to current file
	}; // write histogram from map to file (using for_each algorithm
	/* some magic ROOT stuff... */
	ClassDef(TTrbCalibration,1);
};

#endif


	
