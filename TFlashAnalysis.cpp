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

Bool_t TFlashAnalysis::AddPixelLECut(UInt_t nUserChan, Double_t fLow, Double_t fHigh){ // add pixel cut definitions
	std::pair<Double_t, Double_t> TempCut (fLow, fHigh);
	std::pair<std::map< UInt_t, std::pair<Double_t, Double_t> >::iterator,bool> ret;
	ret = PixelLECuts.insert(std::make_pair(nUserChan,TempCut));
	return (ret.second);
}

UInt_t TFlashAnalysis::AddPixelLECuts(string cUserFileName){
	UInt_t nAddedLECuts = 0;
	if(cUserFileName.empty()){ // check if user pair list name string is empty
//		if(bVerboseMode)
			cout << "Filename string is empty!" << endl;
		return (0);
	}
	ifstream UserInputFile(cUserFileName.c_str(),ifstream::in); // open text file containing pixel pair combinations
	if(!UserInputFile.is_open()){ // check if opening text file was successful
		cerr << "Could not open Leading Edge cuts file " << cUserFileName << endl;
		return (0);
	}
	while(UserInputFile.good()){ // start loop over input file
		string cCurrentLine;
		getline(UserInputFile,cCurrentLine); // get line from input file
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		vector<string> tokens = LineParser(cCurrentLine,' ');
		switch (tokens.size()) {
			case 3: // leading edge cut definition must contain three values
				if(AddPixelLECut((UInt_t)strtoul(tokens.at(0).c_str(),NULL,10),(Double_t)strtod(tokens.at(1).c_str(),NULL),(Double_t)strtod(tokens.at(2).c_str(),NULL))) // try to add this pixel pair combination
					nAddedLECuts++; // in case of success, increment counter
				break;
			default:
				continue; // do nothing
		}
	}// end of loop over input file
	return (nAddedLECuts);
}

Bool_t TFlashAnalysis::AddPixelToTCut(UInt_t nUserChan, Double_t fLow, Double_t fHigh){ // add pixel cut definitions
	std::pair<Double_t, Double_t> TempCut (fLow, fHigh);
	std::pair<std::map< UInt_t, std::pair<Double_t, Double_t> >::iterator,bool> ret;
	ret = PixelTotCuts.insert(std::make_pair(nUserChan,TempCut));
	return (ret.second);
}

UInt_t TFlashAnalysis::AddPixelToTCuts(string cUserFileName){
	UInt_t nAddedTotCuts = 0;
	if(cUserFileName.empty()){ // check if user pair list name string is empty
//		if(bVerboseMode)
			cout << "Filename string is empty!" << endl;
		return (0);
	}
	ifstream UserInputFile(cUserFileName.c_str(),ifstream::in); // open text file containing pixel pair combinations
	if(!UserInputFile.is_open()){ // check if opening text file was successful
		cerr << "Could not open Leading Edge cuts file " << cUserFileName << endl;
		return (0);
	}
	while(UserInputFile.good()){ // start loop over input file
		string cCurrentLine;
		getline(UserInputFile,cCurrentLine); // get line from input file
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		vector<string> tokens = LineParser(cCurrentLine,' ');
		switch (tokens.size()) {
			case 3: // time-over-threshold cut definition must contain three values
				if(AddPixelToTCut((UInt_t)strtoul(tokens.at(0).c_str(),NULL,10),(Double_t)strtod(tokens.at(1).c_str(),NULL),(Double_t)strtod(tokens.at(2).c_str(),NULL))) // try to add this pixel pair combination
					nAddedTotCuts++; // in case of success, increment counter
				break;
			default:
				continue; // do nothing
		}
	}// end of loop over input file
	return (nAddedTotCuts);
}

Bool_t TFlashAnalysis::AddPixelPair(UInt_t nUserChanA, UInt_t nUserChanB){
	std::pair<std::set< std::pair<UInt_t,UInt_t> >::iterator,bool> ret;
	ret = PixelPairs.insert(std::make_pair(nUserChanA,nUserChanB));
	//cout << PixelPairs.size() << endl;
	bIsSortedListOfPairs = kFALSE;
	if(ret.second){ // adding new pair was successful
		PairResults temp;
		temp.Init();
		PairHistograms.insert(std::make_pair(std::make_pair(nUserChanA,nUserChanB),temp));
	}
	bSettingsHaveChanged = kTRUE;
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

Bool_t TFlashAnalysis::AddTriggerChannel(UInt_t nUserChannel){
	std::pair<std::set<UInt_t>::iterator,bool> ret;
	ret = TriggerChannels.insert(nUserChannel);
	return (ret.second);
}

void TFlashAnalysis::Analyse(){
	if(bSettingsHaveChanged && pLogFile!=NULL){ // print status to log file if changes have happened
		PrintExcludedChannels(1);
		PrintListOfPixelPairs(1);
		bSettingsHaveChanged = kFALSE; // reset flag
	}
	Clear(); // reset all variables needed in analysis of one event
	if(!bIsSortedListOfPairs){
		//std::sort(PixelPairs.begin(),PixelPairs.end());
		bIsSortedListOfPairs = kTRUE;
	}
	//Int_t nEntrySize = GetEntry(nEntryIndex);
	//if(nEntrySize<1){// if entry is smaller than 1, something went wrong
	//	return;
	//}
	nNumberOfHitPixels = (UInt_t)EvtReconHits.size(); // number of reconstructed hits is just the size of this map object
	ApplyPixelCuts();
	if(!PixelPairs.empty()){
		for (std::set< std::pair<UInt_t,UInt_t> >::const_iterator it=PixelPairs.begin(); it!=PixelPairs.end(); ++it){ // loop over all user-defined pixel pairs
			std::map< UInt_t,std::list<PixelHitModel> >::const_iterator itA;
			std::map< UInt_t,std::list<PixelHitModel> >::const_iterator itB;
			itA = EvtReconHits.find(it->first); // find first pixel
			itB = EvtReconHits.find(it->second); // find second pixel
			if(itA!=EvtReconHits.end() && itB!=EvtReconHits.end()){
				std::list<PixelHitModel>::const_iterator FirstHit = itA->second.begin();
				std::list<PixelHitModel>::const_iterator LastHit = itB->second.begin();
				std::pair<PIXELPAIR::const_iterator,bool> ret;
				ret = DetectedPixelPairs.insert(std::make_pair(*it,std::make_pair(FirstHit,LastHit)));
				FillHistograms(ret.first);
			}
		} // end of loop over all user-defined pixel pairs
	}
}

void TFlashAnalysis::AnalyseTrigger(string cUserAnalysisFilename){

}

void TFlashAnalysis::ApplyPixelCuts(){
	std::map< UInt_t,std::list<PixelHitModel> >::iterator first = EvtReconHits.begin();
	std::map< UInt_t,std::list<PixelHitModel> >::iterator last = EvtReconHits.end();
	std::map< UInt_t,std::list<PixelHitModel> >::iterator it = first;
	while(it != last){ // begin loop over all reconstructed hits
		if(bApplyLECuts){
			std::map< UInt_t, std::pair<Double_t, Double_t> >::const_iterator LECut = PixelLECuts.find(it->first); // find leading edge cut for this pixel
			if(LECut!=PixelLECuts.end()){ // found cut and apply now
				Double_t fLETemp = it->second.begin()->GetLeadEdgeTime();
				if(fLETemp<LECut->second.first || fLETemp>LECut->second.second){ // hit not passing leading edge cut
					EvtReconHits.erase(it++); // increment iterator and delete element
					continue; // skip rest of loop
				}
			}
		}
		if(bApplyTotCuts){
			std::map< UInt_t, std::pair<Double_t, Double_t> >::const_iterator ToTCut = PixelTotCuts.find(it->first); // find ToT cut for this pixel
			if(ToTCut != PixelTotCuts.end()){ // found cut and apply now
				Double_t fTotTemp = it->second.begin()->GetToT();
				if(fTotTemp<ToTCut->second.first || fTotTemp>ToTCut->second.second){
					EvtReconHits.erase(it++); // increment iterator and delete element
					continue; // skip rest of loop
				}
			}
		}
		++it;
	}// end of loop over all reconstructed hits
}

void TFlashAnalysis::Clear(){ // clear results of event analysis
	nNumberOfHitPixels		= 0;
	nNumberOfFiredTriggers	= 0;
	DetectedPixelPairs.clear();
}

void TFlashAnalysis::ClearPixelTimeOffset(UInt_t nUserSeqId){
	cout << PixelTimeOffsets.size() << endl;
	std::map<UInt_t,Double_t>::iterator it;
	it = PixelTimeOffsets.find(nUserSeqId);
	if(it!=PixelTimeOffsets.end())
		PixelTimeOffsets.erase(it);
	cout << PixelTimeOffsets.size() << endl;
}


Double_t TFlashAnalysis::ComputePixelTimeDiff(PIXELPAIR::const_iterator UserPair) const {
	Double_t fTimeDiff = 0.0;
	Double_t fTimePixelA = (bApplyOffset)? UserPair->second.first->GetLeadEdgeTime() - GetPixelOffset(UserPair->first.first) : UserPair->second.first->GetLeadEdgeTime(); // compute corrected leading edge time of first pixel
	Double_t fTimePixelB = (bApplyOffset)? UserPair->second.second->GetLeadEdgeTime() - GetPixelOffset(UserPair->first.second) : UserPair->second.second->GetLeadEdgeTime(); // compute corrected leading edge time of first pixel
	fTimeDiff = fTimePixelA - fTimePixelB;
	return (fTimeDiff);
}

void TFlashAnalysis::FillHistograms(PIXELPAIR::const_iterator it){
	std::map< std::pair<UInt_t,UInt_t>, PairResults >::iterator itResults;
	itResults = PairHistograms.find(std::make_pair(it->first.first,it->first.second));
	if(itResults!=PairHistograms.end()){
		if(itResults->second.RegisteredHistograms.test(HTIME)){
			itResults->second.hTimeDifference->Fill(ComputePixelTimeDiff(it));
		}
		if(itResults->second.RegisteredHistograms.test(HTOTCORR)){
			itResults->second.hToTCorrelation->Fill(it->second.first->GetToT(),it->second.second->GetToT());
		}
		if(itResults->second.RegisteredHistograms.test(HWALK)){
			itResults->second.hTimeWalk->Fill(it->second.second->GetToT(),ComputePixelTimeDiff(it));
		}
	}
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
		std::list<PixelHitModel>::const_iterator FirstHit = it->second.begin();
			std::list<PixelHitModel>::const_iterator LastHit = it->second.end();
		for(hit=FirstHit; hit!=LastHit; ++hit){ // begin loop over all hits in channel
			hTimingHist.Fill((Double_t)hit->GetLeadEdgeChan(),hit->GetLeadEdgeTime());
		} // end of loop over all hits in channel
	} // end of loop over all channels

}

void TFlashAnalysis::FillToTCorrelation(UInt_t nChanA, UInt_t nChanB, TH2D& hUserHist) const {
	PIXELPAIR::const_iterator it;
	std::pair<UInt_t,UInt_t> UserPair (nChanA,nChanB);
	it = DetectedPixelPairs.find(UserPair);
	if(it!=DetectedPixelPairs.end()){
		hUserHist.Fill(it->second.first->GetToT(),it->second.second->GetToT());
	}
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

void TFlashAnalysis::FillWalkHistogram(UInt_t nRefChan, Double_t fToTLow, Double_t fTotHigh, UInt_t nUserChan, TH2D& hUserHist) const {
	PIXELPAIR::const_iterator it;
	std::pair<UInt_t,UInt_t> UserPair (nRefChan,nUserChan);
	it = DetectedPixelPairs.find(UserPair);
	if(it!=DetectedPixelPairs.end()){
		if(it->second.first->GetToT()>fToTLow && it->second.first->GetToT()<fTotHigh)
			hUserHist.Fill(it->second.second->GetToT(),ComputePixelTimeDiff(it));
	}
}

const std::set< std::pair<UInt_t,UInt_t> >* TFlashAnalysis::GetListOfPixelPairs() {
	if(!bIsSortedListOfPairs){ // check if set of pixel pair combinations is sorted
		//std::sort(PixelPairs.begin(),PixelPairs.end()); // if not, sort it
		bIsSortedListOfPairs = kTRUE;
	}
	return &PixelPairs; // return pointer to set
}

Bool_t TFlashAnalysis::GetPairTimeDiff(std::pair<UInt_t,UInt_t> UserPair, Double_t& fDelta) const { // get leading edge time difference between two pixels
	PIXELPAIR::const_iterator it;
	it = DetectedPixelPairs.find(UserPair);
	if(it!=DetectedPixelPairs.end()){
		fDelta = ComputePixelTimeDiff(it);
		return (kTRUE);
	}
	return (kFALSE);
}

Double_t TFlashAnalysis::GetPixelOffset(UInt_t nSeqId) const{
	std::map< UInt_t, Double_t >::const_iterator ThisOffset = PixelTimeOffsets.find(nSeqId);
	if(ThisOffset!=PixelTimeOffsets.end()) // pixel has offset
		return (ThisOffset->second);
	else // pixel has no offset
		return (0.0);
}

void TFlashAnalysis::Init(){
	bApplyLECuts	= kTRUE;
	bApplyOffset	= kTRUE;
	bApplyTotCuts	= kTRUE;
	bIsSortedListOfPairs = kFALSE; // list of pixels is not sorted
	bSettingsHaveChanged = kFALSE; // indicate if analysis settings have changed
	nNumberOfHitPixels = 0;
	PixelTimeOffsets.clear();
	PixelPairs.clear();
	PixelLECuts.clear();
	PixelTotCuts.clear();
	PairHistograms.clear();
	DetectedPixelPairs.clear();
	TriggerChannels.clear();
	pLogFile = NULL;
	//buf = std::cout.rdbuf();
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

void TFlashAnalysis::PrintExcludedChannels(Bool_t bWriteToLog) const {
	// redirect cout buffer to logfile
	std::streambuf *psbuf, *backup;
	backup = std::cout.rdbuf();
	if(bWriteToLog && pLogFile!=NULL){
		if(!pLogFile->is_open()) // no file is associated with LogFileBuffer
			return;
		psbuf = pLogFile->rdbuf();
		std::cout.rdbuf(psbuf);         // assign streambuf to cout
	}
	TTrbAnalysisBase::PrintExcludedChannels();
	// reset cout buffer to terminal
	std::cout.rdbuf(backup);        // restore cout's original streambuf
	psbuf = NULL;
	backup = NULL;
}

void TFlashAnalysis::PrintListOfPixelPairs(Bool_t bWriteToLog) const {
	if(PixelPairs.empty()){
		cout << "ERROR: No pixel pairs declared!" << endl;
		return;
	}
	// redirect cout buffer to logfile
	std::streambuf *psbuf, *backup;
	backup = std::cout.rdbuf();
	if(bWriteToLog && pLogFile!=NULL){
		if(!pLogFile->is_open()) // no file is associated with LogFileBuffer
			return;
		psbuf = pLogFile->rdbuf();
		std::cout.rdbuf(psbuf);         // assign streambuf to cout
	}
	std::set< std::pair<UInt_t,UInt_t> >::const_iterator start = PixelPairs.begin();
	std::set< std::pair<UInt_t,UInt_t> >::const_iterator stop = PixelPairs.end();
	std::set< std::pair<UInt_t,UInt_t> >::const_iterator it;
	cout << "++++++++++++++++++++++++++++++++++" << endl;
	cout << "+++ User Declared Pixel Pairs  +++" << endl;
	cout << "++++++++++++++++++++++++++++++++++" << endl; 
	for(it=start; it!=stop; ++it){
		cout << it->first << "\t" << it->second << endl;
	}
	// reset cout buffer to terminal
	std::cout.rdbuf(backup);        // restore cout's original streambuf
	psbuf = NULL;
	backup = NULL;
}

Bool_t TFlashAnalysis::RegisterTimeDiffHist(UInt_t nChanA, UInt_t nChanB, TH1D* hUserHist){
	if(hUserHist==NULL) // histogram pointer does not exist
		return kFALSE;
	std::map< std::pair<UInt_t,UInt_t>, PairResults >::iterator it;
	it = PairHistograms.find(std::make_pair(nChanA,nChanB));
	if(it!=PairHistograms.end()){ // found pixel pair
		it->second.hTimeDifference		= hUserHist;
		it->second.RegisteredHistograms.set(HTIME);
		return (kTRUE);
	}
	return (kFALSE);
}

Bool_t TFlashAnalysis::RegisterTimeWalkHist(UInt_t nChanA, UInt_t nChanB, TH2D* hUserHist){
	if(hUserHist==NULL) // histogram pointer does not exist
		return kFALSE;
	std::map< std::pair<UInt_t,UInt_t>, PairResults >::iterator it;
	it = PairHistograms.find(std::make_pair(nChanA,nChanB));
	if(it!=PairHistograms.end()){ // found pixel pair
		it->second.hTimeWalk		= hUserHist;
		it->second.RegisteredHistograms.set(HWALK);
		return (kTRUE);
	}
	return (kFALSE);
}

Bool_t TFlashAnalysis::RegisterTotCorrHist(UInt_t nChanA, UInt_t nChanB, TH2D* hUserHist){
	if(hUserHist==NULL) // histogram pointer does not exist
		return kFALSE;
	std::map< std::pair<UInt_t,UInt_t>, PairResults >::iterator it;
	it = PairHistograms.find(std::make_pair(nChanA,nChanB));
	if(it!=PairHistograms.end()){ // found pixel pair
		it->second.hToTCorrelation		= hUserHist;
		it->second.RegisteredHistograms.set(HTOTCORR);
		return (kTRUE);
	}
	return (kFALSE);
}

Bool_t TFlashAnalysis::SetPixelTimeOffset(UInt_t nUserSeqId, Double_t fUserOffset){
	std::pair<std::map<UInt_t,Double_t>::iterator,bool> ret;
	ret = PixelTimeOffsets.insert(std::make_pair(nUserSeqId,fUserOffset)); // try to insert pixel time offset into map
	if(!ret.second){ // initial insertion failed because a value already existed
		if(ret.first!=PixelTimeOffsets.end()){
			PixelTimeOffsets.erase(ret.first); // erase exisiting entry
			ret = PixelTimeOffsets.insert(std::make_pair(nUserSeqId,fUserOffset)); // insert again
		}
	}
	if(ret.second){
		ApplyOffsets();
	}
	return (ret.second);
}

UInt_t TFlashAnalysis::SetPixelTimeOffsets(string cUserFileName){
	UInt_t nAddedPixelOffsets = 0;
	if(cUserFileName.empty()){ // check if user pair list name string is empty
//		if(bVerboseMode)
			cout << "File name string is empty!" << endl;
		return (0);
	}
	ifstream UserInputFile(cUserFileName.c_str(),ifstream::in); // open text file containing pixel pair combinations
	if(!UserInputFile.is_open()){ // check if opening text file was successful
		cerr << "Could not open pixel offset file " << cUserFileName << endl;
		return (0);
	}
	while(UserInputFile.good()){ // start loop over input file
		string cCurrentLine;
		getline(UserInputFile,cCurrentLine); // get line from input file
		if(cCurrentLine.empty()) // skip empty lines
			continue;
		vector<string> tokens = LineParser(cCurrentLine,' ');
		switch (tokens.size()) {
			case 2: // pixel offset should contain only two values
				if(SetPixelTimeOffset((UInt_t)strtoul(tokens.at(0).c_str(),NULL,10),(Double_t)strtod(tokens.at(1).c_str(),NULL))) // try to set this pixel offset
					nAddedPixelOffsets++; // in case of success, increment counter
				break;
			default:
				continue; // do nothing
		}
	}// end of loop over input file
	return (nAddedPixelOffsets);
}

void TFlashAnalysis::WriteLogfileHeader(){
	if(pLogFile!=NULL && !pLogFile->is_open()) // no file is associated with LogFileBuffer
		return;
	time(&RawTime);
	std::streambuf *psbuf, *backup;
	*pLogFile << "+++++++++++++++++++++++++++++++++++++" << endl;
	*pLogFile << "+      FLASH Analysis Logfile       +" << endl;
	*pLogFile << "+++++++++++++++++++++++++++++++++++++" << endl;
	*pLogFile << "+    Analysis Setup Information     +" << endl;
	*pLogFile << "+++++++++++++++++++++++++++++++++++++" << endl;
	*pLogFile << "\t" << ctime(&RawTime) << endl;
	*pLogFile << endl;
	// redirect cout buffer to logfile
	backup = std::cout.rdbuf();
	psbuf = pLogFile->rdbuf();
	std::cout.rdbuf(psbuf);         // assign streambuf to cout
	// print status information to logfile
	PrintStatus(); // print general status information of analysis setup
	*pLogFile << endl; // add empty line
	//PrintTdcAddresses(); // print list of TDC addresses
	//LogFileBuffer << endl; // add empty line
	//PrintSwapList(); // print list of TDC where leading & trailing edges need to be swapped
	//LogFileBuffer << endl; // add empty line
	//PrintExcludedChannels(); // print list of excluded channels
	//LogFileBuffer << endl; // add empty line
	//PrintTimingWindow(); // print limits of timing window
	//LogFileBuffer << endl; // add empty line
	//PrintTriggerAddress();	 // print address of trigger channel
	//LogFileBuffer << endl; // add empty line
	//PrintTriggerWindow(); // print limits of trigger time window
	//LogFileBuffer << endl; // add empty line
	//LogFileBuffer << "+++++++++++++++++++++++++++++++++++++" << endl;
	//LogFileBuffer << "+ End of Analysis Setup Information +" << endl;
	//LogFileBuffer << "+++++++++++++++++++++++++++++++++++++" << endl;
	//LogFileBuffer << endl; // add empty line
	// reset cout buffer to terminal
	std::cout.rdbuf(backup);        // restore cout's original streambuf
	psbuf = NULL;
	backup = NULL;

}

void TFlashAnalysis::WriteMessageToLog(string cUserMessage){
	if(pLogFile==NULL) // log file does not exist
		return;
	*pLogFile << ctime(&RawTime) << "\t" << cUserMessage << endl;
}