void FlashAnalysisExample(){
	gROOT->ProcessLine(".x BuildFlashAnalysis.cpp");
	TFlashAnalysis a("../gsi/pilas_cc14195184051.hld.root_calibrated.root","../GbE/GSI_BarrelDIRC_July2014_TDC_Addresses.txt");
	//a.KeepMultiHits();
	// define histograms
	TH1D hPixelMultiplicity("hPixelMultiplicity","Pixel multiplicity; # of hit pixels per event; freq.",100,-0.5,99.5);
	TH1D hTiming("hTiming","hit time; time (ns); freq",5000,-1000.0,1000.0);
	TH1D hTimingChan12("hTimingChan12","hit time of chan #12; time (ns); freq",5000,-1000.0,1000.0);
	TH1D hTimeDelta("hTimeDelta","Time difference chan #84/#86; #Delta t (ns);freq",1000,-10.0,10.0);
	TH1D hTimeDeltanew("hTimeDeltanew","Time difference chan #82/#84; #Delta t (ns);freq",1000,-10.0,10.0);
	TH2D hTimingAllPixels("hTimingAllPixels","channel vs hit time; channel seq ID; time (ns); freq",a.GetSizeOfMapTable()+1,-0.5,a.GetSizeOfMapTable()+0.5,5000,-1000.0,1000.0);
	a.AddPixelPair(722,726);
	a.AddPixelPair(722,752);
	for(Int_t i=0; i<(Int_t)a.GetNEvents(); i++){
		a.Analyse(i);
		hPixelMultiplicity.Fill((Double_t)a.GetNumberOfHitPixels());
		//cout << a.GetNumberOfHitPixels() << endl;
		a.FillTimingHistogram(hTiming);
		a.FillTimingHistogram(12,hTimingChan12);
		a.FillTimingHistogram(hTimingAllPixels);
		Double_t fDelta;
		if(a.GetPixelCorrelation(722,726,fDelta))
			hTimeDelta.Fill(fDelta);
		if(a.GetPixelCorrelation(722,752,fDelta))
			hTimeDeltanew.Fill(fDelta);
	}
	//std::list<PixelHitModel> m = a.GetPixelHits();
	//hPixelMultiplicity.DrawCopy();
	TCanvas *can = new TCanvas();
	can->Divide(2,2);
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
}