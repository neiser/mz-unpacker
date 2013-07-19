#include "TDircAnalysisBase.h"

ClassImp(TDircAnalysisBase);

TDircAnalysisBase::TDircAnalysisBase(string cUserDataFilename) : TTrbAnalysisBase(cUserDataFilename) {
	// do nothing so far...
	cout << "This is the constructor of TDircAnalysisBase class..." << endl;
	Init();
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

//void TDircAnalysisBase::Analyse(string cUserAnalysisFilename){
	//ofstream EvtListFileOut("EvtList.txt");
	//TFile *AnalysisOut = new TFile(cUserAnalysisFilename.c_str(),"RECREATE"); // open RooT file for analysis results
	//// define histograms
	//TH1D hEvtStats("hEvtStats","hEvtStats; ; frequency",15,-0.5,14.5);
	//enum DQCuts {NO_CUTS,DECODE_ERR,RNDM_BIT_ERR,SYNC_ERR,NO_HITS_ERR,NO_MATCH_ERR,NO_LASER_ERR}; //empty TDC cut,missing reference signal cut,laser trig missing,};
	//hEvtStats.GetXaxis()->SetBinLabel(NO_CUTS+1,"no cuts");
	//hEvtStats.GetXaxis()->SetBinLabel(DECODE_ERR+1,"decoding error");
	//hEvtStats.GetXaxis()->SetBinLabel(RNDM_BIT_ERR+1,"random bit error");
	//hEvtStats.GetXaxis()->SetBinLabel(SYNC_ERR+1,"TDC sync error");
	//hEvtStats.GetXaxis()->SetBinLabel(NO_HITS_ERR+1,"no TDC hits error");
	//hEvtStats.GetXaxis()->SetBinLabel(NO_MATCH_ERR+1,"no matched hits error");
	//hEvtStats.GetXaxis()->SetBinLabel(NO_LASER_ERR+1,"no laser trigger error");
	//TH1D hLaserTrigWidth("hLaserTrigWidth","hLaserTrigWidth; signal width (ns); frequency",1000,0.0,50.0); // width of laser trigger signal
	//TH1D hLaserLeadingEdge("hLaserLeadingEdge","hLaserLeadingEdge",1000,-180.0,-150.0);
	//TH1D hMcpSpotSigWidth("hMcpSpotSigWidth","hMcpSpotSigWidth; MCP-PMT signal width (ns); frequency",1000,0.0,25.0); // signal width of MCP-PMT spot signal
	//TH1D hMcpSpotTiming("hMcpSpotTiming","hMcpSpotTiming",1000,20.0,40.0);
	//TH2D hMcpSigWalk("hMcpSigWalk","hMcpSigWalk",1000,0.0,25.0,1000,20.0,40.0);
	//TH2D hMcpTimeShift("hMcpTimeShift","hMcpTimeShift",2000,-10.0,10.0,1000,20.0,40.0);
	//TH1D hTdcSync("hTdcSync","hTdcSync",(Int_t)GetNTdcs()+2,-0.5,GetNTdcs()+1.5);
	//TH1D hEvtMultiplicity("hEvtMultiplicity","hEvtMultiplicity;",16,-0.5,15.5);
	//TH1D hTdcHits("hTdcHits","hTdcHits",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5);
	//TH2D hTdcHitMult("hTdcHitMult","hTdcHitMult",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5,26,-0.5,25.5);
	//TH1D hPixelHits("hPixelHits","hPixelHits",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5);
	//TH2D hPixelTot("hPixelTot","hPixelTot",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5,610,-1.0,50.0);
	//TH2D hHitTimeVsChannel("hHitTimeVsChannel","hHitTimeVsChannel",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5,2000,-2500,2500);
	////AnalysisOut->mkdir("Synchronisation");
	//TH2D hTdcSyncTimestamps("hTdcSyncTimestamps","hTdcSyncTimestamps",1000,99786.0,99802.0,1000,99786.0,99802.0);
	//TH1D hSyncJitter("hSyncJitter","hSyncJitter",4000,-10.0,10.0);
	//TH2D hSyncJitterFT("hSyncJitterFT","hSyncJitterFT",2000,-10.0,10.0,600,0.0,600.0);
	//TH2D hSyncJitterEvtNo("hSyncJitterEvtNo","hSyncJitterEvtNo",2000,-10.0,10.0,5000,0.0,(Double_t)GetNEvents());
	//TH1D hSyncFtDiff("hSyncFtDiff","hSyncFtDiff",1200,-600.0,600.0);
	//TH2D hSyncFineTime("hSyncFineTime","hSyncFineTime",600,0.0,600.0,600,0.0,600.0);
	////AnalysisOut->cd(cUserAnalysisFilename.c_str());
	//// analysis variables
	//// main analysis loop
	//for(Int_t i=0; i<GetNEvents(); i++){ // begin loop over all events
	//	// first, get event data
	//	if (GetEntry(i)<1){ // this entry is not valid
	//		continue; // skip rest of loop
	//	}
	//	hEvtStats.Fill((Double_t)NO_CUTS);
	//	// now check data quality
	//	if(!CheckDecodingStatus()) // check if there were any problems during conversion
	//		continue; // skip rest of loop
	//	hEvtStats.Fill((Double_t)DECODE_ERR);
	//	if(!CheckRandomBits()){ // random bits in event do not match
	//		continue; // skip rest of loop
	//	}
	//	hEvtStats.Fill((Double_t)RNDM_BIT_ERR);
	//	UInt_t nSyncTimestampsFound = GetNSyncTimestamps();
	//	hTdcSync.Fill((Double_t)nSyncTimestampsFound);
	//	if(nSyncTimestampsFound!=GetNTdcs()){ // check if all TDCs have a sync timestamp
	//		continue; // skip rest of loop
	//	}
	//	//hTdcSyncTimestamps.Fill(GetTdcSyncTimestamp(LaserTriggerAddress.first),GetTdcSyncTimestamp(McpSpotAddress.first));
	//	//Double_t fSyncTimeJitter = GetTdcSyncTimestamp(LaserTriggerAddress.first)-GetTdcSyncTimestamp(McpSpotAddress.first);
	//	//Int_t nSyncFtDiff = TrbData->Hits_nFineTime[GetTdcSyncIndex(LaserTriggerAddress.first)]-TrbData->Hits_nFineTime[GetTdcSyncIndex(McpSpotAddress.first)];
	//	hSyncFtDiff.Fill((Double_t)nSyncFtDiff);
	//	hSyncJitter.Fill(fSyncTimeJitter);
	//	if(fSyncTimeJitter<0.0){
	//		EvtListFileOut << i << "\t" << TrbData->nEvtSeqNr << endl;
	//	}
	//	//hSyncJitterFT.Fill(fSyncTimeJitter,TrbData->Hits_nFineTime[GetTdcSyncIndex(LaserTriggerAddress.first)]);
	//	//hSyncFineTime.Fill((Double_t)TrbData->Hits_nFineTime[GetTdcSyncIndex(LaserTriggerAddress.first)],(Double_t)TrbData->Hits_nFineTime[GetTdcSyncIndex(McpSpotAddress.first)]);
	//	hSyncJitterEvtNo.Fill(fSyncTimeJitter,(Double_t)TrbData->nEvtSeqNr);
	//	hEvtStats.Fill((Double_t)SYNC_ERR);
	//	if(EvtTdcHits.empty()){ // check if there are no TDC hits in this event
	//		continue; // skip rest of loop
	//	}
	//	hEvtStats.Fill((Double_t)NO_HITS_ERR);
	//	// fill channel hit distribution plots
	//	std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator FirstChan = MappingTable.begin();
	//	std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator LastChan	= MappingTable.end();
	//	for(std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator CurChan=FirstChan; CurChan!=LastChan; ++CurChan){ // begin of loop over all TDC channels
	//		UInt_t nTempMult = EvtTdcHits.count(CurChan->second); // count number of TDC hits in current channel
	//		hTdcHits.Fill((Double_t)CurChan->second,(Double_t)nTempMult);
	//		hTdcHitMult.Fill((Double_t)CurChan->second,(Double_t)nTempMult);
	//	} // end of loop over all TDC channels
	//	hEvtMultiplicity.Fill((Double_t)MatchedHits.size());
	//	if(MatchedHits.empty()){
	//		continue; // skip rest of loop
	//	}
	//	hEvtStats.Fill((Double_t)NO_MATCH_ERR);
	//	// now check if we have the laser trigger signal
	//	std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator LaserTrigHit = FindHitByValue(nLaserTrigSeqId);
	//	if(LaserTrigHit==MatchedHits.end()){ // no laser trigger signal found
	//		continue; // skip rest of loop
	//	}
	//	hEvtStats.Fill((Double_t)NO_LASER_ERR);
	//	Double_t fLaserTrigTime = GetSynchronisedTime(*LaserTrigHit);
	//	//Double_t fLaserTrigTime = TrbData->Hits_fTime[LaserTrigHit->first.first];
	//	hLaserLeadingEdge.Fill(fLaserTrigTime);
	//	hLaserTrigWidth.Fill(fabs(LaserTrigHit->second.fTimeOverThreshold));
	//	// now check if we have a signal in the expected MCP-PMT channel
	//	std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator McpSpotHit = FindHitByValue(nMcpSpotSeqId);
	//	if(McpSpotHit!=MatchedHits.end()){ // MCP-PMT spot signal found
	//		//Double_t fMcpSpotTime = GetSynchronisedTime(*McpSpotHit);
	//		Double_t fMcpSpotTime = TrbData->Hits_fTime[McpSpotHit->first.first];
	//		hMcpSpotSigWidth.Fill(McpSpotHit->second.fTimeOverThreshold);
	//		hMcpSpotTiming.Fill(fMcpSpotTime-fLaserTrigTime);
	//		hMcpSigWalk.Fill(McpSpotHit->second.fTimeOverThreshold,fMcpSpotTime-fLaserTrigTime);
	//		hMcpTimeShift.Fill(fSyncTimeJitter,fMcpSpotTime-fLaserTrigTime);
	//	}
	//	std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator FirstHit	= MatchedHits.begin();
	//	std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator LastHit	= MatchedHits.end();
	//	for(std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator CurHit=FirstHit; CurHit!=LastHit; ++CurHit){ // begin loop over all matched hits
	//		hPixelHits.Fill((Double_t)CurHit->second.nChannelA);
	//		if(CurHit->second.bHasSyncTime){
	//			Double_t fLeadingEdge = GetSynchronisedTime(*CurHit);//TrbData->Hits_fTime[CurHit->first.first] - TrbData->Hits_fTime[CurHit->second.nSyncIndex];
	//			hHitTimeVsChannel.Fill((Double_t)CurHit->second.nChannelA,fLeadingEdge);
	//			if(bApplyTimingCut){
	//				if(fLeadingEdge>TimingWindow.first && fLeadingEdge<TimingWindow.second){ // apply timing cut
	//					hPixelTot.Fill((Double_t)CurHit->second.nChannelA,CurHit->second.fTimeOverThreshold);
	//				}
	//			}
	//			else
	//				hPixelTot.Fill((Double_t)CurHit->second.nChannelA,CurHit->second.fTimeOverThreshold);
	//		}
	//	}// end loop over all matched hits
	//	
	//} // end of loop over all events
	//AnalysisOut->Write(); // write all histograms in memeory to this file
	//delete AnalysisOut; // close RooT file and delete pointer
	//EvtListFileOut.close();
//}

std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator TDircAnalysisBase::FindHitByValue(UInt_t nUserSeqId) const {
	if(MatchedHits.empty())
		return MatchedHits.end();
	std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator TempIt=MatchedHits.begin();
	while (TempIt!=MatchedHits.end() && TempIt->second.nChannelA!=nUserSeqId){
		++TempIt;
	} 
	return (TempIt);
}

Double_t TDircAnalysisBase::GetSynchronisedTime(const std::pair< std::pair< Int_t,Int_t >,TrbPixelHit>& UserHit ) const {
	Double_t fTempSynchronisedTime = -1.0;
	fTempSynchronisedTime = TrbData->Hits_fTime[UserHit.first.first] - TrbData->Hits_fTime[UserHit.second.nSyncIndex];
	return (fTempSynchronisedTime);
}

Int_t TDircAnalysisBase::HitMatching(){
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
				TempHitIndices = make_pair(CurLeadEdge->second,CurTrailEdge->second); // here we have the array indices of the leading and trailing edge
				TempPixelHit.nChannelA = CurLeadEdge->first;
				TempPixelHit.nChannelB = CurTrailEdge->first;
				TempPixelHit.fTimeOverThreshold = TrbData->Hits_fTime[CurTrailEdge->second] - TrbData->Hits_fTime[CurLeadEdge->second];
				TempPixelHit.nSyncIndex = GetTdcSyncIndex(TrbData->Hits_nTrbAddress[CurLeadEdge->second]);
				TempPixelHit.bHasSyncTime = (TempPixelHit.nSyncIndex<0)? kFALSE : kTRUE;
				if(TempPixelHit.bHasSyncTime){
					TempPixelHit.fSyncLETime = TrbData->Hits_fTime[CurLeadEdge->second] - TrbData->Hits_fTime[TempPixelHit.nSyncIndex];
					if(bApplyTimingCut){ // check timing of leading edge
						if(TempPixelHit.fSyncLETime<TimingWindow.first || TempPixelHit.fSyncLETime>TimingWindow.second){
							CurrentTdcHit = TrailingEdges.second;
							break; // continue w/o entering this hit into the multimap
						}
					}
				
				}
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
				//cout << "Matching multi hits now..." << endl;
				// loop over hits
				do{
					TempHitIndices = make_pair(CurLeadEdge->second,CurTrailEdge->second);
					TempPixelHit.nChannelA = CurLeadEdge->first;
					TempPixelHit.nChannelB = CurTrailEdge->first;
					TempPixelHit.fTimeOverThreshold = TrbData->Hits_fTime[CurTrailEdge->second] - TrbData->Hits_fTime[CurLeadEdge->second];
					TempPixelHit.nSyncIndex = GetTdcSyncIndex(TrbData->Hits_nTrbAddress[CurLeadEdge->second]);
					TempPixelHit.bHasSyncTime = (TempPixelHit.nSyncIndex<0)? kFALSE : kTRUE;
					if(TempPixelHit.bHasSyncTime){
						TempPixelHit.fSyncLETime = TrbData->Hits_fTime[CurLeadEdge->second] - TrbData->Hits_fTime[TempPixelHit.nSyncIndex];
						if(bApplyTimingCut){ // check timing of leading edge
							if(TempPixelHit.fSyncLETime<TimingWindow.first || TempPixelHit.fSyncLETime>TimingWindow.second){
								//CurrentTdcHit = TrailingEdges.second;
								//break; // continue w/o entering this hit into the multimap
								++CurLeadEdge;
								++CurTrailEdge;
								continue; // continue w/o entering this hit into the multimap
							}
						}
				
					}
					//if(bApplyTimingCut){ // check timing of leading edge
					//	if(GetSynchronisedTime(make_pair(TempHitIndices,TempPixelHit))<TimingWindow.first || GetSynchronisedTime(make_pair(TempHitIndices,TempPixelHit))>TimingWindow.second){
					//		++CurLeadEdge;
					//		++CurTrailEdge;
					//		continue; // continue w/o entering this hit into the multimap
					//	}
					//}
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

void TDircAnalysisBase::Init(){
	cout << "This is TDircAnalysisBase::Init()..." << endl;
	MatchedHits.clear();
	bSkipMultiHits	= kTRUE;
	bApplyTimingCut = kFALSE;
}


void TDircAnalysisBase::PrintMatchedHits() const {
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
		cout << distance(FirstEntry,CurEntry) << "\t" << CurEntry->first.first << " , " << CurEntry->first.second << "\t" << CurEntry->second.nChannelA  << " , " << CurEntry->second.nChannelB << "\t" << CurEntry->second.fSyncLETime  << "\t" <<CurEntry->second.fTimeOverThreshold << endl;
	} // end of loop over all event hit entries
	cout << "+++++++++++++++++++++++++++" << endl;
}


void TDircAnalysisBase::PrintTimingWindow() const {
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "+++    Timing Window    +++" << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
	cout << "t_0: " << TimingWindow.first << "\t t_1: " << TimingWindow.second << endl;
	cout << "+++++++++++++++++++++++++++" << endl;
}

void TDircAnalysisBase::Show(){
	if(!GetTreeStatus())
		return;
	TrbData->Show(0);
}