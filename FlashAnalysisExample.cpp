void FlashAnalysisExample(string cUserDataFile, string cUserTdcList){
	gROOT->ProcessLine(".x BuildFlashAnalysis.cpp");
	TFlashAnalysis a(cUserDataFile,cUserTdcList); // adjust these addresses according to your folder structure!
	a.KeepMultiHits();
	a.SetTimingWindow(-400.0,100.0);
	// define histograms
	TH1D hPixelMultiplicity("hPixelMultiplicity","Pixel multiplicity; # of hit pixels per event; freq.",100,-0.5,99.5);
	TH1D hTiming("hTiming","hit time; time (ns); freq",5000,-1000.0,1000.0);
	TH1D hTimingChan12("hTimingChan12","hit time of chan #12; time (ns); freq",5000,-1000.0,1000.0);
	TH1D hTimeDelta("hTimeDelta","Time difference chan #84/#86; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltanew("hTimeDeltanew","Time difference chan #82/#84; #Delta t (ns);freq",1000,-10.0,10.0);
	TH2D hTimingAllPixels("hTimingAllPixels","channel vs hit time; channel seq ID; time (ns); freq",a.GetSizeOfMapTable()+1,-0.5,a.GetSizeOfMapTable()+0.5,5000,-1000.0,1000.0);
	a.AddPixelPair(56,120);
	a.AddPixelPair(56,198);
	for(Int_t i=0; i<(Int_t)a.GetNEvents(); i++){
		a.Analyse(i);
		hPixelMultiplicity.Fill((Double_t)a.GetNumberOfHitPixels());
		//cout << a.GetNumberOfHitPixels() << endl;
		a.FillTimingHistogram(hTiming);
		a.FillTimingHistogram(56,hTimingChan12);
		a.FillTimingHistogram(hTimingAllPixels);
		Double_t fDelta;
		if(a.GetPixelCorrelation(56,120,fDelta))
			hTimeDelta.Fill(fDelta);
		if(a.GetPixelCorrelation(56,198,fDelta))
			hTimeDeltanew.Fill(fDelta);
	}
	//std::list<PixelHitModel> m = a.GetPixelHits();
	//hPixelMultiplicity.DrawCopy();
	TCanvas *can = new TCanvas();
	can->Divide(3,2);
	can->cd(1);
	hTiming.DrawCopy();
	can->cd(2);
	hTimingAllPixels.DrawCopy("COLZ");
	can->cd(3);
	hTimingChan12.DrawCopy();
	can->cd(4);
	hTimeDelta.DrawCopy();
	hTimeDeltanew.SetLineColor(2);
	hTimeDeltanew.DrawCopy("SAME");
	can->cd(5);
	hPixelMultiplicity.DrawCopy();
}

void FlashAnalysisOverview(string cUserDataFile, string cUserTdcList, Double_t fTimeLow=-400.0, Double_t fTimeHigh=100.0){
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
	for(Int_t i=0; i<(Int_t)Overview.GetNEvents(); i++){ // begin of loop over all events
		Overview.Analyse(i); // get entry
		hHitMultiplicity.Fill((Double_t)Overview.GetNumberOfHitPixels()); // get number of hit channels per event
		Overview.FillTimingHistogram(hTiming); // fill timing histogram with all leading edge timestamps
		Overview.FillTimingHistogram(hTimingAllPixels); // fill 2D histogram of leading edge times
		Overview.FillToTHistogram(hToTAllPixels); // fill 2D ToT histogram
		for(UInt_t j=0; j<Overview.GetSizeOfMapTable(); j += 2){ // begin of loop over all channels
			hPixelEvtMultiplicity.Fill((Double_t)j,(Double_t)Overview.GetChanMultiplicity(j));
		} // end of loop over all channels
	} // end of loop over all events
	hPixelEvtMultiplicity.Scale(1.0/(Double_t)Overview.GetNEvents());
	TCanvas *can = new TCanvas();
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