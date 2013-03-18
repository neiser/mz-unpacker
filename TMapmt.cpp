/* standard C++ header files */
#include <algorithm>
#include <cctype>
#include <cfloat>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>


// #include <exception>
/* RooT header files */
//#include <TROOT.h>
//#include "TGraph.h"
#include "TH2D.h"
#include "TH2I.h"

#include "MapmtPixel.cpp"
#include "TPixelCluster.cpp"

using namespace std;

class TMapmt : public TObject{
private:
	Bool_t bGainIsNormalised;
	Double_t fAbsoluteGain; // absolute gain value corresponding to best pixel
	Double_t fPhi; // MAPMT rotation angle in rad
	Double_t fPixelPitchX; // horizontal MAPMT pixel pitch 
	Double_t fPixelPitchY; // vertical MAPMT pixel pitch
	Double_t fX; // absolute horizontal position of pixel centre of first pixel
	Double_t fY; // absolute vertical position of pixel centre of first pixel
	Int_t nNumberOfColumns; // number of columns of MAPMT pixel matrix
	Int_t nNumberOfPixels; // number of pixels of MAPMT
	Int_t nNumberOfRows; // number of rows of MAPMT pixel matrix
	string cMapmtName; // name of MAPMT object (used in histogram titles etc)
	string cMapmtNameChecked;
	vector<MAPMT_PIXEL> ListOfPixels; // vector containing all MAPMT_PIXELs associated with this MAPMT
	void ComputePixelPosition();
	void CreateMaps(); // create all 2D maps
	void FillMaps(); // fill all 2D maps
	void NormaliseGain(); // normalise gain parameters so that largest gain is 1
	void SortPixels(); // sort list of pixels according to pixel ID
	void UpdateGainMap();
	void UpdateThresholdMap();
	TH1D *hGainDistribution;
	TH1D *hThresholdDistribution;
	TH2D *hGainMap;
	TH2I *hPixelMap;
	TH2I *hQdcMap;
	TH2D *hThresholdMap;
	
public:
	TMapmt(Int_t nUserNumberOfColumns, Int_t nUserNumberOfRows, Double_t fUserAbsoluteGain=1.0);
	virtual ~TMapmt();

	Bool_t AddPixel(MAPMT_PIXEL UserPixel);	// add a new pixel to list of pixels
	Bool_t AddPixel(Int_t nUserPixelId, Int_t nUserQdcChannel, Double_t fUserGain=1.0, Double_t fUserThreshold=0.0); // add a new pixel to list of pixels
	Int_t AddPixels(string cUserFileName); // add new pixels to list of pixels, reading from a text file
	friend vector<TPixelCluster> FindClusters(const TMapmt&, Int_t nUserSearPattern=0); // find clusters of pixels above threshold
	Double_t GetAbsoluteGain() const { return (fAbsoluteGain); } // return absolute gain parameter for this MAPMT
	void GetAbsolutePosition(Double_t &fUserX, Double_t &fUserY) const { fUserX=fX; fUserY=fY;}
	Double_t GetAverageGain() const;
	Int_t GetCapacity() const { return (nNumberOfPixels); } // return capacity of pixel vector, i.e. number of pixels of MAPMT
	TH2D* GetEventMap(Bool_t bUserFlag=kFALSE); // return 2D map of MAPMT signals for current event data
	TH2I* GetHitMap(); // return 2D map indicating pixels above threshold
	vector<MAPMT_PIXEL> GetHitPixels() const;
	Int_t GetMaxPixelId() const { return (ListOfPixels.back().nPixelId); } // return highest pixel ID
	Int_t GetN() const { return (ListOfPixels.size()); } // return size of pixel vector, i.e. number of pixels added by user
	Bool_t GetPixel(Int_t nUserPixelId, MAPMT_PIXEL& UserPixel) const; // get data for one pixel
	Int_t GetPixelId(Int_t nUserQdcChanId) const; // return pixel ID corresponding to QDC channel ID
	Int_t GetPixelId(Int_t nUserColId, Int_t nUserRowId) const; // return pixel ID corresponding to row and column ID
	void GetPixelPitch(Double_t &fUserPixelPitchX, Double_t &fUserPixelPitchY) const { fUserPixelPitchX=fPixelPitchX; fUserPixelPitchY=fPixelPitchY; }
	vector<MAPMT_PIXEL> GetPixels() { return (ListOfPixels); } // get std::vector with all pixels
	Double_t GetRotationAngle() const { return(fPhi); } // get MAPMT rotation angle phi
	void Print(); // print pixels properties to terminal
	Int_t ReadSparseData(std::map<Int_t,Double_t>& UserSparseData); // read data from sparsified arrays
	Bool_t RemovePixel(Int_t nUserPixelId); // remove pixel from MAPMT (due to noise etc.)
	Int_t RemovePixels(const vector<Int_t>& nUserPixelIds); // remove list of pixels
	void ResetPixelAmplitudes() { for_each(ListOfPixels.begin(),ListOfPixels.end(),ResetPixelAmplitude); }
	void ResetPixelGains() { for_each(ListOfPixels.begin(),ListOfPixels.end(),ResetPixelGain); }
	void ResetPixelThresholds() { for_each(ListOfPixels.begin(),ListOfPixels.end(),ResetPixelThreshold); }
	void SetAbsoluteGain(Double_t fUserAbsoluteGain) {fAbsoluteGain = fUserAbsoluteGain;}
	void SetAbsolutePosition(Double_t fUserX, Double_t fUserY) { fX=fUserX; fY=fUserY; ComputePixelPosition(); }
	void SetName(string cUserName); // set name of this MAPMT, affects name of gain, pixel and threshold maps (avoids same-name problem in ROOT)
	Int_t SetPixelGain(const TH2D* const hUserGainMap, Bool_t bInvertMap=kFALSE); // read pixel gain from 2D histogram, invert if 2D histogram is upside down, e.g. laser scanner images
	void SetPixelPitch(Double_t fUserPixelPitchX, Double_t fUserPixelPitchY) { fPixelPitchX=fabs(fUserPixelPitchX); fPixelPitchY=fabs(fUserPixelPitchY); ComputePixelPosition(); }
	Int_t SetQdcSignal(const vector<Double_t>* const fUserQdcData); // supply event QDC data using a std::vector containing the data
	Int_t SetQdcSignal(const vector<Int_t>* const nUserQdcData); // supply event QDC data using a std::vector containing the data
	Int_t SetQdcSignal(const Double_t* const fUserQdcData, Int_t nUserArrayLength); // supply event QDC data using an array containing the data
	void SetRotationAngle(Double_t fUserPhi) { fPhi = fUserPhi; ComputePixelPosition(); }
	Int_t SetThresholds(const vector<Double_t>* const fUserThresholds, Bool_t bAdjust=kFALSE); // set pixel thresholds
	Int_t SetThresholds(const Double_t* const fUserThresholds, Int_t nUserArrayLength, Bool_t bAdjust=kFALSE);
	TH1D* ShowGainDistribution(Bool_t bUserSetShowFlag=kTRUE);
	TH2D* ShowGainMap(Bool_t bUserSetShowFlag=kTRUE);
	TH2I* ShowPixelMap(Bool_t bUserSetShowFlag=kTRUE);
	TH2I* ShowQdcMap(Bool_t bUserSetShowFlag=kTRUE);
	TH1D* ShowThresholdDistribution(Bool_t bUserSetShowFlag=kTRUE);
	TH2D* ShowThresholdMap(Bool_t bUserSetShowFlag=kTRUE);
	void WriteSetup();
	
	/* some magic ROOT stuff... */
  ClassDef(TMapmt,1);
};

ClassImp(TMapmt);

TMapmt::TMapmt(Int_t nUserNumberOfColumns, Int_t nUserNumberOfRows, Double_t fUserAbsoluteGain) : TObject(){ // constructor
	hGainDistribution		= NULL;
	hThresholdDistribution	= NULL;
	hGainMap			= NULL;
	hPixelMap			= NULL;
	hQdcMap				= NULL;
	hThresholdMap		= NULL;
	bGainIsNormalised	= kFALSE;
	fPhi		 = 0.0;
	fPixelPitchX = 1.0;
	fPixelPitchY = 1.0;
	fX = 0.0;
	fY = 0.0;
	// user input
	fAbsoluteGain		= fUserAbsoluteGain;
	nNumberOfColumns	= nUserNumberOfColumns;
	nNumberOfRows		= nUserNumberOfRows;
	nNumberOfPixels		= nNumberOfColumns * nNumberOfRows;
	if(nNumberOfPixels<1){
		MakeZombie();
		nNumberOfColumns	= 0;
		nNumberOfRows		= 0;
		nNumberOfPixels		= 0;
	}
	else{
		CreateMaps();
	}
	ListOfPixels.reserve(nNumberOfPixels);
	//cout << ListOfPixels.capacity() << endl;
}


TMapmt::~TMapmt(){
	delete hGainMap;
	delete hPixelMap;
	delete hQdcMap;
	delete hThresholdMap;
	if(hGainDistribution!=NULL)
		delete hGainDistribution;
	if(hThresholdDistribution!=NULL)
		delete hThresholdDistribution;
}

Bool_t TMapmt::AddPixel(MAPMT_PIXEL UserPixel){
	if(ListOfPixels.size() >= nNumberOfPixels) 
		return (kFALSE);
	if(UserPixel.nPixelId<1 || UserPixel.nPixelId>nNumberOfPixels)
		return (kFALSE);
	if(find_if(ListOfPixels.begin(),ListOfPixels.end(),MAPMT_PIXEL(UserPixel))!=ListOfPixels.end())
		return (kFALSE);
	UserPixel.nPixelRow = (UserPixel.nPixelId-1)/nNumberOfColumns +1;
	UserPixel.nPixelCol = UserPixel.nPixelId - nNumberOfColumns*(UserPixel.nPixelRow-1);
	UserPixel.fX = fX + (UserPixel.nPixelCol-1) * fPixelPitchX * cos(fPhi) - (UserPixel.nPixelRow-1) * fPixelPitchY * sin(fPhi);
	UserPixel.fY = fY - ((UserPixel.nPixelRow-1) * fPixelPitchY * cos(fPhi) + (UserPixel.nPixelCol-1) * fPixelPitchX * sin(fPhi));
	ListOfPixels.push_back(UserPixel);
	FillMaps();
	SortPixels();
	bGainIsNormalised = kFALSE;
	return (kTRUE);
}

Bool_t TMapmt::AddPixel(Int_t nUserPixelId, Int_t nUserQdcChannel, Double_t fUserGain, Double_t fUserThreshold){
	if(ListOfPixels.size() >= nNumberOfPixels) 
		return (kFALSE);
	MAPMT_PIXEL TempPixel(nUserPixelId, nUserQdcChannel, -1.0, fUserGain, fUserThreshold);
	return (AddPixel(TempPixel));
}

Int_t TMapmt::AddPixels(string cUserFileName){
	Int_t nPixelsAdded = 0; // number of pixels successfully added
	ifstream UserInputFile(cUserFileName.c_str(),ifstream::in);
	Int_t nLineIndex = 0; // input file line index
	while(UserInputFile.good()){ // start loop over input file
		string cCurrentLine;
		getline(UserInputFile,cCurrentLine); // get line from input file
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		nLineIndex++; // increment line index
		string cBuffer;
		vector<string> tokens;
		stringstream cParsingLine(cCurrentLine);
		while(!cParsingLine.eof()){ // parse string containing line from input file
			cParsingLine >> cBuffer;
			if(!cBuffer.empty()) // if token is not empty add to list of tokens
				tokens.push_back(cBuffer);
			cBuffer.clear();
		}
		switch (tokens.size()) {
			case 1: // only QDC channel number supplied, assume line index is pixel number
				if(AddPixel(nLineIndex,atoi(tokens.at(0).c_str())))
					nPixelsAdded++;
				break;
			case 2: // Pixel ID first, then QDC channel ID
				if(AddPixel(atoi(tokens.at(0).c_str()),atoi(tokens.at(1).c_str())))
					nPixelsAdded++;
				break;
			case 3: // Pixel ID first, then QDC channel ID, pixel gain last
				if(AddPixel(atoi(tokens.at(0).c_str()),atoi(tokens.at(1).c_str()),atof(tokens.at(2).c_str())))
					nPixelsAdded++;
				break;
			case 4: // Pixel ID first, then QDC channel ID first, pixel gain, and then pixel threshold
				if(AddPixel(atoi(tokens.at(0).c_str()),atoi(tokens.at(1).c_str()),atof(tokens.at(2).c_str()),atof(tokens.at(3).c_str())))
					nPixelsAdded++;
				break;
			default:
				continue; // do nothing
		}
	} // end loop over input file
	UserInputFile.close();
	return (nPixelsAdded);
}

void TMapmt::ComputePixelPosition(){
	if(ListOfPixels.empty())
		return;
	for(vector<MAPMT_PIXEL>::iterator CurrentPixel=ListOfPixels.begin(); CurrentPixel!=ListOfPixels.end(); CurrentPixel++){
		CurrentPixel->fX = fX + (CurrentPixel->nPixelCol-1) * fPixelPitchX * cos(fPhi) - (CurrentPixel->nPixelRow-1) * fPixelPitchY * sin(fPhi);
		CurrentPixel->fY = fY - ((CurrentPixel->nPixelRow-1) * fPixelPitchY * cos(fPhi) + (CurrentPixel->nPixelCol-1) * fPixelPitchX * sin(fPhi));
	}
}

void TMapmt::CreateMaps(){
	string cHistName;
	cHistName = (cMapmtName.empty())? "hGainMap" : "hGainMap" + cMapmtName;
	hGainMap	= new TH2D(cHistName.c_str(),"MAPMT Gain Map",nNumberOfColumns,0.5,nNumberOfColumns+0.5,nNumberOfRows,0.5,nNumberOfRows+0.5);
	cHistName.clear();
	cHistName = (cMapmtName.empty())? "hPixelMap" : "hPixelMap" + cMapmtName;
	hPixelMap	= new TH2I(cHistName.c_str(),"MAPMT Pixel Map",nNumberOfColumns,0.5,nNumberOfColumns+0.5,nNumberOfRows,0.5,nNumberOfRows+0.5);
	cHistName.clear();
	cHistName = (cMapmtName.empty())? "hQdcMap" : "hQdcMap" + cMapmtName;
	hQdcMap		= new TH2I(cHistName.c_str(),"MAPMT QDC Map",nNumberOfColumns,0.5,nNumberOfColumns+0.5,nNumberOfRows,0.5,nNumberOfRows+0.5);
	cHistName.clear();
	cHistName = (cMapmtName.empty())? "hThresholdMap" : "hThresholdMap" + cMapmtName;
	hThresholdMap = new TH2D(cHistName.c_str(),"MAPMT Threshold Map",nNumberOfColumns,0.5,nNumberOfColumns+0.5,nNumberOfRows,0.5,nNumberOfRows+0.5);
}

void TMapmt::FillMaps(){ // need to change to a loop over all pixels, but delete contents of maps first
	hGainMap->Reset();
	hPixelMap->Reset();
	hQdcMap->Reset();
	hThresholdMap->Reset();
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		hGainMap->Fill(CurrentPixel->nPixelCol,nNumberOfRows-CurrentPixel->nPixelRow+1,CurrentPixel->fGain); // change to iterator
		hPixelMap->Fill(CurrentPixel->nPixelCol,nNumberOfRows-CurrentPixel->nPixelRow+1,CurrentPixel->nPixelId);
		hQdcMap->Fill(CurrentPixel->nPixelCol,nNumberOfRows-CurrentPixel->nPixelRow+1,CurrentPixel->nQdcChannelId);
		hThresholdMap->Fill(CurrentPixel->nPixelCol,nNumberOfRows-CurrentPixel->nPixelRow+1,CurrentPixel->fThreshold);
	}
}

vector<TPixelCluster> FindClusters(const TMapmt& UserMapmt, Int_t nUserSearPattern){
	vector<MAPMT_PIXEL> PixelsAboveThreshold = UserMapmt.GetHitPixels(); // get pixels above threshold for this event
	//cout << PixelsAboveThreshold.size() << endl;
	vector<TPixelCluster> ClusterList; // vector containing cluster found
	if(!PixelsAboveThreshold.empty()){
		sort(PixelsAboveThreshold.begin(),PixelsAboveThreshold.end(),ComparePixelsByEnergy); // sort clusters according to their signal, highest is last
		vector<MAPMT_PIXEL>::iterator CurrentPixel;
		//cout << "Finding clusters..." << endl;
		do{ // start loop over all pixels above threshold
			TPixelCluster ThisCluster(nUserSearPattern); // create new TCluster
			ThisCluster.AddPixel(PixelsAboveThreshold.back()); // add pixel with largest signal to this cluster
			PixelsAboveThreshold.pop_back(); // remove this pixel from list of pixels above threshold
			CurrentPixel = PixelsAboveThreshold.begin();
			while (CurrentPixel != PixelsAboveThreshold.end() && !PixelsAboveThreshold.empty()){ // find all pixels belonging to this cluster
				if(ThisCluster.AddPixel(*CurrentPixel)){ // check if pixel is neighbour to any pixel already in the cluster
					PixelsAboveThreshold.erase(CurrentPixel); // remove pixel from list of pixels above threshold
					CurrentPixel = PixelsAboveThreshold.begin();
					continue;
				}
				CurrentPixel++;
			} // all pixels for this cluster found
			ClusterList.push_back(ThisCluster); // enter cluster into list of clusters
		}while (!PixelsAboveThreshold.empty()); // end loop over all pixels above threshold
	}
	return (ClusterList);
}

Double_t TMapmt::GetAverageGain() const {
	Double_t fGainSum = 0.0;
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		fGainSum += CurrentPixel->fGain;
	}
	Double_t fAvgGain = (ListOfPixels.size()>0)? fGainSum/(Double_t)ListOfPixels.size() : -1.0;
	return (fAvgGain);
}

TH2D* TMapmt::GetEventMap(Bool_t bUserFlag){
	// adjust row index!
	TH2D *hEventMap = new TH2D("hEventMap","MAPMT Event Map",nNumberOfColumns,0.5,nNumberOfColumns+0.5,nNumberOfRows,0.5,nNumberOfRows+0.5);
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		if(!bUserFlag)
			hEventMap->Fill(CurrentPixel->nPixelCol,nNumberOfRows-CurrentPixel->nPixelRow+1,CurrentPixel->fAmplitude);
		else if(CurrentPixel->fAmplitude>CurrentPixel->fThreshold)
			hEventMap->Fill(CurrentPixel->nPixelCol,nNumberOfRows-CurrentPixel->nPixelRow+1,CurrentPixel->fAmplitude);
	}
	return (hEventMap);
}

TH2I* TMapmt::GetHitMap(){
	TH2I* hHitMap = new TH2I("hHitMap","",nNumberOfColumns,0.5,nNumberOfColumns+0.5,nNumberOfRows,0.5,nNumberOfRows+0.5);
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		if(CurrentPixel->fAmplitude>CurrentPixel->fThreshold)
			hHitMap->Fill(CurrentPixel->nPixelCol,nNumberOfRows-CurrentPixel->nPixelRow+1);
	}
	return (hHitMap);
}

vector<MAPMT_PIXEL> TMapmt::GetHitPixels() const {
	vector<MAPMT_PIXEL> PixelsAboveThreshold; // vector storing pixels above threshold for this event
	PixelsAboveThreshold.reserve(ListOfPixels.size());
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){ // start loop over all pixels
		if(CurrentPixel->fAmplitude > CurrentPixel->fThreshold)
			PixelsAboveThreshold.push_back(*CurrentPixel); // enter pixel into vector
	} // end loop over all pixels
	return (PixelsAboveThreshold);
}

Bool_t TMapmt::GetPixel(Int_t nUserPixelId, MAPMT_PIXEL& UserPixel) const {
	vector<MAPMT_PIXEL>::const_iterator CurrentPixel = find(ListOfPixels.begin(),ListOfPixels.end(),nUserPixelId);
	if(CurrentPixel==ListOfPixels.end()){
		MAPMT_PIXEL DummyPixel(-1,-1,-1.0,-1.0,-1,-1);
		UserPixel = DummyPixel;
		return (kFALSE);
	}
	UserPixel = *CurrentPixel;
	return (kTRUE);
}

Int_t TMapmt::GetPixelId(Int_t nUserQdcChanId) const {
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		if(CurrentPixel->nQdcChannelId==nUserQdcChanId)
			return (CurrentPixel->nPixelId);
	}
	return (-1);
}

Int_t TMapmt::GetPixelId(Int_t nUserColId, Int_t nUserRowId) const {
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		if(CurrentPixel->nPixelCol==nUserColId && CurrentPixel->nPixelRow==nUserRowId)
			return (CurrentPixel->nPixelId);
	}
	return (-1);
}

void TMapmt::NormaliseGain(){
	Double_t fMaxGain = max_element(ListOfPixels.begin(),ListOfPixels.end(),ComparePixelsByGain)->fGain;
	for(vector<MAPMT_PIXEL>::iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		CurrentPixel->fGain /= (fMaxGain>0.0)? fMaxGain : 1.0;
	}
	bGainIsNormalised = kTRUE;
}

void TMapmt::Print(){
	cout << "##### " << cMapmtName << " #####" << endl;
	if(ListOfPixels.size() > 0) {
		cout << "Pixel \t QDC \t Gain \t Signal \t Threshold \t Row \t Column" << endl;
		for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
			cout << *CurrentPixel << endl;
		}
	}
}

Int_t TMapmt::ReadSparseData(std::map<Int_t,Double_t>& UserSparseData){
	Int_t nPixelsSet = 0; // number of pixels with QDC data set
	if(!bGainIsNormalised) // normalise gain values if neccessary
		NormaliseGain();
	ResetPixelAmplitudes();
	std::map<Int_t,Double_t>::iterator MatchedElement;
	for(vector<MAPMT_PIXEL>::iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		MatchedElement = UserSparseData.find(CurrentPixel->nQdcChannelId);
		if(MatchedElement!=UserSparseData.end()){
			CurrentPixel->fAmplitude = (CurrentPixel->fGain>0.0) ? (*MatchedElement).second / CurrentPixel->fGain : -1.0 * (*MatchedElement).second;
			nPixelsSet++; // increment number of pixels with QDC data set
		}
	}
	return (nPixelsSet);
}

Bool_t TMapmt::RemovePixel(Int_t nUserPixelId){
	for(vector<MAPMT_PIXEL>::iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		if(CurrentPixel->nPixelId==nUserPixelId){
			ListOfPixels.erase(CurrentPixel);
			FillMaps();
			return (kTRUE);
		}
	}
	return (kFALSE);
}

Int_t TMapmt::RemovePixels(const vector<Int_t>& nUserPixelIds){
	Int_t nRemovedPixels = 0;
	for(vector<Int_t>::const_iterator CurrentEntry = nUserPixelIds.begin(); CurrentEntry != nUserPixelIds.end(); CurrentEntry++){
		if (RemovePixel(*CurrentEntry))
			nRemovedPixels++;
	}
	return (nRemovedPixels);
}

void TMapmt::SetName(string cUserName){ 
	cMapmtName = cUserName;
	cMapmtNameChecked = cMapmtName;
	cMapmtNameChecked.erase(std::remove_if(cMapmtNameChecked.begin(), cMapmtNameChecked.end(), isspace), cMapmtNameChecked.end()); // remove blank characters
	string cHistName;
	string cNewHistTitle;
	cHistName = (cMapmtNameChecked.empty())? "hGainMap" : "hGainMap" + cMapmtNameChecked;
	cNewHistTitle = (cMapmtName.empty())? "MAPMT Gain Map" : cMapmtName + " Gain Map";
	hGainMap->SetName(cHistName.c_str());
	hGainMap->SetTitle(cNewHistTitle.c_str());
	cHistName.clear();
	cNewHistTitle.clear();
	cHistName = (cMapmtNameChecked.empty())? "hPixelMap" : "hPixelMap" + cMapmtNameChecked;
	cNewHistTitle = (cMapmtName.empty())? "MAPMT Pixel Map" : cMapmtName + " Pixel Map";
	hPixelMap->SetName(cHistName.c_str());
	hPixelMap->SetTitle(cNewHistTitle.c_str());
	cHistName.clear();
	cNewHistTitle.clear();
	cHistName = (cMapmtNameChecked.empty())? "hQdcMap" : "hQdcMap" + cMapmtNameChecked;
	cNewHistTitle = (cMapmtName.empty())? "MAPMT QDC Map" : cMapmtName + " QDC Map";
	hQdcMap->SetName(cHistName.c_str());
	hQdcMap->SetTitle(cNewHistTitle.c_str());
	cHistName.clear();
	cNewHistTitle.clear();
	cHistName = (cMapmtNameChecked.empty())? "hThresholdMap" : "hThresholdMap" + cMapmtNameChecked;
	cNewHistTitle = (cMapmtName.empty())? "MAPMT Threshold Map" : cMapmtName + " Threshold Map";
	hThresholdMap->SetName(cHistName.c_str());
	hThresholdMap->SetTitle(cNewHistTitle.c_str());
}

Int_t TMapmt::SetPixelGain(const TH2D* const hUserGainMap, Bool_t bInvertMap){
	if(hUserGainMap==NULL || hUserGainMap->IsZombie())
		return (-1);
	// histogram statistics
	Int_t nHistRows = hUserGainMap->GetNbinsY();
	Int_t nHistCols = hUserGainMap->GetNbinsX();
	if(nHistRows!=nNumberOfRows || nHistCols!=nNumberOfColumns) // gain histogram does not match MAPMT dimensions
		return (-1);
	hGainMap->Reset();
	Int_t nPixelsSet = 0; // number of pixels for which gain was set
	for (Int_t nCurrentRow=1; nCurrentRow<nHistRows+1; nCurrentRow++){ // begin loop over histogram rows
		for(Int_t nCurrentCol=1; nCurrentCol<nHistCols+1; nCurrentCol++){ // begin loop over histogram columns
			Int_t nCurrentPixelId = (bInvertMap)? (nCurrentRow-1)*nHistRows + nCurrentCol : (nHistRows-nCurrentRow)*nHistRows + nCurrentCol;
			vector<MAPMT_PIXEL>::iterator CurrentPixel = find(ListOfPixels.begin(),ListOfPixels.end(),nCurrentPixelId);
			if(CurrentPixel==ListOfPixels.end()) // skip rest of loop if pixel was not found
				continue;
			Double_t fCurrentPixelGain = hUserGainMap->GetBinContent(nCurrentCol,nCurrentRow);
			CurrentPixel->fGain = fCurrentPixelGain;
			hGainMap->Fill(CurrentPixel->nPixelCol,nNumberOfRows-CurrentPixel->nPixelRow+1,CurrentPixel->fGain);
			nPixelsSet++;
		} // end loop over histogram columns
	} // end of loop over histogram rows
	NormaliseGain();
	UpdateGainMap();
	return (nPixelsSet);
}

Int_t TMapmt::SetQdcSignal(const vector<Double_t>* const fUserQdcData){
	Int_t nPixelsSet = 0; // number of pixels with QDC data set
	if(!bGainIsNormalised) // normalise gain values if neccessary
		NormaliseGain();
	for(vector<MAPMT_PIXEL>::iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		try{
			CurrentPixel->fAmplitude = (CurrentPixel->fGain>0.0) ? fUserQdcData->at(CurrentPixel->nQdcChannelId) / CurrentPixel->fGain : -1.0 * fUserQdcData->at(CurrentPixel->nQdcChannelId) ;
		} catch ( std::out_of_range outOfRange ) // out_of_range exception
		{
			cout << "\n\nException: " << outOfRange.what();
			CurrentPixel->fAmplitude = -1.0;
			continue; // skip rest of loop
		}
		nPixelsSet++; // increment number of pixels with QDC data set
	}
	return (nPixelsSet);
}

Int_t TMapmt::SetQdcSignal(const vector<Int_t>* const nUserQdcData){
	Int_t nPixelsSet = 0; // number of pixels with QDC data set
	if(!bGainIsNormalised)
		NormaliseGain();
	for(vector<MAPMT_PIXEL>::iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		try{
			CurrentPixel->fAmplitude = (CurrentPixel->fGain>0.0) ? (Double_t)nUserQdcData->at(CurrentPixel->nQdcChannelId) / CurrentPixel->fGain : -1.0 * (Double_t)nUserQdcData->at(CurrentPixel->nQdcChannelId);
		} catch ( std::out_of_range outOfRange ) // out_of_range exception
		{
			cout << "\n\nException: " << outOfRange.what();
			CurrentPixel->fAmplitude = -1.0;
			continue;
		}
		nPixelsSet++; // increment number of pixels with QDC data set
	}
	return (nPixelsSet);
}

Int_t TMapmt::SetQdcSignal(const Double_t* const fUserQdcData, Int_t nUserArrayLength){
	vector<Double_t> fTempQdcData;
	fTempQdcData.assign(fUserQdcData,fUserQdcData+nUserArrayLength);
	return (SetQdcSignal(&fTempQdcData));
}

Int_t TMapmt::SetThresholds(const vector<Double_t>* const fUserThresholds, Bool_t bAdjust){
	Int_t nPixelsSet = 0; // number of pixels with QDC data set
	for(vector<MAPMT_PIXEL>::iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		try{
			CurrentPixel->fThreshold = (bAdjust && CurrentPixel->fGain>0.0)? fUserThresholds->at(CurrentPixel->nQdcChannelId)/CurrentPixel->fGain : fUserThresholds->at(CurrentPixel->nQdcChannelId);
		} catch ( std::out_of_range outOfRange ) // out_of_range exception
		{
			cout << "\n\nException: " << outOfRange.what();
			CurrentPixel->fThreshold = 0.0;
			continue; // skip rest of loop
		}
		nPixelsSet++; // increment number of pixels with QDC data set
	}
	if(nPixelsSet>0)
		UpdateThresholdMap();
	return (nPixelsSet);
}

Int_t TMapmt::SetThresholds(const Double_t* const fUserThresholds, Int_t nUserArrayLength, Bool_t bAdjust){
	vector<Double_t> fTempThresholds;
	fTempThresholds.assign(fUserThresholds,fUserThresholds+nUserArrayLength);
	return (SetThresholds(&fTempThresholds,bAdjust));
}

TH1D* TMapmt::ShowGainDistribution(Bool_t bUserSetShowFlag){
	Double_t fMinGain = ListOfPixels.front().fGain;
	Double_t fMaxGain = ListOfPixels.front().fGain;
	// search for extreme gain values, will be used as boundaries for histogram
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin()+1; CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		if(CurrentPixel->fGain < fMinGain) 
			fMinGain = CurrentPixel->fGain;
		else if(CurrentPixel->fGain > fMaxGain) 
			fMaxGain = CurrentPixel->fGain;
	}
	Double_t fGainRange = fMaxGain - fMinGain;
	if(hGainDistribution!=NULL)
		delete hGainDistribution;
	string cHistName = (cMapmtNameChecked.empty())? "hGainDistribution" : "hGainDistribution" + cMapmtNameChecked;
	string cNewHistTitle = (cMapmtName.empty())? "MAPMT Pixel Gain Distribution" : cMapmtName + " Pixel Gain Distribution";
	hGainDistribution = new TH1D(cHistName.c_str(),cNewHistTitle.c_str(),20,fMinGain-0.1*fGainRange,fMaxGain+0.1*fGainRange);
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		hGainDistribution->Fill(CurrentPixel->fGain);
	}
	if(bUserSetShowFlag)
		hGainDistribution->DrawCopy();
	return ((TH1D*)hGainDistribution->Clone());
}

TH2D* TMapmt::ShowGainMap(Bool_t bUserSetShowFlag){
	if(hGainMap!=NULL){
		if(bUserSetShowFlag)
			hGainMap->DrawCopy("COLZ TEXT");
		return ((TH2D*)hGainMap->Clone());
	}
	else
		return (NULL);
}	

TH2I* TMapmt::ShowPixelMap(Bool_t bUserSetShowFlag){
	if(hPixelMap!=NULL){
		if(bUserSetShowFlag)
			hPixelMap->DrawCopy("COLZ TEXT");
		return ((TH2I*)hPixelMap->Clone());
	}
	else
		return (NULL);
}

TH2I* TMapmt::ShowQdcMap(Bool_t bUserSetShowFlag){
	if(hQdcMap!=NULL){
		if(bUserSetShowFlag)
			hQdcMap->DrawCopy("COLZ TEXT");
		return ((TH2I*)hQdcMap->Clone());
	}
	else
		return (NULL);
}

TH1D* TMapmt::ShowThresholdDistribution(Bool_t bUserSetShowFlag){
	Double_t fMinThreshold = ListOfPixels.front().fThreshold;
	Double_t fMaxThreshold = ListOfPixels.front().fThreshold;
	// search for extreme gain values, will be used as boundaries for histogram
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin()+1; CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		if(CurrentPixel->fThreshold < fMinThreshold) 
			fMinThreshold = CurrentPixel->fThreshold;
		else if(CurrentPixel->fThreshold > fMaxThreshold) 
			fMaxThreshold = CurrentPixel->fThreshold;
	}
	Double_t fThresholdRange = fMaxThreshold - fMinThreshold;
	if(hThresholdDistribution!=NULL)
		delete hThresholdDistribution;
	string cHistName = (cMapmtNameChecked.empty())? "hThresholdDistribution" : "hThresholdDistribution" + cMapmtNameChecked;
	string cNewHistTitle = (cMapmtName.empty())? "MAPMT Pixel Threshold Distribution" : cMapmtName + " Pixel Threshold Distribution";
	hThresholdDistribution = new TH1D(cHistName.c_str(),cNewHistTitle.c_str(),20,fMinThreshold-0.1*fThresholdRange,fMaxThreshold+0.1*fThresholdRange);
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		hThresholdDistribution->Fill(CurrentPixel->fThreshold);
	}
	if(bUserSetShowFlag)
		hThresholdDistribution->DrawCopy();
	return ((TH1D*)hThresholdDistribution->Clone());
}

TH2D* TMapmt::ShowThresholdMap(Bool_t bUserSetShowFlag){
	if(hThresholdMap!=NULL){
		if(bUserSetShowFlag)
			hThresholdMap->DrawCopy("COLZ TEXT");
		return ((TH2D*)hThresholdMap->Clone());
	}
	else
		return (NULL);
}

void TMapmt::SortPixels(){
	if(ListOfPixels.size()<2) return;
	sort(ListOfPixels.begin(),ListOfPixels.end());
}

void TMapmt::UpdateGainMap(){
	if(hGainMap!=NULL){
		hGainMap->Reset();
		for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
			hGainMap->Fill(CurrentPixel->nPixelCol,nNumberOfRows-CurrentPixel->nPixelRow+1,CurrentPixel->fGain);
		}
	}
}

void TMapmt::UpdateThresholdMap(){
	if(hThresholdMap!=NULL){
		hThresholdMap->Reset();
		for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
			hThresholdMap->Fill(CurrentPixel->nPixelCol,nNumberOfRows-CurrentPixel->nPixelRow+1,CurrentPixel->fThreshold);
		}
	}
}

void TMapmt::WriteSetup(){
	ofstream SetupFile("myMapmtSetup.txt");
	for(vector<MAPMT_PIXEL>::const_iterator CurrentPixel = ListOfPixels.begin(); CurrentPixel != ListOfPixels.end(); CurrentPixel++){
		SetupFile << CurrentPixel->nPixelId << " " << CurrentPixel->nQdcChannelId << " " << CurrentPixel->fGain << " " << CurrentPixel->fThreshold << endl;
	}
	SetupFile.close();
}