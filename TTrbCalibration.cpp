#include "TTrbCalibration.h"

ClassImp(TTrbCalibration);

TTrbCalibration::TTrbCalibration(string cUserDataFilename, Int_t nUserCalibrationType, UInt_t nUserLimit, Bool_t bUserVerboseMode){
	bVerboseMode		= bUserVerboseMode;
	cInputFilename		= cUserDataFilename;
	SetCalibrationMethod(nUserCalibrationType);
	Init();
	SetStatsLimit(nUserLimit);
	if (!OpenTrbTree(cUserDataFilename)){
		cerr << "ERROR: Could not open file " << cUserDataFilename << endl;
		exit (-1); // exit RooT
	}
}

TTrbCalibration::~TTrbCalibration() {
	delete TrbData;
	delete CalibrationOutfile;
	//if(!TdcCalibrationData.empty())
	//	DeleteCalibrationPlots();
}

void TTrbCalibration::ApplyTdcCalibration(){
	cout << "Creating TClonesArray..." << endl;
	TClonesArray TdcHits;
	TdcHits.SetClass("TTrbHit", 1000); // declare that TTrbHits are being stored in the TClonesArray
	cout << "Opening output RooT file..." << endl;
	OpenRootFile();
	cout << "Creating TTree..." << endl;
	CreateTree();
	TTrbEventData *CurrentEventData = new TTrbEventData(TdcHits);
	//TBranch* TrbEvtBranch =
	OutputTree->Branch("event","TTrbEventData",&CurrentEventData);

	cout << "Looping over " << nEventsMax << " events..." << endl;
	for(Long64_t nEntryIndex=0; nEntryIndex<nEventsMax; nEntryIndex++){ // begin of loop over events
		if(bVerboseMode)
			cout << "Event Index " << nEntryIndex << endl;
		TdcHits.Clear("C"); // clear hits array (option "C" is important!)
		if(TrbData->GetEntry(nEntryIndex)<1) // check if entry exists and is valid
			continue; // skip rest of loop
		// copy event related data
		HLD_HEADER TempEvtHeader = { TrbData->nEvtSize ,TrbData->nEvtDecoding, TrbData->nEvtId, TrbData->nEvtSeqNr, TrbData->nEvtDate, TrbData->nEvtTime, TrbData->nEvtRun, TrbData->nEvtPad };
		CurrentEventData->AddEvtHeader(TempEvtHeader);
		SUB_HEADER TempSubEvtHeader = { TrbData->nSubEvtSize,
		                                TrbData->nSubEvtDecoding,
		                                TrbData->nSubEvtId,
		                                TrbData->nSubEvtTrigger };
		CurrentEventData->AddSubEvt(TempSubEvtHeader,
		                            TrbData->nCTSExtTrigger,
		                            TrbData->nCTSExtTriggerStatus,
		                            TrbData->nSebErrCode,
		                            TrbData->nTrbs,
		                            TrbData->nTdcs,
		                            TrbData->nSubEvtDecError);
		// calibrate fine time
		for(Int_t nHitIndex=0; nHitIndex<TrbData->Hits_; nHitIndex++){ // begin of loop over hits
			// copy existing hit data
			TTrbHit* CurrentHit = (TTrbHit*)TdcHits.ConstructedAt(nHitIndex); // create new TTrbHit in TclonesArray Hits
			CurrentHit->Set(TrbData->Hits_nTrbAddress[nHitIndex],TrbData->Hits_nTdcChannel[nHitIndex],
			                TrbData->Hits_nSubEvtId[nHitIndex],TrbData->Hits_nTdcErrCode[nHitIndex],
			                TrbData->Hits_nSignalEdge[nHitIndex],TrbData->Hits_nEpochCounter[nHitIndex],
			                TrbData->Hits_nCoarseTime[nHitIndex],
			                TrbData->Hits_nFineTime[nHitIndex],TrbData->Hits_bIsRefChannel[nHitIndex]);
			Double_t fHitTime = -1.0;
			std::pair< UInt_t,UInt_t > TdcChanAddress (TrbData->Hits_nTrbAddress[nHitIndex],TrbData->Hits_nTdcChannel[nHitIndex]); // extract TDC channel address
			std::map< std::pair< UInt_t,UInt_t >,TTrbFineTime >::const_iterator FindTdcChannel = ChannelCalibrations.find(TdcChanAddress); // find channel address in calibration map
			if(FindTdcChannel!=ChannelCalibrations.end()){ // check if channel is calibrated
				// first calculate time roughly, including epoch counter, which counts the overflows of coarse time
				// we don't use << binary operator due to overflow
				Double_t fHitTimeCoarse = CLOCK_CYCLE_LENGTH*(TrbData->Hits_nEpochCounter[nHitIndex]*pow(2.0,COARSE_TIME_BITS)
				                                              + TrbData->Hits_nCoarseTime[nHitIndex]);
				// Have a look, which calibration for the fine time we use...
				// Any yes, fine time needs to be SUBTRACTED, see TDC documentation!
				if(FindTdcChannel->second.IsCalibrated()) { // channel calibration is valid, then use it
					fHitTime = fHitTimeCoarse - FindTdcChannel->second.GetCalibratedTime(TrbData->Hits_nFineTime[nHitIndex]);
				}
				else{ // need to use substitute calibration from reference channel on same FPGA/TRB endpoint
					std::map< UInt_t, TTrbFineTime >::const_iterator SubIndex = ReferenceCalibrations.find(TrbData->Hits_nTrbAddress[nHitIndex]);
					if(SubIndex!=ReferenceCalibrations.end()){
						fHitTime = fHitTimeCoarse - SubIndex->second.GetCalibratedTime(TrbData->Hits_nFineTime[nHitIndex]);
					}
				}
				// fHitTime could be -1 if no calibration could be found
				CurrentHit->SetCalibratedTime(fHitTime); // add calibrated time to hit information
			}
		} // end of loop over hits
		OutputTree->Fill();
		} // end of loop over events
	cout << "Writing Tree to disk..." << endl;
	OutputRootFile = OutputTree->GetCurrentFile();
	OutputRootFile->Write();
	cout << "Cleaning up after decoding..." << endl;
	if(bVerboseMode)
		cout << "Deleting Tree..." << hex << OutputTree << dec << endl;
	delete OutputTree;
	OutputTree = NULL;
	if(bVerboseMode)
		cout << "Deleting RooT file..." << endl;
	delete OutputRootFile;
	OutputRootFile = NULL;
	if(bVerboseMode)
		cout << "Deleting Event Object..." << endl;
	delete CurrentEventData;
	TdcHits.Clear();
	if(bVerboseMode)
		cout << "Returning to terminal..." << endl;
}


Bool_t TTrbCalibration::CreateTree(){
	if(OutputTree!=NULL){
		delete OutputTree;
		OutputTree = NULL;
	}
	OutputTree = new TTree("T","UMainz TRBv3 HLD Data (Calibrated)",2);
	OutputTree->SetAutoSave(TREE_AUTOSAVE);
	return (!OutputTree->IsZombie());
}


void TTrbCalibration::DoTdcCalibration(){
	// tell ROOT that we manage TH1D memory management by ourselves
	// this solves the segfault when quitting ROOT (on Linux)
	TH1D::AddDirectory(kFALSE);

	cout << "Starting TDC Calibration..." << endl;
	cout << "Working on \"" << cInputFilename << "\"..." << endl;
	cout << "Max Events: " << nEventsMax << endl;

	for(Long64_t nEntryIndex=0; nEntryIndex<nEventsMax; ++nEntryIndex){ // begin of loop over events
		if(TrbData->GetEntry(nEntryIndex)<1) // check if entry exists and is valid
			continue; // skip rest of loop
		if(TrbData->Hits_>0){ // check if there are any TDC hits in this event
			if(bVerboseMode)
				cout << "Event ID: " << nEntryIndex << endl;
			FillFineTimeHistograms(); // fill fine time histograms
		}
	} // end of loop over events
	if(bVerboseMode)
		cout << "Number of Calibration Channels: " << ChannelCalibrations.size() << endl;
	cout << "Filling calibration tables..." << endl;
	FillReferenceCalibrationTables(); // fill reference channel calibration tables first
	FillCalibrationTable(); // fill TDC channel calibration tables
	if(bVerboseMode)
		PrintRefChannels(); // print addresses of found reference channels

	cout << "Writing calibration tables to disk..." << endl;
	WriteToFile(); // write TDC fine time histograms to file
	// after all that we need to loop over data again and generate calibrated timestamps
	cout << "Applying calibration..." << endl;
	ApplyTdcCalibration(); // apply TDC calibration to raw data

	// switch back to standard memory management (i.e. managed by ROOT)
	TH1D::AddDirectory(kTRUE);
}

Bool_t TTrbCalibration::ExcludeChannel(UInt_t nUserTrbAddress, UInt_t nUserTdcChannel){
	std::pair< UInt_t,UInt_t > TempPair (nUserTrbAddress, nUserTdcChannel); // create pair consisting of FPGA address and TDC channel
	std::vector< pair< UInt_t,UInt_t > >::iterator CheckPair; // iterator on vector containing excluded channel addresses
	CheckPair = find(ExcludedChannels.begin(),ExcludedChannels.end(),TempPair); // check if channel is already excluded
	if(CheckPair!=ExcludedChannels.end())
		return (kFALSE);
	ExcludedChannels.push_back(TempPair); // enter this channel into vector
	return(kTRUE);
}

UInt_t TTrbCalibration::ExcludeChannels(string UserFilename){
	UInt_t nExcludedChannels = 0;
	ifstream UserInputFile(UserFilename.c_str(),ifstream::in);
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
		UInt_t nTempAddress = (UInt_t)strtol(tokens.at(0).c_str(),NULL,0); // decode FPGA address
		UInt_t nTempChannel = (UInt_t)strtol(tokens.at(1).c_str(),NULL,10); // decode TDC channel
		switch (tokens.size()) {
			case 2: // FPGA address first (hex) then TDC channel
				if(ExcludeChannel(nTempAddress,nTempChannel))
					nExcludedChannels++;
				break;
			default:
				continue; // do nothing
		}
	} // end loop over input file
	UserInputFile.close();
	return(nExcludedChannels);
}

void TTrbCalibration::FillCalibrationTable(){
	for(std::map< std::pair< UInt_t, UInt_t >, TTrbFineTime >::iterator CurrentChannel=ChannelCalibrations.begin(); CurrentChannel!=ChannelCalibrations.end(); ++CurrentChannel){
		CurrentChannel->second.SetCalibrationMethod(nCalibrationType); // set calibration method type
		CurrentChannel->second.ComputeCalibrationTable(); // compute calibration look-up tables
	}
	if(bVerboseMode)
		for_each(ChannelCalibrations.begin(),ChannelCalibrations.end(),PrintStatus); // print status of calibration for each channel
}

void TTrbCalibration::FillReferenceCalibrationTables(){
	const Int_t nSubstituteCalibrationMethod = 0; // define substitute calibration type
	if(!ReferenceCalibrations.empty())
		ReferenceCalibrations.clear();
	if(TdcRefChannels.empty())
		return;
	std::map< std::pair< UInt_t, UInt_t >, TTrbFineTime >::const_iterator CopyThisChannel;
	for(std::map< UInt_t,UInt_t >::const_iterator CopyRefChannel=TdcRefChannels.begin(); CopyRefChannel!=TdcRefChannels.end(); ++CopyRefChannel) {
		CopyThisChannel = ChannelCalibrations.find(make_pair(CopyRefChannel->first,CopyRefChannel->second));
		if(CopyThisChannel==ChannelCalibrations.end())
			continue; // skip rest of loop, however this should never happen (so I need an error flag here)
		ReferenceCalibrations.insert(make_pair(CopyRefChannel->first,CopyThisChannel->second));
	}
	if(bVerboseMode)
		cout << "Number of Reference Calibrations: " << ReferenceCalibrations.size() << endl;
	if(ReferenceCalibrations.empty()){
		return;
	}
	for(std::map< UInt_t, TTrbFineTime >::iterator CurrentSubstitute=ReferenceCalibrations.begin(); CurrentSubstitute!=ReferenceCalibrations.end(); ++CurrentSubstitute){ // start of loop over all substitute calibrations
		CurrentSubstitute->second.SetCalibrationMethod(nSubstituteCalibrationMethod); // set calibration method (has to be the simple version)
		CurrentSubstitute->second.ComputeCalibrationTable(); // compute calibration look-up tables
		if(bVerboseMode)
			CurrentSubstitute->second.PrintStatus();
	} // end of loop over all substitute calibrations
}

void TTrbCalibration::FillFineTimeHistograms(){
	std::map< std::pair< UInt_t,UInt_t >, TTrbFineTime >::iterator ChannelRegistered;
	if(bVerboseMode)
		cout << "Looping over hits in event..." << endl;
	for(Int_t nHitIndex=0; nHitIndex<TrbData->Hits_; nHitIndex++){ // begin of loop over hits in this event
		// create address pair consisting of FPGA address and TDC channel ID
		pair<UInt_t,UInt_t> ChanAddress (TrbData->Hits_nTrbAddress[nHitIndex],TrbData->Hits_nTdcChannel[nHitIndex]);
		if(find(ExcludedChannels.begin(),ExcludedChannels.end(),ChanAddress)!=ExcludedChannels.end())
			continue; // skip creation as this channel has been excluded
		if(bVerboseMode)
			cout << "Channel @ 0x" << hex << ChanAddress.first << dec << " No " << ChanAddress.second << endl;
		// check for reference channels
		if(TrbData->Hits_bIsRefChannel[nHitIndex]){
			TdcRefChannels.insert(ChanAddress); // try inserting channel into reference channel map (since it is a map, no duplicates are allowed)
		}
		ChannelRegistered = ChannelCalibrations.find(ChanAddress);
		if(ChannelRegistered==ChannelCalibrations.end()){
			if(bVerboseMode)
				cout << "Creating new Fine Time object..." << endl;
			TTrbFineTime temp;
			temp.SetVerboseMode(bVerboseMode);
			temp.SetChannelAddress(ChanAddress);
			temp.SetStatsLimit(nEntriesMin);
			if(bVerboseMode)
				cout << "Inserting temporary fine time object into map..." << endl;
			pair<map<pair<UInt_t,UInt_t>,TTrbFineTime>::iterator,bool> insert =
				ChannelCalibrations.insert(make_pair(ChanAddress,temp));
			ChannelRegistered = insert.first;
		}
		if(bVerboseMode)
			cout << "Filling Fine Time Histogram..." << endl;
		ChannelRegistered->second.FillHistogram((Int_t)TrbData->Hits_nFineTime[nHitIndex]);
	} // end of loop over hits in this event
}

void TTrbCalibration::Init(){
	TrbData = NULL; // initialise pointer to data tree
	TdcRefChannels.clear(); // clear reference channels ID map
	ExcludedChannels.clear();
	fBinThreshold = 0.0; // set bin threshold to 0
	cCalibrationFilename = cInputFilename + "_CalibrationData.root";
	CalibrationOutfile = new TFile(cCalibrationFilename.c_str(),"RECREATE"); // need to change the creation of this file
	cRootFilename	= cInputFilename + "_calibrated.root";
	OutputRootFile	= NULL;
	OutputTree		= NULL;
	nEventsMax = 0;
}


Bool_t TTrbCalibration::OpenRootFile(){
	if(OutputRootFile!=NULL)
		delete OutputRootFile;
	OutputRootFile = new TFile(cRootFilename.c_str(),"RECREATE");
	return (!OutputRootFile->IsZombie());
}

Bool_t TTrbCalibration::OpenTrbTree(string cUserDataFilename){
	if(cUserDataFilename.empty()){
		cerr << "TRB data filename is empty!" << endl;
		nEventsMax = 0;
		return (kFALSE);
	}
	TFile *TrbTreeFile = new TFile(cUserDataFilename.c_str());
	if(TrbTreeFile->IsZombie()){
		cerr << "Error opening TRB data file " << cUserDataFilename << endl;
		nEventsMax = 0;
		return (kFALSE);
	}
	TTree *TrbTree	= (TTree*)TrbTreeFile->Get("T");
	TrbData			= new TTrbDataTree(TrbTree);
	nEventsMax		= TrbData->fChain->GetEntriesFast();
	return (kTRUE);
}

void TTrbCalibration::PrintExcludedChannels() const {
	for(std::vector< pair<UInt_t,UInt_t> >::const_iterator ChannelIndex=ExcludedChannels.begin(); ChannelIndex!=ExcludedChannels.end(); ChannelIndex++){
		cout << hex << ChannelIndex->first << dec << " " << ChannelIndex->second << endl;
	}
}

void TTrbCalibration::PrintRefChannels() const {

	cout << "++++++++++++++++++++++++++++++++++++" << endl;
	cout << "+ TRBv3 TDC Reference Channel List +" << endl;
	cout << "++++++++++++++++++++++++++++++++++++" << endl;
	for(std::map<UInt_t,UInt_t>::const_iterator MapIndex=TdcRefChannels.begin(); MapIndex!=TdcRefChannels.end(); MapIndex++)
		cout << hex << MapIndex->first << dec << " " << MapIndex->second << " " << ReferenceCalibrations.find(MapIndex->first)->second.GetEntries() << endl;
	cout << "++++++++++++++++++++++++++++++++++++" << endl;

}

void TTrbCalibration::WriteToFile(){
	CalibrationOutfile->cd(); // change to the output file
	// write histograms and graphs to file
	//for_each(TdcCalibrationData.begin(),TdcCalibrationData.end(),WriteHistogram);
	for_each(ChannelCalibrations.begin(),ChannelCalibrations.end(),WriteHistogram);
	// write statistics to tree
	//UInt_t nTempTdcAddress;
	//UInt_t nTempTdcChannel;
	//TDC_CAL_STATS TempStats;
	//TTree *CalibrationStats = new TTree("T","TRBv3 TDC Fine Time Calibration");
	//CalibrationStats->Branch("nTdcAddress",&nTempTdcAddress);
	//CalibrationStats->Branch("nTdcChannel",&nTempTdcChannel);
	//CalibrationStats->Branch("nEntries",&TempStats.nEntries);
	//CalibrationStats->Branch("fLowerEdge",&TempStats.fLowerEdge);
	//CalibrationStats->Branch("fUpperEdge",&TempStats.fUpperEdge);
	//CalibrationStats->Branch("fWidth",&TempStats.fWidth);
	//for(std::map<std::pair<UInt_t,UInt_t>,TDC_CAL_DATA>::const_iterator CurChannel=TdcCalibrationData.begin(); CurChannel!=TdcCalibrationData.end(); CurChannel++){
	//	nTempTdcAddress = CurChannel->first.first;
	//	nTempTdcChannel = CurChannel->first.second;
	//	TempStats = CurChannel->second.ChannelStats;
	//	CalibrationStats->Fill();
	//}
	//CalibrationStats->Write();
	//delete CalibrationStats;
	//CalibrationStats = NULL;
}
