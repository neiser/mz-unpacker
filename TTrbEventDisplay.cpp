#include "TTrbEventDisplay.h"

ClassImp(TTrbEventDisplay);

TTrbEventDisplay::TTrbEventDisplay(string cUserSetupFilename, string cUserThresholdFilename, string cUserDataFilename) : TTrbAnalysisBase(cUserDataFilename){ // standard constructor
	cout << "Initialising TRB Event Display..." << endl;
	Init();
	InitHistograms();
	cout << "Loading setup configuration file " << cUserSetupFilename << endl;
	LoadSetup(cUserSetupFilename);
	// base analysis class needs to know TDC address list, TDC size and TDC offset
}

TTrbEventDisplay::~TTrbEventDisplay(){ // standard destructor
	cout << "This is the destructor of TTrbEventDisplay class..." << endl;
	DeleteSetup();
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
		for(std::vector<MAPMT_PIXEL>::const_iterator CurrentPixel=ListOfPixels.begin(); CurrentPixel!=ListOfPixels.end(); CurrentPixel++){ // begin of loop over all pixels of current MAPMT
			hPixelCentreXMap.Fill(CurrentPixel->fX,CurrentPixel->fY,CurrentPixel->fX);
			hPixelCentreYMap.Fill(CurrentPixel->fX,CurrentPixel->fY,CurrentPixel->fY);
		} // end of loop over all pixels of current MAPMT
	} // end of loop over all MAPMTs in this setup
	bPixelCentreMapsAreFilled = kTRUE;
	hPixelCentreXMap.SetMinimum(hPixelCentreXMap.GetMinimum()-fabs(hPixelCentreXMap.GetMinimum())*1.0e-3);
	hPixelCentreYMap.SetMinimum(hPixelCentreYMap.GetMinimum()-fabs(hPixelCentreYMap.GetMinimum())*1.0e-3);
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

Int_t TTrbEventDisplay::HitMatching(){
	// match leading and trailing edge timestamps
	// use TdcHits multimap as starting point
	Int_t nMultipleHits = 0;
	//MatchedHits.clear(); // clear map containing matched hits
	LETimestamps.clear(); // clear leading edge map
	if(EvtTdcHits.empty()) // no TDC hits available
		return (nMultipleHits);
	std::multimap< UInt_t,UInt_t >::const_iterator CurrentTdcHit=EvtTdcHits.begin();
	while(CurrentTdcHit!=EvtTdcHits.end()){ // begin of loop over all TDC hits (excluding reference channels & user exclude list)
		//TrbPixelHit TempPixelHit;
		//TempPixelHit.bHasSyncTime = kFALSE;
		std::pair< std::multimap< UInt_t,UInt_t >::const_iterator,std::multimap< UInt_t,UInt_t >::const_iterator > LeadingEdges;
		std::pair< std::multimap< UInt_t,UInt_t >::const_iterator,std::multimap< UInt_t,UInt_t >::const_iterator > TrailingEdges;
		if(((CurrentTdcHit->first) % 2)!=0){ // channel number not even, skip this entry (hit must start with an even-numbered channel
					CurrentTdcHit = EvtTdcHits.upper_bound(CurrentTdcHit->first);
					continue; // skip rest of loop
				}
		LeadingEdges = EvtTdcHits.equal_range(CurrentTdcHit->first); // find range of entries for leading edges
		if(LeadingEdges.second==EvtTdcHits.end()){
			break;
		}
		TrailingEdges	= EvtTdcHits.equal_range(CurrentTdcHit->first+1); // find range of entries for trailing edges, trailing edge channel is leading edge + 1
		UInt_t nMultLeadEdge = (UInt_t) std::distance(LeadingEdges.first,LeadingEdges.second); // count hits for this channel
		UInt_t nMultTrailEdge = (UInt_t) std::distance(TrailingEdges.first,TrailingEdges.second);
		//cout << "Multiplicities: " << nMultLeadEdge << "\t" << nMultTrailEdge << endl;
		if(nMultLeadEdge!=nMultTrailEdge){ //mismatch of leading & trailing edge multiplicities
			CurrentTdcHit = TrailingEdges.second; // skip these entries
			//if(bVerboseMode){ // some error message

			//}
			continue; // skip rest of loop
		}
		Double_t fSyncLETime = -1.0;
		Double_t fCurTot = 0.0;
		std::pair< Int_t,Int_t > TempHitIndices;
		std::multimap< UInt_t,UInt_t >::const_iterator CurLeadEdge	= LeadingEdges.first;
		std::multimap< UInt_t,UInt_t >::const_iterator CurTrailEdge = TrailingEdges.first;
		switch(nMultLeadEdge){
			case 1: // single hit 		
				TempHitIndices = make_pair(CurLeadEdge->second,CurTrailEdge->second);
				fCurTot = TrbData->Hits_fTime[CurTrailEdge->second] - TrbData->Hits_fTime[CurLeadEdge->second];
				if(fCurTot<fMinTot){
					CurrentTdcHit = TrailingEdges.second;
					continue;
				}
				//TempPixelHit.nSyncIndex = GetTdcSyncIndex(TrbData->Hits_nTrbAddress[CurLeadEdge->second]);
				//}
				fSyncLETime = TrbData->Hits_fTime[CurLeadEdge->second] - TrbData->Hits_fTime[GetTdcSyncIndex(TrbData->Hits_nTrbAddress[CurLeadEdge->second])];
				if(bUseTimeWindow){
					if(fSyncLETime>TimingWindow.first && fSyncLETime<TimingWindow.second)
						LETimestamps.insert(make_pair(CurLeadEdge->first,fSyncLETime));
				}
				else{
					LETimestamps.insert(make_pair(CurLeadEdge->first,fSyncLETime));
					
				}
				CurrentTdcHit = TrailingEdges.second;
				break;
			default: // multiple hits
				++nMultipleHits;
				if(bSkipMultiHits || !bUseTimeWindow){ // user decision to skip multiple hits
					CurrentTdcHit = EvtTdcHits.upper_bound(CurrentTdcHit->first); // increment iterator to skip multiple hits
					continue; // skip rest of loop
				}
				// need to get iterators to leading and trailing edges
				//cout << "Matching multi hits now..." << endl;
				// loop over hits
				do{
					TempHitIndices = make_pair(CurLeadEdge->second,CurTrailEdge->second);
					fSyncLETime = TrbData->Hits_fTime[CurLeadEdge->second] - TrbData->Hits_fTime[GetTdcSyncIndex(TrbData->Hits_nTrbAddress[CurLeadEdge->second])];
					fCurTot = TrbData->Hits_fTime[CurTrailEdge->second] - TrbData->Hits_fTime[CurLeadEdge->second];
					if(fSyncLETime>TimingWindow.first && fSyncLETime<TimingWindow.second && fCurTot>fMinTot) // check that hit is within user-defined timing window
						LETimestamps.insert(make_pair(CurLeadEdge->first,fSyncLETime));
					++CurLeadEdge;
					++CurTrailEdge;

				} while (CurLeadEdge!=LeadingEdges.second);
				CurrentTdcHit = TrailingEdges.second;
				break;
		}
	}
	//cout << MatchedHits.size() << endl;
	return (nMultipleHits);
}

void TTrbEventDisplay::Init(){ // initialise event display class variables
	DetectorSetup.clear();
	LETimestamps.clear();

	bSetupIsValid	= kFALSE;
	bDataIsValid	= kFALSE;
	bPixelCentreMapsAreFilled	= kFALSE;
	bPixelMapIsFilled			= kFALSE;
	bReadoutMapIsFilled			= kFALSE;

	bSkipMultiHits = kTRUE;
	bUseTimeWindow = kFALSE;

	nNumberOfBins	= 0;
	nNumberOfEvents = 0;
	nNumberOfPixels	= 0;
	nNumberOfPmts	= 0;

	fMinTot = 0.0;

	cTreeName = "T";
	cEventDisplayTitle.str("DIRC@Mainz Event Display");
	// intialise display canvases
	canActiveCanvas		= NULL;
	canPixelMap			= NULL;
	canThresholdMap		= NULL;
	canPixelCentreMaps	= NULL;
	canReadoutMap		= NULL;
	canEventDisplay		= NULL;
}

void TTrbEventDisplay::InitHistograms(){
	hEventMap.AddDirectory(0); // do not add histogram to current directory
	hEventMap.SetStats(kFALSE); // switch off statistics box
	hEventMap.SetName("hEventMap"); hEventMap.SetTitle("DIRC@Mainz Event Map; horizontal coordinate; vertical coordinate");
	hPixelIndexMap.AddDirectory(0); // do not add histogram to current directory
	hPixelIndexMap.SetStats(kFALSE); // switch off statistics box
	hPixelIndexMap.SetName("hPixelIndexMap"); hPixelIndexMap.SetTitle("DIRC@Mainz Pixel Index Map; horizontal coordinate; vertical coordinate");
	hChannelMap.AddDirectory(0); // do not add histogram to current directory
	hChannelMap.SetStats(kFALSE); // switch off statistics box
	hChannelMap.SetName("hChannelMap"); hChannelMap.SetTitle("DIRC@Mainz Readout Channel Index Map; horizontal coordinate; vertical coordinate");
	hThresholdMap.AddDirectory(0); // do not add histogram to current directory
	hThresholdMap.SetStats(kFALSE); // switch off statistics box
	hThresholdMap.SetName("hThresholdMap"); hThresholdMap.SetTitle("DIRC@Mainz Threshold Map; horizontal coordinate; vertical coordinate");
	hPixelCentreXMap.AddDirectory(0); // do not add histogram to current directory
	hPixelCentreXMap.SetStats(kFALSE); // switch off statistics box
	hPixelCentreXMap.SetName("hPixelCentreXMap"); hPixelCentreXMap.SetTitle("DIRC@Mainz Pixel Centre X Map; horizontal coordinate; vertical coordinate");
	hPixelCentreYMap.AddDirectory(0); // do not add histogram to current directory
	hPixelCentreYMap.SetStats(kFALSE); // switch off statistics box
	hPixelCentreYMap.SetName("hPixelCentreYMap"); hPixelCentreYMap.SetTitle("DIRC@Mainz Pixel Centre Y Map; horizontal coordinate; vertical coordinate");
}

void TTrbEventDisplay::LoadSetup(string cUserSetupFilename){ // load user-defined MAPMT setup
	if(cUserSetupFilename.empty()){
		bSetupIsValid = kFALSE;
		return;
	}
	Int_t nPmtsAdded = 0; // number of MAPMTs successfully added
	Int_t nLineIndex = 0;
	ifstream UserSetupFile(cUserSetupFilename.c_str(),ifstream::in);
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
		vector<string> tokens = LineParser(cCurrentLine);
		TMapmt *CurrentMapmt = NULL;
		switch (tokens.size()) {
			case 9:
				CurrentMapmt = new TMapmt(atoi(tokens.at(1).c_str()),atoi(tokens.at(2).c_str()));
				CurrentMapmt->SetName(tokens.at(0));
				CurrentMapmt->SetPixelPitch(atof(tokens.at(3).c_str()),atof(tokens.at(4).c_str()));
				CurrentMapmt->SetAbsolutePosition(atof(tokens.at(6).c_str()),atof(tokens.at(7).c_str()));
				CurrentMapmt->SetRotationAngle(atof(tokens.at(8).c_str())*TMath::DegToRad());
				if(CurrentMapmt->AddPixels(tokens.at(5))>0){
					CurrentMapmt->IgnoreThresholds();
					DetectorSetup.push_back(CurrentMapmt); // add MAPMT to setup vector
					nNumberOfPixels += CurrentMapmt->GetN(); // increment number of pixels in setup
					nNumberOfPmts++; // increment number of MAPMTs in setup
				}
				else{
					delete CurrentMapmt;
					CurrentMapmt = NULL;
				}
				break;
			default:
				cout << "Skipping incomplete MCP-PMT definition on line " << nLineIndex << endl;
				cout << "!!!\t" << cCurrentLine << "\t!!!" << endl;
				break; // do nothing
		}
	} // end loop over input file
	UserSetupFile.close();
	if(nNumberOfPmts>0 && nNumberOfPixels>0){
		bSetupIsValid = kTRUE;
		DefineDisplayBins();
	}
}

void TTrbEventDisplay::PrintLETimestamps() const{
	if(LETimestamps.empty())
		return;
	std::map<Int_t,Double_t>::const_iterator FirstEntry = LETimestamps.begin();
	std::map<Int_t,Double_t>::const_iterator LastEntry = LETimestamps.end();
	cout << "+++++++++++++++++++++++++++++++++" << endl;
	cout << "+++  Sync LE Timestamps Hits  +++" << endl;
	cout << "+++++++++++++++++++++++++++++++++" << endl;
	cout << LETimestamps.size() << " MATCHED TDC HITS FOUND" << endl;
	cout << "+++++++++++++++++++++++++++++++++" << endl;
	for(std::map<Int_t,Double_t>::const_iterator CurEntry=FirstEntry; CurEntry!=LastEntry; ++CurEntry){ // begin loop over all event hit entries
		cout << distance(FirstEntry,CurEntry) << "\t" << CurEntry->first << "\t" << CurEntry->second << endl;
	} // end of loop over all event hit entries
	cout << "+++++++++++++++++++++++++++++++++" << endl;
}

void TTrbEventDisplay::SetCanvasStyle(TCanvas *canThisCanvas){
	if(canThisCanvas==NULL)
		return;
	canThisCanvas->SetTicks(); // set tick marks on opposite side of axis (both X and Y)
	canThisCanvas->SetGrid(); // set grid lines for canvas
}

void TTrbEventDisplay::SetTimingWindow( Double_t fUserLow, Double_t fUserUpper){
	if(fUserLow==fUserUpper) // check if boundaries are equal
		return;
	TimingWindow.first = (fUserLow<fUserUpper)? fUserLow : fUserUpper; // set lower boundary
	TimingWindow.second = (fUserLow<fUserUpper)? fUserUpper : fUserLow; // set upper boundary
	bUseTimeWindow = kTRUE;
}

void TTrbEventDisplay::Show(Int_t nUserEventId){
	hEventMap.ClearBinContents();
	hEventMap.Reset("M");
	if(!GetStatus()){
		cout << "TRB data not valid!" << endl;
		return;
	}
	if(GetEntry(nUserEventId)<1){
		cout << "Event " << nUserEventId << " is not valid!" << endl;
		return;
	}
	if(LETimestamps.empty()){
		return;
	}
	PrintLETimestamps();
	for(std::vector<TMapmt*>::const_iterator CurrentPmt=DetectorSetup.begin(); CurrentPmt!=DetectorSetup.end(); CurrentPmt++){ // start of loop over all MAPMTs in this setup
		(*CurrentPmt)->ReadSparseData(LETimestamps);
		vector<MAPMT_PIXEL> Hits = (*CurrentPmt)->GetHitPixels();
		for(std::vector<MAPMT_PIXEL>::const_iterator CurrentPixel=Hits.begin(); CurrentPixel!=Hits.end(); CurrentPixel++){ // begin loop over all PMT pixels
			if(bUseTimeWindow){ // apply timing cut
				if(CurrentPixel->fAmplitude>TimingWindow.first&&CurrentPixel->fAmplitude<TimingWindow.second)
					hEventMap.Fill(CurrentPixel->fX,CurrentPixel->fY,CurrentPixel->fAmplitude);
			}
			else // no timing window set
				hEventMap.Fill(CurrentPixel->fX,CurrentPixel->fY,CurrentPixel->fAmplitude);
		} // end of loop over all PMT pixels
	} // end of loop over all MAPMTs in this setup
	Double_t fHistMinVal = hEventMap.GetMinimum()-fabs(hEventMap.GetMinimum())*1.0e-3;
	hEventMap.SetMinimum(fHistMinVal);
	if(canEventDisplay==NULL){
		canEventDisplay = new TCanvas("canEventDisplay",cEventDisplayTitle.str().c_str(),-700,700);
		SetCanvasStyle(canEventDisplay);
	}
	////	delete canEventDisplay;
	if(canActiveCanvas!=canEventDisplay)
		canEventDisplay->cd();
	canActiveCanvas = canEventDisplay;
	cEventDisplayTitle.str("");
	cEventDisplayTitle << "Event " << nUserEventId;
	hEventMap.SetTitle(cEventDisplayTitle.str().c_str());
	hEventMap.DrawCopy("COLTEXT");
}


void TTrbEventDisplay::Show(UInt_t nUserStart, UInt_t nUserStop){
	hEventMap.ClearBinContents();
	hEventMap.Reset("M");
	if(!GetStatus()){
		cout << "TRB data not valid!" << endl;
		return;
	}
	// need to check user event indices
	UInt_t nStartIndex	= std::min(nUserStart,nUserStop);
	UInt_t nStopIndex	= std::max(nUserStart,nUserStop);
	if(nStopIndex>(UInt_t)GetNEvents()) // check if upper index is too large
		nStopIndex = (UInt_t)GetNEvents();
	for(Int_t nEventIndex=nStartIndex; nEventIndex<nStopIndex; ++nEventIndex){ // begin of loop over events
		if(GetEntry(nEventIndex)<1){
			cout << "Skipping event " << nEventIndex << endl;
			continue;
		}
		if(LETimestamps.empty()){ // no hits in event
			continue; // skip rest of loop
		}
		for(std::vector<TMapmt*>::const_iterator CurrentPmt=DetectorSetup.begin(); CurrentPmt!=DetectorSetup.end(); CurrentPmt++){ // start of loop over all MAPMTs in this setup
			(*CurrentPmt)->ReadSparseData(LETimestamps);
			vector<MAPMT_PIXEL> Hits = (*CurrentPmt)->GetHitPixels();
			for(std::vector<MAPMT_PIXEL>::const_iterator CurrentPixel=Hits.begin(); CurrentPixel!=Hits.end(); CurrentPixel++){ // begin loop over all PMT pixels
				if(bUseTimeWindow){ // apply timing cut
					if(CurrentPixel->fAmplitude>TimingWindow.first&&CurrentPixel->fAmplitude<TimingWindow.second){
						hEventMap.Fill(CurrentPixel->fX,CurrentPixel->fY,1.0);
					}
				}
				else // no timing window set
					hEventMap.Fill(CurrentPixel->fX,CurrentPixel->fY,1.0);
			} // end of loop over all PMT pixels
			Hits.clear();
		} // end of loop over all MAPMTs in this setup
	} // end of loop over events
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
	hEventMap.DrawCopy("COLTEXT");
}

void TTrbEventDisplay::ShowPixelCentreMaps(){
	if(!FillPixelCentreMaps())
		return;
	canPixelCentreMaps = new TCanvas("canPixelCentreMaps","MCP-PMT Pixel Centre Maps",-500,1000);
	SetCanvasStyle(canPixelCentreMaps);
	canActiveCanvas = canPixelCentreMaps;
	canPixelCentreMaps->Divide(1,2);
	canPixelCentreMaps->cd(1);
	hPixelCentreXMap.DrawCopy("COLTEXT");
	mSetupCentre.DrawClone("SAME");
	canPixelCentreMaps->cd(2);
	hPixelCentreYMap.DrawCopy("COLTEXT");
	mSetupCentre.DrawClone("SAME");
}

void TTrbEventDisplay::ShowPixelMap(){
	if(!FillPixelMap())
		return;
	canPixelMap = new TCanvas("canPixelMap","MCP-PMT Pixel Map",-500,500); // If form < 0  the menubar is not shown.
	SetCanvasStyle(canPixelMap);
	canActiveCanvas = canPixelMap;
	canPixelMap->cd();
	//canPixelMap->Clear();
	hPixelIndexMap.DrawCopy("COLTEXT");
	mSetupCentre.DrawClone("SAME");
}

void TTrbEventDisplay::ShowReadoutMap(){
	if(!FillReadoutMap())
		return;
	canReadoutMap = new TCanvas("canReadoutMap","MCP-PMT Readout Channel Map",-500,500);
	SetCanvasStyle(canReadoutMap);
	canActiveCanvas = canReadoutMap;
	canReadoutMap->cd();
	hChannelMap.DrawCopy("COLTEXT");
	mSetupCentre.DrawClone("SAME");
}