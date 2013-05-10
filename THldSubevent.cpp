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

Bool_t THldSubEvent::CheckHubAddress(const UInt_t& nUserHubAddress){
	return ( find(TrbSettings->nHubAddress.begin(),TrbSettings->nHubAddress.end(),nUserHubAddress) != TrbSettings->nHubAddress.end());
}

Bool_t THldSubEvent::CheckTdcAddress(const UInt_t& nUserTdcAddress){
	return ( find(TrbSettings->nTdcAddress.begin(),TrbSettings->nTdcAddress.end(),nUserTdcAddress) != TrbSettings->nTdcAddress.end());
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
		cout << "ERROR parsing the TRBv3 data (CHECK THIS!)" << endl;
		if(bVerboseMode)
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

Bool_t THldSubEvent::DecodeTdcHeader(UInt_t& DataWord, TDC_HEADER& TdcHeader){
	ClearTdcHeader(TdcHeader);
	if(((DataWord>>29) & 0x7) != TDC_HEADER_MARKER) { // check 3 bits reserved for TDC header marker
		return (kFALSE);
	}
	TdcHeader.nRandomBits	= (DataWord>>16) & 0xFF;
	TdcHeader.nErrorBits	= DataWord & 0xFFFF;
	nTdcLastChannelNo = -1; // reset the last channel number...
	if(bVerboseMode){
		cout << "TDC Header word found!" << endl;
		cout << "TDC Error Code is " << hex << TdcHeader.nErrorBits << dec << endl;
		cout << "TDC Random Bits " << hex << TdcHeader.nRandomBits << dec << endl;
	}
	return (kTRUE);
}

Bool_t THldSubEvent::DecodeTdcWord(UInt_t& DataWord, UInt_t& nUserTdcAddress, TDC_HEADER& TdcHeader) { // decode TDC data word

	// first check if word is EPOCH or DEBUG
	UInt_t FirstThreeBits =  (DataWord>>29) & 0x7;

	// put the EPOCH number into the SubEvent's member
	if(FirstThreeBits == TDC_EPOCH_MARKER) {
		if(bVerboseMode)
			cout << "Found EPOCH word:  " << hex << DataWord << ", " << nUserTdcAddress
			     <<	" , " << dec << nTdcHits << endl;
		// lowest 28bits represent epoch counter
		nTdcEpochCounter = DataWord & 0xFFFFFFF;
		// this indicates that we found an EPOCH counter 
		nTdcLastChannelNo = -2;
		return kFALSE;
	}	
	
	// check for DEBUG, we don't use this info at the moment
	if(FirstThreeBits == TDC_DEBUG_MARKER) {
		if(bVerboseMode)
			cout << "Found DEBUG word:  " << hex << DataWord << ", " << nUserTdcAddress
			     <<	" , " << dec << nTdcHits << endl;
		return kFALSE;
	}
	
	// now we expect a TIMEDATA word...if not, we don't know :)
	if((DataWord>>31) != 1) { // check time data marker i.e. MSB==1
		if(bVerboseMode)
			cout << "Found ??UNKNOWN?? word (maybe spurious header): " << hex << DataWord << ", " << nUserTdcAddress
			     <<	" , " << dec << nTdcHits << endl;
		
		return kFALSE;
	}
	
	UInt_t nTdcChannelNo	= (DataWord>>22) & 0x7F; // TDC channel number is represented by 7 bits
	// check here if we need to reset the EPOCH counter
	if(nTdcLastChannelNo>=0 && (Int_t)nTdcChannelNo != nTdcLastChannelNo) {
		if(bVerboseMode)
			cout << "Epoch Counter reset since channel has changed" << endl;
		nTdcEpochCounter = 0;
	}

	Bool_t bIsRefChannel	= (nTdcChannelNo==TrbSettings->nTdcRefChannel
		   || nUserTdcAddress==TrbSettings->nCtsAddress) ? kTRUE : kFALSE;
	UInt_t nTdcFineTime		= (DataWord>>12) & 0x3FF; // TDC fine time is represented by 10 bits
	UInt_t nTdcEdge			= (DataWord>>11) & 0x1; // TDC edge indicator: 1->rising edge, 0->falling edge
	UInt_t nTdcCoarseTime	= DataWord & 0x7FF; // TDC coarse time is represented by 11 bits
	TTrbHit* CurrentHit = (TTrbHit*)Hits->ConstructedAt(nTdcHits); // create new TTrbHit in TclonesArray Hits
	CurrentHit->SetVerboseMode(bVerboseMode);
	CurrentHit->Set(nUserTdcAddress,nTdcChannelNo,TdcHeader.nRandomBits,TdcHeader.nErrorBits,
	                nTdcEdge,nTdcEpochCounter,nTdcCoarseTime,nTdcFineTime,bIsRefChannel);
	if(bVerboseMode)
		cout << "Found TIMEDATA word:  " <<hex << DataWord << dec << ", channel " << nTdcChannelNo
		     << " Epoch:" << nTdcEpochCounter  << " last: " << nTdcLastChannelNo << endl;
	nTdcLastChannelNo = (Int_t)nTdcChannelNo;
	return kTRUE;
}

UInt_t THldSubEvent::DecodeCTSData(unsigned i0) {
	// i0 should point the CTS header
	UInt_t header = nTrbData[i0];
	// we extract only stuff which is interesting for now...
	UInt_t nInputs = header >> 16 & 0xf;
	UInt_t nTrigChannels = header >> 20 & 0xf;
	UInt_t bIncludeLastIdle = header >> 25 & 0x1;
	UInt_t bIncludeCounters = header >> 26 & 0x1;
	UInt_t bIncludeTimestamp = header >> 27 & 0x1;
	UInt_t nExtTrigFlag = header >> 28 & 0x3;
	
	// how many words each data needs is documented in trb3docu.pdf
	// don't forget the header, which is one word
	UInt_t nCTSwords = 1 + nInputs*2 + nTrigChannels*2 +
		bIncludeLastIdle*2 + bIncludeCounters*3 + bIncludeTimestamp*1;

	// now, the external trigger module (ETM) is missing	
	// i should point to the first ETM word
	unsigned i = i0+nCTSwords;
	
	if(nExtTrigFlag==0x1) {
		// ETM sends one word, is probably MBS Vulom Recv
		// this is not yet tested
		nCTSwords+=1;
		nCTSExtTrigger = nTrbData[i] & 0x00ffffff; // lower 24 bits are trigger number 
		nCTSExtTriggerStatus = nTrbData[i] & 0xff000000; // upper 8 bits are status/error
	}
	else if(nExtTrigFlag==0x2) {
		// ETM sends four words, is probably a Mainz A2 recv
		nCTSwords+=4;
		nCTSExtTrigger = nTrbData[i];
		nCTSExtTriggerStatus = nTrbData[i+1];
		// word 3+4 are 0xdeadbeef i.e. not used at the moment...
	}
	else if(nExtTrigFlag==0x3) {
		if(bVerboseMode)
			cout << "ERROR: Unknown ETM found" << endl;
		// return 0, which skips the CTS stuff
		nCTSwords = 0;
	}

	if(bVerboseMode)
		cout << "External Trigger ID " << hex << nCTSExtTrigger << ", Status: " << nCTSExtTriggerStatus <<endl;

	
	return nCTSwords;
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
	nCTSExtTrigger = 0; // by default, it's zero
	nCTSExtTriggerStatus = 0; // by default, it's zero
	
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

	// declare i outside loop to check in the end
	// if we processed everything correctly 
	unsigned i=0;
	for(i=0;i<nTrbData.size();i++){ // loop over all TRB data and decode TDC hits
		
		UInt_t CurrentDataWord = nTrbData[i]; 
		if(bVerboseMode)
			cout << "i=" <<i<<" Data: "<<setfill('0') << setw(8)<<hex<<CurrentDataWord<<endl;
		if(nTdcWords==0) {			
			// look for DataWord with matching TrbAddress
			nTrbAddress = CurrentDataWord & 0xFFFF;
			nTrbWords	= CurrentDataWord>>16;


			if(nTrbAddress==TrbSettings->nCtsAddress) {
				// Check first if it's CTS, because it might contain a TDC 
				// except for the integrated TDC and external trigger id
				bFoundCtsPacket = kTRUE;
				if(bVerboseMode)
					cout << "Found CTS readout packet, ";
				if(CheckTdcAddress(TrbSettings->nCtsAddress)) {
					if(bVerboseMode)
						cout << "decoding it (" << nTrbWords << " words)" << endl;
					// the CTS address is also mentioned in the TDC list,
					// so we expect to find some TDC information
					// we also save the external trigger id (if we find one)
					UInt_t nCTSwords = DecodeCTSData(i+1);

					if(nCTSwords==0 || nCTSwords>nTrbWords) {
						if(bVerboseMode)
							cout << "ERROR in decoding CTS word, skipping it." << endl;
						i += nTrbWords;
					}
					else {
						// TDC words of the CTS are at the very end of this data block
						// so we fake at this point to have found a "normal" TDC data block now
						// skip the already parsed CTS words
						i += nCTSwords;
						// but not the appended TDC words
						nTdcWords = nTrbWords-nCTSwords;
						nTrbWords = nTdcWords;
						nTdcAddress = nTrbAddress;
						if(bVerboseMode)
							cout << "TDC in CTS Endpoint found at 0x" << setfill('0') << setw(4)
							     << hex << nTdcAddress << dec << ", Payload " << nTdcWords << endl;
	
					}
				}
				else {
					// we are not interested in the CTS stuff,
					// i.e. the CTS address is not mentioned as a TDC endpoint
					if(bVerboseMode)
						cout << "skipping it (" << nTrbWords << " words)" << endl;
					i += nTrbWords;
				}
				
				// skip rest of loop, but we might decode TDC stuff from the CTS now
				continue; 
			}
			else if(CheckHubAddress(nTrbAddress)) {
				// we found a interesting hub, we simply do nothing but
				// to go one level deeper in the topology
				// a consistency check could verify if nTrbWords extracted now
				// will fit to the words of the following subsubevent
				// (but it's complicated since you don't know the depth of the topology tree) 
				if(bVerboseMode)
					cout << "Known HUB found at 0x" << setfill('0') << setw(4)
					     << hex << nTrbAddress << dec << ", Subsubevent size " << nTrbWords << endl;
	
			}
			else if(CheckTdcAddress(nTrbAddress)) {
				// we recognized it as an TDC endpoint
				nTdcWords	= nTrbWords;
				nTdcAddress = nTrbAddress;
				if(bVerboseMode)
					cout << "TDC Endpoint found at 0x" << setfill('0') << setw(4)
					     << hex << nTdcAddress << dec << ", Payload " << nTdcWords << endl;
			}
			else {
				// this doesn't seem to be interesting data, so skip it
				if(bVerboseMode)
					cout << "Found uninteresting data at 0x" << setfill('0') << setw(4)
					     << hex << nTrbAddress << dec << " (not HUB, not TDC, not CTS), skipping it ("  << nTrbWords << " words)" << endl;
				i += nTrbWords;
				continue;
			}
			
		}
		else if(nTdcWords==nTrbWords) {
			// At beginning, we expect a TDC Header
			if(!DecodeTdcHeader(CurrentDataWord, TdcHeader)) {
				ErrorCode.set(2);
				cerr << "ERROR in Subevent " << hex << SubEventHeader.nTrigger << ": TDC Header "
				     << CurrentDataWord << dec << " invalid, skipping payload ("<< nTdcWords << " words)" << endl;
				i += nTdcWords-1;
				nTdcWords = 0;				
				continue;
			}
			// successfully parsed Tdc Header
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
		
	} // end of loop over all TRB data

	
	
	if(i != nTrbData.size()) {
		cerr << "ERROR in Subevent " << hex << SubEventHeader.nTrigger <<": Skipped too many words: "<<dec <<i<<" "<<nTrbData.size() << endl;
		ErrorCode.set(5);	
	}

	if(!bFoundCtsPacket) {
		cerr << "ERROR in Subevent " << hex << SubEventHeader.nTrigger <<": No CTS packet found " << endl;
		ErrorCode.set(6);
	}
	
	return ErrorCode.any() ? kFALSE : kTRUE;
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
