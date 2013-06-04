#include "TTrbDummyAnalysis.h"

ClassImp(TTrbDummyAnalysis);

TTrbDummyAnalysis::TTrbDummyAnalysis(string cUserDataFilename) : TTrbAnalysisBase(cUserDataFilename){
	// do nothing so far...
	cout << "This is the constructor of TTrbDummyAnalysis class..." << endl;
	Init();
}

TTrbDummyAnalysis::TTrbDummyAnalysis(string cUserDataFilename, string cUserTdcAddressFile, UInt_t nUserTdcWidth, UInt_t nUserTdcOffset) : TTrbAnalysisBase(cUserDataFilename){
	Init();
	SetTdcSize(nUserTdcWidth); // set TDC channel number
	SetTdcOffset(nUserTdcOffset); // set TDC channel offset
	cout << SetTdcAddresses(cUserTdcAddressFile) << " TDC addresses registered" << endl;
}

TTrbDummyAnalysis::~TTrbDummyAnalysis(){
	// do nothing
	cout << "This is the destructor of TTrbDummyAnalysis class..." << endl;
}

void TTrbDummyAnalysis::Analyse(string cUserAnalysisFilename){
	TFile *AnalysisOut = new TFile(cUserAnalysisFilename.c_str(),"RECREATE"); // open RooT file for analysis results
	// define histograms
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
		if (GetEntry(i)<1){ // this entry is not valid
			continue; // skip rest of loop
		}
		if(!CheckRandomBits()){ // random bits in event do not match
			continue; // skip rest of loop
		}
		//ScanEvent(); // filling TDC sync timestamps and hit maps needed further in analysis
		hTdcSync.Fill((Double_t)GetNSyncTimestamps());
		std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator FirstChan = MappingTable.begin();
		std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator LastChan = MappingTable.end();
		for(std::map< std::pair< UInt_t,UInt_t >,UInt_t >::const_iterator CurChan=FirstChan; CurChan!=LastChan; ++CurChan){
			UInt_t nTempMult = EvtTdcHits.count(CurChan->second);
			hTdcHits.Fill((Double_t)CurChan->second,(Double_t)nTempMult);
			hTdcHitMult.Fill((Double_t)CurChan->second,(Double_t)nTempMult);
		}
		hEvtMultiplicity.Fill((Double_t)MatchedHits.size());
		if(MatchedHits.empty())
			continue;
		std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator FirstHit = MatchedHits.begin();
		std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator LastHit = MatchedHits.end();
		for(std::map< std::pair< Int_t,Int_t >,TrbPixelHit >::const_iterator CurHit=FirstHit; CurHit!=LastHit; ++CurHit){ // begin loop over all matched hits
			hPixelHits.Fill((Double_t)CurHit->second.nChannelA);
			hPixelTot.Fill((Double_t)CurHit->second.nChannelA,CurHit->second.fTimeOverThreshold);
			if(CurHit->second.bHasSyncTime){
				Double_t fLeadingEdge = TrbData->Hits_fTime[CurHit->first.first] - TrbData->Hits_fTime[CurHit->second.nSyncIndex];
				hHitTimeVsChannel.Fill((Double_t)CurHit->second.nChannelA,fLeadingEdge);
			}
		}// end loop over all matched hits
		
	} // end of loop over all events
	AnalysisOut->Write(); // write all histograms in memeory to this file
	delete AnalysisOut; // close RooT file and delete pointer
}

Int_t TTrbDummyAnalysis::HitMatching(){
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
		cout << "Multiplicities: " << nMultLeadEdge << "\t" << nMultTrailEdge << endl;
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
				//LeadingEdge = CurrentTdcHit; // store pointer to leading-edge entry
				//++CurrentTdcHit; // increment iterator to point to the next element
				//if(CurrentTdcHit==EvtTdcHits.end()) // check if we reached end of hit map
				//	continue; // if end of hit map is reached, exit this loop
				//if((CurrentTdcHit->first-LeadingEdge->first)==1){ // found hit sequence
				
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
				//for(LeadingEdge	= EvtTdcHits.lower_bound(CurrentTdcHit->first),TrailingEdge = EvtTdcHits.lower_bound(CurrentTdcHit->first+1); LeadingEdge!=EvtTdcHits.upper_bound(CurrentTdcHit->first); ++LeadingEdge,++TrailingEdge){
					//std::pair< Int_t,Int_t > TempHitIndices = make_pair(LeadingEdges.first->second,TrailingEdges.first->second);
				//	TempPixelHit.nChannelA = LeadingEdge->first;
				//	TempPixelHit.nChannelB = TrailingEdge->first;
				//	TempPixelHit.fTimeOverThreshold = TrbData->Hits_fTime[TrailingEdge->second] - TrbData->Hits_fTime[LeadingEdge->second];
				//	TempPixelHit.nSyncIndex = GetTdcSyncIndex(TrbData->Hits_nTrbAddress[LeadingEdge->second]);
				//	TempPixelHit.bHasSyncTime = (TempPixelHit.nSyncIndex<0)? kFALSE : kTRUE;
				//	MatchedHits.insert(make_pair(TempHitIndices,TempPixelHit)); // enter this combination into pixel hit map
				//}
				//CurrentTdcHit = EvtTdcHits.upper_bound(CurrentTdcHit->first+1);
				break;
		}
	}
	//cout << MatchedHits.size() << endl;
	return (nMultipleHits);
}

void TTrbDummyAnalysis::Init(){
	cout << "This is TTrbDummyAnalysis::Init()..." << endl;
	MatchedHits.clear();
	bSkipMultiHits = kTRUE;
}

void TTrbDummyAnalysis::PrintMatchedHits() const {
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

void TTrbDummyAnalysis::Show(){
	if(!GetTreeStatus())
		return;
	TrbData->Show(0);
}