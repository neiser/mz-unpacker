#include "TDircAnalysisPostdoc.h"

ClassImp(TDircAnalysisPostdoc);

TDircAnalysisPostdoc::TDircAnalysisPostdoc(TChain &UserChain) : TDircAnalysisBase(UserChain){
	cout << "Initialising TDircAnalysisPostdoc..." << endl;
	Init();
}

TDircAnalysisPostdoc::TDircAnalysisPostdoc(string cUserDataFilename, string cUserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth) : TDircAnalysisBase(cUserDataFilename,cUserTdcAddressFile,nUserTdcOffset,nUserTdcWidth) {
	cout << "Initialising TDircAnalysisPostdoc..." << endl;
	Init();
}

TDircAnalysisPostdoc::TDircAnalysisPostdoc(string cUserDataFilename, string cUserTdcAddressFile) : TDircAnalysisBase(cUserDataFilename, cUserTdcAddressFile) {
	cout << "Initialising TDircAnalysisPostdoc..." << endl;
	Init();
}

TDircAnalysisPostdoc::~TDircAnalysisPostdoc(){

}

void TDircAnalysisPostdoc::Analyse(string cUserAnalysisFilename){
	TFile *AnalysisOut = new TFile(cUserAnalysisFilename.c_str(),"RECREATE"); // open RooT file for analysis results
	string cLogFilename = cUserAnalysisFilename + ".log";
	LogFileBuffer.open(cLogFilename.c_str());
	WriteLogfileHeader();
	// define histograms
	TH1D hEvtStats("hEvtStats","hEvtStats; ; frequency",15,-0.5,14.5);
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
	TH2D hChanMultiplicity("hChanMultiplicity","hChanMultiplicity; channel ID; event multiplicity; frequency",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5,21,-0.5,20.5);
	TH1D hLETiming("hLETiming","Synchronised Leading Edge Timing; sync LE time (ns); frequency",5000,-1000.0,0.0); // synchronised leading edge time of all channels
	TH2D hHitToT("hHitToT","hHitToT;pixel ID; Time-over-Threshold (ns); frequency",GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5,1000,-1.0,50.0); // hit Time-over-Threshold for all channels
	TH1D hTriggerMultiplicity("hTriggerMultiplicity","Trigger Channel Hit Multiplicity; hit multiplicity per event; frequency",16,-0.5,15.5);
	TH1D hTriggerTime("hTriggerTime","hTriggerTime;trigger LE (ns); frequency",2500,-500.0,0.0);
	// main analysis loop
	Bool_t bTrigChanIsSet = GetTriggerStatus();
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
		DoDataQualityCheck();
		FillDQHistogram(hEvtStats);
		if(DataQualityStatus.count()>0) // check if any bits in the data quality register are set
			continue; // skip rest of analysis
		Double_t fTrigTime;
		GetTriggerTime(fTrigTime);
		hTriggerTime.Fill(fTrigTime);
		hEvtMultiplicity.Fill((Double_t)GetNMatchedHits());
		hMultiHitPixels.Fill((Double_t)GetNMultiHits());
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator FirstChannel	= EvtReconHits.begin();
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator LastChannel	= EvtReconHits.end();
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator CurChannel;
		for(CurChannel=FirstChannel; CurChannel!=LastChannel; ++CurChannel){ // begin of loop over all matched hits
			hChanMultiplicity.Fill((Double_t)CurChannel->first,(Double_t)GetChanMultiplicity(CurChannel->first));
			if(bTrigChanIsSet){
				if(CurChannel->first==(UInt_t)GetTriggerChannel()){ // this is the trigger channel
					hTriggerMultiplicity.Fill((Double_t)GetChanMultiplicity(CurChannel->first));
					continue; // skip rest of loop
				}
			}
			std::list<PixelHitModel>::const_iterator FirstHit	= CurChannel->second.begin();
			std::list<PixelHitModel>::const_iterator LastHit	= CurChannel->second.end();
			std::list<PixelHitModel>::const_iterator CurHit;
			for(CurHit=FirstHit; CurHit!=LastHit; ++CurHit){ // begin of loop over all hits in this channel
				hPixelHits.Fill((Double_t)CurHit->GetLeadEdgeChan());
				hPixelHits.Fill((Double_t)CurHit->GetTrailEdgeChan());
				hLETiming.Fill(CurHit->GetLeadEdgeTime());
				hHitToT.Fill((Double_t)CurChannel->first,CurHit->GetToT());
			} // end of loop over all hits in this channel

		} // end of loop over all matched hits
	} // end of loop over all events
	cout << endl;
	AnalysisOut->Write(); // write all histograms in memeory to this file
	delete AnalysisOut; // close RooT file and delete pointer
	LogFileBuffer.close();
}

void TDircAnalysisPostdoc::DoDataQualityCheck(){ // check event data quality
	Double_t fTrigTime;
	DataQualityStatus.reset();
	if(!CheckDecodingStatus()){ // check if there were any problems during conversion
		DataQualityStatus.set(0); // set corresponding error bit
		return; // this error is so severe that no further checks are carried out
	}
	if(!CheckRandomBits()){ // random bits in event do not match
		DataQualityStatus.set(1); // set corresponding error bit
		return; // this error is so severe that no further checks are carried out
	}
	if(GetNSyncTimestamps()!=GetNTdcs()){ // check if all TDCs have a sync timestamp
		DataQualityStatus.set(2); // set corresponding error bit
	}
	if(EvtTdcHits.empty()){ // check if there are no TDC hits in this event
		DataQualityStatus.set(3); // set corresponding error bit
		return; // this error is so severe that no further checks are carried out
	}
	if(GetNMatchedHits()<1){ // check if there are no matched hits in this event
		DataQualityStatus.set(4); // set corresponding error bit
		return; // this error is so severe that no further checks are carried out
	}
	if(!GetTriggerTime(fTrigTime)){ // no event trigger found
		DataQualityStatus.set(5); // set corresponding error bit
	}
}

void TDircAnalysisPostdoc::FillDQHistogram(TH1D& hDataQuality){
	hDataQuality.Fill((Double_t)NO_CUTS); // this is the reference number of events
	if(DataQualityStatus.test(0))
		return;
	hDataQuality.Fill((Double_t)DECODE_ERR);
	if(DataQualityStatus.test(1))
		return;
	hDataQuality.Fill((Double_t)RNDM_BIT_ERR);
	if(DataQualityStatus.test(2))
		return;
	hDataQuality.Fill((Double_t)SYNC_ERR);
	if(DataQualityStatus.test(3))
		return;
	hDataQuality.Fill((Double_t)NO_HITS_ERR);
	if(DataQualityStatus.test(4))
		return;
	hDataQuality.Fill((Double_t)NO_MATCH_ERR);
	if(DataQualityStatus.test(5))
		return;
	hDataQuality.Fill((Double_t)NO_TRIG_ERR);
}

void TDircAnalysisPostdoc::Init(){
	DataQualityStatus.reset();
}

void TDircAnalysisPostdoc::WriteLogfileHeader(){
	if(!LogFileBuffer.is_open()) // no file is associated with LogFileBuffer
		return;
	std::streambuf *psbuf, *backup;
	LogFileBuffer << "+++++++++++++++++++++++++++++++++++++" << endl;
	LogFileBuffer << "+     Postdoc Analysis Logfile      +" << endl;
	LogFileBuffer << "+++++++++++++++++++++++++++++++++++++" << endl;
	LogFileBuffer << endl;
	// redirect cout buffer to logfile
	backup = std::cout.rdbuf();
	psbuf = LogFileBuffer.rdbuf();
	std::cout.rdbuf(psbuf);         // assign streambuf to cout
	// print status information to logfile
	PrintTriggerAddress();
	LogFileBuffer << endl; // add empty line
	PrintSwapList();
	LogFileBuffer << endl; // add empty line
	PrintTimingWindow();
	LogFileBuffer << endl; // add empty line
	PrintExcludedChannels();
	// reset cout buffer to terminal
	std::cout.rdbuf(backup);        // restore cout's original streambuf
	psbuf = NULL;
	backup = NULL;

}