#include "TTrbUnpacker.h"

ClassImp(TTrbUnpacker);

TTrbUnpacker::TTrbUnpacker(string cUserHldFilename, string cUserSubEventId, string cUserCtsAddress, string cUserTrbAddresses,
                           UInt_t nUserRefChannel, Bool_t bUserVerboseMode, Bool_t bUserSkipSubEvents) : TObject(){ // standard constructor
	if(sizeof(UInt_t) != SIZE_OF_DATAWORD){ // check size of UInt_t (should be 4 bytes)
		cerr << "Size of UInt_t and HLD data word do NOT match!" << endl;
		exit (-1);
	}
	bVerboseMode	= bUserVerboseMode; // set verbose mode flag
	bSkipSubEvents	= bUserSkipSubEvents; // set skip subevents flag
	Init(); // initialise variables
	SetHldFilename(cUserHldFilename); // set HLD filename
	if(!OpenHldFile()){ // open HLD file containing raw data
		cerr << "Error open HLD file " << cHldFilename << " !" << endl;
		exit (-1);
	}
	if(!SetSubEventId(cUserSubEventId)){
		cerr << "No subevent ID provided!" << endl;
		exit (-1);
	}
	if(!SetCtsAddress(cUserCtsAddress)){
		cerr << "No Central Trigger System address provided!" << endl;
		exit (-1);
	}
	if(SetTrbAddresses(cUserTrbAddresses)<1){ // setting addresses of TRB boards, need at least 1 address
		cerr << "Error decoding TRB addresses!" << endl;
		exit (-1);
	}
	if(!SetRefChannel(nUserRefChannel)){
		cerr << "Error setting TRB reference channel!" << endl;
	}
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
	RewindFile(); // set get pointer to beginning of HLD file
	if(nUserOffset>nEvtIndex.size()) {// user offset outside invent index range
		cerr << "Offset exceeds number of events" << endl;
		return 0;
	}
	if(nUserEvents==0)
		nUserEvents=nEvtIndex.size()-nUserOffset;
	InputHldFile.seekg(nEvtIndex.at(nUserOffset),ios::beg); // set file get pointer to first event
	Bool_t bLogUnpacking = OpenLogFile(); // open log file
	(void)bLogUnpacking;
	//if(bLogUnpacking)
	//	WriteSettingsToLog();
	//PrintTrbAddresses(bLogUnpacking);
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
	for(UInt_t i=0; i<nUserEvents; i++){
		//cout << i << endl;
		Hits.Clear("C"); // clear TRB hits array
		if(!ThisEvent.ReadIt()){
			break;
		}
		CurrentEventData->Fill(ThisEvent);
		nDecodedEvents++;
		OutputTree->Fill();
	}
	cout << "Writing Tree to disk..." << endl;
	OutputRootFile = OutputTree->GetCurrentFile();
	OutputRootFile->Write();
	cout << "Cleaning up after decoding..." << endl;
	cout << "Deleting Tree..." << hex << OutputTree << dec << endl;
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
		if(!DummyEvent.ReadIt()){
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

	cTrbAddresses.clear();
	cTrbAddresses.reserve(NO_OF_TRB_BOARDS);
	TrbSettings.nSubEventId = 0; // set subevent ID to 0 (should be 0x8C00 for TRBv3)
	TrbSettings.nRefChannel = 0; // set TRB reference channel to 0
	TrbSettings.nTrbAddress.clear();
	TrbSettings.nTrbAddress.reserve(NO_OF_TRB_BOARDS);

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
	//if(LogFile.is_open()){
	//	ClogBackup = clog.rdbuf();
	//	clog.rdbuf(LogFile.rdbuf());
	//}
	return (!LogFile.fail());
}

Bool_t TTrbUnpacker::OpenRootFile(){
	if(OutputRootFile!=NULL)
		delete OutputRootFile;
	OutputRootFile = new TFile(cRootFilename.c_str(),"RECREATE");
	return (!OutputRootFile->IsZombie());
}

void TTrbUnpacker::PrintSubEventId(){
	cout << "Sub event ID is: " << hex << TrbSettings.nSubEventId << dec << endl;
}

void TTrbUnpacker::PrintCtsAddress(){
	cout << "Trigger Control System (TCS) is at 0x" << hex << TrbSettings.nSubEventId << dec << endl;
}


void TTrbUnpacker::PrintTrbAddresses(Bool_t bWriteToLog){
	streambuf *backup;
	if(bWriteToLog){
		backup = cout.rdbuf();
		cout.rdbuf(LogFile.rdbuf());
	}
	std::vector<string>::const_iterator CurrentTrbString;
	CurrentTrbString = cTrbAddresses.begin();
	std::vector<UInt_t>::const_iterator CurrentTrbUInt;
	CurrentTrbUInt = TrbSettings.nTrbAddress.begin();
	for(;CurrentTrbString!=cTrbAddresses.end(); CurrentTrbString++,CurrentTrbUInt++){
		cout << *CurrentTrbString << ", " << hex << *CurrentTrbUInt << ", " << dec << *CurrentTrbUInt << endl;
	}
	if(bWriteToLog){
		cout.rdbuf(backup);
		backup = NULL;
	}
}

void TTrbUnpacker::PrintTrbRefChannel(){
	cout << "TRB reference channel set to: " << TrbSettings.nRefChannel << endl;
}

void TTrbUnpacker::PrintUnpackerSettings(){
	PrintSubEventId();
	PrintCtsAddress();
	PrintTrbRefChannel();
	PrintTrbAddresses();
}

void TTrbUnpacker::SetLogFilename(){
	cLogFilename = cHldFilename + ".log";
	if(bVerboseMode)
		cout << "Logfile name set to: " << cLogFilename << endl;
}

Bool_t TTrbUnpacker::SetRefChannel(UInt_t nUserRefChannel){
	/* 
		implement allowed range of reference channels here
	*/
	TrbSettings.nRefChannel = nUserRefChannel;
	return (kTRUE);
}

void TTrbUnpacker::SetRootFilename(){
	cRootFilename = cHldFilename;
	cRootFilename.erase(std::remove_if(cRootFilename.begin(),cRootFilename.end(),::isspace),cRootFilename.end()); // remove any white spaces from ROOT file name
	cRootFilename += ".root";
	if(bVerboseMode)
		cout << "RooT file name set to: " << cRootFilename << endl;
}

Bool_t TTrbUnpacker::SetSubEventId(string cUserSubEventId){
	if(cUserSubEventId.empty())
		return (kFALSE);
	TrbSettings.nSubEventId = HexStringToInt(cUserSubEventId);
	return (kTRUE);
}

Bool_t TTrbUnpacker::SetCtsAddress(string cUserCtsAddress){
	if(cUserCtsAddress.empty())
		return (kFALSE);
	TrbSettings.nCtsAddress = HexStringToInt(cUserCtsAddress);
	return (kTRUE);
}


Int_t TTrbUnpacker::SetTrbAddresses(string cUserTrbAddresses){
	ifstream UserInputFile(cUserTrbAddresses.c_str(),ifstream::in);
	while(UserInputFile.good()){ // start loop over input file
		string cCurrentLine;
		getline(UserInputFile,cCurrentLine); // get line from input file
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		vector<string> tokens = LineParser(cCurrentLine,' ',bVerboseMode); 
		switch (tokens.size()) {
			case 1: 
				cTrbAddresses.push_back(tokens.at(0));
				break;
			default:
				continue; // do nothing
		}
	} // end loop over input file
	UserInputFile.close();
	if(bVerboseMode){
		cout << cTrbAddresses.size() << " TRB addresses decoded." << endl;
	}
	TrbSettings.nTrbAddress.resize(cTrbAddresses.size());
	transform(cTrbAddresses.begin(),cTrbAddresses.end(),TrbSettings.nTrbAddress.begin(),HexStringToInt);
	return(cTrbAddresses.size());
}


//void TTrbUnpacker::WriteSettingsToLog(){
//	clog << "+++++++++++++++++++++++++++++++" << endl;
//	clog << "+++ TRBv3 Unpacker Settings +++" << endl;
//	clog << "+++++++++++++++++++++++++++++++" << endl;
//	clog << "Subevent ID: " << TrbSettings.nSubEventId << dec << endl;
//	clog << "Reference Channel ID: " << TrbSettings.nRefChannel << endl;
//	clog << "TRB board addresses:" <<  endl;
//	for(std::vector<UInt_t>::const_iterator CurrentTrbUInt=TrbSettings.nTrbAddress.begin(); CurrentTrbUInt!=TrbSettings.nTrbAddress.end(); CurrentTrbUInt++){
//		clog << hex << *CurrentTrbUInt << dec << endl;
//	}
//	clog << "+++++++++++++++++++++++++++++++" << endl;
//}
