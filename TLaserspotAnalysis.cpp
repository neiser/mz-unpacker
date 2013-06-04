#include "TLaserspotAnalysis.h"

ClassImp(TLaserspotAnalysis);

TLaserspotAnalysis::TLaserspotAnalysis(string cUserDataFilename, UInt_t nUserLaserTrbAddress, UInt_t nUserLaserTdcChan, UInt_t nUserMcpSpotTrbAddress, UInt_t nUserMcpSpotTdcChan) : TTrbAnalysisBase(cUserDataFilename) {
	// do nothing so far...
	cout << "This is the constructor of TLaserspotAnalysis class..." << endl;
	Init();

}

TLaserspotAnalysis::TLaserspotAnalysis(string cUserDataFilename, string cUserTdcAddressFile, UInt_t nUserLaserTrbAddress, UInt_t nUserLaserTdcChan, UInt_t nUserMcpSpotTrbAddress, UInt_t nUserMcpSpotTdcChan, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth) : TTrbAnalysisBase(cUserDataFilename){
	Init();
	SetTdcSize(nUserTdcWidth); // set TDC channel number
	SetTdcOffset(nUserTdcOffset); // set TDC channel offset
	cout << SetTdcAddresses(cUserTdcAddressFile) << " TDC addresses registered" << endl;
	SetLaserTriggerAddress(nUserLaserTrbAddress,nUserLaserTdcChan);
	SetMcpSpotAddress(nUserMcpSpotTrbAddress,nUserMcpSpotTdcChan);
}

TLaserspotAnalysis::~TLaserspotAnalysis(){
	// do nothing
	cout << "This is the destructor of TLaserspotAnalysis class..." << endl;
}

void TLaserspotAnalysis::Analyse(string cUserAnalysisFilename){
	Int_t nLaserTrigSeqId	= GetSeqId(LaserTriggerAddress.first,LaserTriggerAddress.second);
	Int_t nMcpSpotSeqId		= GetSeqId(McpSpotAddress.first,McpSpotAddress.second);
	if(nLaserTrigSeqId<0 || nMcpSpotSeqId<0){ // check if laser trigger and MCP-PMT spot addresses are valid
		cout << "Laser trigger channel or MCP-PMT spot channel out of range!" << endl;
		PrintLaserTriggerAddress();
		PrintMcpSpotAddress();
		return; // abort analysis
	}
	TFile *AnalysisOut = new TFile(cUserAnalysisFilename.c_str(),"RECREATE"); // open RooT file for analysis results
	// define histograms
	TH1D hEvtStats("hEvtStats","hEvtStats; ; frequency",15,-0.5,14.5);
	enum DQCuts {NO_CUTS,DECODE_ERR,RNDM_BIT_ERR,SYNC_ERR,NO_HITS_ERR,NO_MATCH_ERR,NO_LASER_ERR}; //empty TDC cut,missing reference signal cut,laser trig missing,};
	hEvtStats.GetXaxis()->SetBinLabel(NO_CUTS+1,"no cuts");
	hEvtStats.GetXaxis()->SetBinLabel(DECODE_ERR+1,"decoding error");
	hEvtStats.GetXaxis()->SetBinLabel(RNDM_BIT_ERR+1,"random bit error");
	hEvtStats.GetXaxis()->SetBinLabel(SYNC_ERR+1,"TDC sync error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_HITS_ERR+1,"no TDC hits error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_MATCH_ERR+1,"no matched hits error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_LASER_ERR+1,"no laser trigger error");
	TH1D hLaserTrigWidth("hLaserTrigWidth","hLaserTrigWidth; signal width (ns); frequency",1000,0.0,50.0); // width of laser trigger signal
	TH1D hMcpSpotSigWidth("hMcpSpotSigWidth","hMcpSpotSigWidth; MCP-PMT signal width (ns); frequency",1000,0.0,25.0); // signal width of MCP-PMT spot signal

	TH1D hTdcSync("hTdcSync","hTdcSync",(Int_t)GetNTdcs()+2,-0.5,GetNTdcs()+1.5);
	TH1D hEvtMultiplicity("hEvtMultiplicity","hEvtMultiplicity;",16,-0.5,15.5);
	TH1D hTdcHits("hTdcHits","hTdcHits",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5);
	TH2D hTdcHitMult("hTdcHitMult","hTdcHitMult",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5,26,-0.5,25.5);
	TH1D hPixelHits("hPixelHits","hPixelHits",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5);
	TH2D hPixelTot("hPixelTot","hPixelTot",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5,610,-1.0,50.0);
	TH2D hHitTimeVsChannel("hHitTimeVsChannel","hHitTimeVsChannel",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5,2000,-2500,2500);
	// analysis variables
	// main analysis loop
	for(Int_t i=0; i<GetNEvents(); i++){ // begin loop over all events
		// first, get event data
		if (GetEntry(i)<1){ // this entry is not valid
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)NO_CUTS);
		// now check data quality
		if(!CheckDecodingStatus()) // check if there were any problems during conversion
			continue; // skip rest of loop
		hEvtStats.Fill((Double_t)DECODE_ERR);
		if(!CheckRandomBits()){ // random bits in event do not match
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)RNDM_BIT_ERR);
		UInt_t nSyncTimestampsFound = GetNSyncTimestamps();
		hTdcSync.Fill((Double_t)nSyncTimestampsFound);
		if(nSyncTimestampsFound!=GetNTdcs()){ // check if all TDCs have a sync timestamp
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)SYNC_ERR);
		if(EvtTdcHits.empty()){ // check if there are no TDC hits in this event
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)NO_HITS_ERR);
		// fill channel hit distribution plots
		std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator FirstChan = MappingTable.begin();
		std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator LastChan	= MappingTable.end();
		for(std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator CurChan=FirstChan; CurChan!=LastChan; ++CurChan){ // begin of loop over all TDC channels
			UInt_t nTempMult = EvtTdcHits.count(CurChan->second); // count number of TDC hits in current channel
			hTdcHits.Fill((Double_t)CurChan->second,(Double_t)nTempMult);
			hTdcHitMult.Fill((Double_t)CurChan->second,(Double_t)nTempMult);
		} // end of loop over all TDC channels
		hEvtMultiplicity.Fill((Double_t)MatchedHits.size());
		if(MatchedHits.empty()){
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)NO_MATCH_ERR);
		// now check if we have the laser trigger signal
		std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator LaserTrigHit = FindHitByValue(nLaserTrigSeqId);
		if(LaserTrigHit==MatchedHits.end()){ // no laser trigger signal found
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)NO_LASER_ERR);
		hLaserTrigWidth.Fill(fabs(LaserTrigHit->second.fTimeOverThreshold));
		// now check if we have a signal in the expected MCP-PMT channel
		std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator McpSpotHit = FindHitByValue(nMcpSpotSeqId);
		if(McpSpotHit!=MatchedHits.end()){ // MCP-PMT spot signal found
			hMcpSpotSigWidth.Fill(McpSpotHit->second.fTimeOverThreshold);
		}
		std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator FirstHit	= MatchedHits.begin();
		std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator LastHit	= MatchedHits.end();
		for(std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator CurHit=FirstHit; CurHit!=LastHit; ++CurHit){ // begin loop over all matched hits
			hPixelHits.Fill((Double_t)CurHit->second.nChannelA);
			if(CurHit->second.bHasSyncTime){
				Double_t fLeadingEdge = TrbData->Hits_fTime[CurHit->first.first] - TrbData->Hits_fTime[CurHit->second.nSyncIndex];
				hHitTimeVsChannel.Fill((Double_t)CurHit->second.nChannelA,fLeadingEdge);
				if(bApplyTimingCut){
					if(fLeadingEdge>TimingWindow.first && fLeadingEdge<TimingWindow.second){ // apply timing cut
						hPixelTot.Fill((Double_t)CurHit->second.nChannelA,CurHit->second.fTimeOverThreshold);
					}
				}
				else
					hPixelTot.Fill((Double_t)CurHit->second.nChannelA,CurHit->second.fTimeOverThreshold);
			}
		}// end loop over all matched hits
		
	} // end of loop over all events
	AnalysisOut->Write(); // write all histograms in memeory to this file
	delete AnalysisOut; // close RooT file and delete pointer
}

std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator TLaserspotAnalysis::FindHitByValue(UInt_t nUserSeqId) const {
	if(MatchedHits.empty())
		return MatchedHits.end();
	std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator TempIt=MatchedHits.begin();
	while (TempIt!=MatchedHits.end() && TempIt->second.nChannelA!=nUserSeqId){
		++TempIt;
	} 
	return (TempIt);
}

Int_t TLaserspotAnalysis::HitMatching(){
	// match leading and trailing edge timestamps
	// use TdcHits multimap as starting point
	Int_t nMultipleHits = 0;
	MatchedHits.clear(); // clear map containing matched hits
	if(EvtTdcHits.empty()) // no TDC hits available
		return (nMultipleHits);
	std::multimap< UInt_t,UInt_t >::const_iterator CurrentTdcHit=EvtTdcHits.begin();
	while(CurrentTdcHit!=EvtTdcHits.end()){ // begin of loop over all TDC hits (excluding reference channels & user exclude list)
		TrbPixelHit TempPixelHit;
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
		TrailingEdges	= EvtTdcHits.equal_range(CurrentTdcHit->first+1); // find range of entries for trailing edges, trailing edge channel is leading edge + 1
		UInt_t nMultLeadEdge = (UInt_t) std::distance(LeadingEdges.first,LeadingEdges.second); // count hits for this channel
		UInt_t nMultTrailEdge = (UInt_t) std::distance(TrailingEdges.first,TrailingEdges.second);
		//cout << "Multiplicities: " << nMultLeadEdge << "\t" << nMultTrailEdge << endl;
		if(nMultLeadEdge!=nMultTrailEdge){ //mismatch of leading & trailing edge multiplicities
			CurrentTdcHit = TrailingEdges.second; // skip these entries
			//if(bVerboseMode){ // some error message

			//}
			continue; // skip rest of loop
		}
		std::pair< Int_t,Int_t > TempHitIndices;
		std::multimap< UInt_t,UInt_t >::const_iterator CurLeadEdge	= LeadingEdges.first;
		std::multimap< UInt_t,UInt_t >::const_iterator CurTrailEdge = TrailingEdges.first;
		switch(nMultLeadEdge){
			case 1: // single hit 

				
				TempHitIndices = make_pair(CurLeadEdge->second,CurTrailEdge->second);
				TempPixelHit.nChannelA = CurLeadEdge->first;
				TempPixelHit.nChannelB = CurTrailEdge->first;
				TempPixelHit.fTimeOverThreshold = TrbData->Hits_fTime[CurTrailEdge->second] - TrbData->Hits_fTime[CurLeadEdge->second];
				TempPixelHit.nSyncIndex = GetTdcSyncIndex(TrbData->Hits_nTrbAddress[CurLeadEdge->second]);
				TempPixelHit.bHasSyncTime = (TempPixelHit.nSyncIndex<0)? kFALSE : kTRUE;
				MatchedHits.insert(make_pair(TempHitIndices,TempPixelHit)); // enter this combination into pixel hit map
				//}
				CurrentTdcHit = TrailingEdges.second;
				break;
			default: // multiple hits
				++nMultipleHits;
				if(bSkipMultiHits){ // user decision to skip multiple hits
					CurrentTdcHit = EvtTdcHits.upper_bound(CurrentTdcHit->first); // increment iterator to skip multiple hits
					continue; // skip rest of loop
				}
				// need to get iterators to leading and trailing edges
				cout << "Matching multi hits now..." << endl;
				// loop over hits
				do{
					TempHitIndices = make_pair(CurLeadEdge->second,CurTrailEdge->second);
					TempPixelHit.nChannelA = CurLeadEdge->first;
					TempPixelHit.nChannelB = CurTrailEdge->first;
					TempPixelHit.fTimeOverThreshold = TrbData->Hits_fTime[CurTrailEdge->second] - TrbData->Hits_fTime[CurLeadEdge->second];
					TempPixelHit.nSyncIndex = GetTdcSyncIndex(TrbData->Hits_nTrbAddress[CurLeadEdge->second]);
					TempPixelHit.bHasSyncTime = (TempPixelHit.nSyncIndex<0)? kFALSE : kTRUE;
					MatchedHits.insert(make_pair(TempHitIndices,TempPixelHit)); // enter this combination into pixel hit map
					++CurLeadEdge;
					++CurTrailEdge;

				} while (CurLeadEdge!=LeadingEdges.second);
				CurrentTdcHit = TrailingEdges.second;
				break;
		}
	}
	//cout << MatchedHits.size() << endl;
	return (nMultipleHits);
}

void TLaserspotAnalysis::Init(){
	cout << "This is TLaserspotAnalysis::Init()..." << endl;
	MatchedHits.clear();
	bSkipMultiHits	= kTRUE;
	bApplyTimingCut = kFALSE;
}

void TLaserspotAnalysis::PrintLaserTriggerAddress() const {
	cout << "+++++++++++++++++++++++++++++" << endl;
	cout << "+++ Laser Trigger Address +++" << endl;
	cout << "+++++++++++++++++++++++++++++" << endl;
	cout << hex << LaserTriggerAddress.first << dec << "\t" << LaserTriggerAddress.second << "\t" << GetSeqId(LaserTriggerAddress.first,LaserTriggerAddress.second) << endl;
	cout << "+++++++++++++++++++++++++++++" << endl;
}

void TLaserspotAnalysis::PrintMatchedHits() const {
	if(MatchedHits.empty())
		return;
	std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator FirstEntry = MatchedHits.begin();
	std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator LastEntry = MatchedHits.end();
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "+++  Matched TDC Hits   +++" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << MatchedHits.size() << " MATCHED TDC HITS FOUND" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	for(std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator CurEntry=FirstEntry; CurEntry!=LastEntry; ++CurEntry){ // begin loop over all event hit entries
		cout << distance(FirstEntry,CurEntry) << "\t" << CurEntry->first.first << " , " << CurEntry->first.second << "\t" << CurEntry->second.nChannelA  << " , " << CurEntry->second.nChannelB << "\t" << CurEntry->second.fTimeOverThreshold << endl;
	} // end of loop over all event hit entries
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TLaserspotAnalysis::PrintMcpSpotAddress() const {
	cout << "++++++++++++++++++++++++++++" << endl;
	cout << "+++ MCP-PMT Spot Address +++" << endl;
	cout << "++++++++++++++++++++++++++++" << endl;
	cout << hex << McpSpotAddress.first << dec << "\t" << McpSpotAddress.second << "\t" << GetSeqId(McpSpotAddress.first,McpSpotAddress.second) << endl;
	cout << "++++++++++++++++++++++++++++" << endl;

}

void TLaserspotAnalysis::PrintTimingWindow() const {
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "+++    Timing Window    +++" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "t_0: " << TimingWindow.first << "\t t_1: " << TimingWindow.second << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TLaserspotAnalysis::Show(){
	if(!GetTreeStatus())
		return;
	TrbData->Show(0);
}