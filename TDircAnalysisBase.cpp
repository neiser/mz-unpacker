#include "TDircAnalysisBase.h"

ClassImp(TDircAnalysisBase);

TDircAnalysisBase::TDircAnalysisBase(string cUserDataFilename) : TTrbAnalysisBase(cUserDataFilename) {
	// do nothing so far...
	cout << "This is the constructor of TDircAnalysisBase class..." << endl;
	Init();
}

TDircAnalysisBase::TDircAnalysisBase(string cUserDataFilename, string cUserTdcAddressFile) : TTrbAnalysisBase(cUserDataFilename){
	Init();
	cout << SetTdcAddresses(cUserTdcAddressFile) << " TDC addresses registered" << endl;
}

TDircAnalysisBase::TDircAnalysisBase(string cUserDataFilename, string cUserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth) : TTrbAnalysisBase(cUserDataFilename){
	Init();
	SetTdcSize(nUserTdcWidth); // set TDC channel number
	SetTdcOffset(nUserTdcOffset); // set TDC channel offset
	cout << SetTdcAddresses(cUserTdcAddressFile) << " TDC addresses registered" << endl;
}

TDircAnalysisBase::~TDircAnalysisBase(){
	// do nothing
	cout << "This is the destructor of TDircAnalysisBase class..." << endl;
}

std::map< std::pair< Int_t,Int_t >,PixelHitModel >::const_iterator TDircAnalysisBase::FindHitByValue(UInt_t nUserSeqId) const {
	if(MatchedHits.empty())
		return MatchedHits.end();
	std::map< std::pair< Int_t,Int_t >,PixelHitModel >::const_iterator TempIt=MatchedHits.begin();
	while (TempIt!=MatchedHits.end() && TempIt->second.nChannelA!=nUserSeqId){
		++TempIt;
	} 
	return (TempIt);
}

Int_t TDircAnalysisBase::GetChanMultiplicity(UInt_t nSeqChanId) const { // count hits in given channel
	if((nSeqChanId%2)!=0){
		nSeqChanId -= 1;
	}
	std::map< UInt_t,Int_t >::const_iterator SeekChannel = EvtChanMultiplicity.find(nSeqChanId);

	if(SeekChannel!=EvtChanMultiplicity.end()){ // channel exists in map
		return (SeekChannel->second);
	}
	else // channel does not exist in map
		return (0);
}

Bool_t TDircAnalysisBase::GetTriggerTime(Double_t &fTriggerTime) const { // return calibrated time of trigger channel
	fTriggerTime = 0.0;
	if(MatchedHits.empty()||!bTrigChanIsSet){ // no matched hits
		return (kFALSE);
	}
	std::map< std::pair< Int_t,Int_t >,PixelHitModel >::const_iterator TriggerHit = FindHitByValue(nTriggerSeqId);
	if(TriggerHit==MatchedHits.end()){ // trigger channel not found in matched hit list
		return (kFALSE);
	}
	fTriggerTime = TriggerHit->second.fSyncLETime;
	return (kTRUE);
}

void TDircAnalysisBase::HitMatching(){ // match leading and trailing edge timestamps, returning the number of channels with multiple hits
	// match leading and trailing edge timestamps
	// use EvtTdcHits multimap from TTrbAnalysisBase as starting point
	if(!GetStatus()) // TTrbAnalysisBase object not properly initialised, do not match hits
		return;
	Int_t nMultipleHits = 0;
	nMultiHitChan		= 0;
	MatchedHits.clear(); // clear map containing matched hits
	EvtChanMultiplicity.clear();
	if(EvtTdcHits.empty()) // no TDC hits available
		return;
	std::multimap< UInt_t,UInt_t >::const_iterator CurrentTdcHit=EvtTdcHits.begin();
	while(CurrentTdcHit!=EvtTdcHits.end()){ // begin of loop over all TDC hits (excluding reference channels & user exclude list)
		PixelHitModel TempPixelHit;
		TempPixelHit.bHasSyncTime = kFALSE;
		std::pair< std::multimap< UInt_t,UInt_t >::const_iterator,std::multimap< UInt_t,UInt_t >::const_iterator > LeadingEdges;
		std::pair< std::multimap< UInt_t,UInt_t >::const_iterator,std::multimap< UInt_t,UInt_t >::const_iterator > TrailingEdges;
		if(((CurrentTdcHit->first) % 2)!=0){ // channel number not even, skip this entry (hit must start with an even-numbered channel
			CurrentTdcHit = EvtTdcHits.upper_bound(CurrentTdcHit->first);
			continue; // skip rest of loop
		}
		LeadingEdges = EvtTdcHits.equal_range(CurrentTdcHit->first); // find range of entries for leading edges
		if(LeadingEdges.second==EvtTdcHits.end()){
			break;
		}
		TrailingEdges = EvtTdcHits.equal_range(CurrentTdcHit->first+1); // find range of entries for trailing edges, trailing edge channel is leading edge + 1
		UInt_t nMultLeadEdge = (UInt_t) std::distance(LeadingEdges.first,LeadingEdges.second); // count hits for this channel
		UInt_t nMultTrailEdge = (UInt_t) std::distance(TrailingEdges.first,TrailingEdges.second);
		if(nMultLeadEdge!=nMultTrailEdge){ //mismatch of leading & trailing edge multiplicities
			CurrentTdcHit = TrailingEdges.second; // skip these entries
			if(bVerboseMode){ 
				cout << "Mismatch of leading and trailing edge hits!" << endl;
				cout << "Multiplicities: " << nMultLeadEdge << "\t" << nMultTrailEdge << endl;
			}
			continue; // skip rest of loop
		}
		Bool_t bSwapEdges = (SwapList.find(GetTdcAddress(LeadingEdges.first->second))!=SwapList.end()) ? kTRUE : kFALSE;
		std::pair< Int_t,Int_t > TempHitIndices;
		std::multimap< UInt_t,UInt_t >::const_iterator CurLeadEdge	= (bSwapEdges) ? TrailingEdges.first : LeadingEdges.first;
		std::multimap< UInt_t,UInt_t >::const_iterator CurTrailEdge = (bSwapEdges) ? LeadingEdges.first : TrailingEdges.first;
		std::pair<std::map<UInt_t,Int_t>::iterator,bool> ret;
		switch(nMultLeadEdge){
			case 1: // single hit
				//if()
				TempHitIndices = make_pair(CurLeadEdge->second,CurTrailEdge->second); // here we have the array indices of the leading and trailing edge
				// check if we have to swap goes here...
				TempPixelHit.nChannelA = CurLeadEdge->first; // get unique channel ID of leading edge
				TempPixelHit.nChannelB = CurTrailEdge->first; // get unique channel ID of trailing edge
				TempPixelHit.fTimeOverThreshold = GetTime(TempHitIndices.second) - GetTime(TempHitIndices.first); // compute time-over-threshold
				TempPixelHit.nSyncIndex = GetTdcSyncIndex(GetTdcAddress(TempHitIndices.first)); // get array index of corresponding synchronisation timestamp
				TempPixelHit.bHasSyncTime = (TempPixelHit.nSyncIndex<0)? kFALSE : kTRUE;
				if(TempPixelHit.bHasSyncTime){
					TempPixelHit.fSyncLETime = GetTime(TempHitIndices.first) - GetTdcSyncTimestamp(GetTdcAddress(TempHitIndices.first));
					if(bApplyTimingCut){ // check timing of leading edge
						if(TempPixelHit.fSyncLETime<TimingWindow.first || TempPixelHit.fSyncLETime>TimingWindow.second){ // check if hit is outwith timing window
							CurrentTdcHit = TrailingEdges.second; // shift iterator to next possible hit
							break; // continue w/o entering this hit into the multimap
						}
					}
				}
				MatchedHits.insert(make_pair(TempHitIndices,TempPixelHit)); // enter this combination into pixel hit map
				ret = EvtChanMultiplicity.insert(make_pair(TempPixelHit.nChannelA,1)); // try to insert this pair into multiplicity map
				if(!ret.second){ // insertion failed, key already exists so we need to increment its value
					ret.first->second += 1;
				}
				//}
				CurrentTdcHit = TrailingEdges.second;
				break; // end of single hits only case
			default: // multiple hits
				++nMultiHitChan;
				if(bSkipMultiHits){ // user decision to skip multiple hits
					CurrentTdcHit = EvtTdcHits.upper_bound(CurrentTdcHit->first); // increment iterator to skip multiple hits
					continue; // skip rest of loop
				}
				// need to get iterators to leading and trailing edges
				//cout << "Matching multi hits now..." << endl;
				do{ // begin of loop over hits
					TempHitIndices = make_pair(CurLeadEdge->second,CurTrailEdge->second);
					TempPixelHit.nChannelA = CurLeadEdge->first;
					TempPixelHit.nChannelB = CurTrailEdge->first;
					TempPixelHit.fTimeOverThreshold = GetTime(TempHitIndices.second) - GetTime(TempHitIndices.first); // compute time-over-threshold
					TempPixelHit.nSyncIndex = GetTdcSyncIndex(TrbData->Hits_nTrbAddress[CurLeadEdge->second]);
					TempPixelHit.bHasSyncTime = (TempPixelHit.nSyncIndex<0)? kFALSE : kTRUE;
					if(TempPixelHit.bHasSyncTime){
						TempPixelHit.fSyncLETime = GetTime(TempHitIndices.first) - GetTdcSyncTimestamp(GetTdcAddress(TempHitIndices.first));
						if(bApplyTimingCut){ // check timing of leading edge
							if(TempPixelHit.fSyncLETime<TimingWindow.first || TempPixelHit.fSyncLETime>TimingWindow.second){
								++CurLeadEdge;
								++CurTrailEdge;
								continue; // continue w/o entering this hit into the multimap
							}
						}
				
					}
					MatchedHits.insert(make_pair(TempHitIndices,TempPixelHit)); // enter this combination into pixel hit map
					ret = EvtChanMultiplicity.insert(make_pair(TempPixelHit.nChannelA,1)); // try to insert this pair into multiplicity map
					if(!ret.second){ // insertion failed, key already exists so we need to increment its value
						ret.first->second += 1;
					}
					++CurLeadEdge;
					++CurTrailEdge;

				} while (CurLeadEdge!=((bSwapEdges)? TrailingEdges.second : LeadingEdges.second)); // end of loop over all hits
				CurrentTdcHit = TrailingEdges.second;
				break;
		}
	}
	if(bVerboseMode){
		cout << MatchedHits.size() << endl;
	}
}

void TDircAnalysisBase::Init(){
	cout << "This is TDircAnalysisBase::Init()..." << endl;
	SwapList.clear(); // clear list of TDC addresses which are being swapped
	MatchedHits.clear(); // clear map containing the matched hits
	EvtChanMultiplicity.clear();
	bSkipMultiHits	= kTRUE;
	bApplyTimingCut = kFALSE;
	bTrigChanIsSet	= kFALSE;
	bVerboseMode	= kFALSE;
	nMultiHitChan = 0;
	nTriggerSeqId = -1;
}

Bool_t TDircAnalysisBase::IsChannel(const PixelHitModel &CurrentHit, UInt_t nSeqChanId) const {
	if((CurrentHit.nChannelA == nSeqChanId) || (CurrentHit.nChannelB == nSeqChanId))
		return (kTRUE);
	else
		return (kFALSE);
}

void TDircAnalysisBase::PrintChanMultiplicity() const {
	if(EvtChanMultiplicity.empty())
		return;
	std::map< UInt_t,Int_t >::const_iterator FirstChannel	= EvtChanMultiplicity.begin();
	std::map< UInt_t,Int_t >::const_iterator LastChannel	= EvtChanMultiplicity.end();
	cout << "++++++++++++++++++++++++++++++" << endl;
	cout << "+++  Channel Multiplicity  +++" << endl;
	cout << "++++++++++++++++++++++++++++++" << endl;
	for(std::map< UInt_t,Int_t >::const_iterator CurChannel=FirstChannel; CurChannel!=LastChannel; ++CurChannel){
		cout << CurChannel->first << "\t" << CurChannel->second << endl;
	}
	cout << "++++++++++++++++++++++++++++++" << endl;
}

void TDircAnalysisBase::PrintMatchedHits() const {
	if(MatchedHits.empty())
		return;
	std::map< std::pair< Int_t,Int_t >,PixelHitModel >::const_iterator FirstEntry = MatchedHits.begin();
	std::map< std::pair< Int_t,Int_t >,PixelHitModel >::const_iterator LastEntry = MatchedHits.end();
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "+++  Matched TDC Hits   +++" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << MatchedHits.size() << " MATCHED TDC HITS FOUND" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	for(std::map< std::pair< Int_t,Int_t >,PixelHitModel >::const_iterator CurEntry=FirstEntry; CurEntry!=LastEntry; ++CurEntry){ // begin loop over all event hit entries
		cout << distance(FirstEntry,CurEntry) << "\t" << CurEntry->first.first << " , " << CurEntry->first.second << "\t" << CurEntry->second.nChannelA  << " , " << CurEntry->second.nChannelB << "\t" << CurEntry->second.fSyncLETime  << "\t" <<CurEntry->second.fTimeOverThreshold << endl;
	} // end of loop over all event hit entries
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TDircAnalysisBase::PrintSwapList() const {
	if(SwapList.empty()) // list is empty, nothing to print
		return;
	std::set< UInt_t >::const_iterator FirstEntry	= SwapList.begin();
	std::set< UInt_t >::const_iterator LastEntry	= SwapList.end();
	cout << "++++++++++++++++++++++++++" << endl;
	cout << "+++   TDC Swap List    +++" << endl;
	cout << "++++++++++++++++++++++++++" << endl;
	for(std::set< UInt_t >::const_iterator CurEntry=FirstEntry; CurEntry!=LastEntry; ++CurEntry) { // begin loop over all entries in swap list
		cout << hex << *CurEntry << dec << endl;
	} // end of loop over all entries in swap list
	cout << "++++++++++++++++++++++++++" << endl;
}

void TDircAnalysisBase::PrintTimingWindow() const {
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "+++    Timing Window    +++" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "t_0: " << TimingWindow.first << "\t t_1: " << TimingWindow.second << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TDircAnalysisBase::PrintTriggerAddress() const {
	if(!bTrigChanIsSet) // trigger channel is not set
		return;
	cout << "+++++++++++++++++++++++++++++++++++++" << endl;
	cout << "+++    Trigger Channel Address    +++" << endl;
	cout << "+++++++++++++++++++++++++++++++++++++" << endl;
	cout << hex << TriggerChannelAddress.first << dec << "\t" << TriggerChannelAddress.second << "\t" << nTriggerSeqId << endl;
	cout << "+++++++++++++++++++++++++++++++++++++" << endl;
}

Bool_t TDircAnalysisBase::SetTriggerChannel(UInt_t nUserTdcAddress, UInt_t nUserTdcChannel) { // set trigger channel address
	Int_t nTempSeqId = GetSeqId(nUserTdcAddress,nUserTdcChannel);
	if(nTempSeqId<0){ // channel address does not exist

		bTrigChanIsSet = kFALSE;
		return (bTrigChanIsSet);
	}
	nTriggerSeqId = nTempSeqId;
	TriggerChannelAddress = std::make_pair(nUserTdcAddress,nUserTdcChannel);
	bTrigChanIsSet = kTRUE;
	return (bTrigChanIsSet);
}

void TDircAnalysisBase::Show(){
	if(!GetTreeStatus())
		return;
	TrbData->Show(0);
}


void TDircAnalysisBase::SwapEdges(UInt_t nUserTdcAddress){
	if(TdcAddresses.find(nUserTdcAddress)==TdcAddresses.end()) // this TDC address is not in the list
		return;
	SwapList.insert(nUserTdcAddress); // enter TDC address in swap list
}