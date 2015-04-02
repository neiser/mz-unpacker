void FlashAnalysisExample(string cUserDataFile, string cUserTdcList){
	gROOT->ProcessLine(".x BuildFlashAnalysis.cpp");
	TFlashAnalysis a(cUserDataFile,cUserTdcList); // adjust these addresses according to your folder structure!
	// create log file
	//std::ofstream filestr;
	//filestr.open ("test.log");
	//a.SetLogFile(filestr);
	// set analysis parameters
	a.KeepMultiHits();
	a.SetTimingWindow(-400.0,100.0);
	a.SetTriggerChannel(0xc00b,9);
	a.SetTriggerWindow(-240.0,-228.0);
	//a.IgnoreOffsets();
	// a.AddPixelPair(56,120); // original pair, but on different MCPs
	// setting channel 38 as reference
	a.AddPixelPair(38,22);
	a.AddPixelPair(38,24);
	a.AddPixelPair(38,36);
	a.AddPixelPair(38,40);
	a.AddPixelPair(38,56);
	a.AddPixelPair(38,198);
	a.AddPixelPair(38,216);
	a.AddPixelPair(38,230);
	// set pixel timing offsets
	a.SetPixelTimeOffset(56,-5.094);
	a.SetPixelTimeOffset(198,-0.6237);
	a.SetPixelTimeOffset(216,-1.709);
	a.SetPixelTimeOffset(230,0.6106);
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
	TH2D hToTCorr_38_56("hToTCorr_38_56","ToT Correlation; ToT_{38} (ns); ToT_{56} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_40("hToTCorr_38_40","ToT Correlation; ToT_{38} (ns); ToT_{40} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_36("hToTCorr_38_36","ToT Correlation; ToT_{38} (ns); ToT_{36} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_24("hToTCorr_38_24","ToT Correlation; ToT_{38} (ns); ToT_{24} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_22("hToTCorr_38_22","ToT Correlation; ToT_{38} (ns); ToT_{22} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_198("hToTCorr_38_198","ToT Correlation; ToT_{38} (ns); ToT_{198} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_216("hToTCorr_38_216","ToT Correlation; ToT_{38} (ns); ToT_{216} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
	TH2D hToTCorr_38_230("hToTCorr_38_230","ToT Correlation; ToT_{38} (ns); ToT_{230} (ns); freq",1000,0.0,50.0,1000,0.0,50.0);
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
	a.RegisterTotCorrHist(38,22,&hToTCorr_38_22);
	a.RegisterTotCorrHist(38,24,&hToTCorr_38_24);
	a.RegisterTotCorrHist(38,36,&hToTCorr_38_36);
	a.RegisterTotCorrHist(38,40,&hToTCorr_38_40);
	a.RegisterTotCorrHist(38,56,&hToTCorr_38_56);
	a.RegisterTotCorrHist(38,198,&hToTCorr_38_198);
	a.RegisterTotCorrHist(38,216,&hToTCorr_38_216);
	a.RegisterTotCorrHist(38,230,&hToTCorr_38_230);

	//a.AddPixelToTCut(38,14.5,14.7);
	//a.AddPixelToTCut(56,16.5,16.9);

	for(Int_t i=0; i<(Int_t)a.GetNEvents(); i+=10){
		if(a.GetEntry(i)<1){
			cout << "DATA ERROR: Skipping event \t" << i << endl;
			nSkippedEvts++;
			continue;
		}
		if(a.GetNSyncTimestamps()!=5){ // there is a problem with the TDC synchronisation
			cout << "SYNC ERROR: Skipping event \t" << i << endl;
			continue;
		}
		Int_t nTriggerMult = a.GetTriggerMultiplicity();
		if(nTriggerMult!=1){
			cout << "Wrong Trigger Multiplicity\t" << nTriggerMult << endl;
			continue;
		}
		a.Analyse();
		hPixelMultiplicity.Fill((Double_t)a.GetNumberOfHitPixels());
		//cout << a.GetNumberOfHitPixels() << endl;
//		a.FillTimingHistogram(hTiming);
//		a.FillTimingHistogram(38,hTimingChan12);
		//a.FillTimingHistogram(hTimingAllPixels);
		Double_t fDelta;
		Double_t fAvgTime = 0.0;
		Int_t nHitPixels = 0;
		// channel 56 between 16.0 and 17.5
		// channel 120 between 16.2 and 17.6
		if(a.GetPairTimeDiff(38,56,fDelta)){
			//hTimeDelta.Fill(fDelta);
			//a.FillToTCorrelation(38,56,hToTCorr_38_56);
			a.FillWalkHistogram(38,14.55,14.65,56,hWalk_56);
			//if(a.GetPairTimeDiff(38,14.5,14.7,56,16.5,16.9,fDelta)){
			//	fAvgTime += fDelta;
			//	nHitPixels++;
			//}
		}
		if(a.GetPairTimeDiff(38,198,fDelta)){
			//hTimeDelta.Fill(fDelta);
			//a.FillToTCorrelation(38,198,hToTCorr_38_198);
			a.FillWalkHistogram(38,14.55,14.65,198,hWalk_198);
			//if(a.GetPairTimeDiff(38,14.4,14.8,198,16.1,16.5,fDelta)){
			//	hTimeDeltaTight_38_198.Fill(fDelta);
			//	fAvgTime += fDelta;
			//	nHitPixels++;
			//}
		}
		if(a.GetPairTimeDiff(38,216,fDelta)){
			//hTimeDelta.Fill(fDelta);
			//a.FillToTCorrelation(38,216,hToTCorr_38_216);
			a.FillWalkHistogram(38,14.55,14.65,216,hWalk_216);
			//if(a.GetPairTimeDiff(38,14.4,14.8,216,16.5,16.9,fDelta)){
			//	hTimeDeltaTight_38_216.Fill(fDelta);
			//	fAvgTime += fDelta;
			//	nHitPixels++;
			//}
		}
		if(a.GetPairTimeDiff(38,230,fDelta)){
			//hTimeDelta.Fill(fDelta);
			//a.FillToTCorrelation(38,230,hToTCorr_38_230);
			a.FillWalkHistogram(38,14.55,14.65,230,hWalk_230);
			//if(a.GetPairTimeDiff(38,14.4,14.8,230,14.4,14.8,fDelta)){
			//	hTimeDeltaTight_38_230.Fill(fDelta);
				//fAvgTime += fDelta;
				//nHitPixels++;
			//}
		}
		//if(a.GetPixelCorrelation(56,198,fDelta))
		//	hTimeDeltanew.Fill(fDelta);
		if(nHitPixels==3){
			fAvgTime /= 3.0;
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
	can_ToToff->Divide(2,2);
	can_ToToff->cd(1);
	hToTCorr_38_40.DrawCopy("COLZ");
	can_ToToff->cd(2);
	hToTCorr_38_36.DrawCopy("COLZ");
	can_ToToff->cd(3);
	hToTCorr_38_24.DrawCopy("COLZ");
	can_ToToff->cd(4);
	hToTCorr_38_22.DrawCopy("COLZ");

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

	a.AddPixelToTCut(38,14.5,14.7);
	a.AddPixelToTCut(56,16.6,16.8);
	a.AddPixelToTCut(198,16.2,16.4);
	a.AddPixelToTCut(216,16.6,16.8);
	a.AddPixelToTCut(230,14.5,14.7);

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
		if(nHitPixels==4){
			fAvgTime /= 4.0;
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

void FlashWalkAnalysis(string cUserDataFile, string cUserTdcList, UInt_t nIncrement=10){
	// build FLASH analysis class and create analysis object
	gROOT->ProcessLine(".x BuildFlashAnalysis.cpp");
	TFlashAnalysis WalkAnalysis(cUserDataFile,cUserTdcList); // adjust these addresses according to your folder structure!
	// create log file
	std::ofstream filestr;
	filestr.open ("test.log");
	WalkAnalysis.SetLogFile(filestr);
	// set analysis details
	WalkAnalysis.KeepMultiHits();
	WalkAnalysis.SetTimingWindow(-400.0,100.0);
	WalkAnalysis.ExcludeChannel(0xc004,1);
	// setting channel 38 as reference
	//WalkAnalysis.AddPixelToTCut(38,14.58,14.63);
	WalkAnalysis.AddPixelToTCuts("FLASH_ToT_Cuts.txt");
	WalkAnalysis.AddPixelPairs("FLASH_PixelPairs_Walk.txt");
	//WalkAnalysis.AddPixelPair(38,56);
	//WalkAnalysis.AddPixelPair(38,198);
	//WalkAnalysis.AddPixelPair(38,216);
	//WalkAnalysis.AddPixelPair(38,230);
	// set pixel timing offsets
	UInt_t nOffsetsAdded = WalkAnalysis.SetPixelTimeOffsets("FLASH_PixelOffsets.txt");
	cout << nOffsetsAdded << endl;
	//WalkAnalysis.SetPixelTimeOffset(56,-5.094);
	//WalkAnalysis.SetPixelTimeOffset(198,-0.6237);
	//WalkAnalysis.SetPixelTimeOffset(216,-1.709);
	//WalkAnalysis.SetPixelTimeOffset(230,0.6106);
	// define histograms
	TH2D hWalk_56("hWalk_56","Walk Correlation; ToT_{56} (ns); LE_{38-56} (ns); freq",1000,0.0,50.0,1000,-5.0,5.0);
	TH2D hWalk_198("hWalk_198","Walk Correlation; ToT_{198} (ns); LE_{38-198} (ns); freq",1000,0.0,50.0,1000,-5.0,5.0);
	TH2D hWalk_216("hWalk_216","Walk Correlation; ToT_{216} (ns); LE_{38-216} (ns); freq",1000,0.0,50.0,1000,-5.0,5.0);
	TH2D hWalk_230("hWalk_230","Walk Correlation; ToT_{230} (ns); LE_{38-230} (ns); freq",1000,0.0,50.0,1000,-5.0,5.0);
	// register histograms
	WalkAnalysis.RegisterTimeWalkHist(38,56,&hWalk_56);
	WalkAnalysis.RegisterTimeWalkHist(38,198,&hWalk_198);
	WalkAnalysis.RegisterTimeWalkHist(38,216,&hWalk_216);
	WalkAnalysis.RegisterTimeWalkHist(38,230,&hWalk_230);
	// analyse events
	for(Int_t i=0; i<(Int_t)WalkAnalysis.GetNEvents(); i+=10){ // begin of loop over all events
		if(WalkAnalysis.GetEntry(i)<1){
			cout << "DATA ERROR: Skipping event \t" << i << endl;
			continue;
		}
		if(WalkAnalysis.GetNSyncTimestamps()!=5){ // there is a problem with the TDC synchronisation
			cout << "SYNC ERROR: Skipping event \t" << i << endl;
			continue;
		}
		WalkAnalysis.Analyse();
		//WalkAnalysis.FillWalkHistogram(38,14.55,14.65,56,hWalk_56);
		//WalkAnalysis.FillWalkHistogram(38,14.55,14.65,198,hWalk_198);
		//WalkAnalysis.FillWalkHistogram(38,14.55,14.65,216,hWalk_216);
		//WalkAnalysis.FillWalkHistogram(38,14.55,14.65,230,hWalk_230);
	} // end of loop over all events

	// display results
	TCanvas *can_Walk = new TCanvas("can_Walk","FLASH - Time Walk");
	can_Walk->Divide(2,2);
	can_Walk->cd(1);
	hWalk_56.DrawCopy("COLZ");
	can_Walk->cd(2);
	hWalk_198.DrawCopy("COLZ");
	can_Walk->cd(3);
	hWalk_216.DrawCopy("COLZ");
	can_Walk->cd(4);
	hWalk_230.DrawCopy("COLZ");

	filestr.close();
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