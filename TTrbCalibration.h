#ifndef _T_TRB_CALIBRATION_H
#define _T_TRB_CALIBRATION_H
// +++ include header files +++
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "TBranch.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1D.h"
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

#define TREE_AUTOSAVE 300000000 // set to 300 MBytes

#define MIN_STATS 10000 // minimum number of entries in fine time histogram for a valid calibration (this is a guide only)

struct TDC_CAL_STATS { // structure for storing TDC calibration statistics
	Int_t nEntries;
	Double_t fLowerEdge;
	Double_t fUpperEdge;
	Double_t fWidth;
};

struct TDC_CAL_DATA {
	TH1D* hFineTime; // TDC channel fine time distribution
	TGraph* grCalibrationTable; // graphical representation of calibration table
	Bool_t bCalibrationIsValid; // flag indication validity of calibration
	std::vector<Double_t> fCalibrationTable; // vector containing calibrated bin timestamps (index is bin centre)
	TDC_CAL_STATS ChannelStats;
};

// +++ class definition +++
class TTrbCalibration : public TObject{
private:
	TFile *CalibrationOutfile;
	TTrbDataTree *TrbData;
	map<pair< UInt_t, UInt_t >, TDC_CAL_DATA > TdcCalibrationData; // map containing the fine time calibration data; the key is a pair of the TDC address and the TDC channel number
	map<pair<UInt_t, UInt_t>, TTrbFineTime> ChannelCalibrations;
	map<UInt_t, TTrbFineTime> ReferenceCalibrations; // calibration of reference channels to be used if channel's own calibration fails, TDC address is used as key, always use simple calibration method
	map<UInt_t, vector< Double_t > > TdcRefCalibration; // map containing fine time calibration of reference channels (alwyas using the simple model based on the width of the fine time distribution)
	map<UInt_t,UInt_t> TdcRefChannels; // TDC reference channel IDs (one per FPGA)
	void ApplyTdcCalibration(); // apply TDC calibration to data
	void ClearFineTimeMap(); // clear fine time map
	Bool_t CreateTree();
	void DeleteCalibrationPlots(); // delete all calibration hostograms and graphs
	void FillCalibrationTable(); // compute calibration look-up table
	void FillFineTimeHistograms(); // fill fine time histograms
	void FillReferenceCalibrationTables(); // compute calibration look-up tables for reference channels
	void Init(); // initialise calibration object
	void InitCalibrationData(TDC_CAL_DATA& ChannelCalibration); // initialise calibration data
	Bool_t OpenRootFile();
	Bool_t OpenTrbTree(string cUserDataFilename); // open HLD data TTree
	static void PrintStatus(std::pair< std::pair< UInt_t,UInt_t >,TTrbFineTime > CurrentEntry){
		if(CurrentEntry.second.IsCalibrated()){
			CurrentEntry.second.PrintSettings();
			CurrentEntry.second.PrintStatus();
		}
	}
	void WriteToFile(); // write calibrations histograms to file
protected:
	std::vector< pair< UInt_t,UInt_t > > ExcludedChannels; // list of channels excluded from calibration(needs to be provided by user)
	Bool_t bVerboseMode; // flag controlling verbose mode
	Double_t fBinThreshold; // threshold for bin entries in TDC fine time histogram
	Long64_t nEventsMax; // maximum number of events in TTree
	UInt_t nEntriesMin; // minimum entries required for valid calibration
	string cCalibrationFilename;
	Int_t nCalibrationType;
	string cInputFilename;
	string cRootFilename;
	TFile* OutputRootFile; // pointer to ROOT file for storing decoded data
	TTree* OutputTree; // pointer to ROOT TTree object for data storage
public:
	TTrbCalibration(string cUserDataFilename, Int_t nUserCalibrationType=0, UInt_t nUserLimit=MIN_STATS, Bool_t bUserVerboseMode=kFALSE); // constructor
	~TTrbCalibration(); // destructor

	void DoTdcCalibration(); // run TDC fine time calibration
	Bool_t ExcludeChannel(UInt_t nUserTrbAddress, UInt_t nUserTdcChannel); // exclude channel from calibration
	UInt_t ExcludeChannels(string UserFilename); // exclude channels stored in text file (first column is the FPGA address (hex) and second column is TDC channel)
	UInt_t GetNChannels() const { return ((UInt_t)TdcCalibrationData.size()); }; // get number of TDC channels 
	UInt_t GetNExclChannels() const { return ((UInt_t)ExcludedChannels.size()); };
	UInt_t GetNRefChannels() const { return ((UInt_t)TdcRefChannels.size()); }; // get number of reference channels
	void PrintExcludedChannels() const;
	void PrintRefChannels() const;
	void SetCalibrationMethod(Int_t nUserCalibrationType) { nCalibrationType=nUserCalibrationType; };
	void SetStatsLimit(UInt_t nUserLimit) { nEntriesMin=nUserLimit; };
	//static void WriteHistogram(std::pair< std::pair< UInt_t,UInt_t >,TDC_CAL_DATA > CurrentEntry) { 
	//	CurrentEntry.second.hFineTime->Write(); // write fine time histograms to current file
	//	CurrentEntry.second.grCalibrationTable->Write(); // write calibration table graphs to current file
	//};
	static void WriteHistogram(std::pair< std::pair< UInt_t,UInt_t >,TTrbFineTime > CurrentEntry) { 
		//cout << "########## Writing histogram..." << endl;
		CurrentEntry.second.GetHistogram().Write(); // write fine time histograms to current file
	}; // write histogram from map to file (using for_each algorithm
	/* some magic ROOT stuff... */
	ClassDef(TTrbCalibration,1);
};

#endif


	
