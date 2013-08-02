#include "TDircAnalysisDummy.h"

ClassImp(TDircAnalysisDummy);

TDircAnalysisDummy::TDircAnalysisDummy(string cUserDataFilename, string cUserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth) : TDircAnalysisBase(cUserDataFilename, cUserTdcAddressFile,nUserTdcOffset,nUserTdcWidth) {
	cout << "Initialising TDircAnalysisDummy..." << endl;
	Init();
}

TDircAnalysisDummy::TDircAnalysisDummy(string cUserDataFilename, string cUserTdcAddressFile) : TDircAnalysisBase(cUserDataFilename, cUserTdcAddressFile) {
	cout << "Initialising TDircAnalysisDummy..." << endl;
	Init();
}

TDircAnalysisDummy::~TDircAnalysisDummy(){

}

void TDircAnalysisDummy::Analyse(string cUserAnalysisFilename){
	TFile *AnalysisOut = new TFile(cUserAnalysisFilename.c_str(),"RECREATE"); // open RooT file for analysis results
	// define histograms
	TH1D hEvtStats("hEvtStats","hEvtStats; ; frequency",15,-0.5,14.5);
	enum DQCuts {NO_CUTS,DECODE_ERR,RNDM_BIT_ERR,SYNC_ERR,NO_HITS_ERR,NO_MATCH_ERR,NO_TRIG_ERR}; //empty TDC cut,missing reference signal cut,laser trig missing,};
	hEvtStats.GetXaxis()->SetBinLabel(NO_CUTS+1,"no cuts");
	hEvtStats.GetXaxis()->SetBinLabel(DECODE_ERR+1,"decoding error");
	hEvtStats.GetXaxis()->SetBinLabel(RNDM_BIT_ERR+1,"random bit error");
	hEvtStats.GetXaxis()->SetBinLabel(SYNC_ERR+1,"TDC sync error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_HITS_ERR+1,"no TDC hits error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_MATCH_ERR+1,"no matched hits error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_TRIG_ERR+1,"no event trigger error");

	TH1D hEvtMultiplicity("hEvtMultiplicity","hEvtMultiplicity;",50,-0.5,49.5); // number of hit pixels per event
	TH1D hPixelHits("hPixelHits","hPixelHits; pixel ID; frequency",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5); // distribution of pixel hits
	TH1D hMultiHitPixels("hMultiHitPixels","hMultiHitPixels; no of channels with mult hits per event; frequency",20,-0.5,19.5);
	TH1D hLETiming("hLETiming","Synchronised Leading Edge Timing; sync LE time (ns); frequency",5000,-1000.0,0.0); // synchronised leading edge time of all channels
	TH2D hHitToT("hHitToT","hHitToT;pixel ID; Time-over-Threshold (ns); frequency",GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5,1000,-1.0,50.0); // hit Time-over-Threshold for all channels
	TH1D hTriggerTime("hTriggerTime","hTriggerTime;trigger LE (ns); frequency",1000,-500.0,0.0);
	// main analysis loop
	Int_t nEvents = GetNEvents();
	Int_t nFraction = nEvents * 0.1;
	for(Int_t i=0; i<nEvents; i++){ // begin loop over all events
		// first, get event data
		if (GetEntry(i)<1){ // this entry is not valid
			continue; // skip rest of loop
		}
		// compute progress
		if(i%nFraction==0){
			cout << (i/nFraction)*(1.0/0.1) << "% of events analysed...\r" ;
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
		if(nSyncTimestampsFound!=GetNTdcs()){ // check if all TDCs have a sync timestamp
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)SYNC_ERR);
		if(EvtTdcHits.empty()){ // check if there are no TDC hits in this event
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)NO_HITS_ERR);
		if(MatchedHits.empty()){
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)NO_MATCH_ERR);
		Double_t fTrigTime;
		if(!GetTriggerTime(fTrigTime)){ // no event trigger found
			continue;
		}
		hEvtStats.Fill((Double_t)NO_TRIG_ERR);
		//PrintMatchedHits();
		hEvtMultiplicity.Fill((Double_t)GetNMatchedHits());
		hMultiHitPixels.Fill((Double_t)GetNMultiHits());
		hTriggerTime.Fill(fTrigTime);
		std::map< std::pair< Int_t,Int_t >,PixelHitModel >::const_iterator FirstHit = MatchedHits.begin();
		std::map< std::pair< Int_t,Int_t >,PixelHitModel >::const_iterator LastHit = MatchedHits.end();
		for(std::map< std::pair< Int_t,Int_t >,PixelHitModel >::const_iterator CurHit=FirstHit; CurHit!=LastHit; ++CurHit){ // begin of loop over all matched hits
			hPixelHits.Fill((Double_t)CurHit->second.nChannelA);
			hPixelHits.Fill((Double_t)CurHit->second.nChannelB);
			hLETiming.Fill(CurHit->second.fSyncLETime);
			hHitToT.Fill((Double_t)CurHit->second.nChannelA,CurHit->second.fTimeOverThreshold);
		} // end of loop over all matched hits
	} // end of loop over all events
	cout << endl;
	AnalysisOut->Write(); // write all histograms in memeory to this file
	delete AnalysisOut; // close RooT file and delete pointer
}

void TDircAnalysisDummy::Init(){

}