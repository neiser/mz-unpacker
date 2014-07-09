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
	string cLogFilename = cUserAnalysisFilename + ".log"; // create log file name
	LogFileBuffer.open(cLogFilename.c_str()); // open log file
	WriteLogfileHeader(); // write header information log file
	// define histograms
	TH1D hEvtStats("hEvtStats","hEvtStats; ; frequency",15,-0.5,14.5);
	hEvtStats.GetXaxis()->SetBinLabel(NO_CUTS+1,"no cuts");
	hEvtStats.GetXaxis()->SetBinLabel(DECODE_ERR+1,"decoding error");
	hEvtStats.GetXaxis()->SetBinLabel(RNDM_BIT_ERR+1,"random bit error");
	hEvtStats.GetXaxis()->SetBinLabel(SYNC_ERR+1,"TDC sync error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_HITS_ERR+1,"no TDC hits error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_TRIG_ERR+1,"no event trigger error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_MATCH_ERR+1,"no matched hits error");
	TH1D hEvtMultiplicity("hEvtMultiplicity","hEvtMultiplicity;",50,-0.5,49.5); // number of hit pixels per event
	TH1D hPixelHits("hPixelHits","hPixelHits; pixel ID; frequency",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5); // distribution of pixel hits
	TH1D hMultiHitPixels("hMultiHitPixels","hMultiHitPixels; no of channels with mult hits per event; frequency",20,-0.5,19.5);
	TH2D hChanMultiplicity("hChanMultiplicity","hChanMultiplicity; channel ID; event multiplicity; frequency",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5,21,-0.5,20.5);
	TH2D hChanCoin("hChanCoin","hChanCoin; channel ID; channel ID; frequency",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5,(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5);
	TH1D hLETiming("hLETiming","Synchronised Leading Edge Timing; sync LE time (ns); frequency",5000,-1000.0,0.0); // synchronised leading edge time of all channels
	TH2D hHitToT("hHitToT","hHitToT;pixel ID; Time-over-Threshold (ns); frequency",GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5,1000,-1.0,50.0); // hit Time-over-Threshold for all channels
	TH1D hTriggerMultiplicity("hTriggerMultiplicity","Trigger Channel Hit Multiplicity; hit multiplicity per event; frequency",16,-0.5,15.5);
	TH1D hTriggerTime("hTriggerTime","hTriggerTime;trigger LE (ns); frequency",2500,-500.0,0.0);
	TH1D hTriggerToT("hTriggerToT","Trigger Channel Time-over-Threshold; Time-over-Threshold (ns); frequency",1000,-1.0,50.0);
	TH1D hHitMatchingErrChan("hHitMatchingErrChan","Hit Matching Error Channels; channel ID; frequency",(Int_t)GetSizeOfMapTable()+2,-0.5,GetSizeOfMapTable()+1.5);
	// main analysis loop
	Bool_t bTrigChanIsSet = GetTriggerStatus();
	Int_t nEvents	= GetNEvents();
	Int_t nFraction = nEvents * 0.1;
	LogFileBuffer << "Number of raw triggers: " << nEvents << endl;
	Int_t nAccEvents = 0;
	Int_t nHitMatchingErr = 0; // number of events with errors during hit matching stage
	time_t StartTime;
	time(&StartTime);
	for(Int_t i=0; i<nEvents; i++){ // begin loop over all events
		// first, get event data
		if (GetEntry(i)<1){ // this entry is not valid
			continue; // skip rest of loop
		}
		// compute progress
		if(i%nFraction==0){
			cout << (i/nFraction)*(1.0/0.1) << "% of events analysed...\r" ;
		}
		if(bHitMatchingError){
			nHitMatchingErr++;
			for(std::list<UInt_t>::const_iterator ThisChannel=EvtHitMatchErrChan.begin(); ThisChannel!=EvtHitMatchErrChan.end(); ++ThisChannel){
				hHitMatchingErrChan.Fill((Double_t)*ThisChannel);
			}
		}
		DoDataQualityCheck();
		FillDQHistogram(hEvtStats);
		if(DataQualityStatus.any()) // check if any bits in the data quality register are set
			continue; // skip rest of analysis
		nAccEvents++;
		// trigger hit stuff
		Double_t fTrigTime;
		GetTriggerTime(fTrigTime);
		hTriggerTime.Fill(fTrigTime);
		hTriggerMultiplicity.Fill((Double_t)GetTriggerMultiplicity());
		hTriggerToT.Fill(EvtTriggerHits.begin()->GetToT());
		// regular pixel hit stuff
		hEvtMultiplicity.Fill((Double_t)GetNMatchedHits());
		hMultiHitPixels.Fill((Double_t)GetNMultiHits());
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator FirstChannel	= EvtReconHits.begin();
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator LastChannel	= EvtReconHits.end();
		std::map< UInt_t,std::list<PixelHitModel> >::const_iterator CurChannel;
		for(CurChannel=FirstChannel; CurChannel!=LastChannel; ++CurChannel){ // begin of loop over all matched hits
			//hChanMultiplicity.Fill((Double_t)CurChannel->first,(Double_t)GetChanMultiplicity(CurChannel->first));
			hChanMultiplicity.Fill((Double_t)CurChannel->first,(Double_t)CurChannel->second.size());
			std::map< UInt_t,std::list<PixelHitModel> >::const_iterator CoinChan = CurChannel;
			while(++CoinChan != LastChannel){
				hChanCoin.Fill((Double_t)CurChannel->first,(Double_t)CoinChan->first);
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
	time_t StopTime;
	time(&StopTime);
	Double_t fAnalysisTime = difftime(StopTime,StartTime);
	cout << "Analysis took " << fAnalysisTime << " seconds to complete" << endl;
	cout << endl;
	LogFileBuffer << "Number of events with error during hit matching: " << nHitMatchingErr << endl;
	LogFileBuffer << "Number of accepted triggers: " << nAccEvents << endl;
	LogFileBuffer << "Analysis duration " << fAnalysisTime << " seconds" << endl;
	AnalysisOut->Write(); // write all histograms in memeory to this file
	delete AnalysisOut; // close RooT file and delete pointer
	LogFileBuffer.close();
}

void TDircAnalysisPostdoc::AnalyseTrigger(string cUserAnalysisFilename){
	if(!GetTriggerStatus()){
		cout << "No trigger channel set!" << endl;
		return;
	}
	// setting up analysis
	TFile *AnalysisOut = new TFile(cUserAnalysisFilename.c_str(),"RECREATE"); // open RooT file for analysis results
	string cLogFilename = cUserAnalysisFilename + ".log"; // create log file name
	LogFileBuffer.open(cLogFilename.c_str()); // open log file
	WriteLogfileHeader(); // write header information log file
	// defining histograms
	TH1D hEvtStats("hEvtStats","hEvtStats; ; frequency",15,-0.5,14.5);
	hEvtStats.GetXaxis()->SetBinLabel(NO_CUTS+1,"no cuts");
	hEvtStats.GetXaxis()->SetBinLabel(DECODE_ERR+1,"decoding error");
	hEvtStats.GetXaxis()->SetBinLabel(RNDM_BIT_ERR+1,"random bit error");
	hEvtStats.GetXaxis()->SetBinLabel(SYNC_ERR+1,"TDC sync error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_HITS_ERR+1,"no TDC hits error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_TRIG_ERR+1,"no event trigger error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_MATCH_ERR+1,"no matched hits error");
	TH1D hTriggerMultiplicity("hTriggerMultiplicity","Trigger Channel Hit Multiplicity; hit multiplicity per event; frequency",16,-0.5,15.5);
	TH1D hTriggerTime("hTriggerTime","hTriggerTime;trigger LE (ns); frequency",2500,-500.0,200.0);
	TH1D hTriggerTimeWide("hTriggerTimeWide","Trigger Time; sync trigger LE (ns); frequency",5000,-10000.0,150.0);
	TH1D hTriggerToT("hTriggerToT","Trigger Channel Time-over-Threshold; Time-over-Threshold (ns); frequency",1000,-1.0,50.0);
	TH2D hTriggerTimeVsToT("hTriggerTimeVsToT","Trigger Time vs Time-over-Threshold; trigger time (ns); time-over-threshold (ns); frequency",2500,-500.0,200.0,1000,-1.0,50.0);
	// main analysis loop
	Int_t nEvents	= GetNEvents();
	Int_t nFraction = nEvents * 0.1;
	LogFileBuffer << "Number of raw triggers: " << nEvents << endl;
	Int_t nAccEvents = 0;
	time_t StartTime; 
	time(&StartTime); // get time before entering analysis loop
	for(Int_t i=0; i<nEvents; i++){ // begin loop over all events
	// first, get event data
		if (GetEntry(i)<1){ // this entry is not valid
			continue; // skip rest of loop
		}
		// compute progress
		if(i%nFraction==0){
			cout << (i/nFraction)*(1.0/0.1) << "% of events analysed...\r" ;
		}
		DoDataQualityCheck(); // check data quality status of event
		FillDQHistogram(hEvtStats);
		DataQualityStatus.reset(5); // ignore no matched hits error in this analysis
		if(DataQualityStatus.any()) // check if any bits in the data quality register are set
			continue; // skip rest of analysis
		nAccEvents++;
		// trigger hit stuff
		hTriggerMultiplicity.Fill((Double_t)GetTriggerMultiplicity());
		std::list<PixelHitModel>::const_iterator FirstTrigger = EvtTriggerHits.begin();
		std::list<PixelHitModel>::const_iterator LastTrigger = EvtTriggerHits.end();
		std::list<PixelHitModel>::const_iterator CurTrigger;
		for(CurTrigger=FirstTrigger; CurTrigger!=LastTrigger; ++CurTrigger){
			hTriggerTime.Fill(CurTrigger->GetLeadEdgeTime());
			hTriggerTimeWide.Fill(CurTrigger->GetLeadEdgeTime());
			hTriggerToT.Fill(CurTrigger->GetToT());
			hTriggerTimeVsToT.Fill(CurTrigger->GetLeadEdgeTime(),CurTrigger->GetToT());
		}
	} // end of loop over all events
	time_t StopTime;
	time(&StopTime);
	Double_t fAnalysisTime = difftime(StopTime,StartTime);
	cout << "Analysis took " << fAnalysisTime << " seconds to complete" << endl;
	cout << endl;
	LogFileBuffer << "Number of accepted triggers: " << nAccEvents << endl;
	LogFileBuffer << "Analysis duration " << fAnalysisTime << " seconds" << endl;
	AnalysisOut->Write(); // write all histograms in memeory to this file
	delete AnalysisOut; // close RooT file and delete pointer
	LogFileBuffer.close();
}

void TDircAnalysisPostdoc::DoDataQualityCheck(){ // check event data quality
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
	if(GetTriggerStatus()&&GetTriggerMultiplicity()<1){ // no event trigger found
		DataQualityStatus.set(4); // set corresponding error bit
		return; // this error is so severe that no further checks are carried out
	}
	if(GetNMatchedHits()<1){ // check if there are no matched hits in this event
		DataQualityStatus.set(5); // set corresponding error bit
		//return; // this error is so severe that no further checks are carried out
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
	hDataQuality.Fill((Double_t)NO_TRIG_ERR);
	if(DataQualityStatus.test(5))
		return;
	hDataQuality.Fill((Double_t)NO_MATCH_ERR);
}

void TDircAnalysisPostdoc::Init(){
	DataQualityStatus.reset();
}

void TDircAnalysisPostdoc::WriteLogfileHeader(){
	if(!LogFileBuffer.is_open()) // no file is associated with LogFileBuffer
		return;
	time(&RawTime);
	std::streambuf *psbuf, *backup;
	LogFileBuffer << "+++++++++++++++++++++++++++++++++++++" << endl;
	LogFileBuffer << "+     Postdoc Analysis Logfile      +" << endl;
	LogFileBuffer << "+++++++++++++++++++++++++++++++++++++" << endl;
	LogFileBuffer << "+    Analysis Setup Information     +" << endl;
	LogFileBuffer << "+++++++++++++++++++++++++++++++++++++" << endl;
	LogFileBuffer << "\t" << ctime(&RawTime) << endl;
	LogFileBuffer << endl;
	// redirect cout buffer to logfile
	backup = std::cout.rdbuf();
	psbuf = LogFileBuffer.rdbuf();
	std::cout.rdbuf(psbuf);         // assign streambuf to cout
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
	std::cout.rdbuf(backup);        // restore cout's original streambuf
	psbuf = NULL;
	backup = NULL;

}