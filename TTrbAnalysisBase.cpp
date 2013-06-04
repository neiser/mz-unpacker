#include "TTrbAnalysisBase.h"

ClassImp(TTrbAnalysisBase);

TTrbAnalysisBase::TTrbAnalysisBase(string cUserDataFilename, Bool_t bUserVerboseMode, string cUserTreeName) : TObject(), cDataFilename(cUserDataFilename), bVerboseMode(bUserVerboseMode), cTreeName(cUserTreeName){
	Init(); // initialise variables
	OpenTree(); // open tree with data
}

TTrbAnalysisBase::~TTrbAnalysisBase(){
	cout << "This is the destructor of the TTrbAnalysisBase class..." << endl;
	delete TrbData;
	TrbData = NULL;
	delete RawData;
	RawData = NULL;
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
				if(TrbData->Hits_nSubEvtId[i]!=nRefRndmBits){ // check if current sequence doesn't equal reference sequence
					return (kFALSE);
				}
			} // end of loop over all hits in entry
			return (kTRUE); // all sequences are the same
	}
	return (kFALSE);
}

Bool_t TTrbAnalysisBase::CheckDecodingStatus(UInt_t nUserStatus) const {
	for(Int_t i=0; i<TrbData->Hits_; ++i){
		if(TrbData->nSubEvtDecError!=nUserStatus)
			return (kFALSE);
	}
	return (kTRUE);
}

void TTrbAnalysisBase::ComputeMappingTable(){
	MappingTable.clear();
	UpdateStatus();
	if(TdcAddresses.empty())
		return;
	// loop over all TDC addresses
	std::map< UInt_t,UInt_t >::const_iterator FirstTdcAddress = TdcAddresses.begin();
	std::map< UInt_t,UInt_t >::const_iterator LastTdcAddress = TdcAddresses.end();
	for(std::map< UInt_t,UInt_t >::const_iterator CurrentTdc=FirstTdcAddress; CurrentTdc!=LastTdcAddress; ++CurrentTdc){ // begin loop over all TDC addresses
		UInt_t nTdcIndex = (UInt_t)distance(FirstTdcAddress,CurrentTdc);
		for(UInt_t i=0; i<CurrentTdc->second; ++i){ // begin loop over all TDC channels
			UInt_t nSeqId = nTdcIndex * CurrentTdc->second + i;
			MappingTable.insert(make_pair(make_pair(CurrentTdc->first,i+nTdcOffset),nSeqId));
		} // end of loop over all TDC channels
	} // end of loop over all TDC addresses
	if(bVerboseMode){
		PrintTdcMapping();
	}
	UpdateStatus();
}

Bool_t TTrbAnalysisBase::ExcludeChannel(UInt_t nUserTrbAddress, UInt_t nUserTdcChannel){
	std::pair< UInt_t,UInt_t > TempPair (nUserTrbAddress, nUserTdcChannel); // create pair consisting of FPGA address and TDC channel
	return ((Bool_t)(ExcludedChannels.insert(TempPair)).second);

	//std::vector< pair< UInt_t,UInt_t > >::iterator CheckPair; // iterator on vector containing excluded channel addresses
	//CheckPair = find(ExcludedChannels.begin(),ExcludedChannels.end(),TempPair); // check if channel is already excluded
	//if(CheckPair!=ExcludedChannels.end())
	//	return (kFALSE);
	//ExcludedChannels.push_back(TempPair); // enter this channel into vector
	//return(kTRUE);
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

Int_t TTrbAnalysisBase::GetEntry(Long64_t nEntryIndex){
	// Get event entry from TTree object and perform basic analysis tasks
	//ClearEventMaps(); // reset all event-level variables and maps
	Int_t nEntrySize = TrbData->GetEntry(nEntryIndex); // retrieve data from tree
	if(nEntrySize<1) // if entry is invalid return now
		return (nEntrySize);
	if(bVerboseMode){
		cout << "Getting entry " << nEntryIndex << " with size "<< nEntrySize << endl;
	}
	ScanEvent();
	//bAllRefChanValid = SetRefTimestamps(); // extract reference timestamps
	//FillTdcHits(); // fill all TDC hits into multimap, reference channels are excluded
	//if(!TdcHits.empty()){ // if there are any TDC hits, do basic analysis tasks
	//	FillTdcLeadingEdge(); // correct leading edge timestamps for reference time and fill into map
	//	nEvtMultHits = HitMatching(); // try matching leading and trailing edges and fill array indices into map, skipping multi-hit channels
	//	FillTimeOverThreshold(); // compute pulse lengths based on matched hits and fill into map
	//}
	return (nEntrySize);
}

Int_t TTrbAnalysisBase::GetSeqId(UInt_t nUserTdcAddress, UInt_t nUserTdcChannel) const { // get sequential channel number
	std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator RequestedEntry = MappingTable.find(make_pair(nUserTdcAddress,nUserTdcChannel));
	if(RequestedEntry==MappingTable.end()){
		return (-1);
	}
	return (RequestedEntry->second);
}

Int_t TTrbAnalysisBase::GetTdcSyncIndex(UInt_t nTdcAddress) const {
	if(EvtSyncTimestamps.empty())
		return (-1);
	std::map< UInt_t,UInt_t >::const_iterator FoundTdc = EvtSyncTimestamps.find(nTdcAddress);
	if(FoundTdc==EvtSyncTimestamps.end()) // TDC address was not found in sync timestamp map
		return (-1);
	return (FoundTdc->second);
}


void TTrbAnalysisBase::Init(){ // initialise object variables
	cout << "This is TTrbAnalysisBase::Init()..." << endl;
	RawData = NULL; // initialise pointer to RooT file
	TrbData = NULL; // initialise pointer TTree with raw data
	nTdcDefaultSize	= 0; // these need to be variables set by the user
	nTdcOffset		= 0; // see above, need to change this later

	bTreeIsOpen = kFALSE;
	bCanAnalyse	= kFALSE;
	bDoHitMatching = kFALSE; // do not match hits in the TDC, i.e. leading and trailing edges

	ExcludedChannels.clear();
	TdcAddresses.clear();
	MappingTable.clear();
	EvtSyncTimestamps.clear();
	EvtTdcHits.clear();

}

void TTrbAnalysisBase::OpenTree(){ // open file and retrieve pointer to tree
	if(cDataFilename.empty()){
		cerr << "TRB data filename is empty!" << endl;
		return;
	}
	RawData = new TFile(cDataFilename.c_str());
	if(RawData->IsZombie()){
		cerr << "Error opening TRB data file " << cDataFilename << endl;
		return;
	}
	TrbData = new TTrbDataTree((TTree*)RawData->Get(cTreeName.c_str()));
	if (TrbData==NULL){
		cerr << "Error opening Tree " << cTreeName << endl;
		return;
	}
	bTreeIsOpen = kTRUE;
}

void TTrbAnalysisBase::PrintExcludedChannels() const {
	if(ExcludedChannels.empty())
		return;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "+++  EXCLUDED CHANNELS  +++" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	std::set< std::pair< UInt_t,UInt_t > >::const_iterator FirstIndex = ExcludedChannels.begin();
	std::set< std::pair< UInt_t,UInt_t > >::const_iterator LastIndex = ExcludedChannels.end();
	for(std::set< std::pair< UInt_t,UInt_t > >::const_iterator CurIndex=FirstIndex; CurIndex!=LastIndex; ++CurIndex){
		cout << distance(FirstIndex,CurIndex) << "\t" << hex << CurIndex->first << dec << "\t" << CurIndex->second << endl;
		//++nTdcIndex;
	}
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TTrbAnalysisBase::PrintSyncTimestamps() const {
	if(EvtSyncTimestamps.empty())
		return;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "+++ TDC Sync Timestamps +++" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << EvtSyncTimestamps.size() << " SYNC TIMESTAMPS FOUND" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	std::map< UInt_t,UInt_t >::const_iterator LastEntry = EvtSyncTimestamps.end();
	for(std::map< UInt_t,UInt_t >::const_iterator CurEntry=EvtSyncTimestamps.begin(); CurEntry!=LastEntry; ++CurEntry){ // begin of loop over all sync timestamps
		cout << hex << CurEntry->first << dec << " , " << CurEntry->second << " :\t" << TrbData->Hits_fTime[CurEntry->second] << endl;
	} // end of loop over all sync timestamps
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TTrbAnalysisBase::PrintTdcAddresses() const {
	if(TdcAddresses.empty()) // no TDC addresses available
		return;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "+++    TDC ADDRESSES    +++" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	std::map< UInt_t,UInt_t >::const_iterator FirstIndex = TdcAddresses.begin();
	std::map< UInt_t,UInt_t >::const_iterator LastIndex = TdcAddresses.end();
	for(std::map< UInt_t,UInt_t >::const_iterator CurIndex=FirstIndex; CurIndex!=LastIndex; ++CurIndex){
		cout << distance(FirstIndex,CurIndex) << "\t" << hex << CurIndex->first << dec << "\t" << CurIndex->second << endl;
		//++nTdcIndex;
	}
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TTrbAnalysisBase::PrintTdcHits() const{
	if(EvtTdcHits.empty())
		return;
	std::multimap< UInt_t,UInt_t >::const_iterator FirstEntry = EvtTdcHits.begin();
	std::multimap< UInt_t,UInt_t >::const_iterator LastEntry = EvtTdcHits.end();
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "+++   Event TDC Hits    +++" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << EvtTdcHits.size() << " TDC HITS FOUND" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	for(std::multimap< UInt_t,UInt_t >::const_iterator CurEntry=FirstEntry; CurEntry!=LastEntry; ++CurEntry){ // begin loop over all event hit entries
		cout << std::distance(FirstEntry,CurEntry) << "\t" << CurEntry->first << "\t" << CurEntry->second << endl;
	} // end of loop over all event hit entries
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TTrbAnalysisBase::PrintTdcMapping() const {
	if(MappingTable.empty())
		return;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "+++     TDC Mapping     +++" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator LastTdc = MappingTable.end();
	for(std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator CurrentTdc=MappingTable.begin(); CurrentTdc!=LastTdc; ++CurrentTdc){ // begin loop over mapping table
		cout << hex << CurrentTdc->first.first << dec << "\t" << CurrentTdc->first.second << "\t" << CurrentTdc->second << endl;
	//	} // end of loop over TDC channels
	} // end of loop over mapping table
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TTrbAnalysisBase::ScanEvent(){
	EvtSyncTimestamps.clear(); // clear sync timestamps map
	EvtTdcHits.clear(); // clear hit index map
	// loop over Hits_nTdcChannel array and extract time whenever we find the reference channel
	for(Int_t nCurEvtIndex=0; nCurEvtIndex<TrbData->Hits_; ++nCurEvtIndex){ // loop over all hits in entry
		if(!TrbData->Hits_bIsCalibrated[nCurEvtIndex]){ // skip uncalibrated channels
			if(bVerboseMode)
				cout << "Entry at index " << nCurEvtIndex << " not calibrated, thus skipping it!" << endl;
			continue; // skip rest of loop
		}
		std::pair< UInt_t,UInt_t > TempChanAddress (TrbData->Hits_nTrbAddress[nCurEvtIndex],TrbData->Hits_nTdcChannel[nCurEvtIndex]); // create address pair consisting of FPGA address and TDC channel ID
//		if(find(ExcludedChannels.begin(),ExcludedChannels.end(),TempChanAddress)!=ExcludedChannels.end()){ // channel found in exclusion list
		if(ExcludedChannels.find(TempChanAddress)!=ExcludedChannels.end()){ // channel found in exclusion list
			continue; // skip rest of loop
		}
		if(TrbData->Hits_bIsRefChannel[nCurEvtIndex]){ // check for TDC reference channels
			if(TdcAddresses.find(TempChanAddress.first)!=TdcAddresses.end())
				EvtSyncTimestamps.insert(make_pair(TempChanAddress.first,(UInt_t)nCurEvtIndex)); // fill TDC address and array index into map
			continue; // skip rest of loop
		}
		else if(TrbData->Hits_nTdcChannel[nCurEvtIndex]<nTdcOffset){ // skip this entry as it is not part of the active TDC channels
			continue; // skip rest of loop
		}
		else{
			Int_t nTempSeqId = GetSeqId(TempChanAddress.first,TempChanAddress.second);
			if(nTempSeqId<0){ // channel address not found in mapping table (should not happen)
				continue;
			}
			EvtTdcHits.insert(make_pair((UInt_t)nTempSeqId,(UInt_t)nCurEvtIndex));
		}
	} // end of loop over all hits in this entry
	if(bVerboseMode){
		PrintSyncTimestamps();
		PrintTdcHits();
		//cout << GetSizeOfEvtEntryMap() << " TDC entries accepted." << endl;
	}
}


Int_t TTrbAnalysisBase::SetTdcAddresses(string cUserTdcAddressesFile){ // set TRB addresses
	TdcAddresses.clear(); // clear TRB address list
	UpdateStatus();
	if(cUserTdcAddressesFile.empty()){ // check if TRB address string is empty
		if(bVerboseMode)
			cout << "TDC address string is empty!" << endl;
		return (-1);
	}
	ifstream UserInputFile(cUserTdcAddressesFile.c_str(),ifstream::in); // open text file containing TRB endpoint addresses
	if(!UserInputFile.is_open()){ // check if opening text file was successful
		cerr << "Could not open TDC addresses file " << cUserTdcAddressesFile << endl;
		return (-1);
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
				if(nTdcDefaultSize<1)
					continue; // skip as TDC has no channels
				TdcAddresses.insert(make_pair(nTempTdcAddress,nTdcDefaultSize));
				break;
			case 2: // first token is TDC address, second token is number of TDC channels
				nTempTdcSize = (UInt_t)strtoul(tokens.at(1).c_str(),NULL,10); // size of TDC
				if(nTempTdcSize<1)
					continue; // skip as TDC has no channels
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
	if(!TdcAddresses.empty())
		ComputeMappingTable();
	UpdateStatus();
	return ((Int_t)TdcAddresses.size());
}

void TTrbAnalysisBase::Show(Int_t nEventIndex){
	if(!bTreeIsOpen)
		return;
	TrbData->Show(nEventIndex);
}

void TTrbAnalysisBase::UpdateStatus(){
	if(!GetTreeStatus()){
		bCanAnalyse = kFALSE;
		return;
	}
	if(TdcAddresses.empty()){
		bCanAnalyse = kFALSE;
		return;
	}
	if(MappingTable.empty()){
		bCanAnalyse = kFALSE;
		return;
	}
	bCanAnalyse = kTRUE;
}

void TTrbAnalysisBase::WriteTdcMapping(string cUserMappingFile) {
	if(cUserMappingFile.empty())
		return;
	ofstream MappingFileOut(cUserMappingFile.c_str()); // open text file for writing mapping scheme
	std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator LastTdc = MappingTable.end();
	for(std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator CurrentTdc=MappingTable.begin(); CurrentTdc!=LastTdc; ++CurrentTdc){ // begin loop over mapping table
		MappingFileOut << hex << CurrentTdc->first.first << dec << "\t" << CurrentTdc->first.second << "\t" << CurrentTdc->second << endl;
	} // end of loop over mapping table
	MappingFileOut.close();
}