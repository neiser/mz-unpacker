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
//#include "TMarocRawData.h"
#include "TTrbAnalysis.h"
#include "TMapmt.cpp"

#define VERBOSE_MODE 0 // 1 switches on verbose output (debugging purposes)

#define MAX_READOUT_CHAN 4096

// use text files to describe setup

// function needed for ring fit

std::vector<MAPMT_PIXEL> CherenkovRing;

void myfcn(Int_t &, Double_t *, Double_t &f, Double_t *par, Int_t) {
    // minimisation function computing the sum of squares of residuals
    f = 0;
    for(std::vector<MAPMT_PIXEL>::const_iterator CurrentPixel=CherenkovRing.begin(); CurrentPixel!=CherenkovRing.end(); CurrentPixel++){
		Double_t u = CurrentPixel->fX - par[0];
		Double_t v = CurrentPixel->fY - par[1];
		Double_t dr = par[2] - sqrt(u*u+v*v);
		f += dr*dr;
	}
}

class TTrbEventDisplay{
private:
	std::vector<TMapmt*> DetectorSetup;
	//TMarocRawData *Events;
	TTrbAnalysis *Events;

	// flags for required data
	Bool_t bSetupIsValid;
	Bool_t bDataIsValid;

	// options flags
	Bool_t bUseThresholdFile; // flag indicating use of user supplied threshold file
	Bool_t bDoRingFit;

	// flags organising display
	Bool_t bPixelCentreMapsAreFilled;
	Bool_t bPixelMapIsFilled;
	Bool_t bChannelMapIsFilled;
	Bool_t bReadoutMapIsFilled;
	Bool_t bThresholdMapIsFilled;
	
	Int_t nNumberOfBins;
	Int_t nNumberOfEvents;
	Int_t nNumberOfPixels;
	Int_t nNumberOfPmts;
	stringstream cEventDisplayTitle;
	string cTreeName;
	
	TMarker mSetupCentre;

	TCanvas *canActiveCanvas;
	TCanvas *canEventDisplay;
	TCanvas *canPixelCentreMaps;
	TCanvas *canPixelMap;
	TCanvas *canReadoutMap;
	TCanvas *canThresholdMap;

	TH2Poly hEventMap; // 2D histogram for plotting hit distribution in an event
	TH2Poly hPixelIndexMap; // 2D histogram for plotting MAPMT pixel indices for given geometry
	TH2Poly hChannelMap; // 2D histogram for plotting readout channel mapping for given geometry
	TH2Poly hThresholdMap; // 2D histogram for plotting pixel thresholds for given geometry
	TH2Poly hPixelCentreXMap; // 2D histogram for plotting horizontal pixel centre coordinates for given geometry
	TH2Poly hPixelCentreYMap; // 2D histogram for plotting vertical pixel centre coordinates for given geometry

	//Bool_t CreateDisplayCanvas(); 
	void DefineDisplayBins();
	void DeleteSetup();
	Bool_t FillPixelCentreMaps();
	Bool_t FillPixelMap();
	Bool_t FillReadoutMap();
	Bool_t FillThresholdMap();
	void FitRing();
	void Init(); // initialise parameters
	Int_t LoadSetup(string cUserFilename);
	void SetCanvasStyle(TCanvas *canThisCanvas);

public:
	TTrbEventDisplay(string cUserSetupFilename, string cUserThresholdFilename, string cUserDataFilename); // standard constructor
	virtual ~TTrbEventDisplay(); // standard destructor
	Int_t GetN() const { return(nNumberOfEvents); }
	void LoadData(string cUserDatafile);
	void LoadThresholds(string cUserThresholdFilename);
	void Show(Int_t nUserEventId=0);
	void Show(Int_t nUserStart, Int_t nUserStop);
	void ShowPixelCentreMaps();
	void ShowPixelMap();
	void ShowReadoutMap();
	void ShowThresholdMap();

	/* some magic ROOT stuff... */
	ClassDef(TTrbEventDisplay,1);
};

ClassImp(TTrbEventDisplay);


TTrbEventDisplay::TTrbEventDisplay(string cUserSetupFilename, string cUserThresholdFilename, string cUserDataFilename){
	Init();
	cout << "Loading setup..." << endl;
	nNumberOfPmts = LoadSetup(cUserSetupFilename);
	if(!bSetupIsValid)
		exit (-1);
	cout << "Loading thresholds..." << endl;
	LoadThresholds(cUserThresholdFilename);
	if(bUseThresholdFile)
		bDoRingFit = kTRUE;
	cout << "Loading data..." << endl;
	LoadData(cUserDataFilename);
	if(!bDataIsValid){
		DeleteSetup();
		exit (-1);
	}
}


TTrbEventDisplay::~TTrbEventDisplay(){ // standard destructor
	CherenkovRing.clear();
	DeleteSetup();
	delete Events;
}

void TTrbEventDisplay::DefineDisplayBins(){
	if(!bSetupIsValid)
		return;
#if VERBOSE_MODE
	cout << "Defining TH2Poly bins..." << endl;
#endif
	for(std::vector<TMapmt*>::const_iterator CurrentPmt=DetectorSetup.begin(); CurrentPmt!=DetectorSetup.end(); CurrentPmt++){ // start of loop over all MAPMTs in this setup
		Double_t fRotationAngle = (*CurrentPmt)->GetRotationAngle();
		Double_t fPixelPitchX, fPixelPitchY;
		(*CurrentPmt)->GetPixelPitch(fPixelPitchX,fPixelPitchY);
		std::vector<MAPMT_PIXEL> ListOfPixels = (*CurrentPmt)->GetPixels();
		nNumberOfBins += (Int_t)ListOfPixels.size();
		for(std::vector<MAPMT_PIXEL>::const_iterator CurrentPixel=ListOfPixels.begin(); CurrentPixel!=ListOfPixels.end(); CurrentPixel++){ // start of loop over all pixels
			std::vector<Double_t> PixelCornerX;
			std::vector<Double_t> PixelCornerY;
			PixelCornerX.reserve(4);
			PixelCornerY.reserve(4);
			PixelCornerX.push_back(CurrentPixel->fX+(0.5*fPixelPitchX * cos(-fRotationAngle) + 0.5*fPixelPitchY * sin(-fRotationAngle)));
			PixelCornerY.push_back(CurrentPixel->fY+(-0.5*fPixelPitchY * cos(-fRotationAngle) + 0.5*fPixelPitchX * sin(-fRotationAngle)));
			PixelCornerX.push_back(CurrentPixel->fX+(0.5*fPixelPitchX * cos(-fRotationAngle) - 0.5*fPixelPitchY * sin(-fRotationAngle)));
			PixelCornerY.push_back(CurrentPixel->fY+(0.5*fPixelPitchY * cos(-fRotationAngle) + 0.5*fPixelPitchX * sin(-fRotationAngle)));
			PixelCornerX.push_back(CurrentPixel->fX+(-0.5*fPixelPitchX * cos(-fRotationAngle) - 0.5*fPixelPitchY * sin(-fRotationAngle)));
			PixelCornerY.push_back(CurrentPixel->fY+(0.5*fPixelPitchY * cos(-fRotationAngle) - 0.5*fPixelPitchX * sin(-fRotationAngle)));
			PixelCornerX.push_back(CurrentPixel->fX+(-0.5*fPixelPitchX * cos(-fRotationAngle) + 0.5*fPixelPitchY * sin(-fRotationAngle)));
			PixelCornerY.push_back(CurrentPixel->fY+(-0.5*fPixelPitchY * cos(-fRotationAngle) - 0.5*fPixelPitchX * sin(-fRotationAngle)));
			hEventMap.AddBin(4,&PixelCornerX[0],&PixelCornerY[0]);
			hPixelIndexMap.AddBin(4,&PixelCornerX[0],&PixelCornerY[0]);
			hChannelMap.AddBin(4,&PixelCornerX[0],&PixelCornerY[0]);
			hThresholdMap.AddBin(4,&PixelCornerX[0],&PixelCornerY[0]);
			hPixelCentreXMap.AddBin(4,&PixelCornerX[0],&PixelCornerY[0]);
			hPixelCentreYMap.AddBin(4,&PixelCornerX[0],&PixelCornerY[0]);
		} // end of loop over all pixels
	} // end of loop over all MAPMTs in this setup
}

void TTrbEventDisplay::DeleteSetup(){
#if VERBOSE_MODE
	cout << "Deleting MAPMT objects... " << DetectorSetup.size() << endl;
#endif
	Int_t i=0;
	for(std::vector<TMapmt*>::iterator CurrentPmt=DetectorSetup.begin(); CurrentPmt!=DetectorSetup.end(); CurrentPmt++){
#if VERBOSE_MODE
		cout << ++i << endl;
#endif
		delete *CurrentPmt;
		*CurrentPmt = NULL;
	}
#if VERBOSE_MODE
	cout << "Clearing vector... " << DetectorSetup.size() << endl;
#endif
}

Bool_t TTrbEventDisplay::FillPixelCentreMaps(){
	if(bPixelCentreMapsAreFilled)
		return (bPixelCentreMapsAreFilled);
#if VERBOSE_MODE
	cout << "Fill Pixel Centre Maps..." << endl;
#endif
	for(std::vector<TMapmt*>::const_iterator CurrentPmt=DetectorSetup.begin(); CurrentPmt!=DetectorSetup.end(); CurrentPmt++){ // start of loop over all MAPMTs in this setup
		std::vector<MAPMT_PIXEL> ListOfPixels = (*CurrentPmt)->GetPixels();
		for(std::vector<MAPMT_PIXEL>::const_iterator CurrentPixel=ListOfPixels.begin(); CurrentPixel!=ListOfPixels.end(); CurrentPixel++){
			hPixelCentreXMap.Fill(CurrentPixel->fX,CurrentPixel->fY,CurrentPixel->fX);
			hPixelCentreYMap.Fill(CurrentPixel->fX,CurrentPixel->fY,CurrentPixel->fY);
		}
	}
	bPixelCentreMapsAreFilled = kTRUE;
	return (bPixelCentreMapsAreFilled);
}

Bool_t TTrbEventDisplay::FillPixelMap(){
	if(bPixelMapIsFilled)
		return (bPixelMapIsFilled);
#if VERBOSE_MODE
	cout << "Fill Pixel ID Map..." << endl;
#endif
	for(std::vector<TMapmt*>::const_iterator CurrentPmt=DetectorSetup.begin(); CurrentPmt!=DetectorSetup.end(); CurrentPmt++){ // start of loop over all MAPMTs in this setup
		std::vector<MAPMT_PIXEL> ListOfPixels = (*CurrentPmt)->GetPixels();
		for(std::vector<MAPMT_PIXEL>::const_iterator CurrentPixel=ListOfPixels.begin(); CurrentPixel!=ListOfPixels.end(); CurrentPixel++){
			hPixelIndexMap.Fill(CurrentPixel->fX,CurrentPixel->fY,(Double_t)CurrentPixel->nPixelId);
		}
	}
	bPixelMapIsFilled = kTRUE;
	return (bPixelMapIsFilled);
}

Bool_t TTrbEventDisplay::FillReadoutMap(){
	if(bReadoutMapIsFilled)
		return (bReadoutMapIsFilled);
#if VERBOSE_MODE
	cout << "Fill Readout Map..." << endl;
#endif
	for(std::vector<TMapmt*>::const_iterator CurrentPmt=DetectorSetup.begin(); CurrentPmt!=DetectorSetup.end(); CurrentPmt++){ // start of loop over all MAPMTs in this setup
		std::vector<MAPMT_PIXEL> ListOfPixels = (*CurrentPmt)->GetPixels();
		for(std::vector<MAPMT_PIXEL>::const_iterator CurrentPixel=ListOfPixels.begin(); CurrentPixel!=ListOfPixels.end(); CurrentPixel++){
			hChannelMap.Fill(CurrentPixel->fX,CurrentPixel->fY,(Double_t)CurrentPixel->nQdcChannelId);
		}
	}
	bReadoutMapIsFilled = kTRUE;
	return (bReadoutMapIsFilled);
}

Bool_t TTrbEventDisplay::FillThresholdMap(){
	if(!bUseThresholdFile)
		return(kFALSE);
	if(bThresholdMapIsFilled)
		return (bThresholdMapIsFilled);
#if VERBOSE_MODE
	cout << "Fill Threshold Map..." << endl;
#endif
	for(std::vector<TMapmt*>::const_iterator CurrentPmt=DetectorSetup.begin(); CurrentPmt!=DetectorSetup.end(); CurrentPmt++){ // start of loop over all MAPMTs in this setup
		std::vector<MAPMT_PIXEL> ListOfPixels = (*CurrentPmt)->GetPixels();
		for(std::vector<MAPMT_PIXEL>::const_iterator CurrentPixel=ListOfPixels.begin(); CurrentPixel!=ListOfPixels.end(); CurrentPixel++){
			hThresholdMap.Fill(CurrentPixel->fX,CurrentPixel->fY,CurrentPixel->fThreshold);
		}
	}
	bThresholdMapIsFilled = kTRUE;
	return (bThresholdMapIsFilled);
}

void TTrbEventDisplay::FitRing(){
	if(!bDoRingFit)
		return;
	if(CherenkovRing.size()<3)
		return;
	// define fit algorithm and starting values
	TVirtualFitter::SetDefaultFitter("Minuit");  //default is Minuit
	TVirtualFitter *fitter = TVirtualFitter::Fitter(0, 3);
	fitter->SetFCN(myfcn);
	fitter->SetParameter(0, "x0",   0.0, 0.1, 0,0);
	fitter->SetParameter(1, "y0",   0.0, 0.1, 0,0);
	fitter->SetParameter(2, "R",    120.0, 0.1, 0,0);
	// switch off verbose mode
	Double_t arglist[1] = {0};
	arglist[0] = -1;
	fitter->ExecuteCommand("SET PRINT",arglist,1);
	arglist[0] = 0;
	fitter->ExecuteCommand("SET NOW",arglist,0);
	arglist[0] = 0;
	fitter->ExecuteCommand("MIGRAD", arglist, 0);
	//Draw the circle on top of the points
	canActiveCanvas->cd();
	TArc arc(fitter->GetParameter(0),fitter->GetParameter(1),fitter->GetParameter(2));
	arc.SetLineColor(kBlack); arc.SetLineWidth(4); arc.SetLineStyle(9); arc.SetFillStyle(0);
	arc.DrawClone("SAME");
	TMarker mRingCentre(fitter->GetParameter(0),fitter->GetParameter(1),29);
	mRingCentre.DrawClone("SAME");
	cout << "++++++++++++++++++++++++" << endl;
	cout << "+++ Ring Fit Results +++" << endl;
	cout << "++++++++++++++++++++++++" << endl;
	cout << "\n";
	cout << "X_0:\t" << fitter->GetParameter(0) << " +- " << fitter->GetParError(0) << endl;
	cout << "\n";
	cout << "Y_0:\t" << fitter->GetParameter(1) << " +- " << fitter->GetParError(1) << endl;
	cout << "\n";
	cout << "R:\t" << fitter->GetParameter(2) << " +- " << fitter->GetParError(2) << endl;
	cout << "\n";
}

void TTrbEventDisplay::Init(){
	// initialise class variables
	nNumberOfBins	= 0;
	nNumberOfEvents = 0;
	nNumberOfPixels	= 0;
	nNumberOfPmts	= 0;

	//cTreeName = "fTdata";
	cTreeName = "T";
	cEventDisplayTitle.str("DIRC@Mainz Event Display");
	// intialise display canvases
	canActiveCanvas		= NULL;
	canPixelMap			= NULL;
	canThresholdMap		= NULL;
	canPixelCentreMaps	= NULL;
	canReadoutMap		= NULL;
	canEventDisplay		= NULL;

	Events = NULL;

	// initialise required data flags
	bSetupIsValid	= kFALSE;
	bDataIsValid	= kFALSE;
	// initialise optional flags
	bUseThresholdFile	= kFALSE;
	bDoRingFit			= kFALSE;
	// intialise display flags
	bChannelMapIsFilled			= kFALSE;
	bPixelMapIsFilled			= kFALSE;
	bReadoutMapIsFilled			= kFALSE;
	bThresholdMapIsFilled		= kFALSE;
	bPixelCentreMapsAreFilled	= kFALSE;

	mSetupCentre.SetX(0.0);
	mSetupCentre.SetY(0.0);
	mSetupCentre.SetMarkerStyle(28);

	hEventMap.AddDirectory(0); // do not add histogram to current directory
	hEventMap.SetStats(kFALSE);
	hEventMap.SetName("hEventMap"); hEventMap.SetTitle("DIRC@Mainz Event Map; horizontal coordinate; vertical coordinate");
	hPixelIndexMap.AddDirectory(0); // do not add histogram to current directory
	hPixelIndexMap.SetStats(kFALSE);
	hPixelIndexMap.SetName("hPixelIndexMap"); hPixelIndexMap.SetTitle("DIRC@Mainz Pixel Index Map; horizontal coordinate; vertical coordinate");
	hChannelMap.AddDirectory(0); // do not add histogram to current directory
	hChannelMap.SetStats(kFALSE);
	hChannelMap.SetName("hChannelMap"); hChannelMap.SetTitle("DIRC@Mainz Readout Channel Index Map; horizontal coordinate; vertical coordinate");
	hThresholdMap.AddDirectory(0); // do not add histogram to current directory
	hThresholdMap.SetStats(kFALSE);
	hThresholdMap.SetName("hThresholdMap"); hThresholdMap.SetTitle("DIRC@Mainz Threshold Map; horizontal coordinate; vertical coordinate");
	hPixelCentreXMap.AddDirectory(0); // do not add histogram to current directory
	hPixelCentreXMap.SetStats(kFALSE);
	hPixelCentreXMap.SetName("hPixelCentreXMap"); hPixelCentreXMap.SetTitle("DIRC@Mainz Pixel Centre X Map; horizontal coordinate; vertical coordinate");
	hPixelCentreYMap.AddDirectory(0); // do not add histogram to current directory
	hPixelCentreYMap.SetStats(kFALSE);
	hPixelCentreYMap.SetName("hPixelCentreYMap"); hPixelCentreYMap.SetTitle("DIRC@Mainz Pixel Centre Y Map; horizontal coordinate; vertical coordinate");
}

void TTrbEventDisplay::LoadData(string cUserDatafile){
	if(cUserDatafile.empty())
		return;
	Events = new TTrbAnalysis(cUserDatafile,"c030|c050|c090|c080");
	cout << Events->GetNEvents() << "\t" << Events->GetNTrb() << endl;
	bDataIsValid = kTRUE;
}

Int_t TTrbEventDisplay::LoadSetup(string cUserFilename){
	if(cUserFilename.empty())
		return (-1);
	Int_t nPmtsAdded = 0; // number of MAPMTs successfully added
	Int_t nLineIndex = 0;
	ifstream UserSetupFile(cUserFilename.c_str(),ifstream::in);
	while(UserSetupFile.good()){ // start loop over input file
		string cCurrentLine;
		getline(UserSetupFile,cCurrentLine); // get line from input file
		nLineIndex++;
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		if(cCurrentLine.find("#")==0){
#if VERBOSE_MODE
			cout << "Ignore comments in line " << nLineIndex << endl;
#endif
			continue;
		}
		string cBuffer;
		vector<string> tokens;
		stringstream cParsingLine(cCurrentLine);
		while(!cParsingLine.eof()){ // parse string containing line from input file
			cParsingLine >> cBuffer;
			if(!cBuffer.empty())
				tokens.push_back(cBuffer);
			cBuffer.clear();
		}
		TMapmt *CurrentMapmt = NULL;
		switch (tokens.size()) {
			case 9:
				CurrentMapmt = new TMapmt(atoi(tokens.at(1).c_str()),atoi(tokens.at(2).c_str()));
				CurrentMapmt->SetName(tokens.at(0));
				CurrentMapmt->SetPixelPitch(atof(tokens.at(3).c_str()),atof(tokens.at(4).c_str()));
				CurrentMapmt->SetAbsolutePosition(atof(tokens.at(6).c_str()),atof(tokens.at(7).c_str()));
				CurrentMapmt->SetRotationAngle(atof(tokens.at(8).c_str())*TMath::DegToRad());
				if(CurrentMapmt->AddPixels(tokens.at(5))>0){
					nNumberOfPixels += CurrentMapmt->GetN();
					DetectorSetup.push_back(CurrentMapmt);
					nPmtsAdded++;
				}
				else
					delete CurrentMapmt;
				break;
			default:
				cout << "Skipping incomplete MCP-PMT definition on line " << nLineIndex << endl;
				cout << "!!!\t" << cCurrentLine << "\t!!!" << endl;
				break; // do nothing
		}
	} // end loop over input file
	UserSetupFile.close();
	if(nPmtsAdded>0 && nNumberOfPixels>0){
		bSetupIsValid = kTRUE;
		DefineDisplayBins();
	}
	return (nPmtsAdded);
}

void TTrbEventDisplay::LoadThresholds(string cUserThresholdFilename){
	if(!bSetupIsValid)
		return;
	if(cUserThresholdFilename.empty())
		return;
	ifstream UserThresholdFile(cUserThresholdFilename.c_str(),ifstream::in);
	Int_t nLineIndex = 0;
	std::vector<Double_t> fUserThresholds;
	fUserThresholds.reserve(MAX_READOUT_CHAN);
	while(UserThresholdFile.good()){ // start of loop over input file
		string cCurrentLine;
		getline(UserThresholdFile,cCurrentLine); // get line from input file
		nLineIndex++;
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		if(cCurrentLine.find("#")==0){
#if VERBOSE_MODE
			cout << "Ignore comments in line " << nLineIndex << endl;
#endif
			continue;
		}
		string cBuffer;
		vector<string> tokens;
		stringstream cParsingLine(cCurrentLine);
		while(!cParsingLine.eof()){ // parse string containing line from input file
			cParsingLine >> cBuffer;
			if(!cBuffer.empty())
				tokens.push_back(cBuffer);
			cBuffer.clear();
		}
#if VERBOSE_MODE
		cout << tokens.size() << endl;
#endif
		switch (tokens.size()) {
			case 1: // just pedestal values
				fUserThresholds.push_back(atof(tokens.at(0).c_str()));
				break;
			case 7: // output from Marco Mirazita's pedestal macro (0->channel index, 1->pedestal mean, 2->pedestal sigma, 3->3sigma cut) 
			case 9: // output from Marco Mirazita's pedestal macro (0->channel index, 1->pedestal mean, 2->pedestal sigma, 3->3sigma cut) 
				fUserThresholds.push_back(atof(tokens.at(3).c_str()));
				break;
		}
	} // end of loop over input file
	UserThresholdFile.close();
	cout << "Threshold vector size is " << fUserThresholds.size() << endl;
	if(fUserThresholds.size()!=MAX_READOUT_CHAN)
		return;
	bUseThresholdFile = kTRUE;
	for(std::vector<TMapmt*>::iterator CurrentPmt=DetectorSetup.begin(); CurrentPmt!=DetectorSetup.end(); CurrentPmt++){ // start of loop over all MAPMTs in this setup
		(*CurrentPmt)->SetThresholds(&fUserThresholds);
	} // end of loop over all MAPMTs in this setup
}

void TTrbEventDisplay::SetCanvasStyle(TCanvas *canThisCanvas){
	if(canThisCanvas==NULL)
		return;
	canThisCanvas->SetTicks(); // set tick marks on opposite side of axis (both X and Y)
	canThisCanvas->SetGrid(); // set grid lines for canvas
}

void TTrbEventDisplay::Show(Int_t nUserEventId){
	hEventMap.ClearBinContents();
	if(Events->GetEntry(nUserEventId)<=0)
		return;
	std::map<Int_t,Double_t>* EventData = Events->GetLeadingEdge();
	cout << EventData->size() << endl;
	for(std::vector<TMapmt*>::const_iterator CurrentPmt=DetectorSetup.begin(); CurrentPmt!=DetectorSetup.end(); CurrentPmt++){ // start of loop over all MAPMTs in this setup
		(*CurrentPmt)->ReadSparseData(*EventData);
		vector<MAPMT_PIXEL> Hits = (*CurrentPmt)->GetHitPixels();
		for(std::vector<MAPMT_PIXEL>::const_iterator CurrentPixel=Hits.begin(); CurrentPixel!=Hits.end(); CurrentPixel++){
			hEventMap.Fill(CurrentPixel->fX,CurrentPixel->fY,CurrentPixel->fAmplitude);
		}
	} // end of loop over all MAPMTs in this setup
	if(canEventDisplay==NULL){
		canEventDisplay = new TCanvas("canEventDisplay",cEventDisplayTitle.str().c_str(),-700,700);
		SetCanvasStyle(canEventDisplay);
	}
	//	delete canEventDisplay;
	if(canActiveCanvas!=canEventDisplay)
		canEventDisplay->cd();
	canActiveCanvas = canEventDisplay;
	cEventDisplayTitle.str("");
	cEventDisplayTitle << "Event " << nUserEventId;
	hEventMap.SetTitle(cEventDisplayTitle.str().c_str());
	hEventMap.DrawCopy("COL");
	mSetupCentre.DrawClone("SAME");
	//FitRing();
}

void TTrbEventDisplay::Show(Int_t nUserStart, Int_t nUserStop){
	hEventMap.ClearBinContents();
	for(Int_t nEventIndex=nUserStart; nEventIndex<nUserStop; nEventIndex++){
		if(Events->GetEntry(nEventIndex)<=0)
			continue;
		std::map<Int_t,Double_t>* EventData = Events->GetLeadingEdge();
		for(std::vector<TMapmt*>::const_iterator CurrentPmt=DetectorSetup.begin(); CurrentPmt!=DetectorSetup.end(); CurrentPmt++){ // start of loop over all MAPMTs in this setup
			(*CurrentPmt)->ReadSparseData(*EventData);
			vector<MAPMT_PIXEL> Hits = (*CurrentPmt)->GetHitPixels();
			for(std::vector<MAPMT_PIXEL>::const_iterator CurrentPixel=Hits.begin(); CurrentPixel!=Hits.end(); CurrentPixel++){
				hEventMap.Fill(CurrentPixel->fX,CurrentPixel->fY,CurrentPixel->fAmplitude);
			}
		} // end of loop over all MAPMTs in this setup
	}
	if(canEventDisplay==NULL){
		canEventDisplay = new TCanvas("canEventDisplay",cEventDisplayTitle.str().c_str(),-700,700);
		SetCanvasStyle(canEventDisplay);
	}
	//	delete canEventDisplay;
	if(canActiveCanvas!=canEventDisplay)
		canEventDisplay->cd();
	canActiveCanvas = canEventDisplay;
	cEventDisplayTitle.str("");
	cEventDisplayTitle << "Events " << nUserStart << " - " << nUserStop;
	hEventMap.SetTitle(cEventDisplayTitle.str().c_str());
	hEventMap.DrawCopy("COL");
	mSetupCentre.DrawClone("SAME");
	//FitRing();
}

void TTrbEventDisplay::ShowPixelCentreMaps(){
	if(!FillPixelCentreMaps())
		return;
	canPixelCentreMaps = new TCanvas("canPixelCentreMaps","MCP-PMT Pixel Centre Maps",-500,1000);
	SetCanvasStyle(canPixelCentreMaps);
	canActiveCanvas = canPixelCentreMaps;
	canPixelCentreMaps->Divide(1,2);
	canPixelCentreMaps->cd(1);
	hPixelCentreXMap.DrawCopy("COL");
	mSetupCentre.DrawClone("SAME");
	canPixelCentreMaps->cd(2);
	hPixelCentreYMap.DrawCopy("COL");
	mSetupCentre.DrawClone("SAME");
}

void TTrbEventDisplay::ShowPixelMap(){
	if(!FillPixelMap())
		return;
	canPixelMap = new TCanvas("canPixelMap","MCP-PMT Pixel Map",-500,500);
	SetCanvasStyle(canPixelMap);
	canActiveCanvas = canPixelMap;
	canPixelMap->cd();
	//canPixelMap->Clear();
	hPixelIndexMap.DrawCopy("COL");
	mSetupCentre.DrawClone("SAME");
}

void TTrbEventDisplay::ShowReadoutMap(){
	if(!FillReadoutMap())
		return;
	canReadoutMap = new TCanvas("canReadoutMap","MCP-PMT Readout Channel Map",-500,500);
	SetCanvasStyle(canReadoutMap);
	canActiveCanvas = canReadoutMap;
	canReadoutMap->cd();
	hChannelMap.DrawCopy("COL");
	mSetupCentre.DrawClone("SAME");
}

void TTrbEventDisplay::ShowThresholdMap(){
	if(!FillThresholdMap()){
		cout << "No valid threshold map available!" << endl;	
		return;
	}
	canThresholdMap = new TCanvas("canThresholdMap","MCP-PMT Threshold Map",-500,500);
	SetCanvasStyle(canThresholdMap);
	canActiveCanvas = canThresholdMap;
	canThresholdMap->cd();
	hThresholdMap.DrawCopy("COL");
	mSetupCentre.DrawClone("SAME");
}