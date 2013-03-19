#include "THldSubevent.h"
#include <iomanip>

//TClonesArray* THldSubEvent::TrbHits = 0;

ClassImp(THldSubEvent);

THldSubEvent::THldSubEvent(ifstream* UserHldFile, const TRB_SETUP* UserTrbSettings, TClonesArray* UserArray, Bool_t bUserVerboseMode){ // constructor
	HldFile			= UserHldFile; // set pointer to HLD file input stream
	TrbSettings		= UserTrbSettings; // set pointer to TRB setup structure
	Hits			= UserArray;
	bVerboseMode	= bUserVerboseMode; // set verbose mode flag
	Init(); // initialise variables
}

THldSubEvent::~THldSubEvent(){ // destructor
	if(bVerboseMode)
		cout << "Calling THldSubEvent destructor..." << endl;
	//delete TrbHits;
}

Bool_t THldSubEvent::CheckTrbAddress(UInt_t nUserTrbAddress){
	return ( find(TrbSettings->nTrbAddress.begin(),TrbSettings->nTrbAddress.end(),nUserTrbAddress) != TrbSettings->nTrbAddress.end());
}

Bool_t THldSubEvent::Decode(){ // decode subevent
	if(bVerboseMode)
		cout << "Subevent: Reading header..." << endl;
	if(!ReadHeader()){ // read subevent header
		return (kFALSE);
	}
	// read TRBv3 header word (4 bytes)
	if(bVerboseMode)
		cout << "Subevent: Reading TRBv3 data..." << endl;

	if(!ReadTrbData()){
		cout << "ERROR parsing the following TRBv3 data (CHECK THIS!):" << endl;
		PrintTrbData();
		// still try reading the trailer of the subevent
		ReadTrailer();			
		return (kFALSE);
	}
	if(bVerboseMode){
		cout << nTdcHits << " TDC Hits found in subevent" << endl;
	}
	// need to know the number of TRBv3 boards in this setup and their addresses
	// read TDC header word (4 bytes)
	// read subevent trailer
	if(!ReadTrailer())
		return (kFALSE);
	
	return (kTRUE);
}

void ClearTdcHeader(TDC_HEADER& TdcHeader){
	TdcHeader.nRandomBits	= 0;
	TdcHeader.nErrorBits	= 0;
}

void THldSubEvent::DecodeBaseEventSize(){
	nBaseEventSize = 1 << ((SubEventHeader.nDecoding >> 16) & 0xFF);
}

Bool_t THldSubEvent::DecodeTdcHeader(std::vector<UInt_t>::const_iterator DataWord, TDC_HEADER& TdcHeader){
	ClearTdcHeader(TdcHeader);
	if(((*DataWord>>29) & 0x7) != TDC_HEADER_MARKER) { // check 3 bits reserved for TDC header marker
		return (kFALSE);
	}
	TdcHeader.nRandomBits	= (*DataWord>>16) & 0xFF;
	TdcHeader.nErrorBits	= *DataWord & 0xFFFF;
	if(bVerboseMode){
		cout << "TDC Header word found!" << endl;
		cout << "TDC Error Code is " << hex << TdcHeader.nErrorBits << dec << endl;
		cout << "TDC Random Bits " << hex << TdcHeader.nRandomBits << dec << endl;
	}
	return (kTRUE);
}

Bool_t THldSubEvent::DecodeTdcWord(std::vector<UInt_t>::const_iterator DataWord,
                                   UInt_t nUserTrbAddress, TDC_HEADER& TdcHeader) { // decode TDC data word

	// first check if word is EPOCH or DEBUG
	UInt_t FirstThreeBits =  (*DataWord>>29) & 0x7;

	// put the EPOCH number into the SubEvent's member
	if(FirstThreeBits == TDC_EPOCH_MARKER) {
		if(bVerboseMode)
			cout << "Found EPOCH word:  " << hex << *DataWord << ", " << nUserTrbAddress
			     <<	" , " << dec << nTdcHits << endl;
		// lowest 28bits represent epoch counter
		nTdcEpochCounter = *DataWord & 0xFFFFFFF;
		// this indicates that we found an EPOCH counter 
		nTdcLastChannelNo = -2;
		return kFALSE;
	}	
	
	// check for DEBUG, we don't use this info at the moment
	if(FirstThreeBits == TDC_DEBUG_MARKER) {
		if(bVerboseMode)
			cout << "Found DEBUG word:  " << hex << *DataWord << ", " << nUserTrbAddress
			     <<	" , " << dec << nTdcHits << endl;
		return kFALSE;
	}
	
	// now we expect a TIMEDATA word...if not, we don't know :)
	if((*DataWord>>31) != 1) { // check time data marker i.e. MSB==1
		if(bVerboseMode)
			cout << "Found ??UNKNOWN?? word (maybe spurious header): " << hex << *DataWord << ", " << nUserTrbAddress
			     <<	" , " << dec << nTdcHits << endl;
		
		return kFALSE;
	}
	
	UInt_t nTdcChannelNo	= (*DataWord>>22) & 0x7F; // TDC channel number is represented by 7 bits
	// check here if we need to reset the EPOCH counter
	if(nTdcLastChannelNo>=0 && (Int_t)nTdcChannelNo != nTdcLastChannelNo) {
		if(bVerboseMode)
			cout << "Epoch Counter reset since channel has changed" << endl;
		nTdcEpochCounter = 0;
	}

	Bool_t bIsRefChannel	= (nTdcChannelNo==TrbSettings->nRefChannel) ? kTRUE : kFALSE;
	UInt_t nTdcFineTime		= (*DataWord>>12) & 0x3FF; // TDC fine time is represented by 10 bits
	UInt_t nTdcEdge			= (*DataWord>>11) & 0x1; // TDC edge indicator: 1->rising edge, 0->falling edge
	UInt_t nTdcCoarseTime	= *DataWord & 0x7FF; // TDC coarse time is represented by 11 bits
	TTrbHit* CurrentHit = (TTrbHit*)Hits->ConstructedAt(nTdcHits); // create new TTrbHit in TclonesArray Hits
	CurrentHit->SetVerboseMode(bVerboseMode);
	CurrentHit->Set(nUserTrbAddress,nTdcChannelNo,TdcHeader.nRandomBits,TdcHeader.nErrorBits,
	                nTdcEdge,nTdcEpochCounter,nTdcCoarseTime,nTdcFineTime,bIsRefChannel);
	if(bVerboseMode)
		cout << "Found TIMEDATA word:  " <<hex << *DataWord << dec << ", channel " << nTdcChannelNo
		     << " Epoch:" << nTdcEpochCounter  << " last: " << nTdcLastChannelNo << endl;
	nTdcLastChannelNo = (Int_t)nTdcChannelNo;
	return kTRUE;
}


void THldSubEvent::Init(){ // initialise subevent variables
	if(bVerboseMode)
		cout << "Subevent: Initialising..." << endl;

	SubEventHeader.nSize		= 0; // subevent size in bytes
	SubEventHeader.nDecoding	= 0; // subevent decoding settings
	SubEventHeader.nEventId		= 0; // subevent ID (should be 0x8c00 for TRBv3)
	SubEventHeader.nTrigger		= 0; // subevent trigger number
	
	SubEventTrailer.nSebHeader	= 0; // subevent trailer Subevent Builder header
	SubEventTrailer.nSebError	= 0; // subevent trailer Subevent Builder error code (should be 1 for good event)
	nTrbData.clear(); // clear TRB data vector
	ErrorCode.reset(); // clear error code bitset

	nTdcHits			= 0; // number of TDC hits found in subevent
	nNumberOfTrbsFound  = 0; // number of TRB boards found in subevent
	nNumberOfTdcsFound	= 0; // number of TDCs found in subevent

	nBaseEventSize	= 0;
	nDataBytes		= 0;
	nDataWords		= 0;
	nSubEventSize	= 0;
	nTrbWordsRead	= 0;
	bIsValid		= kFALSE;
	nTdcEpochCounter = 0;
	nTdcLastChannelNo = -1; // at the beginning of a subevent, there's no such channel
	
	Hits->Clear("C");
}

void THldSubEvent::PrintHeader(){
	cout << "+++ SubEvent Header +++" << endl;
	cout << "Size: \t\t" << SubEventHeader.nSize << endl;
	cout << "Decoding: \t" << hex << SubEventHeader.nDecoding << dec << endl;
	cout << "Event ID: \t" << hex << SubEventHeader.nEventId << dec << endl;
	cout << "Trigger: \t" << hex << SubEventHeader.nTrigger << dec << endl;
	cout << "+++++++++++++++++++++++" << endl;
}

void THldSubEvent::PrintTrailer(){
	cout << "+++ SubEvent Trailer +++" << endl;
	cout << "SEB Header: \t" << hex << SubEventTrailer.nSebHeader << dec << endl;
	cout << "SEB Error Code: " << hex << SubEventTrailer.nSebError << dec << endl;
	cout << "++++++++++++++++++++++++" << endl;
}

void THldSubEvent::PrintTrbData() {
	//std::vector<UInt_t>::iterator CurrentDataWord=nTrbData.begin();
	cout << "+++ Trb Data +++";
	for(size_t i=0;i<nTrbData.size();++i) {
		if(i % 4 == 0)
			cout << endl << "    ";
		cout << setfill('0') << setw(8) << hex << nTrbData[i] << " ";
	}
	cout << endl;
	cout << "++++++++++++++++++++++++" << endl;
}

Bool_t THldSubEvent::ReadHeader(){ // read subevent header information
	HldFile->read((char*)&SubEventHeader,sizeof(SUB_HEADER));
	nSubEventSize += HldFile->gcount(); // add read header bytes to subevent size
	if(HldFile->gcount() != sizeof(SUB_HEADER)){
		cerr << "Error reading subevent header from HLD file!" << endl;
		bIsValid = kFALSE;
		return (kFALSE);
	}
	SwapHeaderWords(); // convert header words from big Endian to little Endian type
	if(SubEventHeader.nEventId != TrbSettings->nSubEventId){ // check if subevent ID matches 
		cerr << "Subevent ID " << hex << SubEventHeader.nEventId << " not matching " << TrbSettings->nSubEventId << dec << endl;
		bIsValid = kFALSE;
		return (kFALSE);
	}
	nDataBytes = SubEventHeader.nSize - sizeof(SUB_HEADER);
	nDataWords = nDataBytes/sizeof(UInt_t);
	DecodeBaseEventSize();
	if(bVerboseMode){
		PrintHeader();
		cout << nDataBytes << ", " << nDataWords << ", " << nBaseEventSize << endl;
	}
	return (kTRUE);
}

Bool_t THldSubEvent::ReadTrailer() { // read subevent trailer information
	HldFile->read((char*)&SubEventTrailer,sizeof(SUB_TRAILER));
	nSubEventSize += HldFile->gcount();
	if(HldFile->gcount() != sizeof(SUB_TRAILER)) {
		cerr << "Error reading subevent trailer from HLD file!" << endl;
		return (kFALSE);
	}
	SwapTrailerWords();
	if(bVerboseMode)
		PrintTrailer();
	if(SubEventTrailer.nSebError != SEB_ERROR_CODE){
		if(bVerboseMode)
			cout << "Error in Subevent Builder detected!" << endl;
		bIsValid = kFALSE;
	}
	else
		bIsValid = kTRUE;
	return (kTRUE);
}

Bool_t THldSubEvent::ReadTrbData() {
	size_t nTrbDataBytes = nDataBytes - sizeof(SUB_TRAILER); // compute number of TRB data bytes to read
	size_t nTrbDataWords = nTrbDataBytes/sizeof(UInt_t); // compute number of TRB data words to read
	if(bVerboseMode)
		cout << nTrbDataWords << " TRB data words to read" << endl;
	nTrbData.resize(nTrbDataWords); // adjust vector size to accommodate all TRB data words
	HldFile->read((char*)&nTrbData[0],nTrbDataBytes); // read TRB data from HLD file
	nSubEventSize += HldFile->gcount(); // increase number of bytes read from HLD file
	if((size_t)HldFile->gcount() != nTrbDataBytes){ // check amount of bytes actually read from HLD file
		cerr << "Error reading subevent TRB data from HLD file!" << endl;
		return kFALSE;
	}
	// swap bytes of all TRB data words
	transform(nTrbData.begin(),nTrbData.end(),nTrbData.begin(),SwapBigEndian); 
	

	// state machine for TDC data decoding needed
	UInt_t nTrbAddress	= 0;
	size_t nTrbWords	= 0;
	UInt_t nTdcAddress	= 0;
	size_t nTdcWords	= 0;
	Bool_t bFoundCtsPacket = kFALSE;
	
	TDC_HEADER TdcHeader;
	
	std::vector<UInt_t>::iterator CurrentDataWord=nTrbData.begin();

	while(CurrentDataWord<nTrbData.end()){ // loop over all TRB data and decode TDC hits
		if(nTdcWords==0) {
			// look for DataWord with matching TrbAddress
			nTrbAddress = *CurrentDataWord & 0xFFFF;
			nTrbWords = *CurrentDataWord>>16;
			if(nTrbWords<=SubEventHeader.nSize) {
				if(CheckTrbAddress(nTrbAddress)) {
					// we recognized it as an TDC endpoint (might still be wrong...)
					nTdcWords = nTrbWords;
					nTdcAddress = nTrbAddress;
					if(bVerboseMode)
						cout << "TDC Endpoint found at 0x" << setfill('0') << setw(4)
						     << hex << nTdcAddress << dec << ", Payload " << nTdcWords << endl;
				}
				else if(nTrbAddress==TrbSettings->nCtsAddress) {
					// this information is not used at the moment,
					// so we skip payload it to exclude misidentifaction as an endpoint word
					if(bVerboseMode)
						cout << "Found CTS readout packet, skipping it" << endl;
					CurrentDataWord += nTrbWords+1;
					bFoundCtsPacket = kTRUE;
					continue;
				}
				// else: We cannot skip all "unknown" TRB endpoints since the events might
				// be encapsulated in exactly such an event (this depends on the TRB hub topology!) 
			}
			
		}
		else if(nTdcWords==nTrbWords) {
			// At beginning, we expect a TDC Header
			if(!DecodeTdcHeader(CurrentDataWord, TdcHeader)) {
				ErrorCode.set(2);
				cerr << "ERROR in Subevent " << SubEventHeader.nTrigger <<": TDC Header " <<hex<<*CurrentDataWord<<dec<< " invalid, start searching again ("<< nTdcWords << " words)" << endl;
				// Let's see if it's a word with an interesting address
				// we cannot really rely on nTdcWords to skip this payload...
				nTdcWords = 0;				
				continue;
			}
			nTdcWords--;
		}
		else {
			if(DecodeTdcWord(CurrentDataWord, nTdcAddress, TdcHeader)) {
				nTdcHits++;
			}
			nTdcWords--;
			if(nTdcWords==0) {
				if(bVerboseMode)
					cout << "Successfully unpacked TDC event" << endl;
				nNumberOfTdcsFound++;	
			}
		}
		
		CurrentDataWord++; // go to next data word
	} // end of loop over all TRB data

	
	
	if(CurrentDataWord!=nTrbData.end()) {
		cerr << "ERROR in Subevent " << SubEventHeader.nTrigger <<": Skipping too many words " << endl;
		ErrorCode.set(5);	
		return (kFALSE);
	}

	if(!bFoundCtsPacket) {
		cerr << "ERROR in Subevent " << SubEventHeader.nTrigger <<": No CTS packet found " << endl;
		ErrorCode.set(6);
		return kFALSE;
	}

	return (kTRUE);
}


void THldSubEvent::SwapHeaderWords(){
	SubEventHeader.nSize		= SwapBigEndian(SubEventHeader.nSize);
	SubEventHeader.nDecoding	= SwapBigEndian(SubEventHeader.nDecoding);
	SubEventHeader.nEventId		= SwapBigEndian(SubEventHeader.nEventId);
	SubEventHeader.nTrigger		= SwapBigEndian(SubEventHeader.nTrigger);
}

void THldSubEvent::SwapTrailerWords(){
	SubEventTrailer.nSebHeader	= SwapBigEndian(SubEventTrailer.nSebHeader);
	SubEventTrailer.nSebError	= SwapBigEndian(SubEventTrailer.nSebError);
}
