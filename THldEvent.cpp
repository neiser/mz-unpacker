#include "THldEvent.h"

ClassImp(THldEvent);

THldEvent::THldEvent(ifstream* UserHldFile, const TRB_SETUP* UserTrbSettings, TClonesArray* UserArray, Bool_t bUserVerboseMode, Bool_t bUserSkipSubEvent) : TObject(){ // constructor
	HldFile		= UserHldFile;
	TrbSettings = UserTrbSettings;
	Hits		= UserArray;
	//nSubEventId		= nUserSubEventId;
	bVerboseMode	= bUserVerboseMode;
	bSkipSubEvent	= bUserSkipSubEvent;
	Init();
}

THldEvent::~THldEvent(){ // destructor
	if(bVerboseMode)
		cout << "Calling THldEvent destructor..." << endl;
	if(SubEventData!=NULL)
		delete SubEventData;
}

void THldEvent::DecodeBaseEventSize(){
	/*
		The second header word contains information about the base event size unit;
		It is encoded in the upper two bytes of this word, so wee need to shift everything by 16 bits to extract the information;
	*/
	nBaseEventSize = 1 << ((EventHeader.nDecoding >> 16) & 0xFF);
}

void THldEvent::Init(){
	// initialise header variables
	EventHeader.nSize		= 0;
	EventHeader.nDecoding	= 0;
	EventHeader.nId			= 0;
	EventHeader.nSeqNr		= 0;
	EventHeader.nDate		= 0;
	EventHeader.nTime		= 0;
	EventHeader.nRun		= 0;
	EventHeader.nPad		= 0;

	SubEventData = NULL;

	nBaseEventSize	= 0;
	nDataBytes		= 0;
	nDataWords		= 0;
	nSkipBytes		= 0;

	bIgnoreEvent = kFALSE;
	bHasSubEvent = kFALSE; // no subevent data present
}

void THldEvent::PrintHeader(){
	cout << "+++ HLD Event Header +++" << endl;
	cout << "Size: \t\t" << EventHeader.nSize << endl;
	cout << "Decoding: \t" << hex << EventHeader.nDecoding << dec << endl;
	cout << "ID: \t\t" << hex << EventHeader.nId << dec << endl;
	cout << "Seq Nr: \t" << EventHeader.nSeqNr << endl;
	cout << "Date: \t\t" << hex << EventHeader.nDate << dec << endl;
	cout << "Time: \t\t" << hex << EventHeader.nTime << dec << endl;
	cout << "Run: \t\t" << hex << EventHeader.nRun << dec << endl;
	cout << "Pad: \t\t" << hex << EventHeader.nPad << dec << endl;
	cout << "++++++++++++++++++++++++" << endl;
}

Bool_t THldEvent::ReadIt(){
	if(SubEventData!=NULL){
		delete SubEventData;
		SubEventData = NULL;
	}
	if(!ReadHeader()){ // read HLD Event header information
		return (kFALSE);
	}
	if(EventHeader.nSize == sizeof(HLD_HEADER)){ // check if event contains only header information
		bHasSubEvent = kFALSE;
		if(bVerboseMode)
			cout << "HLD event " << EventHeader.nSeqNr << " contains only header information!" << endl;
		SkipPaddingBytes(EventHeader.nSize);
		return (kTRUE);
	}
	if(bSkipSubEvent){
		HldFile->ignore(nDataBytes);
	}
	else{
		if(bVerboseMode)
			cout << "Reading subevent data..." << endl;
		bHasSubEvent = kTRUE;
		SubEventData = new THldSubEvent(HldFile,TrbSettings,Hits,bVerboseMode);
		if(!SubEventData->Decode()){ // error while decoding HLD subevent
			cout << "Error decoding HLD subevent!" << endl;
			Hits->Clear();
			bHasSubEvent = kFALSE;
		}
		if(SubEventData->GetNBytes() != nDataBytes){
			cerr << "Error: Bytes read in SubEvent decoder does not match Event Header information!" << endl;
			HldFile->ignore(nDataBytes-SubEventData->GetNBytes());
			Hits->Clear();
			bHasSubEvent = kFALSE;
		}
	}
	SkipPaddingBytes(EventHeader.nSize); // if event length is not a multiple of 8 empty bytes will be added before next event starts
	return (kTRUE);
}

Bool_t THldEvent::ReadHeader(){
	HldFile->read((char*)&EventHeader,sizeof(HLD_HEADER)); // read event header data from HLD file
	if(HldFile->gcount() != sizeof(HLD_HEADER)){ // error occured while reading event header
		cerr << "Error reading header from HLD file!" << endl;
		return (kFALSE);
	}
	nDataBytes = EventHeader.nSize - sizeof(HLD_HEADER); // compute number of data bytes remaining in event
	nDataWords = nDataBytes/sizeof(UInt_t); // compute number of data words remaining in event
	DecodeBaseEventSize(); // decode base event size
	if(bVerboseMode){
		PrintHeader();
		cout << nDataBytes << ", " << nDataWords << endl;
	}
	return (kTRUE);
}

void THldEvent::SkipPaddingBytes(size_t nBytesRead){
	if(bVerboseMode)
		cout << "Event base size is " << nBaseEventSize << " bytes" << endl;
	nSkipBytes = nBaseEventSize * (size_t)((nBytesRead-1)/nBaseEventSize + 1) - nBytesRead;
	if(nSkipBytes>0){
		HldFile->ignore(nSkipBytes);
		if(bVerboseMode)
			cout << "Skipping " << nSkipBytes << " bytes!" << endl;
	}
}
