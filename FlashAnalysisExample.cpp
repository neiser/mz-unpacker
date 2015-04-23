class TFlashAnalysis; // forward declaration of FLASH analysis class (needed on Linux systems)

struct SetupFiles {
	string cUserTdcList;
	string cUserTotCutFile;
	string cUserLECutFile;
	string cUserPixelPairFile;
	string cUserRequiredPixelFile;
	string cUserPixelOffsetFile;
	SetupFiles() {
		cUserTdcList = "";
		cUserTotCutFile = "";
		cUserLECutFile = "";
		cUserPixelPairFile = "";
		cUserRequiredPixelFile = "";
		cUserPixelOffsetFile = "";
	};
} FlashAnalysisSetup;

SetupFiles TestCaseTight(){
	SetupFiles TestCase;
	TestCase.cUserTdcList = "TDC_FLASH.txt";
}



TF1* AsymTiming(){
	TF1* temp = new TF1("asyt","[0]*TMath::Gaus(x,[1],[2])*pow(0.5*TMath::Erfc(x-[1]),[3])",-5.0,5.0);
	return (temp);
}

void SetTriggerConditions(TFlashAnalysis& DataSet) {
	DataSet.KeepMultiHits(); // keep channels with multiple hits in time window
	DataSet.SetTimingWindow(-400.0,100.0); // set time window
	DataSet.SetTriggerChannel(0xc00b,9); // select trigger channel
	DataSet.SetTriggerWindow(-240.0,-228.0); // set trigger time window
}

TList* RunBasicAnalysis(TFlashAnalysis& DataSet, UInt_t nIncrement=10){
	enum AnalysisStats {ALL=0, PIXEL_CUTS=1, TRIGGER_CUTS=2};
	// set analysis parameters
	// this can be put in a different function to ba available for all kinds of analyses
	SetTriggerConditions(DataSet); // set trigger and analysis parameters
	// create histograms and add to TList for displaying by analysis macro
	TH1D* hAnalysisStats = new TH1D("hAnalysisStats","Analysis Statistics; event type; freq",10,-1.5,8.5);
	TH1D* hPixelMultiplicity = new TH1D("hPixelMultiplicity","Pixel multiplicity; # of hit pixels per event; freq.",100,-0.5,99.5);
	TH1D* hPixelPairs = new TH1D("hPixelPairs","Number of found Pixel Pairs per Event; N_{PP}; freq",DataSet.GetNumberOfPixelPairs()+2,-0.5,DataSet.GetNumberOfPixelPairs()+1.5);
	TH1D* hTriggerTime = new TH1D("hTriggerTime","Trigger Channel Time; T_{LE} (ns); freq",1000,-300.0,-200.0);
	TH1D* hTriggerMult = new TH1D("hTriggerMult","Trigger Multiplicity; trigger multiplicity; freq",10,-1.5,8.5);
	TList* ListOfAnalysisHistograms = new TList(); // this list stores the pointers to basic analysis histograms needed for the general analysis
	ListOfAnalysisHistograms->Add(hAnalysisStats);
	ListOfAnalysisHistograms->Add(hPixelMultiplicity);
	ListOfAnalysisHistograms->Add(hTriggerTime);
	ListOfAnalysisHistograms->Add(hTriggerMult);
	ListOfAnalysisHistograms->Add(hPixelPairs);
	// define analysis loop
	for(Int_t i=0; i<(Int_t)DataSet.GetNEvents(); i+=nIncrement){ // begin of loop over all events
		hAnalysisStats->Fill((Double_t)ALL);
		if(DataSet.GetEntry(i)<1){
			//cout << "DATA ERROR: Skipping event \t" << i << endl;
			//nSkippedEvts++;
			continue;
		}
		hAnalysisStats->Fill((Double_t)PIXEL_CUTS);
		if(DataSet.GetNSyncTimestamps()!=5){ // there is a problem with the TDC synchronisation
			cout << "SYNC ERROR: Skipping event \t" << i << endl;
			continue;
		}
		Int_t nTriggerMult = DataSet.GetTriggerMultiplicity();
		hTriggerMult->Fill((Double_t)nTriggerMult);
		if(nTriggerMult!=1){
			//cout << "Wrong Trigger Multiplicity\t" << nTriggerMult << endl;
			continue;
		}
		hAnalysisStats->Fill((Double_t)TRIGGER_CUTS);
		Double_t fTrigTime = 0.0;
		if(DataSet.GetTriggerTime(fTrigTime))
			hTriggerTime->Fill(fTrigTime);
		DataSet.Analyse(); // analyse current entry
		hPixelMultiplicity->Fill((Double_t)DataSet.GetNumberOfHitPixels());
		hPixelPairs->Fill((Double_t)DataSet.GetNumberOfCorrelations());
	} // end of loop over all events
	return (ListOfAnalysisHistograms);
}

TList* RunTimingAnalysis(TFlashAnalysis& DataSet, UInt_t nIncrement=10){
	enum AnalysisStats {ALL=0, PIXEL_CUTS=1, TRIGGER_CUTS=2};
	// set analysis parameters
	// this can be put in a different function to ba available for all kinds of analyses
	SetTriggerConditions(DataSet); // set trigger and analysis parameters
	// create histograms and add to TList for displaying by analysis macro
	TH1D* hAnalysisStats = new TH1D("hAnalysisStats","Analysis Statistics; event type; freq",10,-1.5,8.5);
	TH1D* hPixelMultiplicity = new TH1D("hPixelMultiplicity","Pixel multiplicity; # of hit pixels per event; freq.",100,-0.5,99.5);
	TH1D* hPixelPairs = new TH1D("hPixelPairs","Number of found Pixel Pairs per Event; N_{PP}; freq",DataSet.GetNumberOfPixelPairs()+2,-0.5,DataSet.GetNumberOfPixelPairs()+1.5);
	TH1D* hTriggerTime = new TH1D("hTriggerTime","Trigger Channel Time; T_{LE} (ns); freq",1000,-300.0,-200.0);
	TH1D* hTriggerMult = new TH1D("hTriggerMult","Trigger Multiplicity; trigger multiplicity; freq",10,-1.5,8.5);
	TH1D* hTimeAvg = new TH1D("hTimeAvg","Averaged Time; avg time (ns); freq",1000,-5.0,5.0);
	TH2D* hTimeAvgvsMult = new TH2D("hTimeAvgvsMult","Averaged Time as function of multiplicity; pixel pair multiplicity; avg time (ns); freq",30,-0.5,29.5,1000,-5.0,5.0);
	TH1D* hTimeRMS = new TH1D("hTimeRMS","Avg Event Time RMS;RMS_{avg time} (ns); freq",1000,-5.0,5.0);
	TH2D* hTimeRMSvsMult = new TH2D("hTimeRMSvsMult","Avg Event Time as function of Multiplicity; pixel pair multiplicity; RMS_{avg time} (ns); freq",30,-0.5,29.5,400,-1.0,1.0);
	//TH1D* hTimeBest = new TH1D("hTimeBest","Best Time; best time (ns); freq",1000,-5.0,5.0);
//	TH2D hTimeAvgMult("hTimeAvgMult","Average Time; # hit pixels; avg time (ns); freq ",(0.5*DataSet.GetSizeOfMapTable())+1,-0.5,(0.5*DataSet.GetSizeOfMapTable())+0.5,500,-5.0,5.0);
	TList* ListOfAnalysisHistograms = new TList(); // this list stores the pointers to basic analysis histograms needed for the general analysis
	//ListOfAnalysisHistograms->Add(hAnalysisStats);
	//ListOfAnalysisHistograms->Add(hPixelMultiplicity);
	//ListOfAnalysisHistograms->Add(hTriggerTime);
	//ListOfAnalysisHistograms->Add(hTriggerMult);
	//ListOfAnalysisHistograms->Add(hPixelPairs);
	ListOfAnalysisHistograms->Add(hTimeAvg);
	ListOfAnalysisHistograms->Add(hTimeAvgvsMult);
	ListOfAnalysisHistograms->Add(hTimeRMS);
	ListOfAnalysisHistograms->Add(hTimeRMSvsMult);
	//ListOfAnalysisHistograms->Add(hTimeBest);
//	ListOfAnalysisHistograms->Add(hTimeAvgMult);
	// define analysis loop
	std::vector<Double_t> fPixelTimes;
	for(Int_t i=0; i<(Int_t)DataSet.GetNEvents(); i+=nIncrement){ // begin of loop over all events
		hAnalysisStats->Fill((Double_t)ALL);
		if(DataSet.GetEntry(i)<1){
			//cout << "DATA ERROR: Skipping event \t" << i << endl;
			//nSkippedEvts++;
			continue;
		}
		hAnalysisStats->Fill((Double_t)PIXEL_CUTS);
		if(DataSet.GetNSyncTimestamps()!=5){ // there is a problem with the TDC synchronisation
			cout << "SYNC ERROR: Skipping event \t" << i << endl;
			continue;
		}
		Int_t nTriggerMult = DataSet.GetTriggerMultiplicity();
		hTriggerMult->Fill((Double_t)nTriggerMult);
		if(nTriggerMult!=1){
			//cout << "Wrong Trigger Multiplicity\t" << nTriggerMult << endl;
			continue;
		}
		hAnalysisStats->Fill((Double_t)TRIGGER_CUTS);
		Double_t fTrigTime = 0.0;
		if(DataSet.GetTriggerTime(fTrigTime))
			hTriggerTime->Fill(fTrigTime);
		DataSet.Analyse(); // analyse current entry
		hPixelMultiplicity->Fill((Double_t)DataSet.GetNumberOfHitPixels());
		hPixelPairs->Fill((Double_t)DataSet.GetNumberOfCorrelations());
		// timing analysis
		if(DataSet.GetNumberOfCorrelations()==0)
			continue;
		Double_t fDelta; // temporary time difference between two pixels
		// loop over all pixel pairs
		std::set< std::pair<UInt_t,UInt_t> >* PixelList = DataSet.GetListOfPixelPairs(); // get pointer to list of all declared pixel pairs
		std::set< std::pair<UInt_t,UInt_t> >::const_iterator first = PixelList->begin();
		std::set< std::pair<UInt_t,UInt_t> >::const_iterator last = PixelList->end();
		std::set< std::pair<UInt_t,UInt_t> >::iterator itr;
		for(itr=first; itr!=last; ++itr){ // begin loop over all pixel pairs
			if(DataSet.GetPairTimeDiff(itr->first,itr->second,fDelta)){ // check if pixel pair combination was found in this event
				fPixelTimes.push_back(fDelta); // enter pixel time differnce into vector
			}
		} // end of loop over all pixel pairs
		if(fPixelTimes.size()>2){
			Double_t fAvgTime		= TMath::Mean(fPixelTimes.size(),&fPixelTimes[0]); // compute average time difference
			Double_t fAvgTimeRMS	= TMath::RMS(fPixelTimes.size(),&fPixelTimes[0]); // compute average RMS of time difference
			hTimeAvg->Fill(fAvgTime);
			hTimeAvgvsMult->Fill((Double_t)fPixelTimes.size(),fAvgTime);
			hTimeRMS->Fill(fAvgTimeRMS);
			hTimeRMSvsMult->Fill((Double_t)fPixelTimes.size(),fAvgTimeRMS);
		}
		fPixelTimes.clear();
	} // end of loop over all events
	return (ListOfAnalysisHistograms);
}


// need 20 pixel pairs maximum
void FlashBasicAnalysis(string cUserDataFile, SetupFiles& AnalysisSetup, UInt_t nUserFraction=10){
	gROOT->ProcessLine(".x BuildFlashAnalysis.cpp");
	TTimeStamp RunDate; // get time and date of analysis execution
	UInt_t nDate = RunDate.GetDate();
	UInt_t nTime = RunDate.GetTime();
	TFlashAnalysis BasicAnalysis(cUserDataFile,AnalysisSetup.cUserTdcList); // define FLASH analysis object
	
	string cBasicOutputName = cUserDataFile.substr(0,cUserDataFile.length()-5); // extract basic output name from data file name
	// create log file
	stringstream cLogName;
	cLogName << cBasicOutputName << "-Flash_BasicAnalysis_" << nDate << "-" << nTime << ".log"; // name for log file
	std::ofstream AnalysisLogFile; // create log file stream object
	AnalysisLogFile.open(cLogName.str().c_str()); // open log file stream
	BasicAnalysis.SetLogFile(AnalysisLogFile); // register log file with FLASH analysis
	// create output file
	stringstream cRootOutputName; 
	cRootOutputName << cBasicOutputName << "-Flash_BasicAnalysis_" << nDate << "-" << nTime << ".root"; // set file name for output file
	TFile OverviewOutput(cRootOutputName.str().c_str(),"RECREATE"); // open output file
	if(OverviewOutput.IsZombie()) // check if output file is opened properly
		return;
	// set analysis details
	UInt_t nPairsAdded		= BasicAnalysis.AddPixelPairs(AnalysisSetup.cUserPixelPairFile);
	UInt_t nOffsetsAdded	= BasicAnalysis.SetPixelTimeOffsets(AnalysisSetup.cUserPixelOffsetFile);
	UInt_t nReqPixelsAdded	= BasicAnalysis.AddRequiredPixels(AnalysisSetup.cUserRequiredPixelFile);
	UInt_t nTotCutsAdded	= BasicAnalysis.AddPixelToTCuts(AnalysisSetup.cUserTotCutFile);
	UInt_t nLECutsAdded		= BasicAnalysis.AddPixelLECuts(AnalysisSetup.cUserLECutFile);
	// define histograms
	TList* ListOfTimingHistograms = new TList();
	TList* ListOfCorrelationHistograms = new TList();
	std::set< std::pair<UInt_t,UInt_t> >* PixelList = BasicAnalysis.GetListOfPixelPairs();
	
	std::set< std::pair<UInt_t,UInt_t> >::const_iterator first = PixelList->begin();
	std::set< std::pair<UInt_t,UInt_t> >::const_iterator last = PixelList->end();
	std::set< std::pair<UInt_t,UInt_t> >::iterator itr;
	for(itr=first; itr!=last; ++itr){
		stringstream cHistName;
		stringstream cHistTitle;
		cHistName << "hTimeDelta_" << itr->first << "_" << itr->second;
		cHistTitle << "Time difference Chan #" << itr->first << "/#" << itr->second << "; #Delta t_{" << itr->first << "-" << itr->second << "} (ns);freq";
		TH1D* hTemp = new TH1D(cHistName.str().c_str(),cHistTitle.str().c_str(),1000,-10.0,10.0);
		ListOfTimingHistograms->Add(hTemp);
		BasicAnalysis.RegisterTimeDiffHist(itr->first,itr->second,hTemp);
		cHistName.str("");
		cHistTitle.str("");
		cHistName << "hToTCorr_" << itr->first << "_" << itr->second;
		cHistTitle << "ToT Correlation Chan #" << itr->first << "/#" << itr->second << "; ToT_{" << itr->first << "} (ns); ToT_{" << itr->second << "} (ns); freq";
		TH2D* hTempCorr = new TH2D(cHistName.str().c_str(),cHistTitle.str().c_str(),600,0.0,30.0,600,0.0,30.0);
		ListOfCorrelationHistograms->Add(hTempCorr);
		BasicAnalysis.RegisterTotCorrHist(itr->first,itr->second,hTempCorr);
	}
	// loop over events
	TList* ListOfStandardHistograms = NULL;
	ListOfStandardHistograms = RunBasicAnalysis(BasicAnalysis,nUserFraction);
	// show results
	// everything related to the trigger system
	string cCanvasTitle = "FLASH Analysis Statistics - " + cBasicOutputName;
	TCanvas *can_Statistics = new TCanvas("can_Statistics",cCanvasTitle.c_str());
	can_Statistics->Divide(3,2);
	can_Statistics->cd(1);
	((TH1D*)ListOfStandardHistograms->At(0))->DrawCopy();
	can_Statistics->cd(2);
	((TH1D*)ListOfStandardHistograms->At(1))->DrawCopy();
	can_Statistics->cd(3);
	((TH1D*)ListOfStandardHistograms->At(2))->DrawCopy();
	can_Statistics->cd(4);
	((TH1D*)ListOfStandardHistograms->At(3))->DrawCopy();
	can_Statistics->cd(5);
	((TH1D*)ListOfStandardHistograms->At(4))->DrawCopy();
	// everything on Time Differences
	//cCanvasTitle.clear();
	cCanvasTitle = "FLASH Analysis Time Differences - " + cBasicOutputName;
	TCanvas *can_Timing = new TCanvas("can_Timing",cCanvasTitle.c_str());
	Double_t fSqrt = sqrt(ListOfTimingHistograms->GetSize());
	Int_t nCanvasColumns = ceil(fSqrt);
	Int_t nCanvasRows = (ListOfTimingHistograms->GetSize()+1)/nCanvasColumns;
	can_Timing->Divide(nCanvasColumns,nCanvasRows);
	for(Int_t n=0; n<ListOfTimingHistograms->GetSize();n++){
		can_Timing->cd(n+1);
		((TH1D*)ListOfTimingHistograms->At(n))->DrawCopy();
	}
	// everything on ToT correlations
	cCanvasTitle = "FLASH Analysis ToT Correlations - " + cBasicOutputName;
	TCanvas *can_Tot = new TCanvas("can_Tot",cCanvasTitle.c_str());
	fSqrt = sqrt(ListOfCorrelationHistograms->GetSize());
	nCanvasColumns = ceil(fSqrt);
	nCanvasRows = (ListOfCorrelationHistograms->GetSize()+1)/nCanvasColumns;
	can_Tot->Divide(nCanvasColumns,nCanvasRows);
	for(Int_t n=0; n<ListOfCorrelationHistograms->GetSize();n++){
		can_Tot->cd(n+1);
		((TH2D*)ListOfCorrelationHistograms->At(n))->DrawCopy("COLZ");
	}
	// write everything to disk
	can_Statistics->Write();
	can_Timing->Write();
	can_Tot->Write();
	OverviewOutput.Write();
	delete ListOfStandardHistograms;
	delete ListOfTimingHistograms;
	delete ListOfCorrelationHistograms;
}


void FlashWalkAnalysis(string cUserDataFile, SetupFiles& AnalysisSetup, UInt_t nIncrement=10){
	// build FLASH analysis class and create analysis object
	gROOT->ProcessLine(".x BuildFlashAnalysis.cpp");
	TFlashAnalysis WalkAnalysis(cUserDataFile,AnalysisSetup.cUserTdcList); // adjust these addresses according to your folder structure!
	string cBasicOutputName = cUserDataFile.substr(0,cUserDataFile.length()-5);
	// create log file
	string cLogName = cBasicOutputName + "-Flash_TimeWalk_Analysis.log"; // name for log file
	std::ofstream AnalysisLogFile; // create log file
	AnalysisLogFile.open(cLogName.c_str()); // open log file
	WalkAnalysis.SetLogFile(AnalysisLogFile); // register log file with FLASH analysis
	// create output file
	string cRootOutputName = cBasicOutputName + "-Flash_TimeWalk_Analysis.root"; // set file name for output file
	TFile TimeWalkOutput(cRootOutputName.c_str(),"RECREATE"); // open output file
	if(TimeWalkOutput.IsZombie()) // check if output file is opened properly
		return;
	// set analysis details
	// setting channel 38 as reference
	//WalkAnalysis.AddPixelToTCut(38,14.58,14.63);
	// set analysis details
	UInt_t nPairsAdded		= WalkAnalysis.AddPixelPairs(AnalysisSetup.cUserPixelPairFile);
	UInt_t nOffsetsAdded	= WalkAnalysis.SetPixelTimeOffsets(AnalysisSetup.cUserPixelOffsetFile);
	UInt_t nReqPixelsAdded	= WalkAnalysis.AddRequiredPixels(AnalysisSetup.cUserRequiredPixelFile);
	UInt_t nTotCutsAdded	= WalkAnalysis.AddPixelToTCuts(AnalysisSetup.cUserTotCutFile);
	UInt_t nLECutsAdded		= WalkAnalysis.AddPixelLECuts(AnalysisSetup.cUserLECutFile);
	// define histograms
	TList* ListOfWalkHistograms = new TList();
	std::set< std::pair<UInt_t,UInt_t> >* PixelList = WalkAnalysis.GetListOfPixelPairs();
	std::set< std::pair<UInt_t,UInt_t> >::const_iterator first = PixelList->begin();
	std::set< std::pair<UInt_t,UInt_t> >::const_iterator last = PixelList->end();
	std::set< std::pair<UInt_t,UInt_t> >::iterator itr;
	for(itr=first; itr!=last; ++itr){ // begin loop over all pixel pairs
		stringstream cHistName;
		stringstream cHistTitle;
		cHistName << "hWalk_" << itr->first << "_" << itr->second;
		cHistTitle << "Walk Correlation; ToT_{" << itr->second << "} (ns); LE_{" << itr->first << "-" << itr->second << "} (ns); freq";
		TH2D* hTempWalk = new TH2D(cHistName.str().c_str(),cHistTitle.str().c_str(),1000,0.0,50.0,1000,-5.0,5.0);
		
		ListOfWalkHistograms->Add(hTempWalk);
		WalkAnalysis.RegisterTimeWalkHist(itr->first,itr->second,hTempWalk);
	} // end of loop over all pixel pairs
	// analyse events
	TList* ListOfStandardHistograms = NULL;
	ListOfStandardHistograms = RunBasicAnalysis(WalkAnalysis,nIncrement);
	delete ListOfStandardHistograms;
	// display results
	cCanvasTitle = "FLASH Time Walk Analysis - " + cBasicOutputName;
	TCanvas *can_TimeWalk = new TCanvas("can_TimeWalk",cCanvasTitle.c_str());
	Double_t fSqrt = sqrt(ListOfWalkHistograms->GetSize());
	Int_t nCanvasColumns = ceil(fSqrt);
	Int_t nCanvasRows = (ListOfWalkHistograms->GetSize()+1)/nCanvasColumns;
	can_TimeWalk->Divide(nCanvasColumns,nCanvasRows);
	for(Int_t n=0; n<ListOfWalkHistograms->GetSize();n++){
		can_TimeWalk->cd(n+1);
		((TH2D*)ListOfWalkHistograms->At(n))->DrawCopy("COLZ");
	}
	// write output
	can_TimeWalk.Write();
	TimeWalkOutput.Write();
	// clean up
	delete ListOfWalkHistograms;
	AnalysisLogFile.close();
}

void FlashTimingAnalysis(string cUserDataFile, SetupFiles& AnalysisSetup, UInt_t nUserFraction=10){
	gROOT->ProcessLine(".x BuildFlashAnalysis.cpp");
	TTimeStamp RunDate; // get time and date of analysis execution
	UInt_t nDate = RunDate.GetDate();
	UInt_t nTime = RunDate.GetTime();
	TFlashAnalysis TimingAnalysis(cUserDataFile,AnalysisSetup.cUserTdcList); // define FLASH analysis object
	
	string cBasicOutputName = cUserDataFile.substr(0,cUserDataFile.length()-5); // extract basic output name from data file name
	// create log file
	stringstream cLogName;
	cLogName << cBasicOutputName << "-Flash_TimingAnalysis_" << nDate << "-" << nTime << ".log"; // name for log file
	std::ofstream AnalysisLogFile; // create log file stream object
	AnalysisLogFile.open(cLogName.str().c_str()); // open log file stream
	TimingAnalysis.SetLogFile(AnalysisLogFile); // register log file with FLASH analysis
	// create output file
	stringstream cRootOutputName; 
	cRootOutputName << cBasicOutputName << "-Flash_TimingAnalysis_" << nDate << "-" << nTime << ".root"; // set file name for output file
	TFile OverviewOutput(cRootOutputName.str().c_str(),"RECREATE"); // open output file
	if(OverviewOutput.IsZombie()) // check if output file is opened properly
		return;
	// set analysis details
	UInt_t nPairsAdded		= TimingAnalysis.AddPixelPairs(AnalysisSetup.cUserPixelPairFile);
	UInt_t nOffsetsAdded	= TimingAnalysis.SetPixelTimeOffsets(AnalysisSetup.cUserPixelOffsetFile);
	UInt_t nReqPixelsAdded	= TimingAnalysis.AddRequiredPixels(AnalysisSetup.cUserRequiredPixelFile);
	UInt_t nTotCutsAdded	= TimingAnalysis.AddPixelToTCuts(AnalysisSetup.cUserTotCutFile);
	UInt_t nLECutsAdded		= TimingAnalysis.AddPixelLECuts(AnalysisSetup.cUserLECutFile);
	// define histograms
	TList* ListOfTimingHistograms = new TList();
	TList* ListOfCorrelationHistograms = new TList();
	std::set< std::pair<UInt_t,UInt_t> >* PixelList = TimingAnalysis.GetListOfPixelPairs();
	
	std::set< std::pair<UInt_t,UInt_t> >::const_iterator first = PixelList->begin();
	std::set< std::pair<UInt_t,UInt_t> >::const_iterator last = PixelList->end();
	std::set< std::pair<UInt_t,UInt_t> >::iterator itr;
	for(itr=first; itr!=last; ++itr){
		stringstream cHistName;
		stringstream cHistTitle;
		cHistName << "hTimeDelta_" << itr->first << "_" << itr->second;
		cHistTitle << "Time difference Chan #" << itr->first << "/#" << itr->second << "; #Delta t_{" << itr->first << "-" << itr->second << "} (ns);freq";
		TH1D* hTemp = new TH1D(cHistName.str().c_str(),cHistTitle.str().c_str(),1000,-10.0,10.0);
		ListOfTimingHistograms->Add(hTemp);
		TimingAnalysis.RegisterTimeDiffHist(itr->first,itr->second,hTemp);
		cHistName.str("");
		cHistTitle.str("");
		cHistName << "hToTCorr_" << itr->first << "_" << itr->second;
		cHistTitle << "ToT Correlation Chan #" << itr->first << "/#" << itr->second << "; ToT_{" << itr->first << "} (ns); ToT_{" << itr->second << "} (ns); freq";
		TH2D* hTempCorr = new TH2D(cHistName.str().c_str(),cHistTitle.str().c_str(),600,0.0,30.0,600,0.0,30.0);
		ListOfCorrelationHistograms->Add(hTempCorr);
		TimingAnalysis.RegisterTotCorrHist(itr->first,itr->second,hTempCorr);
	}
	// loop over events
	TList* ListOfStandardHistograms = NULL;
	ListOfStandardHistograms = RunTimingAnalysis(TimingAnalysis,nUserFraction);
	// show results
	// everything related to the trigger system
	string cCanvasTitle = "FLASH AnalysisTiming - " + cBasicOutputName;
	TCanvas *can_Statistics = new TCanvas("can_Statistics",cCanvasTitle.c_str());
	can_Statistics->Divide(2,2);
	can_Statistics->cd(1);
	((TH1D*)ListOfStandardHistograms->At(0))->DrawCopy();
	can_Statistics->cd(2);
	((TH2D*)ListOfStandardHistograms->At(1))->DrawCopy("COLZ");
	can_Statistics->cd(3);
	((TH1D*)ListOfStandardHistograms->At(2))->DrawCopy();
	can_Statistics->cd(4);
	((TH2D*)ListOfStandardHistograms->At(3))->DrawCopy("COLZ");
	// everything on Time Differences
	//cCanvasTitle.clear();
	cCanvasTitle = "FLASH Analysis Time Differences - " + cBasicOutputName;
	TCanvas *can_Timing = new TCanvas("can_Timing",cCanvasTitle.c_str());
	Double_t fSqrt = sqrt(ListOfTimingHistograms->GetSize());
	Int_t nCanvasColumns = ceil(fSqrt);
	Int_t nCanvasRows = (ListOfTimingHistograms->GetSize()+1)/nCanvasColumns;
	can_Timing->Divide(nCanvasColumns,nCanvasRows);
	for(Int_t n=0; n<ListOfTimingHistograms->GetSize();n++){
		can_Timing->cd(n+1);
		((TH1D*)ListOfTimingHistograms->At(n))->DrawCopy();
	}
	// everything on ToT correlations
	cCanvasTitle = "FLASH Analysis ToT Correlations - " + cBasicOutputName;
	TCanvas *can_Tot = new TCanvas("can_Tot",cCanvasTitle.c_str());
	fSqrt = sqrt(ListOfCorrelationHistograms->GetSize());
	nCanvasColumns = ceil(fSqrt);
	nCanvasRows = (ListOfCorrelationHistograms->GetSize()+1)/nCanvasColumns;
	can_Tot->Divide(nCanvasColumns,nCanvasRows);
	for(Int_t n=0; n<ListOfCorrelationHistograms->GetSize();n++){
		can_Tot->cd(n+1);
		((TH2D*)ListOfCorrelationHistograms->At(n))->DrawCopy("COLZ");
	}
	// write everything to disk
	can_Statistics->Write();
	can_Timing->Write();
	can_Tot->Write();
	OverviewOutput.Write();
	delete ListOfStandardHistograms;
	delete ListOfTimingHistograms;
	delete ListOfCorrelationHistograms;
}

void FlashAnalysisExample(string cUserDataFile, string cUserTdcList){
	gROOT->ProcessLine(".x BuildFlashAnalysis.cpp");
	enum AnalysisStats {ALL=0, PIXEL_CUTS=1, TRIGGER_CUTS=2};
	TFlashAnalysis a(cUserDataFile,cUserTdcList); // adjust these addresses according to your folder structure!
	// create log file
	std::ofstream filestr;
	filestr.open ("test.log");
	a.SetLogFile(filestr);
	// set analysis parameters
	a.KeepMultiHits(); // keep channels with multiple hits in time window
	a.SetTimingWindow(-400.0,100.0); // set time window
	a.SetTriggerChannel(0xc00b,9); // select trigger channel
	a.SetTriggerWindow(-240.0,-228.0); // set trigger time window
	a.AddRequiredPixel(38);
	a.AddRequiredPixel(102);
	a.AddRequiredPixel(166);
	//a.AddPixelToTCut(166,14.75,15.25);
	a.AddPixelToTCut(166,21.5,22.5); // for high gain, threshold 1.3 data set
	a.AddRequiredPixel(230);
	// add LE cuts
	a.AddPixelLECuts("FLASH_PixelLECuts_566250.txt");
	// add ToT cuts
	a.AddPixelToTCuts("FLASH_HG_ToT_Cuts.txt");
	//a.AddPixelToTCut(38,13.96,15.16);
	//a.AddPixelToTCut(56,16.12,17.32);
	//a.AddPixelToTCut(198,15.77,16.97);
	//a.AddPixelToTCut(216,16.16,17.36);
	//a.AddPixelToTCut(230,13.95,15.15);
	//a.IgnoreOffsets();
	// setting channel 38 as reference
	a.AddPixelPair(36,40);
	a.AddPixelPair(38,22);
	a.AddPixelPair(38,24);
	a.AddPixelPair(38,36);
	a.AddPixelPair(38,40);
	a.AddPixelPair(38,56);
	a.AddPixelPair(38,198);
	a.AddPixelPair(38,216);
	a.AddPixelPair(38,230);
	a.AddPixelPair(230,248);
	// set pixel timing offsets
	//a.SetPixelTimeOffsets("FLASH_PixelOffsets.txt");
	a.SetPixelTimeOffsets("FLASH_PixelOffsets_HG.txt");
	//a.SetPixelTimeOffset(56,-5.085);
	//a.SetPixelTimeOffset(198,-0.6237);
	//a.SetPixelTimeOffset(216,-1.709);
	//a.SetPixelTimeOffset(230,0.6106);
	// define histograms
	TH1D hAnalysisStats("hAnalysisStats","Analysis Statistics; event type; freq",10,-1.5,8.5);
	TH1D hPixelMultiplicity("hPixelMultiplicity","Pixel multiplicity; # of hit pixels per event; freq.",(0.5*a.GetSizeOfMapTable())+1,-0.5,(0.5*a.GetSizeOfMapTable())+0.5);
	TH1D hTriggerTime("hTriggerTime","Trigger Channel Time; T_{LE} (ns); freq",1000,-300.0,-200.0);
	TH1D hTriggerMult("hTriggerMult","Trigger Multiplicity; trigger multiplicity; freq",10,-1.5,8.5);
	//TH1D hTiming("hTiming","hit time; time (ns); freq",5000,-1000.0,1000.0);
//	TH1D hTimingChan12("hTimingChan12","hit time of chan #12; time (ns); freq",5000,-1000.0,1000.0);
//	TH1D hTimeDelta("hTimeDelta","Time difference chan #84/#86; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltaTight_38_56("hTimeDeltaTight_38_56","Time difference (tight ToT cuts) Chan #38/#56; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltaTight_38_198("hTimeDeltaTight_38_198","Time difference (tight ToT cuts) Chan #38/#198; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltaTight_38_216("hTimeDeltaTight_38_216","Time difference (tight ToT cuts) Chan #38/#216; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltaTight_38_230("hTimeDeltaTight_38_230","Time difference (tight ToT cuts) Chan #38/#230; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeAvg_Mcp1("hTimeAvg_Mcp1","Avg Time for MCP 1; avg time (ns); freq",1000,-5.0,5.0);
	TH1D hTimeBest_Mcp1("hTimeBest_Mcp1","Best Time for MCP 1; best time (ns); freq",1000,-5.0,5.0);
	TH2D hTimeAvgMult("hTimeAvgMult","hTimeAvgMult",(0.5*a.GetSizeOfMapTable())+1,-0.5,(0.5*a.GetSizeOfMapTable())+0.5,500,-5.0,5.0);
//	TH2D hTimingAllPixels("hTimingAllPixels","channel vs hit time; channel seq ID; time (ns); freq",a.GetSizeOfMapTable()+1,-0.5,a.GetSizeOfMapTable()+0.5,5000,-1000.0,1000.0);
	TH2D hToTCorr_36_40("hToTCorr_36_40","ToT Correlation; ToT_{36} (ns); ToT_{40} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_56("hToTCorr_38_56","ToT Correlation; ToT_{38} (ns); ToT_{56} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_40("hToTCorr_38_40","ToT Correlation; ToT_{38} (ns); ToT_{40} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_36("hToTCorr_38_36","ToT Correlation; ToT_{38} (ns); ToT_{36} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_24("hToTCorr_38_24","ToT Correlation; ToT_{38} (ns); ToT_{24} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_22("hToTCorr_38_22","ToT Correlation; ToT_{38} (ns); ToT_{22} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_198("hToTCorr_38_198","ToT Correlation; ToT_{38} (ns); ToT_{198} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_216("hToTCorr_38_216","ToT Correlation; ToT_{38} (ns); ToT_{216} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_230("hToTCorr_38_230","ToT Correlation; ToT_{38} (ns); ToT_{230} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_230_248("hToTCorr_230_248","ToT Correlation; ToT_{230} (ns); ToT_{248} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
//	TH2D hWalk_56("hWalk_56","Walk Correlation; ToT_{56} (ns); LE_{56} (ns); freq",1000,0.0,50.0,1000,0.0,10.0);
	TH2D hWalk_56("hWalk_56","Walk Correlation; ToT_{56} (ns); LE_{56} (ns); freq",1000,0.0,50.0,1000,-5.0,5.0);
	TH2D hWalk_198("hWalk_198","Walk Correlation; ToT_{198} (ns); LE_{198} (ns); freq",1000,0.0,50.0,1000,-5.0,5.0);
	TH2D hWalk_216("hWalk_216","Walk Correlation; ToT_{216} (ns); LE_{216} (ns); freq",1000,0.0,50.0,1000,-5.0,5.0);
	TH2D hWalk_230("hWalk_230","Walk Correlation; ToT_{230} (ns); LE_{230} (ns); freq",1000,0.0,50.0,1000,-5.0,5.0);
	
	// register histograms for analysis
	a.RegisterTimeDiffHist(38,56,&hTimeDeltaTight_38_56);
	a.RegisterTimeDiffHist(38,198,&hTimeDeltaTight_38_198);
	a.RegisterTimeDiffHist(38,216,&hTimeDeltaTight_38_216);
	a.RegisterTimeDiffHist(38,230,&hTimeDeltaTight_38_230);
	a.RegisterTotCorrHist(36,40,&hToTCorr_36_40);
	a.RegisterTotCorrHist(38,22,&hToTCorr_38_22);
	a.RegisterTotCorrHist(38,24,&hToTCorr_38_24);
	a.RegisterTotCorrHist(38,36,&hToTCorr_38_36);
	a.RegisterTotCorrHist(38,40,&hToTCorr_38_40);
	a.RegisterTotCorrHist(38,56,&hToTCorr_38_56);
	a.RegisterTotCorrHist(38,198,&hToTCorr_38_198);
	a.RegisterTotCorrHist(38,216,&hToTCorr_38_216);
	a.RegisterTotCorrHist(38,230,&hToTCorr_38_230);
	a.RegisterTotCorrHist(230,248,&hToTCorr_230_248);

	//a.AddPixelToTCut(38,14.5,14.7);
	//a.AddPixelToTCut(56,16.5,16.9);

	for(Int_t i=0; i<(Int_t)a.GetNEvents(); i+=10){
		hAnalysisStats.Fill((Double_t)ALL);
		if(a.GetEntry(i)<1){
			//cout << "DATA ERROR: Skipping event \t" << i << endl;
			//nSkippedEvts++;
			continue;
		}
		hAnalysisStats.Fill((Double_t)PIXEL_CUTS);
		if(a.GetNSyncTimestamps()!=5){ // there is a problem with the TDC synchronisation
			cout << "SYNC ERROR: Skipping event \t" << i << endl;
			continue;
		}
		Int_t nTriggerMult = a.GetTriggerMultiplicity();
		hTriggerMult.Fill((Double_t)nTriggerMult);
		if(nTriggerMult!=1){
			cout << "Wrong Trigger Multiplicity\t" << nTriggerMult << endl;
			continue;
		}
		hAnalysisStats.Fill((Double_t)TRIGGER_CUTS);
		Double_t fTrigTime = 0.0;
		if(a.GetTriggerTime(fTrigTime))
			hTriggerTime.Fill(fTrigTime);
		a.Analyse(); // analyse current entry
		hPixelMultiplicity.Fill((Double_t)a.GetNumberOfHitPixels());
		//cout << a.GetNumberOfHitPixels() << endl;
//		a.FillTimingHistogram(hTiming);
//		a.FillTimingHistogram(38,hTimingChan12);
		//a.FillTimingHistogram(hTimingAllPixels);
		Double_t fDelta;
		Double_t fAvgTime = 0.0;
		Double_t fBestDelta = 1000.0;
		Int_t nHitPixels = 0;
		// channel 56 between 16.0 and 17.5
		// channel 120 between 16.2 and 17.6
		if(a.GetPairTimeDiff(38,56,fDelta)){
			//hTimeDelta.Fill(fDelta);
			//a.FillToTCorrelation(38,56,hToTCorr_38_56);
			//a.FillWalkHistogram(38,14.55,14.65,56,hWalk_56);
			//if(a.GetPairTimeDiff(38,14.5,14.7,56,16.5,16.9,fDelta)){
			fAvgTime += fDelta;
			if(fabs(fDelta)<fabs(fBestDelta))
				fBestDelta = fDelta;
			nHitPixels++;
			//}
		}
		if(a.GetPairTimeDiff(38,198,fDelta)){
			//hTimeDelta.Fill(fDelta);
			//a.FillToTCorrelation(38,198,hToTCorr_38_198);
			//a.FillWalkHistogram(38,14.55,14.65,198,hWalk_198);
			//if(a.GetPairTimeDiff(38,14.4,14.8,198,16.1,16.5,fDelta)){
			//	hTimeDeltaTight_38_198.Fill(fDelta);
			fAvgTime += fDelta;
			if(fabs(fDelta)<fabs(fBestDelta))
				fBestDelta = fDelta;
			nHitPixels++;
			//}
		}
		if(a.GetPairTimeDiff(38,216,fDelta)){
			//hTimeDelta.Fill(fDelta);
			//a.FillToTCorrelation(38,216,hToTCorr_38_216);
			//a.FillWalkHistogram(38,14.55,14.65,216,hWalk_216);
			//if(a.GetPairTimeDiff(38,14.4,14.8,216,16.5,16.9,fDelta)){
			//	hTimeDeltaTight_38_216.Fill(fDelta);
			fAvgTime += fDelta;
			if(fabs(fDelta)<fabs(fBestDelta))
				fBestDelta = fDelta;
			nHitPixels++;
			//}
		}
		if(a.GetPairTimeDiff(38,230,fDelta)){
			//hTimeDelta.Fill(fDelta);
			//a.FillToTCorrelation(38,230,hToTCorr_38_230);
			//a.FillWalkHistogram(38,14.55,14.65,230,hWalk_230);
			//if(a.GetPairTimeDiff(38,14.4,14.8,230,14.4,14.8,fDelta)){
			//	hTimeDeltaTight_38_230.Fill(fDelta);
			fAvgTime += fDelta;
			if(fabs(fDelta)<fabs(fBestDelta))
				fBestDelta = fDelta;
			nHitPixels++;
			//}
		}
		//if(a.GetPixelCorrelation(56,198,fDelta))
		//	hTimeDeltanew.Fill(fDelta);
		if(nHitPixels>2){
			fAvgTime /= (Double_t)nHitPixels;
			hTimeAvg_Mcp1.Fill(fAvgTime);
			hTimeAvgMult.Fill((Double_t)a.GetNumberOfHitPixels(),fAvgTime);
			hTimeBest_Mcp1.Fill(fBestDelta);
		}
	}
	//std::list<PixelHitModel> m = a.GetPixelHits();
	//hPixelMultiplicity.DrawCopy();
	// draw histograms to canvases
	// everything related to the trigger system
	TCanvas *can_Trigger = new TCanvas("can_Trigger","FLASH - Trigger");
	can_Trigger->Divide(2,2);
	can_Trigger->cd(1);
	hAnalysisStats.DrawCopy();
	can_Trigger->cd(2);
	hTriggerTime.DrawCopy();
	can_Trigger->cd(3);
	hTriggerMult.DrawCopy();

	// everything related to the timing results
	TCanvas *can_Timing = new TCanvas("can_Timing","FLASH - Timing");
	can_Timing->Divide(2,2);
	can_Timing->cd(1);
	hPixelMultiplicity.DrawCopy();
	can_Timing->cd(2);
	hTimeAvg_Mcp1.DrawCopy();
	can_Timing->cd(3);
	hTimeAvgMult.DrawCopy("COLZ");
	can_Timing->cd(4);
	TF1* fc = new TF1("fc", "[2]*TMath::CauchyDist(x, [0], [1])", -1.0, 1.0);
	fc->SetParameters(0,1,100);
	TF1* fcg = new TF1("fcg","[0]/TMath::Pi()*[1]*sqrt([2])/(pow(x-[2],2)+pow([1],2)*[2])",-1.0,1.0);
	hTimeBest_Mcp1.Fit("fc","R");
	hTimeBest_Mcp1.DrawCopy();
	
	// everything related to ToT correlations
	TCanvas *can_ToT = new TCanvas("can_ToT","FLASH - ToT Correlations (on Beam)");
	can_ToT->Divide(2,2);
	can_ToT->cd(1);
	hToTCorr_38_56.DrawCopy("COLZ");
	can_ToT->cd(2);
	hToTCorr_38_198.DrawCopy("COLZ");
	can_ToT->cd(3);
	hToTCorr_38_216.DrawCopy("COLZ");
	can_ToT->cd(4);
	hToTCorr_38_230.DrawCopy("COLZ");

	TCanvas *can_ToToff = new TCanvas("can_ToToff","FLASH - ToT Correlations (off Beam)");
	can_ToToff->Divide(2,3);
	can_ToToff->cd(1);
	hToTCorr_38_40.DrawCopy("COLZ");
	can_ToToff->cd(2);
	hToTCorr_38_36.DrawCopy("COLZ");
	can_ToToff->cd(3);
	hToTCorr_38_24.DrawCopy("COLZ");
	can_ToToff->cd(4);
	hToTCorr_38_22.DrawCopy("COLZ");
	can_ToToff->cd(5);
	hToTCorr_36_40.DrawCopy("COLZ");
	can_ToToff->cd(6);
	hToTCorr_230_248.DrawCopy("COLZ");

	// everything correlated to time differences
	TCanvas *can_dT = new TCanvas("can_dT","FLASH - #Delta T");
	can_dT->Divide(2,2);
	can_dT->cd(1);
	hTimeDeltaTight_38_56.DrawCopy();
	can_dT->cd(2);
	hTimeDeltaTight_38_198.DrawCopy();
	can_dT->cd(3);
	hTimeDeltaTight_38_216.DrawCopy();
	can_dT->cd(4);
	hTimeDeltaTight_38_230.DrawCopy();

	a.PrintListOfPixelPairs();
	a.PrintRequiredPixels();
}


void Flash3BarAnalysis(string cUserDataFile, string cUserTdcList){
	gROOT->ProcessLine(".x BuildFlashAnalysis.cpp");
	enum AnalysisStats {ALL=0, PIXEL_CUTS=1, TRIGGER_CUTS=2};
	TFlashAnalysis a(cUserDataFile,cUserTdcList); // adjust these addresses according to your folder structure!
	// create log file
	std::ofstream filestr;
	filestr.open ("FLASH_3Bar_Analysis.log");
	a.SetLogFile(filestr);
	// set analysis parameters
	a.KeepMultiHits(); // keep channels with multiple hits in time window
	a.SetTimingWindow(-400.0,100.0); // set time window
	a.SetTriggerChannel(0xc00b,9); // select trigger channel
	a.SetTriggerWindow(-240.0,-228.0); // set trigger time window
	a.AddRequiredPixel(206);
	a.AddRequiredPixel(142);
	// add ToT cuts
	a.AddPixelToTCut(206,13.35,13.95);
	a.AddPixelToTCut(208,17.05,17.40);
	a.AddPixelToTCut(238,14.7,15.10);
	//a.IgnoreOffsets();
	a.AddPixelPair(206,208);
	a.AddPixelPair(206,238);
	a.AddPixelPair(208,238);
	a.AddPixelPair(142,144);
	a.AddPixelPair(142,174);
	a.AddPixelPair(144,174);
	// set pixel timing offsets
	a.SetPixelTimeOffsets("FLASH_PixelOffsets_3Bars.txt");
	//a.SetPixelTimeOffset(56,-5.085);
	//a.SetPixelTimeOffset(198,-0.6237);
	//a.SetPixelTimeOffset(216,-1.709);
	//a.SetPixelTimeOffset(230,0.6106);
	// define histograms
	TH1D hAnalysisStats("hAnalysisStats","Analysis Statistics; event type; freq",10,-1.5,8.5);
	TH1D hPixelMultiplicity("hPixelMultiplicity","Pixel multiplicity; # of hit pixels per event; freq.",100,-0.5,99.5);
	TH1D hTriggerTime("hTriggerTime","Trigger Channel Time; T_{LE} (ns); freq",1000,-300.0,-200.0);
	TH1D hTriggerMult("hTriggerMult","Trigger Multiplicity; trigger multiplicity; freq",10,-1.5,8.5);
	//TH1D hTiming("hTiming","hit time; time (ns); freq",5000,-1000.0,1000.0);
//	TH1D hTimingChan12("hTimingChan12","hit time of chan #12; time (ns); freq",5000,-1000.0,1000.0);
//	TH1D hTimeDelta("hTimeDelta","Time difference chan #84/#86; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltaTight_206_208("hTimeDeltaTight_206_208","Time difference (tight ToT cuts) Chan #206/#208; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltaTight_206_238("hTimeDeltaTight_206_238","Time difference (tight ToT cuts) Chan #206/#238; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltaTight_142_144("hTimeDeltaTight_142_144","Time difference (tight ToT cuts) Chan #142/#144; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltaTight_142_174("hTimeDeltaTight_142_174","Time difference (tight ToT cuts) Chan #142/#174; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeAvg_Mcp1("hTimeAvg_Mcp1","Avg Time for MCP 1; avg time (ns); freq",1000,-5.0,5.0);
	TH1D hTimeBest_Mcp1("hTimeBest_Mcp1","Best Time for MCP 1; best time (ns); freq",1000,-5.0,5.0);
	TH2D hTimeAvgMult("hTimeAvgMult","hTimeAvgMult",100,-0.5,99.5,500,-5.0,5.0);
//	TH2D hTimingAllPixels("hTimingAllPixels","channel vs hit time; channel seq ID; time (ns); freq",a.GetSizeOfMapTable()+1,-0.5,a.GetSizeOfMapTable()+0.5,5000,-1000.0,1000.0);
	TH2D hToTCorr_206_208("hToTCorr_206_208","ToT Correlation; ToT_{206} (ns); ToT_{208} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_206_238("hToTCorr_206_238","ToT Correlation; ToT_{206} (ns); ToT_{238} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_208_238("hToTCorr_208_238","ToT Correlation; ToT_{208} (ns); ToT_{238} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_142_144("hToTCorr_142_144","ToT Correlation; ToT_{142} (ns); ToT_{144} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_142_174("hToTCorr_142_174","ToT Correlation; ToT_{142} (ns); ToT_{174} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_144_174("hToTCorr_144_174","ToT Correlation; ToT_{144} (ns); ToT_{174} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	// register histograms for analysis
	a.RegisterTimeDiffHist(206,208,&hTimeDeltaTight_206_208);
	a.RegisterTimeDiffHist(206,238,&hTimeDeltaTight_206_238);
	a.RegisterTimeDiffHist(142,144,&hTimeDeltaTight_142_144);
	a.RegisterTimeDiffHist(142,174,&hTimeDeltaTight_142_174);
	a.RegisterTotCorrHist(206,208,&hToTCorr_206_208);
	a.RegisterTotCorrHist(206,238,&hToTCorr_206_238);
	a.RegisterTotCorrHist(208,238,&hToTCorr_208_238);
	a.RegisterTotCorrHist(142,144,&hToTCorr_142_144);
	a.RegisterTotCorrHist(142,174,&hToTCorr_142_174);
	a.RegisterTotCorrHist(144,174,&hToTCorr_144_174);
	//a.AddPixelToTCut(38,14.5,14.7);
	//a.AddPixelToTCut(56,16.5,16.9);

	for(Int_t i=0; i<(Int_t)a.GetNEvents(); i+=10){
		hAnalysisStats.Fill((Double_t)ALL);
		if(a.GetEntry(i)<1){
			//cout << "DATA ERROR: Skipping event \t" << i << endl;
			//nSkippedEvts++;
			continue;
		}
		hAnalysisStats.Fill((Double_t)PIXEL_CUTS);
		if(a.GetNSyncTimestamps()!=5){ // there is a problem with the TDC synchronisation
			cout << "SYNC ERROR: Skipping event \t" << i << endl;
			continue;
		}
		Int_t nTriggerMult = a.GetTriggerMultiplicity();
		hTriggerMult.Fill((Double_t)nTriggerMult);
		if(nTriggerMult!=1){
			cout << "Wrong Trigger Multiplicity\t" << nTriggerMult << endl;
			continue;
		}
		hAnalysisStats.Fill((Double_t)TRIGGER_CUTS);
		Double_t fTrigTime = 0.0;
		if(a.GetTriggerTime(fTrigTime))
			hTriggerTime.Fill(fTrigTime);
		a.Analyse(); // analyse current entry
		hPixelMultiplicity.Fill((Double_t)a.GetNumberOfHitPixels());
		//cout << a.GetNumberOfHitPixels() << endl;
//		a.FillTimingHistogram(hTiming);
//		a.FillTimingHistogram(38,hTimingChan12);
		//a.FillTimingHistogram(hTimingAllPixels);
		Double_t fDelta;
		Double_t fAvgTime = 0.0;
		Double_t fBestDelta = 1000.0;
		Int_t nHitPixels = 0;
		// channel 56 between 16.0 and 17.5
		// channel 120 between 16.2 and 17.6
		if(a.GetPairTimeDiff(206,208,fDelta)){
			//hTimeDelta.Fill(fDelta);
			//a.FillToTCorrelation(38,56,hToTCorr_38_56);
			//a.FillWalkHistogram(38,14.55,14.65,56,hWalk_56);
			//if(a.GetPairTimeDiff(38,14.5,14.7,56,16.5,16.9,fDelta)){
			fAvgTime += fDelta;
			if(fabs(fDelta)<fabs(fBestDelta))
				fBestDelta = fDelta;
			nHitPixels++;
			//}
		}
		if(a.GetPairTimeDiff(206,238,fDelta)){
			//hTimeDelta.Fill(fDelta);
			//a.FillToTCorrelation(38,198,hToTCorr_38_198);
			//a.FillWalkHistogram(38,14.55,14.65,198,hWalk_198);
			//if(a.GetPairTimeDiff(38,14.4,14.8,198,16.1,16.5,fDelta)){
			//	hTimeDeltaTight_38_198.Fill(fDelta);
			fAvgTime += fDelta;
			if(fabs(fDelta)<fabs(fBestDelta))
				fBestDelta = fDelta;
			nHitPixels++;
			//}
		}
		//if(a.GetPairTimeDiff(38,216,fDelta)){
		//	//hTimeDelta.Fill(fDelta);
		//	//a.FillToTCorrelation(38,216,hToTCorr_38_216);
		//	a.FillWalkHistogram(38,14.55,14.65,216,hWalk_216);
		//	//if(a.GetPairTimeDiff(38,14.4,14.8,216,16.5,16.9,fDelta)){
		//	//	hTimeDeltaTight_38_216.Fill(fDelta);
		//	fAvgTime += fDelta;
		//	if(fabs(fDelta)<fabs(fBestDelta))
		//		fBestDelta = fDelta;
		//	nHitPixels++;
		//	//}
		//}
		//if(a.GetPairTimeDiff(38,230,fDelta)){
		//	//hTimeDelta.Fill(fDelta);
		//	//a.FillToTCorrelation(38,230,hToTCorr_38_230);
		//	a.FillWalkHistogram(38,14.55,14.65,230,hWalk_230);
		//	//if(a.GetPairTimeDiff(38,14.4,14.8,230,14.4,14.8,fDelta)){
		//	//	hTimeDeltaTight_38_230.Fill(fDelta);
		//	fAvgTime += fDelta;
		//	if(fabs(fDelta)<fabs(fBestDelta))
		//		fBestDelta = fDelta;
		//	nHitPixels++;
		//	//}
		//}
		//if(a.GetPixelCorrelation(56,198,fDelta))
		//	hTimeDeltanew.Fill(fDelta);
		if(nHitPixels==2){
			fAvgTime /= (Double_t)nHitPixels;
			hTimeAvg_Mcp1.Fill(fAvgTime);
			hTimeAvgMult.Fill((Double_t)a.GetNumberOfHitPixels(),fAvgTime);
			hTimeBest_Mcp1.Fill(fBestDelta);
		}
	}
	//std::list<PixelHitModel> m = a.GetPixelHits();
	//hPixelMultiplicity.DrawCopy();
	// draw histograms to canvases
	// everything related to the trigger system
	TCanvas *can_Trigger = new TCanvas("can_Trigger","FLASH - Trigger");
	can_Trigger->Divide(2,2);
	can_Trigger->cd(1);
	hAnalysisStats.DrawCopy();
	can_Trigger->cd(2);
	hTriggerTime.DrawCopy();
	can_Trigger->cd(3);
	hTriggerMult.DrawCopy();

	// everything related to the timing results
	TCanvas *can_Timing = new TCanvas("can_Timing","FLASH - Timing");
	can_Timing->Divide(2,2);
	can_Timing->cd(1);
	hPixelMultiplicity.DrawCopy();
	can_Timing->cd(2);
	hTimeAvg_Mcp1.DrawCopy();
	can_Timing->cd(3);
	hTimeAvgMult.DrawCopy("COLZ");
	can_Timing->cd(4);
	TF1* fc = new TF1("fc", "[2]*TMath::CauchyDist(x, [0], [1])", -1.0, 1.0);
	fc->SetParameters(0,1,100);
	TF1* fcg = new TF1("fcg","[0]/TMath::TwoPi()*sinh([1])/(cosh([1])-cos(x-[2]))",-1.0,1.0);
	fcg->SetParameters(100,1.0,0.01);
	fcg->SetLineColor(3);
	hTimeBest_Mcp1.Fit("fc","R");
	hTimeBest_Mcp1.Fit("fcg","R+");
	hTimeBest_Mcp1.DrawCopy();
	
	// everything related to ToT correlations
	TCanvas *can_ToT = new TCanvas("can_ToT","FLASH - ToT Correlations (on Beam)");
	can_ToT->Divide(3,2);
	can_ToT->cd(1);
	hToTCorr_206_208.DrawCopy("COLZ");
	can_ToT->cd(2);
	hToTCorr_206_238.DrawCopy("COLZ");
	can_ToT->cd(3);
	hToTCorr_208_238.DrawCopy("COLZ");
	can_ToT->cd(4);
	hToTCorr_142_144.DrawCopy("COLZ");
	can_ToT->cd(5);
	hToTCorr_142_174.DrawCopy("COLZ");
	can_ToT->cd(6);
	hToTCorr_144_174.DrawCopy("COLZ");

	// everything correlated to time differences
	TCanvas *can_dT = new TCanvas("can_dT","FLASH - #Delta T");
	can_dT->Divide(2,2);
	can_dT->cd(1);
	hTimeDeltaTight_206_208.DrawCopy();
	can_dT->cd(2);
	hTimeDeltaTight_206_238.DrawCopy();
	can_dT->cd(3);
	hTimeDeltaTight_142_144.DrawCopy();
	can_dT->cd(4);
	hTimeDeltaTight_142_174.DrawCopy();

	a.PrintListOfPixelPairs();
	a.PrintRequiredPixels();
}

void FlashTimingAnalysis(string cUserDataFile, string cUserTdcList, UInt_t nIncrement=10){
	gROOT->ProcessLine(".x BuildFlashAnalysis.cpp");
	TFlashAnalysis a(cUserDataFile,cUserTdcList); // adjust these addresses according to your folder structure!
	// create log file
	std::ofstream filestr;
	filestr.open ("FLASH_TimingAnalysis.log");
	a.SetLogFile(filestr);
	// set analysis parameters
	a.KeepMultiHits();
	a.SetTimingWindow(-400.0,100.0);
	a.SetTriggerChannel(0xc00b,9);
	a.SetTriggerWindow(-240.0,-228.0);
	// a.AddPixelPair(56,120); // original pair, but on different MCPs
	// setting channel 38 as reference
	a.AddPixelPair(38,56);
	a.AddPixelPair(38,198);
	a.AddPixelPair(38,216);
	a.AddPixelPair(38,230);
	// set pixel timing offsets
	//a.SetPixelTimeOffset(56,-5.094);
	//a.SetPixelTimeOffset(198,-0.6237);
	//a.SetPixelTimeOffset(216,-1.709);
	//a.SetPixelTimeOffset(230,0.6106);
	a.SetPixelTimeOffsets("FLASH_PixelOffsets.txt");
	//a.IgnoreOffsets();
	// define histograms
	TH1D hPixelMultiplicity("hPixelMultiplicity","Pixel multiplicity; # of hit pixels per event; freq.",100,-0.5,99.5);
	//TH1D hTiming("hTiming","hit time; time (ns); freq",5000,-1000.0,1000.0);
//	TH1D hTimingChan12("hTimingChan12","hit time of chan #12; time (ns); freq",5000,-1000.0,1000.0);
//	TH1D hTimeDelta("hTimeDelta","Time difference chan #84/#86; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltaTight_38_56("hTimeDeltaTight_38_56","Time difference (tight ToT cuts) Chan #38/#56; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltaTight_38_198("hTimeDeltaTight_38_198","Time difference (tight ToT cuts) Chan #38/#198; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltaTight_38_216("hTimeDeltaTight_38_216","Time difference (tight ToT cuts) Chan #38/#216; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltaTight_38_230("hTimeDeltaTight_38_230","Time difference (tight ToT cuts) Chan #38/#230; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeAvg_Mcp1("hTimeAvg_Mcp1","Avg Time for MCP 1; avg time (ns); freq",1000,-5.0,5.0);
	TH2D hTimeAvgMult("hTimeAvgMult","hTimeAvgMult",100,-0.5,99.5,500,-5.0,5.0);
//	TH2D hTimingAllPixels("hTimingAllPixels","channel vs hit time; channel seq ID; time (ns); freq",a.GetSizeOfMapTable()+1,-0.5,a.GetSizeOfMapTable()+0.5,5000,-1000.0,1000.0);
	
	// register histograms for analysis
	a.RegisterTimeDiffHist(38,56,&hTimeDeltaTight_38_56);
	a.RegisterTimeDiffHist(38,198,&hTimeDeltaTight_38_198);
	a.RegisterTimeDiffHist(38,216,&hTimeDeltaTight_38_216);
	a.RegisterTimeDiffHist(38,230,&hTimeDeltaTight_38_230);

	//a.AddPixelToTCut(38,14.5,14.7);
	//a.AddPixelToTCut(56,16.6,16.8);
	//a.AddPixelToTCut(198,16.2,16.4);
	//a.AddPixelToTCut(216,16.6,16.8);
	//a.AddPixelToTCut(230,14.5,14.7);

	a.AddPixelToTCuts("FLASH_HG_ToT_cuts.txt");

	UInt_t nSkippedEvts = 0;

	for(Int_t i=0; i<(Int_t)a.GetNEvents(); i+=nIncrement){
		if(a.GetEntry(i)<1){
			cout << "DATA ERROR: Skipping event \t" << i << endl;
			nSkippedEvts++;
			continue;
		}
		if(a.GetNSyncTimestamps()!=5){ // there is a problem with the TDC synchronisation
			cout << "SYNC ERROR: Skipping event \t" << i << endl;
			nSkippedEvts++;
			continue;
		}
		Int_t nTriggerMult = a.GetTriggerMultiplicity();
		if(nTriggerMult!=1){
			//cout << "Wrong Trigger Multiplicity\t" << nTriggerMult << endl;
			nSkippedEvts++;
			continue;
		}
		hPixelMultiplicity.Fill((Double_t)a.GetNumberOfHitPixels());
		a.Analyse();
		Double_t fDelta;
		Double_t fAvgTime = 0.0;
		Int_t nHitPixels = 0;
		if(a.GetPairTimeDiff(38,56,fDelta)){
			fAvgTime += fDelta;
			nHitPixels++;
		}
		if(a.GetPairTimeDiff(38,198,fDelta)){
			fAvgTime += fDelta;
			nHitPixels++;
		}
		if(a.GetPairTimeDiff(38,216,fDelta)){
			fAvgTime += fDelta;
			nHitPixels++;
		}
		if(a.GetPairTimeDiff(38,230,fDelta)){
			fAvgTime += fDelta;
			nHitPixels++;
		}
		if(nHitPixels>1){
			fAvgTime /= (Double_t)nHitPixels;
			hTimeAvg_Mcp1.Fill(fAvgTime);
			hTimeAvgMult.Fill((Double_t)a.GetNumberOfHitPixels(),fAvgTime);
		}
	}
	//std::list<PixelHitModel> m = a.GetPixelHits();
	//hPixelMultiplicity.DrawCopy();
	TCanvas *can_Timing = new TCanvas("can_Timing","FLASH - Timing");
	can_Timing->Divide(2,2);
	can_Timing->cd(1);
	hPixelMultiplicity.DrawCopy();
	can_Timing->cd(2);
	hTimeAvg_Mcp1.DrawCopy();
	can_Timing->cd(3);
	hTimeAvgMult.DrawCopy("COLZ");
	

	TCanvas *can_dT = new TCanvas("can_dT","FLASH - #Delta T");
	can_dT->Divide(2,2);
	can_dT->cd(1);
	hTimeDeltaTight_38_56.DrawCopy();
	can_dT->cd(2);
	hTimeDeltaTight_38_198.DrawCopy();
	can_dT->cd(3);
	hTimeDeltaTight_38_216.DrawCopy();
	can_dT->cd(4);
	hTimeDeltaTight_38_230.DrawCopy();

	a.PrintListOfPixelPairs();

	cout << nSkippedEvts << endl;
	
}




void FlashSyncAnalysis(string cUserDataFile, string cUserTdcList, UInt_t nIncrement=10){
	// build FLASH analysis class and create analysis object
	gROOT->ProcessLine(".x BuildFlashAnalysis.cpp");
	TFlashAnalysis SyncAnalysis(cUserDataFile,cUserTdcList); // adjust these addresses according to your folder structure!
	// define histograms
	Int_t nHistBins = 2000;
	TH1D hTdcSync1_2("hTdcSync1_2","TDC Synchronisation FPGA 1/2; #Delta T_{sync} (ns); freq",nHistBins,-10.0,10.0);
	TH1D hTdcSync1_3("hTdcSync1_3","TDC Synchronisation FPGA 1/3; #Delta T_{sync} (ns); freq",nHistBins,-10.0,10.0);
	TH1D hTdcSync1_4("hTdcSync1_4","TDC Synchronisation FPGA 1/4; #Delta T_{sync} (ns); freq",nHistBins,-10.0,10.0);
	TH1D hTdcSync2_3("hTdcSync2_3","TDC Synchronisation FPGA 2/3; #Delta T_{sync} (ns); freq",nHistBins,-10.0,10.0);
	TH1D hTdcSync2_4("hTdcSync2_4","TDC Synchronisation FPGA 2/4; #Delta T_{sync} (ns); freq",nHistBins,-10.0,10.0);
	TH1D hTdcSync3_4("hTdcSync3_4","TDC Synchronisation FPGA 3/4; #Delta T_{sync} (ns); freq",nHistBins,-10.0,10.0);
	// analyse events
	for(Int_t i=0; i<(Int_t)SyncAnalysis.GetNEvents(); i+=nIncrement){ // begin of loop over all events
		if(SyncAnalysis.GetEntry(i)<1){
			cout << "DATA ERROR: Skipping event \t" << i << endl;
			continue;
		}
		if(SyncAnalysis.GetNSyncTimestamps()!=5){ // there is a problem with the TDC synchronisation
			cout << "SYNC ERROR: Skipping event \t" << i << endl;
			continue;
		}
		Double_t fTdcSyncdT = 0.0;
		fTdcSyncdT = SyncAnalysis.GetTdcSyncTimestamp(0xc004) - SyncAnalysis.GetTdcSyncTimestamp(0xc005);
		hTdcSync1_2.Fill(fTdcSyncdT);
		fTdcSyncdT = SyncAnalysis.GetTdcSyncTimestamp(0xc004) - SyncAnalysis.GetTdcSyncTimestamp(0xc006);
		hTdcSync1_3.Fill(fTdcSyncdT);
		fTdcSyncdT = SyncAnalysis.GetTdcSyncTimestamp(0xc004) - SyncAnalysis.GetTdcSyncTimestamp(0xc007);
		hTdcSync1_4.Fill(fTdcSyncdT);
		fTdcSyncdT = SyncAnalysis.GetTdcSyncTimestamp(0xc005) - SyncAnalysis.GetTdcSyncTimestamp(0xc006);
		hTdcSync2_3.Fill(fTdcSyncdT);
		fTdcSyncdT = SyncAnalysis.GetTdcSyncTimestamp(0xc005) - SyncAnalysis.GetTdcSyncTimestamp(0xc007);
		hTdcSync2_4.Fill(fTdcSyncdT);
		fTdcSyncdT = SyncAnalysis.GetTdcSyncTimestamp(0xc006) - SyncAnalysis.GetTdcSyncTimestamp(0xc007);
		hTdcSync3_4.Fill(fTdcSyncdT);

	} // end of loop over all events
	// Display results
	TCanvas *can_Sync = new TCanvas("can_Sync","FLASH - TDC Synchronisation");
	can_Sync->Divide(2,3);
	can_Sync->cd(1);
	hTdcSync1_2.DrawCopy();
	can_Sync->cd(2);
	hTdcSync1_3.DrawCopy();
	can_Sync->cd(3);
	hTdcSync1_4.DrawCopy();
	can_Sync->cd(4);
	hTdcSync2_3.DrawCopy();
	can_Sync->cd(5);
	hTdcSync2_4.DrawCopy();
	can_Sync->cd(6);
	hTdcSync3_4.DrawCopy();

}

void FlashAnalysisOverview(string cUserDataFile, string cUserTdcList, Double_t fTimeLow=-400.0, Double_t fTimeHigh=100.0, Int_t nUserFraction=10){
	gROOT->ProcessLine(".x BuildFlashAnalysis.cpp");
	TFlashAnalysis Overview(cUserDataFile,cUserTdcList); // adjust these addresses according to your folder structure!
	Overview.KeepMultiHits(); // keep multiple hits per channel
	Overview.SetTimingWindow(fTimeLow,fTimeHigh); // set time window for analysis
	// create output file
	string cOverviewOutputName = cUserDataFile.substr(0,cUserDataFile.length()-5);
	cOverviewOutputName += "-Flash_Analysis_Overview.root"; // set file name for output file
	TFile OverviewOutput(cOverviewOutputName.c_str(),"RECREATE"); // open output file
	if(OverviewOutput.IsZombie()) // check if output file is opened properly
		return;
	// define histograms
	TH1D hHitMultiplicity("hHitMultiplicity","Hit Multiplicity; # of hit pixels per event; freq.",100,-0.5,99.5);
	TH2D hPixelEvtMultiplicity("hPixelEvtMultiplicity","Pixel Hit Multiplicity per Event; channel seq ID; hit multiplicity; freq",Overview.GetSizeOfMapTable()+1,-0.5,Overview.GetSizeOfMapTable()+0.5,20,-0.5,19.5);
	TH1D hTiming("hTiming","Hit Timestamps; time (ns); freq",5000,-1000.0,1000.0);
	TH2D hTimingAllPixels("hTimingAllPixels","Channel Hit Timestamps; channel seq ID; time (ns); freq",Overview.GetSizeOfMapTable()+1,-0.5,Overview.GetSizeOfMapTable()+0.5,5000,fTimeLow,fTimeHigh);
	TH2D hToTAllPixels("hToTAllPixels","Time-over-Threshold; channel seq ID; Time-over-Threshold (ns); freq",Overview.GetSizeOfMapTable()+1,-0.5,Overview.GetSizeOfMapTable()+0.5,3200,-10.0,150.0);
	for(Int_t i=0; i<(Int_t)Overview.GetNEvents(); i+=10){ // begin of loop over all events
		if(Overview.GetEntry(i)<1){
			cout << "DATA ERROR: Skipping event \t" << i << endl;
			continue;
		}
		Overview.Analyse(); // get entry
		hHitMultiplicity.Fill((Double_t)Overview.GetNumberOfHitPixels()); // get number of hit channels per event
		Overview.FillTimingHistogram(hTiming); // fill timing histogram with all leading edge timestamps
		Overview.FillTimingHistogram(hTimingAllPixels); // fill 2D histogram of leading edge times
		Overview.FillToTHistogram(hToTAllPixels); // fill 2D ToT histogram
		for(UInt_t j=0; j<Overview.GetSizeOfMapTable(); j += 2){ // begin of loop over all channels
			hPixelEvtMultiplicity.Fill((Double_t)j,(Double_t)Overview.GetChanMultiplicity(j));
		} // end of loop over all channels
	} // end of loop over all events
	hPixelEvtMultiplicity.Scale(1.0/(Double_t)Overview.GetNEvents());
	TCanvas *can = new TCanvas("Overview",cOverviewOutputName.c_str());
	can->Divide(3,2);
	can->cd(1);
	hHitMultiplicity.DrawCopy();
	can->cd(4);
	hPixelEvtMultiplicity.DrawCopy("COLZ");
	can->cd(2);
	hTiming.DrawCopy();
	can->cd(5);
	hTimingAllPixels.DrawCopy("COLZ");
	can->cd(6);
	hToTAllPixels.DrawCopy("COLZ");
	OverviewOutput.WriteTObject(can);
	OverviewOutput.Write();
}

void Test(){
	std::set< std::pair<UInt_t,UInt_t> > PixelPairs;
	//std::pair<std::set< std::pair<UInt_t,UInt_t> >::iterator,bool> ret;
	std::pair<UInt_t,UInt_t> a(1,2);
	PixelPairs.insert(a);
	cout << PixelPairs.size() << ", " << a.first << endl;

}

void PadiwaAnalysis(string cUserDataFile, string cUserTdcList, UInt_t nPixelA, UInt_t nPixelB){
	gROOT->ProcessLine(".x BuildFlashAnalysis.cpp");
	TFlashAnalysis a(cUserDataFile,cUserTdcList); // adjust these addresses according to your folder structure!
	a.KeepMultiHits();
	// define histograms
	TH1D hPixelMultiplicity("hPixelMultiplicity","Pixel multiplicity; # of hit pixels per event; freq.",10,-0.5,9.5);
	TH1D hTiming("hTiming","hit time; time (ns); freq",5000,-4500.0,500.0);
	TH1D hTimeDelta("hTimeDelta","Time difference; #Delta t (ns);freq",2000,5.0,15.0);
	a.AddPixelPair(nPixelA,nPixelB);
	for(Int_t i=0; i<(Int_t)a.GetNEvents(); i++){
		a.Analyse();
		hPixelMultiplicity.Fill((Double_t)a.GetNumberOfHitPixels());
		a.FillTimingHistogram(hTiming);
		Double_t fDelta;
		if(a.GetPixelCorrelation(nPixelA,nPixelB,fDelta)){
			hTimeDelta.Fill(fDelta);
		}
	}
	TCanvas *can = new TCanvas();
	can->Divide(3,1);
	can->cd(1);
	hPixelMultiplicity.DrawCopy();
	can->cd(2);
	hTiming.DrawCopy();
	can->cd(3);
	hTimeDelta.DrawCopy();
}