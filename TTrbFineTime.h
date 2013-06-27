#ifndef _T_TRB_FINETIME_H
#define _T_TRB_FINETIME_H
// +++ include header files +++
// standard C++ headers
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
// ROOT headers
#include "TBranch.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TObject.h"
#include "TTree.h"
// my own headers
#include "myUtilities.h"
//#include "TTrbDataTree.h"
//#include "TTrbEventData.h"
//#include "TTrbHit.h"
//#include "TrbStructs.h"

// +++ define constants +++
#define STATIC_LOWER_LIMIT 31 // lower limit needed for static calibration
#define STATIC_UPPER_LIMIT 500 // upper limit needed for static calibration
#define FINE_TIME_BINS 601 // number of bins in fine time histogram
#define MIN_STATS 10000 // minimum number of entries in fine time histogram for a valid calibration (this is a guide only)
#define CLOCK_CYCLE_LENGTH 5.0 // clock cycle length in ns
#define MIN_HISTOGRAM_WIDTH 0 // minimum width of histogram to do Calibration0

// +++ class definition +++
class TTrbFineTime : public TObject{
private:
	void AnalyseHistogram(); // extract information from fine time histogram needed during calibration
	void CalibrationMode0(); // compute calibration table using simple method
	void CalibrationMode1(); // compute calibration table using advanced method
	void CalibrationMode2(); // compute calibration table using static limits
	void FillBinWidthGraph(); // fill fine time bin width graph
	void FillCalibrationGraph(); // fill calibration table graph
	void Init(); // initialise object
	void InitHistogram(); // initialise fine time histogram
	void SetChannelName(Bool_t bSetRandom); // set channel name based on TDC address and TDC channel ID
	void SetHistogramNames(); // set histogram names
protected:
	Bool_t bVerboseMode; // flag controlling verbose mode
	Bool_t bCalibrationIsValid; // flag indicating validity of calibration for this channel
	Bool_t bTableIsComputed; // flag indicating if calibration table has been computed
	Double_t fBinThreshold; // threshold used in fine time histogram analysis
	Double_t fClockCycle; // clock cycle length in ns
	Double_t fMinWidth; // threshold for fine time distribution width
	Int_t nCalibrationType; // select type of calibration: (0) standard, (1) precision
	Int_t nLowerEdgeBin; // lowest bin in fine time histogram
	Int_t nUpperEdgeBin; // largest bin in fine time histogram
	Int_t nMinEntries; // minimum number of entries to compute a valid calibration table
	Int_t nTdcAddress; // unique TDC address (hexadecimal)
	Int_t nTdcChannel; // TDC channel ID (range is 0->33)
	stringstream cChannelName; // name of readout channel
	TH1D hFineTimeDistribution; // histogram containing fine time data
	TH1D hBinWidth; // histogram containing distribution of fine time bin widths
	TGraph grCalibrationTable; // graph showing the calibration correlation
	TGraph grFineTimeBinWidth; // fine time bin width as a function of fine time bin index
	std::map< UInt_t,Double_t > CalibrationTable; // map containing calibration constants (value) for each fine time bin (index)
	std::map< UInt_t,Double_t > BinWidthTable; // map containing computed fine time bin width (value) for each fine time bin (index)
public:
	TTrbFineTime(); // standard constructor
	TTrbFineTime(const TTrbFineTime &a); // copy constructor
	~TTrbFineTime(); // standard destructor
	void ComputeCalibrationTable(); // compute calibration constants
	void FillHistogram(Int_t nUserFineTime) { hFineTimeDistribution.Fill((Double_t)nUserFineTime); }; // fill fine time histogram
	Double_t GetCalibratedTime(UInt_t nUserFineTime) const; // return calibrated fine time
	std::pair< UInt_t,UInt_t > GetChannelAddress() const { return(make_pair((UInt_t)nTdcAddress,(UInt_t)nTdcChannel)); }; // get address of channel, first TDC address and second TDC channel
	Int_t GetEntries() const { return((Int_t)hFineTimeDistribution.GetEntries()); }; // get number of entries in fine time histogram
	TH1D& GetHistogram() { return(hFineTimeDistribution); }; // return reference to fine time histogram
	Int_t GetLength() const { return((Int_t)CalibrationTable.size()); }; // get length of calibration table
	Int_t GetTdcAddress() const { return(nTdcAddress); }; // return TDC address
	Int_t GetTdcChannel() const { return(nTdcChannel); }; // return TDC channel ID
	Bool_t IsCalibrated() const { return(bCalibrationIsValid); }; // return if calibration is valid
	Bool_t IsComputed() const { return(bTableIsComputed); }; // return if calibration look-up table has been computed
	void PrintSettings() const; // print calibration settings on screen
	void PrintStatus() const; // print status of channel calibration
	void SetCalibrationMethod(Int_t nUserMethod) { nCalibrationType=nUserMethod; }; // set calibration method
	// 0-> simple method based on width of fine time distribution only
	// 1-> advanced calibration taking bin width variations into account
	void SetChannelAddress(std::pair< UInt_t, UInt_t > UserAddress); // set TDC channel address
	void SetChannelAddress( UInt_t nUserTdcAddress, UInt_t nUserTdcChannel); // set TDC channel address
	void SetCycleLength(Double_t fUserCycleLength) { fClockCycle = fabs(fUserCycleLength); }; // set clock cycle length in ns
	void SetStatsLimit(Int_t nUserStatLimit) { nMinEntries=nUserStatLimit; }; // set minimum number of events in fine time histogram required to attempt calibration
	void SetVerboseMode(Bool_t bUserVerboseMode) { bVerboseMode=bUserVerboseMode; }; // set verbose mode
	void WriteHistograms() const; // write histograms to current directory
	ClassDef(TTrbFineTime,1);
};

#endif
