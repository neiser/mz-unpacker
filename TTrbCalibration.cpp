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
	if(CalibrationOutfile!=NULL){
		delete CalibrationOutfile;
		CalibrationOutfile = NULL;
	}
	/*delete CalibrationOutfile;*/
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
		HLD_HEADER TempEvtHeader = { TrbData->nEvtSize ,TrbData->nEvtDecoding, TrbData->nEvtId, TrbData->nEvtSeqNr, TrbData->nEvtDate, TrbData->nEvtTime, TrbData->nEvtRun, TrbData->nEvtPad }; // copy event header data into temporary structure
		CurrentEventData->AddEvtHeader(TempEvtHeader); // add event header to event data
		SUB_HEADER TempSubEvtHeader = { TrbData->nSubEvtSize, TrbData->nSubEvtDecoding, TrbData->nSubEvtId, TrbData->nSubEvtTrigger }; // copy subevent header data into temporary structure
		CurrentEventData->AddSubEvt(TempSubEvtHeader,TrbData->nSebErrCode,TrbData->nTrbs,TrbData->nTdcs,TrbData->nSubEvtDecError); // add subevent header data to event data
		// calibrate fine time
		for(Int_t nHitIndex=0; nHitIndex<TrbData->Hits_; nHitIndex++){ // begin of loop over hits
			// copy existing hit data
			TTrbHit* CurrentHit = (TTrbHit*)TdcHits.ConstructedAt(nHitIndex); // create new TTrbHit in TclonesArray Hits
			CurrentHit->Set(TrbData->Hits_nTrbAddress[nHitIndex],TrbData->Hits_nTdcChannel[nHitIndex],
			                TrbData->Hits_nSubEvtId[nHitIndex],TrbData->Hits_nTdcErrCode[nHitIndex],
			                TrbData->Hits_nSignalEdge[nHitIndex],TrbData->Hits_nEpochCounter[nHitIndex],
			                TrbData->Hits_nCoarseTime[nHitIndex],
			                TrbData->Hits_nFineTime[nHitIndex],TrbData->Hits_bIsRefChannel[nHitIndex]);
			Double_t fHitTime = -1.0; // set standard value (there are no unphysical values)
			std::pair< UInt_t,UInt_t > TdcChanAddress (TrbData->Hits_nTrbAddress[nHitIndex],TrbData->Hits_nTdcChannel[nHitIndex]); // extract TDC channel address
			std::map< std::pair< UInt_t,UInt_t >,TTrbFineTime >::const_iterator FindTdcChannel = ChannelCalibrations.find(TdcChanAddress); // find channel address in calibration map
			if(FindTdcChannel!=ChannelCalibrations.end()){ // check if channel is calibrated
				// first calculate time roughly, including epoch counter, which counts the overflows of coarse time
				// we don't use << binary operator due to overflow
				Double_t fHitTimeCoarse = CLOCK_CYCLE_LENGTH*(TrbData->Hits_nEpochCounter[nHitIndex]*pow(2.0,COARSE_TIME_BITS) + TrbData->Hits_nCoarseTime[nHitIndex]);
				// Have a look, which calibration for the fine time we use...
				// Any yes, fine time needs to be SUBTRACTED, see TDC documentation!
				if(FindTdcChannel->second.IsCalibrated()) { // channel calibration is valid, then use it
					fHitTime = fHitTimeCoarse - FindTdcChannel->second.GetCalibratedTime(TrbData->Hits_nFineTime[nHitIndex]);
				}
				// fHitTime could be -1 if no calibration could be found
				CurrentHit->SetCalibratedTime(fHitTime); // add calibrated time to hit information
			}
			else{ // could NOT find channel in calibration map
				if(std::find(MissingChannels.begin(),MissingChannels.end(),TdcChanAddress)==MissingChannels.end())
					MissingChannels.push_back(TdcChanAddress);
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

std::pair<UInt_t,UInt_t> TTrbCalibration::DecodeChannelId(string cGraphName){
	std::vector<string> tokens = LineParser(cGraphName,'_');
	UInt_t nTdcId;
	UInt_t nTdcChan;
	if(tokens.size()==4){
		std::istringstream cTemp;
		cTemp.str(tokens.at(1));
		cTemp >> hex >> nTdcId;
		cTemp.str(""); // clear string
		cTemp.clear(); // clear error flags (IMPORTANT)
		cTemp.str(tokens.at(3));
		cTemp >> dec >> nTdcChan;
		//cout << hex << nTdcId << " " << dec << nTdcChan << endl;
	}
	return (std::make_pair(nTdcId,nTdcChan));
}

void TTrbCalibration::DoTdcCalibration(){
	// tell ROOT that we manage TH1D memory by ourselves
	// this solves the segfault when quitting ROOT (on Linux)
	TH1D::AddDirectory(kFALSE);
	CalibrationOutfile = new TFile(cCalibrationFilename.c_str(),"RECREATE"); // need to change the creation of this file

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
//	FillReferenceCalibrationTables(); // fill reference channel calibration tables first
	FillCalibrationTable(); // fill TDC channel calibration tables
	cout << "Writing calibration tables to disk..." << endl;
	WriteToFile(); // write TDC fine time histograms to file
	delete CalibrationOutfile; // close calibration data ROOT file
	CalibrationOutfile = NULL;
	// after all that we need to loop over data again and generate calibrated timestamps
	// write log file
	OpenLogFile(); // open log file
	WriteSettingsToLog();
	CloseLogFile();
	cout << "Applying calibration..." << endl;
	ApplyTdcCalibration(); // apply TDC calibration to raw data
	// switch back to standard memory management (i.e. managed by ROOT)
	TH1D::AddDirectory(kTRUE);
}

void TTrbCalibration::DoTdcCalibration(string cCalibrationFile){
	TH1D::AddDirectory(kFALSE);
	TFile CalibrationFile(cCalibrationFile.c_str()); // open external calibration file to load old calibration constants
	if(CalibrationFile.IsZombie()){
		exit(0);
	};
	TList *list = (TList*)CalibrationFile.GetListOfKeys(); // get list of keys, i.e. names of THs and TGraphs etc
	TIter iter((TList*)list);
	list->Sort();
	//iter.Begin(); // this should not be used (ROOT's idiotic idea of an iterator)
	string cTempGraphName;
	TObject *obj;
	while(obj = iter()){
		cTempGraphName = obj->GetName();
		//cout << cTempGraphName << endl;
		string cCalibGraph = "grCalibrationTable";
		size_t found = cTempGraphName.find(cCalibGraph);
		if(found!=std::string::npos){
			//cout << cTempGraphName << endl;
			std::pair<UInt_t,UInt_t> ChanAddress = DecodeChannelId(cTempGraphName);
			TGraph *grTempCalib = (TGraph*)CalibrationFile.Get(cTempGraphName.c_str());
			if(bVerboseMode)
				cout << "Creating new Fine Time object..." << endl;
			TTrbFineTime temp(*grTempCalib);
			temp.SetVerboseMode(bVerboseMode);
			temp.SetChannelAddress(ChanAddress);
			//temp.SetStatsLimit(nEntriesMin);
			if(bVerboseMode)
				cout << "Inserting temporary fine time object into map..." << endl;
			pair<map<pair<UInt_t,UInt_t>,TTrbFineTime>::iterator,bool> insert =
				ChannelCalibrations.insert(make_pair(ChanAddress,temp));
			if(insert.second){
				//insert.first->second.PrintStatus();
				//cout << insert.first->second.GetCalibratedTime(100) << endl;
			}
		}
	}
	if(ChannelCalibrations.empty()) // check if calibration map is empty
		exit (0); // exit program
	OpenLogFile(); // open log file
	LogFile << "Load calibration file: " << cCalibrationFile << endl;
	WriteSettingsToLog();
	CloseLogFile();
	//cout << ChannelCalibrations.size() << endl;
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
	std::map< std::pair< UInt_t, UInt_t >, TTrbFineTime >::iterator CurrentChannel;
	for(CurrentChannel=ChannelCalibrations.begin(); CurrentChannel!=ChannelCalibrations.end(); ++CurrentChannel){ // begin loop over all TDC channels found
		CurrentChannel->second.SetCalibrationMethod(nCalibrationType); // set calibration method type
		CurrentChannel->second.ComputeCalibrationTable(); // compute calibration look-up tables
	} // end of loop over all TDC channels found
	if(bVerboseMode)
		for_each(ChannelCalibrations.begin(),ChannelCalibrations.end(),PrintStatus); // print status of calibration for each channel
}

void TTrbCalibration::FillFineTimeHistograms(){
	std::map< std::pair< UInt_t,UInt_t >, TTrbFineTime >::iterator ChannelRegistered;
	if(bVerboseMode)
		cout << "Looping over hits in event..." << endl;
	for(Int_t nHitIndex=0; nHitIndex<TrbData->Hits_; nHitIndex++){ // begin of loop over hits in this event
		pair<UInt_t,UInt_t> ChanAddress (TrbData->Hits_nTrbAddress[nHitIndex],TrbData->Hits_nTdcChannel[nHitIndex]); // create address pair consisting of FPGA address and TDC channel ID
		if(find(ExcludedChannels.begin(),ExcludedChannels.end(),ChanAddress)!=ExcludedChannels.end())
			continue; // skip creation as this channel has been excluded
		if(bVerboseMode)
			cout << "Channel @ " << std::hex << std::showbase << ChanAddress.first << std::noshowbase << dec << " No " << ChanAddress.second << endl;
		ChannelRegistered = ChannelCalibrations.find(ChanAddress); // try finding channel in calibration map
		if(ChannelRegistered==ChannelCalibrations.end()){ // this channel is not in the list yet, so create a new TTrbFineTime object
			if(bVerboseMode)
				cout << "Creating new Fine Time object..." << endl;
			TTrbFineTime temp; // create temporary fine time object
			temp.SetVerboseMode(bVerboseMode); // set verbosity level
			temp.SetChannelAddress(ChanAddress); // set channel address
			temp.SetStatsLimit(nEntriesMin); // set min limit for statistics
			if(bVerboseMode)
				cout << "Inserting temporary fine time object into map..." << endl;
			pair<map<pair<UInt_t,UInt_t>,TTrbFineTime>::iterator,bool> insert =
				ChannelCalibrations.insert(make_pair(ChanAddress,temp)); // enter fine time object into calibration map
			ChannelRegistered = insert.first;
		}
		if(bVerboseMode)
			cout << "Filling Fine Time Histogram..." << endl;
		ChannelRegistered->second.FillHistogram((Int_t)TrbData->Hits_nFineTime[nHitIndex]);
	} // end of loop over hits in this event
}

void TTrbCalibration::Init(){
	TrbData = NULL; // initialise pointer to data tree
	//TdcRefChannels.clear(); // clear reference channels ID map
	ExcludedChannels.clear();
	MissingChannels.clear();
	fBinThreshold = 0.0; // set bin threshold to 0
	cCalibrationFilename = cInputFilename + "_CalibrationData.root";
	CalibrationOutfile = NULL;
	cRootFilename	= cInputFilename + "_calibrated.root";
	cLogFilename	= cInputFilename + "_calibration_log.txt";
	OutputRootFile	= NULL;
	OutputTree		= NULL;
	nEventsMax = 0;
}

Bool_t TTrbCalibration::OpenLogFile(){
	if(LogFile.is_open()){ // check, if log file is already open
		LogFile.close(); // close it then
	}
	time_t RawTime; // time structure
	LogFile.open(cLogFilename.c_str(), ios::out);
	LogFile << "++++++++++++++++++++++++++++++++++" << endl;
	LogFile << "+++ TRBv3 Calibration  +++" << endl;
	LogFile << "++++++++++++++++++++++++++++++++++" << endl;
	time(&RawTime);
	LogFile << "\t" << ctime(&RawTime) << endl;
	return (!LogFile.fail());
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
	if(ExcludedChannels.empty())
		return;
	cout << "++++++++++++++++++++++++++++++++++++" << endl;
	cout << "+ TRBv3 TDC Excluded Channels List +" << endl;
	cout << "++++++++++++++++++++++++++++++++++++" << endl;
	for(std::vector< pair<UInt_t,UInt_t> >::const_iterator ChannelIndex=ExcludedChannels.begin(); ChannelIndex!=ExcludedChannels.end(); ChannelIndex++){
		cout << hex << ChannelIndex->first << dec << " " << ChannelIndex->second << endl;
	}
}

void TTrbCalibration::PrintMissingChannels() const {
	if(MissingChannels.empty())
		return;
	cout << "+++++++++++++++++++++++++++++++++++" << endl;
	cout << "+ TRBv3 TDC Missing Channels List +" << endl;
	cout << "+++++++++++++++++++++++++++++++++++" << endl;
	std::list<std::pair<UInt_t,UInt_t>>::const_iterator ChannelIndex = MissingChannels.begin();
	for(ChannelIndex; ChannelIndex!=MissingChannels.end(); ++ChannelIndex){
		cout << hex << ChannelIndex->first << dec << " " << ChannelIndex->second << endl;
	}
}

void TTrbCalibration::PrintSettings() const {
	cout << "++++++++++++++++++++++++++++++++++" << endl;
	cout << "+++ Calibration Settings +++" << endl;
	cout << "++++++++++++++++++++++++++++++++++" << endl;
	cout << "Number of Fine Time Bins:\t" << FINE_TIME_BINS << endl;
	cout << "Clock cycle length:\t" << CLOCK_CYCLE_LENGTH << "ns" << endl;
	cout << "Number of Coarse Time bits:\t" << COARSE_TIME_BITS << endl;
	cout << "Min required statistics:\t" << nEntriesMin << endl;
	cout << "++++++++++++++++++++++++++++++++++" << endl;
}

void TTrbCalibration::WriteSettingsToLog(){
	if(!LogFile.is_open()) // log file is not open
		return;
	// redirect cout buffer to logfile
	std::streambuf *psbuf, *backup;
	backup	= std::cout.rdbuf();
	psbuf	= LogFile.rdbuf();
	std::cout.rdbuf(psbuf);         // assign streambuf to cout
	LogFile << "Input data file: " << cInputFilename << endl;
	LogFile << "Output data file: " << cRootFilename << endl;
	// print status information to logfile
	PrintSettings();
	PrintExcludedChannels();
	PrintMissingChannels();
	LogFile << "+++++++++++++++++++++++++++++++" << endl;
	// reset cout buffer to terminal
	std::cout.rdbuf(backup);        // restore cout's original streambuf
	psbuf = NULL;
	backup = NULL;
}


void TTrbCalibration::WriteToFile(){
	CalibrationOutfile->cd(); // change to the output file
	// write histograms and graphs to file
	//for_each(TdcCalibrationData.begin(),TdcCalibrationData.end(),WriteHistogram);
	for_each(ChannelCalibrations.begin(),ChannelCalibrations.end(),WriteHistogram);
}
