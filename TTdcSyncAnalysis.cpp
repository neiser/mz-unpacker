#include "TTdcSyncAnalysis.h"

ClassImp(TTdcSyncAnalysis);

//TTdcSyncAnalysis::TTdcSyncAnalysis(string cUserDataFilename, string cUserTdcAddressFile) : TTrbAnalysisBase(cUserDataFilename) {
//	// do nothing so far...
//	cout << "This is the constructor of TTdcSyncAnalysis class..." << endl;
//	Init();
//}

TTdcSyncAnalysis::TTdcSyncAnalysis(string cUserDataFilename, string cUserTdcAddressFile, UInt_t nUserTdcOffset, UInt_t nUserTdcWidth) : TTrbAnalysisBase(cUserDataFilename){
	Init();
	SetTdcSize(nUserTdcWidth); // set TDC channel number
	SetTdcOffset(nUserTdcOffset); // set TDC channel offset
	SetTdcAddresses(cUserTdcAddressFile);
	GenerateTdcPairs();
	cout << nNPairs << endl;

}

TTdcSyncAnalysis::~TTdcSyncAnalysis(){
	cout << "This is the destructor of TTdcSyncAnalysis class..." << endl;
	//ClearAllPlots();
	TdcPairs.clear();
}

void TTdcSyncAnalysis::Analyse(string cUserAnalysisFilename){
//	if(TdcAddresses.find(nUserTdcAddressA)==TdcAddresses.end() || TdcAddresses.find(nUserTdcAddressB)==TdcAddresses.end())
//		return;
//	ofstream EvtListFileOut("EvtList.txt");
	InitHistograms();
	TFile *AnalysisOut = new TFile(cUserAnalysisFilename.c_str(),"RECREATE"); // open RooT file for analysis results
//	// define histograms
	TH1D hEvtStats("hEvtStats","hEvtStats; ; frequency",15,-0.5,14.5);
	enum DQCuts {NO_CUTS,DECODE_ERR,RNDM_BIT_ERR,SYNC_ERR,NO_HITS_ERR,NO_MATCH_ERR,NO_LASER_ERR}; //empty TDC cut,missing reference signal cut,laser trig missing,};
	hEvtStats.GetXaxis()->SetBinLabel(NO_CUTS+1,"no cuts");
	hEvtStats.GetXaxis()->SetBinLabel(DECODE_ERR+1,"decoding error");
	hEvtStats.GetXaxis()->SetBinLabel(RNDM_BIT_ERR+1,"random bit error");
	hEvtStats.GetXaxis()->SetBinLabel(SYNC_ERR+1,"TDC sync error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_HITS_ERR+1,"no TDC hits error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_MATCH_ERR+1,"no matched hits error");
	hEvtStats.GetXaxis()->SetBinLabel(NO_LASER_ERR+1,"no laser trigger error");
	TH1D hTdcSync("hTdcSync","hTdcSync; no of sync timestamps/event; frequency",(Int_t)GetNTdcs()+2,-0.5,GetNTdcs()+1.5);
	// need to define histograms for TDC pairs here!!!
//	TH2D hTdcSyncTimestamps("hTdcSyncTimestamps","hTdcSyncTimestamps",1000,99786.0,99802.0,1000,99786.0,99802.0);
//	TH1D hSyncJitter("hSyncJitter","hSyncJitter",4000,-10.0,10.0);
//	TH2D hSyncJitterFT("hSyncJitterFT","hSyncJitterFT",2000,-10.0,10.0,600,0.0,600.0);
//	TH2D hSyncJitterEvtNo("hSyncJitterEvtNo","hSyncJitterEvtNo",2000,-10.0,10.0,5000,0.0,(Double_t)GetNEvents());
//	//AnalysisOut->cd(cUserAnalysisFilename.c_str());
//	// analysis variables
//	// main analysis loop
	for(Int_t i=0; i<GetNEvents(); i++){ // begin loop over all events
//		// first, get event data
		if (GetEntry(i)<1){ // this entry is not valid
			continue; // skip rest of loop
		}
		hEvtStats.Fill((Double_t)NO_CUTS);
//		// now check data quality
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
		// now loop over all sync timestamps
		std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::iterator FirstPair = TdcPairs.begin();
		std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::iterator LastPair	= TdcPairs.end();
		for(std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::iterator CurPair=FirstPair; CurPair!=LastPair; ++CurPair){
			Double_t fTimeDelta = GetTdcSyncTimestamp(CurPair->first.first)-GetTdcSyncTimestamp(CurPair->first.second);
			CurPair->second.hTimeDiff->Fill(fTimeDelta);
			Int_t nCoarseTimeDelta = GetTdcSyncCoarseTimestamp(CurPair->first.first) - GetTdcSyncCoarseTimestamp(CurPair->first.second);
			CurPair->second.hCoarseTimeDiff->Fill(nCoarseTimeDelta);
			Int_t nFineTimeDelta = GetTdcSyncFineTimestamp(CurPair->first.first) - GetTdcSyncFineTimestamp(CurPair->first.second);
			CurPair->second.hFineTimeDiff->Fill(nFineTimeDelta);
			// now deal with correlations
			CurPair->second.hTimeCorrelation->Fill(GetTdcSyncTimestamp(CurPair->first.first),GetTdcSyncTimestamp(CurPair->first.second));
			CurPair->second.hCoarseTimeCorrelation->Fill((Double_t)GetTdcSyncCoarseTimestamp(CurPair->first.first),(Double_t)GetTdcSyncCoarseTimestamp(CurPair->first.second));
			CurPair->second.hFineTimeCorrelation->Fill((Double_t)GetTdcSyncFineTimestamp(CurPair->first.first),(Double_t)GetTdcSyncFineTimestamp(CurPair->first.second));
		}
	
	} // end of loop over all events
	std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::const_iterator FirstPair = TdcPairs.begin();
	std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::const_iterator LastPair	= TdcPairs.end();
	for(std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::const_iterator CurPair=FirstPair; CurPair!=LastPair; ++CurPair){
		CurPair->second.WritePlots();
	}
	AnalysisOut->Write(); // write all histograms in memeory to this file
	ClearAllPlots();
	delete AnalysisOut; // close RooT file and delete pointer
//	EvtListFileOut.close();
}

void TTdcSyncAnalysis::ClearAllPlots(){
	if(TdcPairs.empty()) // nothing to delete
		return;
	std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::iterator FirstPair = TdcPairs.begin();
	std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::iterator LastPair	= TdcPairs.end();
	for(std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::iterator CurPair=FirstPair; CurPair!=LastPair; ++CurPair){
		CurPair->second.ClearPlots();
	}
}

//Double_t TTdcSyncAnalysis::GetSynchronisedTime(const std::pair< std::pair< Int_t,Int_t >,TrbPixelHit>& UserHit ) const {
//	Double_t fTempSynchronisedTime = -1.0;
//	fTempSynchronisedTime = TrbData->Hits_fTime[UserHit.first.first] - TrbData->Hits_fTime[UserHit.second.nSyncIndex];
//	return (fTempSynchronisedTime);
//}
//


void TTdcSyncAnalysis::GenerateTdcPairs(){
	if(nNPairs<2)
		return;
	std::map< UInt_t,UInt_t >::const_iterator FirstTdc	= TdcAddresses.begin();
	std::map< UInt_t,UInt_t >::const_iterator LastTdc	= TdcAddresses.end();
	for(std::map< UInt_t,UInt_t >::const_iterator CurTdcA=FirstTdc; CurTdcA!=LastTdc; ++CurTdcA){
		for(std::map< UInt_t,UInt_t >::const_iterator CurTdcB=CurTdcA; CurTdcB!=LastTdc; ++CurTdcB){
			if(CurTdcB==CurTdcA) // skip if same TDC
				continue;
			//std::pair< UInt_t,UInt_t > PairTdcAddresses = make_pair(CurTdcA->first,CurTdcB->first);
			stringstream TempName;
			TempName << hex << CurTdcA->first << dec << "_" << hex << CurTdcB->first;
			SyncHistograms Temp;
			Temp.cBasename = TempName.str();
			TdcPairs.insert(make_pair(make_pair(CurTdcA->first,CurTdcB->first),Temp));
		}
	}
}

UInt_t TTdcSyncAnalysis::GetTdcSyncCoarseTimestamp(UInt_t nTdcAddress) const {
	Int_t nTempSyncIndex = GetTdcSyncIndex(nTdcAddress);
	if(nTempSyncIndex<0)
		return (-1.0);
	else
		return (TrbData->Hits_nCoarseTime[nTempSyncIndex]);
}

UInt_t TTdcSyncAnalysis::GetTdcSyncFineTimestamp(UInt_t nTdcAddress) const {
	Int_t nTempSyncIndex = GetTdcSyncIndex(nTdcAddress);
	if(nTempSyncIndex<0)
		return (-1.0);
	else
		return (TrbData->Hits_nFineTime[nTempSyncIndex]);
}

void TTdcSyncAnalysis::Init(){
	cout << "This is TTdcSyncAnalysis::Init()..." << endl;
	nNPairs = 0;
	fHist2dMin = 0.0;
	fHist2dMax = 100000.;
	fHistCoarseMin = 0.0;
	fHistCoarseMax = 10000.0;
	TdcPairs.clear();
}


void TTdcSyncAnalysis::InitHistograms(){
	if(TdcPairs.empty()) // nothing to delete
		return;
	std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::iterator FirstPair = TdcPairs.begin();
	std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::iterator LastPair	= TdcPairs.end();
	for(std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::iterator CurPair=FirstPair; CurPair!=LastPair; ++CurPair){
		string cHistoName = "hTimeDiff_" + CurPair->second.cBasename;
		string cHistoTitle = "Time Difference " + CurPair->second.cBasename;
		CurPair->second.hTimeDiff = new TH1D(cHistoName.c_str(),cHistoTitle.c_str(),300,-15.0,15.0);
		cHistoName.clear();	cHistoTitle.clear();
		cHistoName = "hCoarseTimeDiff_" + CurPair->second.cBasename;
		cHistoTitle = "Coarse Time Difference " + CurPair->second.cBasename;
		CurPair->second.hCoarseTimeDiff = new TH1D(cHistoName.c_str(),cHistoTitle.c_str(),200,-10.0,10.0);
		cHistoName.clear();	cHistoTitle.clear();
		cHistoName = "hFineTimeDiff_" + CurPair->second.cBasename;
		cHistoTitle = "Fine Time Difference " + CurPair->second.cBasename;
		CurPair->second.hFineTimeDiff = new TH1D(cHistoName.c_str(),cHistoTitle.c_str(),1200,-600.0,600.0);
		cHistoName.clear();	cHistoTitle.clear();
		cHistoName = "hTimeCorrelation_" + CurPair->second.cBasename;
		cHistoTitle = "Time Correlation " + CurPair->second.cBasename;
		CurPair->second.hTimeCorrelation = new TH2D(cHistoName.c_str(),cHistoTitle.c_str(),1000,fHist2dMin,fHist2dMax,1000,fHist2dMin,fHist2dMax);
		cHistoName.clear();	cHistoTitle.clear();
		cHistoName = "hCoarseTimeCorrelation_" + CurPair->second.cBasename;
		cHistoTitle = "Coarse Time Correlation " + CurPair->second.cBasename;
		CurPair->second.hCoarseTimeCorrelation = new TH2D(cHistoName.c_str(),cHistoTitle.c_str(),100,fHistCoarseMin,fHistCoarseMax,100,fHistCoarseMin,fHistCoarseMax);
		cHistoName.clear();	cHistoTitle.clear();
		cHistoName = "hFineTimeCorrelation_" + CurPair->second.cBasename;
		cHistoTitle = "Fine Time Correlation " + CurPair->second.cBasename;
		CurPair->second.hFineTimeCorrelation = new TH2D(cHistoName.c_str(),cHistoTitle.c_str(),300,0.0,600.0,300,0.0,600.0);
	}
}

void TTdcSyncAnalysis::PrintTdcPairs() const {
	if(TdcPairs.empty())
		return;
	cout << "++++++++++++++++++++++++++" << endl;
	cout << "+++     TDC Pairs      +++" << endl;
	cout << "++++++++++++++++++++++++++" << endl;
	std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::const_iterator FirstPair	= TdcPairs.begin();
	std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::const_iterator LastPair	= TdcPairs.end();
	for(std::map< std::pair<UInt_t,UInt_t>,SyncHistograms >::const_iterator CurPair=FirstPair; CurPair!=LastPair; ++CurPair){
		cout << hex << CurPair->first.first << "\t" << CurPair->first.second << "\t" << CurPair->second.cBasename << endl;
	}
}

//void TTdcSyncAnalysis::Show(){
//	if(!GetTreeStatus())
//		return;
//	TrbData->Show(0);
//}

Int_t TTdcSyncAnalysis::SetTdcAddresses(string cUserTdcAddressFile){
	Int_t nSizeOfTdcList = TTrbAnalysisBase::SetTdcAddresses(cUserTdcAddressFile);
	if(nSizeOfTdcList>0)
		nNPairs = Combinations(nSizeOfTdcList,2);
	return (nSizeOfTdcList);
}