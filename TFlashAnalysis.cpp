#include "TFlashAnalysis.h"

ClassImp(TFlashAnalysis);

//TFlashAnalysis::TFlashAnalysis(TChain &UserChain) : TDircAnalysisBase(UserChain){
//	cout << "Initialising TFlashAnalysis..." << endl;
//	Init();
//}

TFlashAnalysis::TFlashAnalysis(string cUserDataFilename, string cUserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth) : TDircAnalysisBase(cUserDataFilename,cUserTdcAddressFile,nUserTdcOffset,nUserTdcWidth) {
	cout << "Initialising TFlashAnalysis..." << endl;
	Init();
}

TFlashAnalysis::TFlashAnalysis(string cUserDataFilename, string cUserTdcAddressFile) : TDircAnalysisBase(cUserDataFilename, cUserTdcAddressFile) {
	cout << "Initialising TFlashAnalysis..." << endl;
	Init();
}

TFlashAnalysis::~TFlashAnalysis(){

}

Bool_t TFlashAnalysis::AddPixelPair(UInt_t nUserChanA, UInt_t nUserChanB){
	std::pair<std::set< std::pair<UInt_t,UInt_t> >::iterator,bool> ret;
	ret = PixelPairs.insert(std::make_pair(nUserChanA,nUserChanB));
	//cout << PixelPairs.size() << endl;
	return (ret.second);
}

UInt_t TFlashAnalysis::AddPixelPairs(string cUserPairList){
	UInt_t nAddedPixelPairs = 0;
	if(cUserPairList.empty()){ // check if user pair list name string is empty
//		if(bVerboseMode)
			cout << "TDC address string is empty!" << endl;
		return (0);
	}
	ifstream UserInputFile(cUserPairList.c_str(),ifstream::in); // open text file containing pixel pair combinations
	if(!UserInputFile.is_open()){ // check if opening text file was successful
		cerr << "Could not open TDC addresses file " << cUserPairList << endl;
		return (0);
	}
	while(UserInputFile.good()){ // start loop over input file
		string cCurrentLine;
		getline(UserInputFile,cCurrentLine); // get line from input file
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		vector<string> tokens = LineParser(cCurrentLine,' ');
		switch (tokens.size()) {
			case 2: // pair definition should contain only two values
				if(AddPixelPair((UInt_t)strtoul(tokens.at(0).c_str(),NULL,10),(UInt_t)strtoul(tokens.at(1).c_str(),NULL,10))) // try to add this pixel pair combination
					nAddedPixelPairs++; // in case of success, increment counter
				break;
			default:
				continue; // do nothing
		}
	}// end of loop over input file
	return (nAddedPixelPairs);
}

void TFlashAnalysis::Analyse(Long64_t nEntryIndex){
	Clear(); // reset all variables needed in analysis of one event
	Int_t nEntrySize = GetEntry(nEntryIndex);
	if(nEntrySize<1){// if entry is smaller than 1, something went wrong
		return;
	}
	nNumberOfHitPixels = (UInt_t)EvtReconHits.size(); // number of reconstructed hits is just the size of this map object
	if(!PixelPairs.empty()){
		for (std::set< std::pair<UInt_t,UInt_t> >::iterator it=PixelPairs.begin(); it!=PixelPairs.end(); ++it){
			Bool_t bIsValid;
			Double_t fDelta = ComputeChannelCorrelation(it->first,it->second,bIsValid);
			if(bIsValid){
				PixelCorrelations.insert(std::make_pair(*it,fDelta));
				//cout << it->first << " - " << it->second << ": " << fDelta << endl;
			}
		}
	}
}

void TFlashAnalysis::AnalyseTrigger(string cUserAnalysisFilename){

}

void TFlashAnalysis::Clear(){
	nNumberOfHitPixels = 0;
	PixelCorrelations.clear();
}

void TFlashAnalysis::ClearPixelTimeOffset(UInt_t nUserSeqId){
	cout << PixelTimeOffsets.size() << endl;
	std::map<UInt_t,Double_t>::iterator it;
	it = PixelTimeOffsets.find(nUserSeqId);
	if(it!=PixelTimeOffsets.end())
		PixelTimeOffsets.erase(it);
	cout << PixelTimeOffsets.size() << endl;
}

Double_t TFlashAnalysis::ComputeChannelCorrelation(UInt_t nChanA, UInt_t nChanB, Bool_t &IsValid){
	IsValid = kFALSE;
	std::map< UInt_t,std::list<PixelHitModel> >::const_iterator itA;
	std::map< UInt_t,std::list<PixelHitModel> >::const_iterator itB;
	itA = EvtReconHits.find(nChanA);
	itB = EvtReconHits.find(nChanB);
	if(itA!=EvtReconHits.end() && itB!=EvtReconHits.end()){
		IsValid = kTRUE;
		Double_t fDelta = itA->second.begin()->GetLeadEdgeTime() - itB->second.begin()->GetLeadEdgeTime();
		return (fDelta);
	}
	return (0.0);
}

Double_t TFlashAnalysis::ComputeChannelCorrelation(UInt_t nChanA, Double_t fChanATotLow, Double_t fChanATotHigh, UInt_t nChanB, Double_t fChanBTotLow, Double_t fChanBTotHigh, Bool_t &IsValid){
	IsValid = kFALSE;
	std::map< UInt_t,std::list<PixelHitModel> >::const_iterator itA;
	std::map< UInt_t,std::list<PixelHitModel> >::const_iterator itB;
	itA = EvtReconHits.find(nChanA); // find channel A
	itB = EvtReconHits.find(nChanB); // find channel B
	if(itA!=EvtReconHits.end() && itB!=EvtReconHits.end()){ // check if both channels were found
		Double_t fToTChanA = itA->second.begin()->GetToT();
		Double_t fToTChanB = itB->second.begin()->GetToT();
		if(fToTChanA>fChanATotLow && fToTChanA<fChanATotHigh && fToTChanB>fChanBTotLow && fToTChanB<fChanBTotHigh){
			IsValid = kTRUE;
			Double_t fDelta = itA->second.begin()->GetLeadEdgeTime() - itB->second.begin()->GetLeadEdgeTime();
			return (fDelta);
		}
	}
	return (0.0);
}

void TFlashAnalysis::FillTimingHistogram(TH1D& hTimingHist){
	std::map< UInt_t,std::list<PixelHitModel> >::const_iterator it;
	std::map< UInt_t,std::list<PixelHitModel> >::const_iterator FirstChannel	= EvtReconHits.begin();
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator LastChannel	= EvtReconHits.end();
	for(it=FirstChannel; it!=LastChannel; ++it){ // begin loop over all channels
		std::list<PixelHitModel>::const_iterator hit;
		std::list<PixelHitModel>::const_iterator FirstHit	= it->second.begin();
			std::list<PixelHitModel>::const_iterator LastHit	= it->second.end();
		for(hit=FirstHit; hit!=LastHit; ++hit){ // begin loop over all hits in channel
			hTimingHist.Fill(hit->GetLeadEdgeTime());
		} // end of loop over all hits in channel
	} // end of loop over all channels
}

void TFlashAnalysis::FillTimingHistogram(UInt_t nUserChan, TH1D& hTimingHist){
	std::map< UInt_t,std::list<PixelHitModel> >::const_iterator it;
	std::map< UInt_t,std::list<PixelHitModel> >::const_iterator FirstChannel	= EvtReconHits.begin();
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator LastChannel	= EvtReconHits.end();
	for(it=FirstChannel; it!=LastChannel; ++it){ // begin loop over all channels
		std::list<PixelHitModel>::const_iterator hit;
		std::list<PixelHitModel>::const_iterator FirstHit	= it->second.begin();
			std::list<PixelHitModel>::const_iterator LastHit	= it->second.end();
		for(hit=FirstHit; hit!=LastHit; ++hit){ // begin loop over all hits in channel
			if(hit->GetLeadEdgeChan()==nUserChan) // only fill histogram if channel ID is correct
				hTimingHist.Fill(hit->GetLeadEdgeTime());
		} // end of loop over all hits in channel
	} // end of loop over all channels
}

void TFlashAnalysis::FillTimingHistogram(TH2D& hTimingHist, UInt_t nSeqIdLow, UInt_t nSeqIdHigh) const {
	std::map< UInt_t,std::list<PixelHitModel> >::const_iterator it;
	std::map< UInt_t,std::list<PixelHitModel> >::const_iterator FirstChannel	= EvtReconHits.begin();
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator LastChannel	= EvtReconHits.end();
	for(it=FirstChannel; it!=LastChannel; ++it){ // begin loop over all channels
		if(it->first < nSeqIdLow || it->first > nSeqIdHigh)
			continue;
		std::list<PixelHitModel>::const_iterator hit;
		std::list<PixelHitModel>::const_iterator FirstHit	= it->second.begin();
			std::list<PixelHitModel>::const_iterator LastHit	= it->second.end();
		for(hit=FirstHit; hit!=LastHit; ++hit){ // begin loop over all hits in channel
			hTimingHist.Fill((Double_t)hit->GetLeadEdgeChan(),hit->GetLeadEdgeTime());
		} // end of loop over all hits in channel
	} // end of loop over all channels

}

void TFlashAnalysis::FillToTHistogram(TH2D& hToTHist) const{
	std::map< UInt_t,std::list<PixelHitModel> >::const_iterator it;
	std::map< UInt_t,std::list<PixelHitModel> >::const_iterator FirstChannel	= EvtReconHits.begin();
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator LastChannel	= EvtReconHits.end();
	for(it=FirstChannel; it!=LastChannel; ++it){ // begin loop over all channels
		std::list<PixelHitModel>::const_iterator hit;
		std::list<PixelHitModel>::const_iterator FirstHit	= it->second.begin();
			std::list<PixelHitModel>::const_iterator LastHit	= it->second.end();
		for(hit=FirstHit; hit!=LastHit; ++hit){ // begin loop over all hits in channel
			hToTHist.Fill((Double_t)hit->GetLeadEdgeChan(),hit->GetToT());
		} // end of loop over all hits in channel
	} // end of loop over all channels
}


Bool_t TFlashAnalysis::GetPixelCorrelation(std::pair<UInt_t,UInt_t> UserPair, Double_t& fDelta) const{
	std::map< std::pair<UInt_t,UInt_t>,Double_t >::const_iterator it;
	it = PixelCorrelations.find(UserPair);
	if(it!=PixelCorrelations.end()){
		fDelta = it->second;
		return (kTRUE);
	}
	return (kFALSE);
}

Bool_t TFlashAnalysis::GetPixelCorrelation(UInt_t nChanA, Double_t fChanATotLow, Double_t fChanATotHigh, UInt_t nChanB, Double_t fChanBTotLow, Double_t fChanBTotHigh, Double_t& fDelta){
	Bool_t bIsValid = kFALSE;
	fDelta = ComputeChannelCorrelation(nChanA,fChanATotLow,fChanATotHigh,nChanB,fChanBTotLow,fChanBTotHigh,bIsValid);
	return (bIsValid);
}

void TFlashAnalysis::Init(){
	nNumberOfHitPixels = 0;
	PixelTimeOffsets.clear();
	PixelPairs.clear();
	PixelCorrelations.clear();
}

TH2D* TFlashAnalysis::MakePixelCorrelationMap(){
	std::map< UInt_t,std::list<PixelHitModel> >::const_iterator it;
	TH2D* hPixelCorMap = new TH2D("hPixelCorMap","seq pixel ID; seq pixel ID, freq",GetSizeOfMapTable()+1,-0.5,GetSizeOfMapTable()+0.5,GetSizeOfMapTable()+1,-0.5,GetSizeOfMapTable()+0.5);
	for(Int_t i=0; i<GetNEvents(); i++){ // begin loop over all events
		Int_t nEntrySize = GetEntry(i);
		if(nEntrySize<1) // something is wrong with this entry
			continue; // skip rest of commands in loop
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator FirstChannel	= EvtReconHits.begin();
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator LastChannel	= EvtReconHits.end();
		for(it=FirstChannel; it!=LastChannel; ++it){ // begin loop over all channels
			std::map< UInt_t,std::list<PixelHitModel> >::const_iterator CoinChan = it;
			while(++CoinChan != LastChannel){
				hPixelCorMap->Fill((Double_t)it->first,(Double_t)CoinChan->first);
			}
		} // end of loop over all channels
	} // end of loop over all pixels
	return (hPixelCorMap);
}

Bool_t TFlashAnalysis::SetPixelTimeOffset(UInt_t nUserSeqId, Double_t fUserOffset){
	std::pair<std::map<UInt_t,Double_t>::iterator,bool> ret;
	ret = PixelTimeOffsets.insert(std::make_pair(nUserSeqId,fUserOffset)); // try to insert pixel time offset into map
	if(!ret.second){
		if(ret.first!=PixelTimeOffsets.end()){
			PixelTimeOffsets.erase(ret.first);
			ret = PixelTimeOffsets.insert(std::make_pair(nUserSeqId,fUserOffset));
		}
	}
	return (ret.second);
}

void TFlashAnalysis::WriteLogfileHeader(){
	if(!LogFileBuffer.is_open()) // no file is associated with LogFileBuffer
		return;
	time(&RawTime);
	std::streambuf *psbuf, *backup;
	LogFileBuffer << "+++++++++++++++++++++++++++++++++++++" << endl;
	LogFileBuffer << "+      FLASH Analysis Logfile       +" << endl;
	LogFileBuffer << "+++++++++++++++++++++++++++++++++++++" << endl;
	LogFileBuffer << "+    Analysis Setup Information     +" << endl;
	LogFileBuffer << "+++++++++++++++++++++++++++++++++++++" << endl;
	LogFileBuffer << "\t" << ctime(&RawTime) << endl;
	LogFileBuffer << endl;
	// redirect cout buffer to logfile
	backup = std::clog.rdbuf();
	psbuf = LogFileBuffer.rdbuf();
	std::clog.rdbuf(psbuf);         // assign streambuf to cout
	// print status information to logfile
	PrintStatus(); // print general status information of analysis setup
	LogFileBuffer << endl; // add empty line
	PrintTdcAddresses(); // print list of TDC addresses
	LogFileBuffer << endl; // add empty line
	PrintSwapList(); // print list of TDC where leading & trailing edges need to be swapped
	LogFileBuffer << endl; // add empty line
	PrintExcludedChannels(); // print list of excluded channels
	LogFileBuffer << endl; // add empty line
	PrintTimingWindow(); // print limits of timing window
	LogFileBuffer << endl; // add empty line
	PrintTriggerAddress();	 // print address of trigger channel
	LogFileBuffer << endl; // add empty line
	PrintTriggerWindow(); // print limits of trigger time window
	LogFileBuffer << endl; // add empty line
	LogFileBuffer << "+++++++++++++++++++++++++++++++++++++" << endl;
	LogFileBuffer << "+ End of Analysis Setup Information +" << endl;
	LogFileBuffer << "+++++++++++++++++++++++++++++++++++++" << endl;
	LogFileBuffer << endl; // add empty line
	// reset cout buffer to terminal
	std::clog.rdbuf(backup);        // restore cout's original streambuf
	psbuf = NULL;
	backup = NULL;

}