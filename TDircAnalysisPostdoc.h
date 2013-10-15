#ifndef _T_DIRC_ANALYSIS_POSTDOC_H
#define _T_DIRC_ANALYSIS_POSTDOC_H
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

// +++ class definition +++
class TDircAnalysisPostdoc : public TDircAnalysisBase {
private:
	Double_t fTimeBinningCoarse; // coarse bin width for timing histograms
	Double_t fTimeBinningFine; // fine bin width for timing histograms
	std::bitset<10> DataQualityStatus; // 10 bits to describe data quality status
	std::ofstream LogFileBuffer;
	time_t RawTime; // time structure
	void DoDataQualityCheck(); // check event data quality
	void Init();
protected:
	enum DQCuts {NO_CUTS,DECODE_ERR,RNDM_BIT_ERR,SYNC_ERR,NO_HITS_ERR,NO_TRIG_ERR,NO_MATCH_ERR}; //empty TDC cut,missing reference signal cut,laser trig missing
	void FillDQHistogram(TH1D& hDataQuality);
	void WriteLogfileHeader(); // write status information to log file
public:
	TDircAnalysisPostdoc(TChain &UserChain);
	TDircAnalysisPostdoc(string cUserDataFilename, string UserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth); // standard constructor
	TDircAnalysisPostdoc(string cUserDataFilename, string UserTdcAddressFile); // constructor, all TDC defintions in text file
	virtual ~TDircAnalysisPostdoc(); // standard destructor
	void Analyse(string cUserAnalysisFilename); // analysis routine goes here, this method is needed!
	void AnalyseTrigger(string cUserAnalysisFilename); // analysis of trigger signal only
	void PrintDataQualityStatus() const { cout << DataQualityStatus << endl; };
	/* some magic ROOT stuff... */
	ClassDef(TDircAnalysisPostdoc,1);
};

#endif