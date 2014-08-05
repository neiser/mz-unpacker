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
		It is encoded in the upper two bytes of this word, so we need to shift everything by 16 bits to extract the information;
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
	if(SubEventData!=NULL){ // clear pointer to subevent object
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
		SkipPaddingBytes(EventHeader.nSize); // skip padding bytes (should noy happen since header contains 8 data words)
		return (kTRUE);
	}
	if(bSkipSubEvent){ // skip subevent data on user request
		HldFile->ignore(nDataBytes);
	}
	else{ // event contains also subevent data
		if(bVerboseMode)
			cout << "Reading subevent data..." << endl;
		bHasSubEvent = kTRUE;
		SubEventData = new THldSubEvent(HldFile,TrbSettings,Hits,bVerboseMode); // create new subevent object
		// variables to keep track of amount of data already read from file
		size_t nBytesRead		= 0;
		size_t nBytesSkipped	= 0;
		size_t nBytesReadOld	= 0;
		do{ // loop over subevent data until all data is read
			Bool_t bDecodeStatus = SubEventData->Decode(); // read in and decode subevent data
			nBytesRead = SubEventData->GetNBytes(); // update number of bytes read from file
			if(!bDecodeStatus){ // error while decoding HLD subevent
				cout << "Error decoding HLD subevent!" << endl;
				Hits->Clear(); // clear hits array
				bHasSubEvent = kFALSE; 
				HldFile->ignore(nDataBytes-nBytesRead); // ignore rest of event
				break; // break from do-loop
			}
			if((nDataBytes-(nBytesRead+nBytesSkipped))<nBaseEventSize) // check if less than 8 Bytes need to be read (can only be padding)
				break;
			nBytesSkipped += SkipPaddingBytes((nBytesRead-nBytesReadOld));
			nBytesReadOld = nBytesRead;
			if(bVerboseMode)
				cout << nDataBytes << ": " << nBytesRead << ", " << nBytesSkipped << endl;
		}while((nBytesRead+nBytesSkipped) != nDataBytes);
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

size_t THldEvent::SkipPaddingBytes(size_t nBytesRead){
	if(bVerboseMode)
		cout << "Event base size is " << nBaseEventSize << " bytes" << endl;
	size_t nSkipBytes = (size_t)nBytesRead%nBaseEventSize;
	if(nSkipBytes>0){
		HldFile->ignore(nSkipBytes);
		if(bVerboseMode)
			cout << "Skipping " << nSkipBytes << " bytes!" << endl;
	}
	return (nSkipBytes);
}
