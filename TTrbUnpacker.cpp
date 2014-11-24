#include "TTrbUnpacker.h"

ClassImp(TTrbUnpacker);

TTrbUnpacker::TTrbUnpacker(string cUserHldFilename, UInt_t cUserSubEventId, UInt_t cUserCtsAddress,
                           string cUserHubAddressesFile, string cUserTdcAddressesFile,
                           UInt_t nUserTdcRefChannel, Bool_t bUserPadding, Bool_t bUserVerboseMode, Bool_t bUserSkipSubEvents) : TObject(){ // standard constructor
	if(sizeof(UInt_t) != SIZE_OF_DATAWORD){ // check size of UInt_t (should be 4 bytes)
		cerr << "Size of UInt_t and HLD data word do NOT match!" << endl;
		exit (-1);
	}
	bApplyPadding	= bUserPadding; // set Padding method
	bVerboseMode	= bUserVerboseMode; // set verbose mode flag
	bSkipSubEvents	= bUserSkipSubEvents; // set skip subevents flag
	Init(); // initialise variables
	SetHldFilename(cUserHldFilename); // set HLD filename
	if(!OpenHldFile()){ // open HLD file containing raw data
		cerr << "Error open HLD file " << cHldFilename << " !" << endl;
		exit (-1);
	}

	TrbSettings.nSubEvtIds.push_back(cUserSubEventId);
	TrbSettings.nCtsAddress = cUserCtsAddress;
	
	
	if(!cUserHubAddressesFile.empty() && SetHubAddresses(cUserHubAddressesFile)<0) { // setting addresses of HUB TRB boards, can be empty! 
		cerr << "Error decoding Hub addresses from file " << cUserHubAddressesFile << endl;
		exit (-1);
	}
	if(SetTdcAddresses(cUserTdcAddressesFile)<1) { // setting addresses of TRB boards with TDC, need at least 1 address
		cerr << "Error decoding TDC addresses from file " << cUserTdcAddressesFile << endl;
		exit (-1);
	}
	CheckHubTdcAddresses(); // Check if HUBs and endpoints are consistent

	TrbSettings.nTdcRefChannel = nUserTdcRefChannel;
	
	if(bVerboseMode)
		PrintUnpackerSettings();
	SetRootFilename(); // set RooT output filename
	SetLogFilename(); // set logfile name
	IndexEvents(); // get positions of events within HLD file
}

TTrbUnpacker::TTrbUnpacker(string cUserHldFilename, string cUserSubEventIdFile, UInt_t cUserCtsAddress, string cUserHubAddressesFile, string cUserTdcAddressesFile, UInt_t nUserTdcRefChannel, Bool_t bUserPadding, Bool_t bUserVerboseMode, Bool_t bUserSkipSubEvents): TObject(){
	if(sizeof(UInt_t) != SIZE_OF_DATAWORD){ // check size of UInt_t (should be 4 bytes)
		cerr << "Size of UInt_t and HLD data word do NOT match!" << endl;
		exit (-1);
	}
	bApplyPadding	= bUserPadding; // set Padding method
	bVerboseMode	= bUserVerboseMode; // set verbose mode flag
	bSkipSubEvents	= bUserSkipSubEvents; // set skip subevents flag
	Init(); // initialise variables
	SetHldFilename(cUserHldFilename); // set HLD filename
	if(!OpenHldFile()){ // open HLD file containing raw data
		cerr << "Error open HLD file " << cHldFilename << " !" << endl;
		exit (-1);
	}
	TrbSettings.nCtsAddress = cUserCtsAddress;
	if(!cUserSubEventIdFile.empty() && SetSubEvtIds(cUserSubEventIdFile)<0) { // setting addresses of HUB TRB boards, can be empty! 
		cerr << "Error decoding subevent IDs from file " << cUserSubEventIdFile << endl;
		exit (-1);
	}
	if(!cUserHubAddressesFile.empty() && SetHubAddresses(cUserHubAddressesFile)<0) { // setting addresses of HUB TRB boards, can be empty! 
		cerr << "Error decoding Hub addresses from file " << cUserHubAddressesFile << endl;
		exit (-1);
	}
	if(SetTdcAddresses(cUserTdcAddressesFile)<1) { // setting addresses of TRB boards with TDC, need at least 1 address
		cerr << "Error decoding TDC addresses from file " << cUserTdcAddressesFile << endl;
		exit (-1);
	}
	CheckHubTdcAddresses(); // Check if HUBs and endpoints are consistent

	TrbSettings.nTdcRefChannel = nUserTdcRefChannel;
	
	if(bVerboseMode)
		PrintUnpackerSettings();
	SetRootFilename(); // set RooT output filename
	SetLogFilename(); // set logfile name
	IndexEvents(); // get positions of events within HLD file

}

TTrbUnpacker::~TTrbUnpacker(){ // destructor
	if(bVerboseMode)
		cout << "Calling TTrbUnpacker destructor..." << endl;
	CloseHldFile(); // close HLD file
	CloseLogFile(); // close logfile
}

void TTrbUnpacker::CheckHubTdcAddresses() {
	if(TrbSettings.nHubAddress.empty())
		return;
	// we should find a matching hub address (regarding upper 28 bits)
	// for each TDC endpoint. If we don't, print a WARNING!
	for(unsigned i=0; i<TrbSettings.nTdcAddress.size(); i++) {
		UInt_t TdcAddressUpper28bits = TrbSettings.nTdcAddress[i] & 0xFFF0;
		Bool_t foundIt = kFALSE;
		for(unsigned j=0; j<TrbSettings.nHubAddress.size(); j++) {
			UInt_t hubAddress = TrbSettings.nHubAddress[j];
			if(hubAddress == TdcAddressUpper28bits)
				foundIt = kTRUE;
		}
		if(!foundIt)
			cout << "WARNING: TDC endpoint address " << hex << TrbSettings.nTdcAddress[i]
			     <<" not fitting to HUB addresses list (forgot to add Hub address?)" << endl;
	}
}

Bool_t TTrbUnpacker::CloseHldFile(){ // close ifstream connected to HLD file
	if(InputHldFile.is_open()){ // check if ifstream is connected to a file
		InputHldFile.close(); // close connection to file
		cHldFilename.clear(); // clear HLD file name string
		return (kTRUE);
	}
	return (kFALSE);
}

Bool_t TTrbUnpacker::CloseLogFile(){
	if(LogFile.is_open()){
		//clog.rdbuf(ClogBackup);
		//ClogBackup = NULL;
		LogFile.close();
		cLogFilename.clear();
		return (kTRUE);
	}
	return (kFALSE);
}

Bool_t TTrbUnpacker::CreateTree(){
	if(OutputTree!=NULL){
		delete OutputTree;
		OutputTree = NULL;
	}
	OutputTree = new TTree("T","UMainz TRB HLD Data",2);
	OutputTree->SetAutoSave(TREE_AUTOSAVE);
	return (!OutputTree->IsZombie());
}

UInt_t TTrbUnpacker::Decode(UInt_t nUserEvents, UInt_t nUserOffset) { // decode HLD raw data
	TClonesArray Hits;
	Hits.SetClass("TTrbHit",1000); // declare that TTrbHits are being stored in the TClonesArray
	UInt_t nDecodedEvents = 0; // counter of decoded events
	RewindFile(); // set get-pointer to beginning of HLD file
	if(nUserOffset>nEvtIndex.size()) {// user offset outside invent index range
		cerr << "Offset exceeds number of events" << endl;
		return 0;
	}
	if(nUserEvents==0) // decode all events from UserOffset onwards
		nUserEvents=nEvtIndex.size()-nUserOffset;
	InputHldFile.seekg(nEvtIndex.at(nUserOffset),ios::beg); // set file get pointer to first event
	Bool_t bLogUnpacking = OpenLogFile(); // open log file
	//(void)bLogUnpacking;
	if(bLogUnpacking)
		WriteSettingsToLog();
	//PrintTdcAddresses(bLogUnpacking);
	if(!OpenRootFile()){
		cerr << "Error open RooT file " << cRootFilename << " !" << endl;
		delete OutputRootFile;
		return (0);
	}
	if(!CreateTree()){
		delete OutputRootFile;
		return (0);
	}
	TTrbEventData *CurrentEventData = new TTrbEventData(Hits);
	OutputTree->Branch("event","TTrbEventData",&CurrentEventData);
	THldEvent ThisEvent(&InputHldFile,&TrbSettings,&Hits,bVerboseMode,bSkipSubEvents);
	// measure time needed for unpacking
	time_t StartTime; 
	time(&StartTime); // get time before entering analysis loop
	for(UInt_t i=0; i<nUserEvents; i++){
		//cout << i << endl;
		Hits.Clear("C"); // clear TRB hits array
		if(!ThisEvent.ReadIt(bApplyPadding)){
			// realign data stream to next event
			if(!InputHldFile.good()) // error reading from file
				break; // do I need to break here? I could just not fill this event and realign to next position?
			else if(!RealignDatastream()) // try to realign data stream
				break; // failed to realign, stop decoding
			continue; // skip rest of loop, i.e. do not put this data into the output tree
		}
		CurrentEventData->Fill(ThisEvent); // fill tree branches with decoded hit data
		nDecodedEvents++;
		OutputTree->Fill(); // fill output tree
	}
	time_t StopTime;
	time(&StopTime); // time when unpacking is finished
	Double_t fRunTime = difftime(StopTime,StartTime);
	// write information to log file
	LogFile << "First event:\t" << nUserOffset << endl;
	LogFile << "No of decoded events:\t" << nUserEvents << endl;
	LogFile << "Unpacking duration " << fRunTime << " seconds" << endl;
	cout << "Unpacking took " << fRunTime << " seconds to complete" << endl;
	cout << "Avg time for unpacking an event " << fRunTime/(Double_t)nDecodedEvents << "s" << endl;
	// write out put to disk
	cout << "Writing Tree to disk..." << endl;
	OutputRootFile = OutputTree->GetCurrentFile();
	OutputRootFile->Write();
	cout << "Cleaning up after decoding..." << endl;
	cout << "Deleting Tree..." << std::hex << std::showbase << OutputTree << std::noshowbase << dec << endl;
	delete OutputTree;
	OutputTree = NULL;
	cout << "Deleting RooT file..." << endl;
	delete OutputRootFile;
	OutputRootFile = NULL;
	cout << "Deleting Event Object..." << endl;
	delete CurrentEventData;
	Hits.Clear();
	cout << "Returning to terminal..." << endl;
	CloseLogFile();
	return (nDecodedEvents);
}

void TTrbUnpacker::IndexEvents(){
	RewindFile(); // set get pointer to beginning of HLD file
	THldEvent DummyEvent(&InputHldFile,&TrbSettings,NULL,kFALSE,kTRUE); // skip subevent decoding part
	while(InputHldFile.good()){ // begin of loop over HLD file
		Int_t nTempEvtIndex = InputHldFile.tellg();
		if(!DummyEvent.ReadIt(bApplyPadding)){
			break;
		}
		nEvtIndex.push_back(nTempEvtIndex); // put file get pointer position into vector, index is event number
		InputHldFile.peek();
	} // end of loop over HLD file
	InputHldFile.clear(); // clear EOF and other ifstream failbits
	RewindFile(); //  set file get pointer to beginning of HLD file
	if(bVerboseMode)
		cout << "Found " << nEvtIndex.size() << " events in HLD file" << endl;
}

void TTrbUnpacker::Init(){ // initialise unpacker
	cHldFilename.clear(); // clear HLD filename string
	cLogFilename.clear(); // clear logfile name string
	cRootFilename.clear(); // clear RooT output filename

	cTdcAddresses.clear();
	cTdcAddresses.reserve(NO_OF_TDC_ADDRESSES);
	TrbSettings.nTdcRefChannel = 0; // set TDC reference channel to 0
	TrbSettings.nTdcAddress.clear();
	TrbSettings.nTdcAddress.reserve(NO_OF_TDC_ADDRESSES);
	TrbSettings.nSubEvtIds.clear(); // set subevent ID to 0 (should be 0x8C00 for TRBv3)

	OutputRootFile	= NULL;
	OutputTree		= NULL;

	nEvtIndex.clear();
	nEvtIndex.reserve(10000000);

	//ClogBackup = NULL;
}

Bool_t TTrbUnpacker::OpenHldFile(){ // open HLD file and connect to isftream
	InputHldFile.open(cHldFilename.c_str(), ifstream::in | ifstream::binary); // open HLD file in read-only mode and assume a binary file
	return (!InputHldFile.fail()); // return failbit which indicates any errors when opening the file
}

Bool_t TTrbUnpacker::OpenLogFile(){
	LogFile.open(cLogFilename.c_str(), ios::out);
	return (!LogFile.fail());
}

Bool_t TTrbUnpacker::OpenRootFile(){
	if(OutputRootFile!=NULL)
		delete OutputRootFile;
	OutputRootFile = new TFile(cRootFilename.c_str(),"RECREATE");
	return (!OutputRootFile->IsZombie());
}


void TTrbUnpacker::PrintSubEventIds(){
	std::vector<UInt_t>::const_iterator CurrentSubEvtId;
	CurrentSubEvtId = TrbSettings.nSubEvtIds.begin();
	cout << "+++ Subevent IDs: " << endl;
	for(;CurrentSubEvtId!=TrbSettings.nSubEvtIds.end(); CurrentSubEvtId++){
		cout << hex << *CurrentSubEvtId << dec << endl;
	}
}


void TTrbUnpacker::PrintCtsAddress(){
	cout << "Trigger Control System (TCS) is at " << hex << showbase << TrbSettings.nCtsAddress << dec << endl;
}


void TTrbUnpacker::PrintHubAddresses(Bool_t bWriteToLog){
	streambuf *backup;
	if(bWriteToLog){
		backup = cout.rdbuf();
		cout.rdbuf(LogFile.rdbuf());
	}
	std::vector<string>::const_iterator CurrentHubString;
	CurrentHubString = cHubAddresses.begin();
	std::vector<UInt_t>::const_iterator CurrentHubUInt;
	CurrentHubUInt = TrbSettings.nHubAddress.begin();
	cout << "+++ HUB addresses: " << endl;
	for(;CurrentHubString!=cHubAddresses.end(); CurrentHubString++, CurrentHubUInt++){
		cout << *CurrentHubString << ", " << hex << *CurrentHubUInt << ", " << dec << *CurrentHubUInt << endl;
	}	
	if(bWriteToLog){
		cout.rdbuf(backup);
		backup = NULL;
	}
}

void TTrbUnpacker::PrintTdcAddresses(Bool_t bWriteToLog){
	streambuf *backup;
	if(bWriteToLog){
		backup = cout.rdbuf();
		cout.rdbuf(LogFile.rdbuf());
	}
	std::vector<string>::const_iterator CurrentTdcString;
	CurrentTdcString = cTdcAddresses.begin();
	std::vector<UInt_t>::const_iterator CurrentTdcUInt;
	CurrentTdcUInt = TrbSettings.nTdcAddress.begin();
	cout << "+++ TDC addresses: " << endl;
	for(;CurrentTdcString!=cTdcAddresses.end(); CurrentTdcString++,CurrentTdcUInt++){
		cout << *CurrentTdcString << ", " << hex << *CurrentTdcUInt << ", " << dec << *CurrentTdcUInt << endl;
	}
	if(bWriteToLog){
		cout.rdbuf(backup);
		backup = NULL;
	}
}

void TTrbUnpacker::PrintTdcRefChannel(){
	cout << "TDC reference channel set to: " << TrbSettings.nTdcRefChannel << endl;
}

void TTrbUnpacker::PrintUnpackerSettings(){
	PrintSubEventIds();
	PrintCtsAddress();
	PrintHubAddresses();
	PrintTdcAddresses();
	PrintTdcRefChannel();
}

Bool_t TTrbUnpacker::RealignDatastream(){
	if(bVerboseMode){
		cout << "Realigning data stream to next event..." << endl;
	}
	Int_t nCurrentStreamPos = InputHldFile.tellg(); // get current position in data stream
	std::vector<Int_t>::iterator NextEvent;
	NextEvent = std::find(nEvtIndex.begin(),nEvtIndex.end(),nCurrentStreamPos);
	if(NextEvent!=nEvtIndex.end()){ // data stream already aligned, no action needed
		if(bVerboseMode)
			cout << nCurrentStreamPos << " -> " << *NextEvent << endl;
		return (kTRUE);
	}
	else{
		NextEvent = std::upper_bound(nEvtIndex.begin(),nEvtIndex.end(),nCurrentStreamPos);
		if(NextEvent!=nEvtIndex.end()){
			if(bVerboseMode)
				cout << nCurrentStreamPos << " -> " << *NextEvent << endl;
			InputHldFile.seekg(*NextEvent,ios::beg); // set file get pointer to next event
		}
		else{ // already at end of file
			return (kFALSE);
		}
	}
	return (kTRUE);
}

void TTrbUnpacker::SetLogFilename(){
	cLogFilename = cHldFilename + ".log";
	if(bVerboseMode)
		cout << "Logfile name set to: " << cLogFilename << endl;
}

void TTrbUnpacker::SetRootFilename(){
	cRootFilename = cHldFilename;
	cRootFilename.erase(std::remove_if(cRootFilename.begin(),cRootFilename.end(),::isspace),cRootFilename.end()); // remove any white spaces from ROOT file name
	cRootFilename += ".root";
	if(bVerboseMode)
		cout << "RooT file name set to: " << cRootFilename << endl;
}


Int_t TTrbUnpacker::SetHubAddresses(string cUserHubAddressesFile){
	ifstream UserInputFile(cUserHubAddressesFile.c_str(),ifstream::in);
	while(UserInputFile.good()){ // start loop over input file
		string cCurrentLine;
		getline(UserInputFile,cCurrentLine); // get line from input file
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		vector<string> tokens = LineParser(cCurrentLine,' ',bVerboseMode); 
		switch (tokens.size()) {
			case 1: 
				cHubAddresses.push_back(tokens.at(0));
				break;
			default:
				continue; // do nothing => ignore line
		}
	} // end loop over input file
	UserInputFile.close();
	if(bVerboseMode){
		cout << cHubAddresses.size() << " HUB addresses decoded." << endl;
	}
	TrbSettings.nHubAddress.resize(cHubAddresses.size());
	transform(cHubAddresses.begin(),cHubAddresses.end(),TrbSettings.nHubAddress.begin(),HexStringToInt);
	return(cHubAddresses.size());
}

Int_t TTrbUnpacker::SetSubEvtIds(string cUserSubEvtIdFile){ // set subevent IDs using list in file
	ifstream UserInputFile(cUserSubEvtIdFile.c_str(),ifstream::in);
	std::vector<string> cSubEvtIds;
	while(UserInputFile.good()){ // start loop over input file
		string cCurrentLine;
		getline(UserInputFile,cCurrentLine); // get line from input file
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		vector<string> tokens = LineParser(cCurrentLine,' ',bVerboseMode); 
		switch (tokens.size()) {
			case 0: // no tokens on this line
				continue; // do nothing
			case 1: 
				cSubEvtIds.push_back(tokens.at(0));
				break;
			default:
				cSubEvtIds.push_back(tokens.at(0));
				//continue; // do nothing
		}
	} // end loop over input file
	UserInputFile.close();
	if(bVerboseMode){
		cout << cSubEvtIds.size() << " Subevent IDs decoded." << endl;
	}
	TrbSettings.nSubEvtIds.resize(cSubEvtIds.size());
	transform(cSubEvtIds.begin(),cSubEvtIds.end(),TrbSettings.nSubEvtIds.begin(),HexStringToInt);
	return((Int_t)cSubEvtIds.size());
}

Int_t TTrbUnpacker::SetTdcAddresses(string cUserTdcAddressesFile){
	ifstream UserInputFile(cUserTdcAddressesFile.c_str(),ifstream::in);
	while(UserInputFile.good()){ // start loop over input file
		string cCurrentLine;
		getline(UserInputFile,cCurrentLine); // get line from input file
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		vector<string> tokens = LineParser(cCurrentLine,' ',bVerboseMode); 
		switch (tokens.size()) {
			case 0: // no tokens on this line
				continue; // do nothing
			case 1: 
				cTdcAddresses.push_back(tokens.at(0));
				break;
			default:
				cTdcAddresses.push_back(tokens.at(0));
				//continue; // do nothing
		}
	} // end loop over input file
	UserInputFile.close();
	if(bVerboseMode){
		cout << cTdcAddresses.size() << " TDC endpoint addresses decoded." << endl;
	}
	TrbSettings.nTdcAddress.resize(cTdcAddresses.size());
	transform(cTdcAddresses.begin(),cTdcAddresses.end(),TrbSettings.nTdcAddress.begin(),HexStringToInt);
	return(cTdcAddresses.size());
}

void TTrbUnpacker::WriteSettingsToLog(){
	if(!LogFile.is_open()) // log file is not open
		return;
	time_t CurrentTime;
	time(&CurrentTime);
	LogFile << "+++++++++++++++++++++++++++++++" << endl;
	LogFile << "+++ TRBv3 Unpacker Settings +++" << endl;
	LogFile << "+++++++++++++++++++++++++++++++" << endl;
	LogFile << "\t" << ctime(&CurrentTime) << endl;
	// redirect cout buffer to logfile
	std::streambuf *psbuf, *backup;
	backup	= std::cout.rdbuf();
	psbuf	= LogFile.rdbuf();
	std::cout.rdbuf(psbuf);         // assign streambuf to cout
	// print status information to logfile
	PrintUnpackerSettings();
	LogFile << "+++++++++++++++++++++++++++++++" << endl;
	// reset cout buffer to terminal
	std::cout.rdbuf(backup);        // restore cout's original streambuf
	psbuf  = NULL;
	backup = NULL;
}
