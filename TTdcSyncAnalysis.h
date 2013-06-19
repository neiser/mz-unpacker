#ifndef _T_TDC_SYNC_ANALYSIS_H
#define _T_TDC_SYNC_ANALYSIS_H
// +++ include header files +++
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
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
// need a structure to keep histograms for each combination
struct SyncHistograms{
	// members of this struct
	string cBasename; // base name of TDC pair, can be used to name the histograms
	TH1D* hTimeDiff; // difference of the calibrated sync timestamps
	TH1D* hCoarseTimeDiff; // difference of the coarse counter sync timestamps
	TH1D* hFineTimeDiff; // difference of the fine time sync timestamps
	TH2D* hTimeCorrelation; // 2D plot of the sync timestamp correlation
	TH2D* hFineTimeCorrelation; // 2D plot of the fine time sync timestamps
	TH2D* hCoarseTimeCorrelation; // 2D plot of the coarse counter sync timestamps
	// methods of this struct
	void WritePlots() const {
		if(hTimeDiff!=NULL)
			hTimeDiff->Write();
		if(hCoarseTimeDiff!=NULL)
			hCoarseTimeDiff->Write();
		if(hFineTimeDiff!=NULL)
			hFineTimeDiff->Write();
		if(hTimeCorrelation!=NULL)
			hTimeCorrelation->Write();
		if(hFineTimeCorrelation!=NULL)
			hFineTimeCorrelation->Write();
		if(hCoarseTimeCorrelation!=NULL)
			hCoarseTimeCorrelation->Write();
	}

	void ClearPlots(){
		if(hTimeDiff!=NULL){
			delete hTimeDiff;
			hTimeDiff = NULL;
		}
		if(hCoarseTimeDiff!=NULL){
			delete hCoarseTimeDiff;
			hCoarseTimeDiff = NULL;
		}
		if(hFineTimeDiff!=NULL){
			delete hFineTimeDiff;
			hFineTimeDiff = NULL;
		}
		if(hTimeCorrelation!=NULL){
			delete hTimeCorrelation;
			hTimeCorrelation = NULL;
		}
		if(hFineTimeCorrelation!=NULL){
			delete hFineTimeCorrelation;
			hFineTimeCorrelation = NULL;
		}
		if(hCoarseTimeCorrelation!=NULL){
			delete hCoarseTimeCorrelation;
			hCoarseTimeCorrelation = NULL;
		}
	}
};


class TTdcSyncAnalysis : public TTrbAnalysisBase {
private:
	Int_t nNPairs;
	Double_t fHist2dMin, fHist2dMax;
	Double_t fHistCoarseMin, fHistCoarseMax;
	std::pair< Int_t,Int_t > TimeBinning;
	std::pair< Int_t,Int_t > CoarseTimeBinning;
	std::pair< Int_t,Int_t > FineTimeBinning;
	std::map< std::pair<UInt_t,UInt_t>,SyncHistograms > TdcPairs;
	void ClearAllPlots();
	void GenerateTdcPairs();
	void Init();
	void InitHistograms();

protected:
	UInt_t GetTdcSyncCoarseTimestamp(UInt_t nTdcAddress) const;
	UInt_t GetTdcSyncFineTimestamp(UInt_t nTdcAddress) const;
	Int_t SetTdcAddresses(string cUserTdcAddressFile);
public:
	//TTdcSyncAnalysis(string cUserDataFilename, string cUserTdcAddressFile); // constructor
	TTdcSyncAnalysis(string cUserDataFilename, string cUserTdcAddressFile, UInt_t nUserTdcOffset=2, UInt_t nUserTdcWidth=32); // constructor
	virtual ~TTdcSyncAnalysis();
	void Analyse(string cUserAnalysisFilename); // user analysis code goes here...
	void PrintTdcPairs() const;
	void SetCoarseTimeBinning(Int_t nUserBinning) { CoarseTimeBinning.first=nUserBinning; CoarseTimeBinning.second=nUserBinning; };
	void SetCoarseTimeLimits(Double_t fUserMin, Double_t fUserMax) { (fUserMin<fUserMax)? fHistCoarseMin=fUserMin: fHistCoarseMin=fUserMax; (fUserMin<fUserMax)? fHistCoarseMax=fUserMax: fHistCoarseMax=fUserMin; };
	void SetFineTimeBinning(Int_t nUserBinning) { FineTimeBinning.first=nUserBinning; FineTimeBinning.second=nUserBinning;};
	void SetTimeBinning(Int_t nUserBinning) { TimeBinning.first=nUserBinning; TimeBinning.second=nUserBinning; };
	void SetTimeLimits(Double_t fUserMin, Double_t fUserMax) { (fUserMin<fUserMin)? fHist2dMin=fUserMin: fHist2dMin=fUserMax; (fUserMin<fUserMin)? fHist2dMax=fUserMax: fHist2dMax=fUserMin; };
	//void Show();
	/* some magic ROOT stuff... */
	ClassDef(TTdcSyncAnalysis,1);
};

#endif