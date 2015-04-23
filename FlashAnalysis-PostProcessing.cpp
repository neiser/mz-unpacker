void TimingMultiplicity(TH2D* hTimingHist, Int_t nMinCountsPerSlice=30){
	if(hTimingHist==NULL)
		return;
	TObjArray TimingSlices;
	hTimingHist->FitSlicesY(0,0,-1,nMinCountsPerSlice,"QN",&TimingSlices);
	TCanvas* canResults = new TCanvas("canResults","FLASH - Timing Results");
	canResults->Divide(1,2);
	canResults->cd(1);
	hTimingHist->DrawCopy("COLZ");
	TVirtualPad* PadFitResults = canResults->cd(2);
	PadFitResults->Divide(2,2);
	for(Int_t i=0; i<4; i++){
		PadFitResults->cd(i+1);
		((TH1D*)TimingSlices.At(i))->DrawCopy();
	}

}

void DrawFlashLogo(){
   string cPathToFlashLogo = "D:/Work/Programing/ROOT/TRB/Flash/flash_print_V2.png";
   TImage *img = TImage::Open(cPathToFlashLogo.c_str());
   
   if (!img) {
      printf("Could not create an image... exit\n");
      return;
   }
   TPad *l = new TPad("l","l",0.,0.9,0.1,1.);
   //gPad->cd(0);
   l->Draw();
   l->cd();
   img->Draw();
}