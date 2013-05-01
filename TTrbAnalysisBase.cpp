#include "TTrbAnalysisBase.h"

ClassImp(TTrbAnalysisBase);

TTrbAnalysisBase::TTrbAnalysisBase(string cUserDataFilename, string cUserTdcAddressesFile, UInt_t nUserTdcChannels, UInt_t nUserTdcOffset, Bool_t bUserVerboseMode) : TObject(), nChanPerTdc(nUserTdcChannels), nTdcOffset(nUserTdcOffset), bVerboseMode(bUserVerboseMode){
	Init(); // initialise variables
	SetTdcAddresses(cUserTdcAddressesFile); // decode TDC addresses provided by user
	if(TdcAddresses.empty()) // no TRB boards defined
		exit (-1);
	if(!OpenTrbTree(cUserDataFilename)) // check if we can open TRB data tree
		exit (-1);
	ComputeMappingTable();
}

TTrbAnalysisBase::~TTrbAnalysisBase(){
	delete TrbData;
}

Bool_t TTrbAnalysisBase::CheckRandomBits(){
	// check that all TDC hits in an event have the same random bits sequence
	switch (TrbData->Hits_){
		case 0: // no TDC hits, shouldn't happen
			return (kFALSE);
			break;
		case 1: // only one TDC hit, is always true
			return (kTRUE);
			break;
		default: // more than 1 TDC hit
			UInt_t nRefRndmBits = TrbData->Hits_nSubEvtId[0]; // set reference sequence
			for(Int_t i=1; i<TrbData->Hits_; ++i){ // loop over all hits in entry
				if(TrbData->Hits_nSubEvtId[1]!=nRefRndmBits){ // check if current sequence doesn't equal reference sequence
					return (kFALSE);
				}
			} // end of loop over all hits in entry
			return (kTRUE); // all sequences are the same
	}
	return (kFALSE);
}

void TTrbAnalysisBase::ClearEventMaps(){
	EvtSyncTimeStamps.clear(); // clear sync timestamps map
	EvtChanEntries.clear(); // clear channel entries map
}

void TTrbAnalysisBase::ComputeMappingTable(){
	MappingTable.clear();
	// loop over all TDC addresses
	std::map< UInt_t,UInt_t >::iterator LastTdcAddress = TdcAddresses.end();
	for(std::map< UInt_t,UInt_t >::iterator CurrentTdc=TdcAddresses.begin(); CurrentTdc!=LastTdcAddress; ++CurrentTdc){ // begin loop over all TDC addresses
		UInt_t nTdcIndex = (UInt_t)distance(TdcAddresses.begin(),CurrentTdc);
		for(UInt_t i=0; i<CurrentTdc->second; ++i){ // begin loop over all TDC channels
			UInt_t nSeqId = nTdcIndex * CurrentTdc->second + i;
			MappingTable.insert(make_pair(make_pair(CurrentTdc->first,i+nTdcOffset),nSeqId));
		} // end of loop over all TDC channels
	} // end of loop over all TDC addresses
}

Bool_t TTrbAnalysisBase::ExcludeChannel(UInt_t nUserTrbAddress, UInt_t nUserTdcChannel){
	std::pair< UInt_t,UInt_t > TempPair (nUserTrbAddress, nUserTdcChannel); // create pair consisting of FPGA address and TDC channel
	std::vector< pair< UInt_t,UInt_t > >::iterator CheckPair; // iterator on vector containing excluded channel addresses
	CheckPair = find(ExcludedChannels.begin(),ExcludedChannels.end(),TempPair); // check if channel is already excluded
	if(CheckPair!=ExcludedChannels.end())
		return (kFALSE);
	ExcludedChannels.push_back(TempPair); // enter this channel into vector
	return(kTRUE);
}

UInt_t TTrbAnalysisBase::ExcludeChannels(string UserFilename){
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
		UInt_t nTempAddress;
		UInt_t nTempChannel;
		switch (tokens.size()) {
			case 2: // FPGA address first (hex) then TDC channel
				nTempAddress = (UInt_t)strtoul(tokens.at(0).c_str(),NULL,16); // decode FPGA address
				nTempChannel = (UInt_t)strtoul(tokens.at(1).c_str(),NULL,10); // decode TDC channel
				if(ExcludeChannel(nTempAddress,nTempChannel)) // try to enter channel into map containing excluded channels
					++nExcludedChannels; // increment counter of excluded channels
				break;
			default:
				continue; // do nothing
		}
	} // end loop over input file
	UserInputFile.close();
	return(nExcludedChannels);
}

void TTrbAnalysisBase::FillSyncTimeStamps(){ // find sync timestamps in an event and write to EvtSyncTimeStamps map, return value
	EvtSyncTimeStamps.clear(); // clear sync timestamps map
	// loop over Hits_nTdcChannel array and extract time whenever we find the reference channel
	for(Int_t nCurEvtIndex=0; nCurEvtIndex<TrbData->Hits_; ++nCurEvtIndex){ // loop over all hits in entry
		if(!TrbData->Hits_bIsRefChannel[nCurEvtIndex]) // check for TDC reference channels
			continue; // skip rest of loop
		EvtSyncTimeStamps.insert(make_pair((UInt_t)TrbData->Hits_nTrbAddress[nCurEvtIndex],(UInt_t)nCurEvtIndex)); // fill TDC address and array index into map
	}
	//if(TdcRefTimes.size()!=TrbAddresses.size())
	//	return (kFALSE);
	//return (kTRUE);
}

Int_t TTrbAnalysisBase::GetEntry(Long64_t nEntryIndex){
	// Get event entry from TTree object and perform basic analysis tasks
	ClearEventMaps(); // reset all event-level variables and maps
	Int_t nEntrySize = TrbData->GetEntry(nEntryIndex); // retrieve data from tree
	if(nEntrySize<1) // if entry is invalid return now
		return (nEntrySize);
	if(bVerboseMode){
		cout << "Getting entry " << nEntryIndex << " with size "<< nEntrySize << endl;
	}
	FillSyncTimeStamps();
	//bAllRefChanValid = SetRefTimestamps(); // extract reference timestamps
	//FillTdcHits(); // fill all TDC hits into multimap, reference channels are excluded
	//if(!TdcHits.empty()){ // if there are any TDC hits, do basic analysis tasks
	//	FillTdcLeadingEdge(); // correct leading edge timestamps for reference time and fill into map
	//	nEvtMultHits = HitMatching(); // try matching leading and trailing edges and fill array indices into map, skipping multi-hit channels
	//	FillTimeOverThreshold(); // compute pulse lengths based on matched hits and fill into map
	//}
	return (nEntrySize);
}

Int_t TTrbAnalysisBase::GetSeqId(UInt_t nUserTdcAddress, UInt_t nUserTdcChannel){ // get sequential channel number
	std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator RequestedEntry = MappingTable.find(make_pair(nUserTdcAddress,nUserTdcChannel));
	if(RequestedEntry==MappingTable.end()){
		return (-1);
	}
	return (RequestedEntry->second);
}

void TTrbAnalysisBase::Init(){
	TrbData = NULL;
	// intialise setup specific variables
	//nTdcOffset	= 0;
	nEventsMax	= -1; // number of events in data set
	//nMaxTdcChannel	= 0; // unique index of last channel
	// initialise event level variables
	//bAllRefChanValid = kFALSE;
	//nEvtMultHits	= 0; // number of channels with multiple hits
	// clear all containers
	TdcAddresses.clear();
	MappingTable.clear();
	ExcludedChannels.clear();
	EvtChanEntries.clear();
}

Bool_t TTrbAnalysisBase::OpenTrbTree(string cUserDataFilename){
	if(cUserDataFilename.empty()){
		cerr << "TRB data filename is empty!" << endl;
		return (kFALSE);
	}
	TFile *TrbTreeFile = new TFile(cUserDataFilename.c_str());
	if(TrbTreeFile->IsZombie()){
		cerr << "Error opening TRB data file " << cUserDataFilename << endl;
		return (kFALSE);
	}
	TTree *TrbTree = (TTree*)TrbTreeFile->Get("T");
	TrbData = new TTrbDataTree(TrbTree);
	nEventsMax = (Int_t)TrbData->fChain->GetEntriesFast();
	if(bVerboseMode)
		cout << "TrbTree opened with " << nEventsMax << " entries" << endl;
	return (kTRUE);
}

void TTrbAnalysisBase::PrintSyncTimeStamps() const {
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "+++ TDC Sync Timestamps +++" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << EvtSyncTimeStamps.size() << " SYNC TIMESTAMPS FOUND" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	std::map< UInt_t,UInt_t >::const_iterator LastEntry = EvtSyncTimeStamps.end();
	for(std::map< UInt_t,UInt_t >::const_iterator CurEntry=EvtSyncTimeStamps.begin(); CurEntry!=LastEntry; ++CurEntry){ // begin of loop over all sync timestamps
		cout << hex << CurEntry->first << dec << " , " << CurEntry->second << " :\t" << TrbData->Hits_fTime[CurEntry->second] << endl;
	} // end of loop over all sync timestamps
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TTrbAnalysisBase::PrintTdcAddresses() const {
	Int_t nTdcIndex = 0;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "+++    TDC ADDRESSES    +++" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	for(std::map< UInt_t,UInt_t >::const_iterator CurIndex=TdcAddresses.begin(); CurIndex!=TdcAddresses.end(); ++CurIndex){
		cout << nTdcIndex << "\t" << hex << CurIndex->first << dec << "\t" << CurIndex->second << endl;
		++nTdcIndex;
	}
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TTrbAnalysisBase::SetTdcAddresses(string cUserTdcAddressesFile){ // set TRB addresses
	TdcAddresses.clear(); // clear TRB address list
	if(cUserTdcAddressesFile.empty()){ // check if TRB address string is empty
		if(bVerboseMode)
			cout << "TDC address string is empty!" << endl;
		return;
	}
	ifstream UserInputFile(cUserTdcAddressesFile.c_str(),ifstream::in); // open text file containing TRB endpoint addresses
	if(!UserInputFile.is_open()){ // check if opening text file was successful
		cerr << "Could not open TDC addresses file " << cUserTdcAddressesFile << endl;
		return;
	}
	while(UserInputFile.good()){ // start loop over input file
		string cCurrentLine;
		getline(UserInputFile,cCurrentLine); // get line from input file
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		vector<string> tokens = LineParser(cCurrentLine,' ',bVerboseMode);
		UInt_t nTempTdcAddress = (UInt_t)strtoul(tokens.at(0).c_str(),NULL,16); // decode FPGA address, expected to be hexadecimal
		UInt_t nTempTdcSize;
		switch (tokens.size()) {
			case 1: // we only expect one address per line
				TdcAddresses.insert(make_pair(nTempTdcAddress,nChanPerTdc));
				break;
			case 2: // first token is TDC address, second token is number of TDC channels
				nTempTdcSize = (UInt_t)strtoul(tokens.at(1).c_str(),NULL,10); // size of TDC
				TdcAddresses.insert(make_pair(nTempTdcAddress,nTempTdcSize));
			default: // anything with more than one token per line will be ignored!
				continue; // do nothing
		}
	} // end loop over input file
	UserInputFile.close(); // close text file
	if(bVerboseMode){
		cout << TdcAddresses.size() << " TDC endpoint addresses decoded." << endl;
		PrintTdcAddresses();
	}
	//nMaxTdcChannel = (Int_t)TdcAddresses.size() * N_TDC_CHAN;
}

void TTrbAnalysisBase::WriteTdcMapping(string cUserMappingFile) {
	if(cUserMappingFile.empty())
		return;
	ofstream MappingFileOut(cUserMappingFile.c_str()); // open text file for writing mapping scheme
	std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator LastTdc = MappingTable.end();
	for(std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator CurrentTdc=MappingTable.begin(); CurrentTdc!=LastTdc; ++CurrentTdc){ // begin loop over mapping table
		MappingFileOut << hex << CurrentTdc->first.first << dec << "\t" << CurrentTdc->first.second << "\t" << CurrentTdc->second << endl;
	//	} // end of loop over TDC channels
	} // end of loop over mapping table
	MappingFileOut.close();
}