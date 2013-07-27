#ifndef _T_TRB_EVENTDISPLAY_
#define _T_TRB_EVENTDISPLAY_
// +++ include header files +++
/* standard C++ header files */
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <cfloat>
#include <climits>
#include <limits>
#include <stdexcept>
#include <string>
#include <ctime>
#include <cctype>
#include <algorithm>
#include <cmath>
#include <map>
#include <iterator>
/* RooT header files */
#include "TArc.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH2Poly.h"
#include "TMarker.h"
#include "TMath.h"
#include "TTree.h"
#include "TVirtualFitter.h"
/* special header files */
//#include "TTrbAnalysis.h"
#include "TMapmt.cpp"
#include "myUtilities.h"
#include "TTrbDataTree.h"
#include "TTrbAnalysisBase.h"

#define VERBOSE_MODE 0 // 1 switches on verbose output (debugging purposes)

//#define MAX_READOUT_CHAN 4096

class TTrbEventDisplay : public TTrbAnalysisBase {
private:
	std::vector<TMapmt*> DetectorSetup; // vector containing MAPMT objects
	void Analyse(string cUserAnalysisFilename) {}; // empty analysis routine
	void DeleteSetup();
	void Init(); // initialise parameters
	void InitHistograms(); // initialise 2D histograms
	void LoadSetup(string cUserSetupFilename);
protected:
	// flags for required data
	Bool_t bSetupIsValid; // indicate if MAPMT setup was created and is valid (only basic checks)
	Bool_t bDataIsValid; // indicate if data is available
	// flags organising display
	Bool_t bPixelCentreMapsAreFilled; // indicate if pixel centre position maps are filled
	Bool_t bPixelMapIsFilled; // indicate if pixel ID maps are filled
	Bool_t bReadoutMapIsFilled; // indicate if readout channel maps are filled
	// flags for data treatment
	Bool_t bSkipMultiHits; // if true skip multiple hits in a channel
	Bool_t bUseTimeWindow; // true if a time window has been set
	// variables for 2D histograms
	Int_t nNumberOfBins;
	Int_t nNumberOfEvents;
	Int_t nNumberOfPixels;
	Int_t nNumberOfPmts;
	stringstream cEventDisplayTitle;
	string cTreeName;
	std::pair< Double_t,Double_t > TimingWindow;
	
	TMarker mSetupCentre;
	// Canvas definitions for display
	TCanvas *canActiveCanvas;
	TCanvas *canEventDisplay;
	TCanvas *canPixelCentreMaps;
	TCanvas *canPixelMap;
	TCanvas *canReadoutMap;
	TCanvas *canThresholdMap;
	// 2D histograms for event display
	TH2Poly hEventMap; // 2D histogram for plotting hit distribution in an event
	TH2Poly hPixelIndexMap; // 2D histogram for plotting MAPMT pixel indices for given geometry
	TH2Poly hChannelMap; // 2D histogram for plotting readout channel mapping for given geometry
	TH2Poly hThresholdMap; // 2D histogram for plotting pixel thresholds for given geometry
	TH2Poly hPixelCentreXMap; // 2D histogram for plotting horizontal pixel centre coordinates for given geometry
	TH2Poly hPixelCentreYMap; // 2D histogram for plotting vertical pixel centre coordinates for given geometry
	void DefineDisplayBins();
	Bool_t FillPixelCentreMaps(); // fill 2D histogram with pixel centre coordinates
	Bool_t FillPixelMap(); // fill 2D histogram with pixel number
	Bool_t FillReadoutMap(); // fill 2D histogram with readout channel number
	Int_t HitMatching(); // match leading and trailing edge timestamps, returning the number of channels with multiple hits
	std::map<Int_t,Double_t> LETimestamps; // map with leading edge timestamps
	void ScanEvent() { TTrbAnalysisBase::ScanEvent(); HitMatching(); }; // analyse a single event and extract hit information
	void SetCanvasStyle(TCanvas *canThisCanvas);
public:
	TTrbEventDisplay(string cUserSetupFilename, string cUserThresholdFilename, string cUserDataFilename); // standard constructor
	virtual ~TTrbEventDisplay(); // standard destructor
	Int_t GetNPixels() const { return (nNumberOfPixels); }; // get number of pixels in setup
	Int_t GetNPmts() const { return (nNumberOfPmts); }; // get number of MAPMTs in setup
	void KeepMultiHits() { bSkipMultiHits=kFALSE; }; // keep multi-hit pixels, requires a timing window
	void PrintLETimestamps() const; // print list of synchronised leading edge timestamps of hits to screen
	void ResetTimingWindow() { TimingWindow.first=0.0; TimingWindow.second=0.0; bUseTimeWindow=kFALSE; }; // reset timing window
	void SetTimingWindow( Double_t fUserLow, Double_t fUserUpper); // set timing window for event hit selection
	void Show(Int_t nUserEventId=0); // show single event
	void Show(UInt_t nUserStart, UInt_t nUserStop); // accumulate events and show hit pattern
	void ShowPixelCentreMaps(); // show 2D histogram with pixel centre coordinates
	void ShowPixelMap(); // show 2D histogram with pixel numbers (MAPMT channel)
	void ShowReadoutMap(); // show 2D histogram with readout channel numbers
	void SkipMultiHits() { bSkipMultiHits=kTRUE; }; // skip multi-hit pixels
	/* some magic ROOT stuff... */
	ClassDef(TTrbEventDisplay,1);

};

#endif