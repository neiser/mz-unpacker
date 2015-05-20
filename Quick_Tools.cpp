void Do_DQ_CERN_2015(string cUserFilename, string cUserTreename="A"){
	// open file and TTree
	TFile TrbData(cUserFilename.c_str(),"READ"); // open data file in read-only mode
	TTree* EvtTree = (TTree*)TrbData.Get(cUserTreename.c_str());
	// create histograms
	TH1D* hEvtMult = new TH1D("hEvtMult","Event Multiplicity; Multiplicity per Event; freq",140,-0.5,139.5);
	TH1D* hHitChannels = new TH1D("hHitChannels","Distribution of hit channels; Seq ID; freq",130,-0.5,129.5);
	TH2D* hChanMult = new TH2D("hChanMult","Multiplicity per channel; Seq ID; Multiplicity per Event; freq",130,-0.5,129.5,21,-0.5,20.5);
	TH1D* hTimeResolution = new TH1D("hTimeResolution","Timing Resolution; T_{70}-T_{72} (ns); freq",600,-10.0,10.0);
	TH2D* hLeadingEdge = new TH2D("hLeadingEdge","Leading Edge Time Distribution; Seq ID; T_{LE} (ns); freq",130,-0.5,129.5,2000,-900.0,100.0);
	TH2D* hTot = new TH2D("hTot","Time-over-Threshold Distribution; Seq ID; ToT (ns); freq",130,-0.5,129.5,200,0.0,40.0);
	// extract data from Tree
	EvtTree->Draw("nHits>>hEvtMult","","goff");
	EvtTree->Draw("nSeqId>>hHitChannels","bValid","goff");
	EvtTree->Draw("nMult:nSeqId>>hChanMult","bValid","goff");
	EvtTree->Draw("fLeadingEdge[72]-fLeadingEdge[70]>>hTimeResolution","bValid[72]==1&&bValid[70]==1","goff");
	EvtTree->Draw("fLeadingEdge:nSeqId>>hLeadingEdge","bValid","goff");
	EvtTree->Draw("fTot:nSeqId>>hTot","bValid","goff");
	// draw histograms
	TCanvas* canDataQuality = new TCanvas("canDataQuality","FLASH@CERN 2015 - Data Quality");
	canDataQuality->Divide(3,1);
	canDataQuality->cd(1);
	hEvtMult->DrawCopy();
	canDataQuality->cd(2);
	hHitChannels->DrawCopy();
	canDataQuality->cd(3);
	hChanMult->DrawCopy("COLZ");
	TCanvas* canTiming = new TCanvas("canTiming","FLASH@CERN 2015 - Timing");
	canTiming->Divide(3,1);
	canTiming->cd(1);
	hTimeResolution->DrawCopy();
	canTiming->cd(2);
	hLeadingEdge->DrawCopy("COLZ");
	canTiming->cd(3);
	hTot->DrawCopy("COLZ");
}